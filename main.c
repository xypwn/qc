#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#ifdef ENABLE_READLINE
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "expr.h"

#define ssub(_x, _y) ((_x) >= (_y) ? (_x) - (_y) : 0)
#define bufprint(_buf, _n, ...) _n += snprintf(_buf + _n, ssub(sizeof(buf), _n), __VA_ARGS__)

static Expr *e;
static bool running = true;
static bool last_status_ok = true;
#ifdef ENABLE_READLINE
static bool sigwinch_received = false;
static size_t completer_idx, completer_len;
static bool completer_funcs;
#endif

static void print_help() {
	fprintf(stderr,
		"Usage:\n"
		"  qc \"<expression>\"  --  evaluate expression\n"
		"  qc                 --  run in REPL mode\n"
		"  qc --help          --  show this page\n"
		"Syntax:\n"
		"  Numbers: 123.45 or 1.2345e2 or 1.2345E2\n"
		"  Precedence | Operations\n"
		"  -----------+------------\n"
		"    1 (LtR)  | +, -\n"
		"    2 (LtR)  | *, /\n"
		"    3 (RtL)  | ^\n"
		"  Other symbols: (, ), - (prefix)\n");
	char buf[32][128];
	size_t maxw[2];
	maxw[0] = 0;
	fprintf(stderr, "Builtin functions:\n");
	for (size_t i = 0; i < expr_n_builtin_funcs; i++) {
		assert(i < 32);
		size_t n = 0;
		bufprint(buf[i], n, "%s(", expr_builtin_funcs[i].name);
		for (size_t j = 0; j < expr_builtin_funcs[i].n_args; j++) {
			if (j != 0)
				bufprint(buf[i], n, ", ");
			bufprint(buf[i], n, "%s", expr_builtin_funcs[i].arg_names[j]);
		}
		bufprint(buf[i], n, ")");
		if (n > maxw[0])
			maxw[0] = n;
	}
	for (size_t i = 0; i < expr_n_builtin_funcs; i++) {
		fprintf(stderr, "  %s%*s  -  %s\n", buf[i], (int)(maxw[0] - strlen(buf[i])), "", expr_builtin_funcs[i].description);
	}
	maxw[0] = maxw[1] = 0;
	fprintf(stderr, "Builtin variables:\n");
	for (size_t i = 0; i < expr_n_builtin_vars; i++) {
		assert(i < 32);
		size_t n = strlen(expr_builtin_vars[i].name);
		if (n > maxw[0])
			maxw[0] = n;
		n = snprintf(buf[i], 128, "%.*g", 15, expr_builtin_vars[i].val);
		if (n > maxw[1])
			maxw[1] = n;
	}
	for (size_t i = 0; i < expr_n_builtin_vars; i++) {
		fprintf(stderr, "  %s%*s = %s%*s  -  %s\n", expr_builtin_vars[i].name, (int)(maxw[0] - strlen(expr_builtin_vars[i].name)), "", buf[i], (int)(maxw[1] - strlen(buf[i])), "", expr_builtin_vars[i].description);
	}
}

static void sig_handler(int signum) {
	running = false;
	fprintf(stderr, "\nExiting\n");
}

static bool run(const char *line) {
	if (line == NULL || line[0] == 0)
		return line != NULL;
	if (strcmp(line, "help") == 0) {
		print_help();
		return true;
	}
	double res;
	ExprError err;
	err = expr_set(e, line);
	if (err.err == NULL)
		err = expr_eval(e, &res);
	if (err.err == NULL) {
		printf("%.*g\n", 15, res);
	} else {
		fprintf(stderr, "Error parsing expression:\n");
		fprintf(stderr, "%s\n", line);
		fprintf(stderr, "%*s", (int)err.start, "");
		for (size_t i = err.start; i <= err.end; i++)
			fprintf(stderr, "^");
		fprintf(stderr, "\n%s\n", err.err);
	}
	return err.err == NULL;
}

#if ENABLE_READLINE
static void winch_handler(int signum) {
	sigwinch_received = true;
}

static void line_handler(char *line) {
	if (line == NULL) {
		rl_callback_handler_remove();
		running = false;
	} else {
		add_history(line);

		last_status_ok = run(line);

		free(line);
	}
}

static char *completer(const char *text, int state) {
	if (!state) {
		completer_idx = 0;
		completer_len = strlen(text);
		completer_funcs = false;
	}

	while (1) {
		if (completer_funcs) {
			if (completer_idx >= expr_n_builtin_funcs) {
				break;
			}
		} else {
			if (completer_idx >= expr_n_builtin_vars) {
				completer_funcs = true;
				completer_idx = 0;
			}
		}

		const char *name;
		if (completer_funcs)
			name = expr_builtin_funcs[completer_idx].name;
		else
			name = expr_builtin_vars[completer_idx].name;
		completer_idx++;
		if (strncmp(name, text, completer_len) == 0) {
			rl_completion_append_character = completer_funcs ? '(' : 0;
			return strdup(name);
		}
	}

	return NULL;
}
#endif

int main(int argc, const char **argv) {
#ifdef ENABLE_READLINE
	rl_catch_signals = false;
	rl_readline_name = "qc";
	rl_completion_entry_function = completer;
	rl_basic_word_break_characters = "+-*/^() ";

	signal (SIGWINCH, winch_handler);
#else
	char buf[2048];
#endif

	struct sigaction action = {0};
	action.sa_handler = sig_handler;
	if (sigaction(SIGINT, &action, NULL) == -1) {
		fprintf(stderr, "Error setting up signal handler: %s\n", strerror(errno));
		return 1;
	}

	e = expr_new();

	if (argc == 1) {
		printf("Running in REPL (read-evaluate-print loop) mode. Type `help` for more information.\n");
		printf("Hit Ctrl+C to exit.\n");
	} else if (argc == 2 && strcmp(argv[1], "-h") != 0 && strcmp(argv[1], "--help") != 0) {
		return !run(argv[1]);
	} else {
		print_help();
		return 1;
	}

#ifdef ENABLE_READLINE
	rl_callback_handler_install("> ", line_handler);
	fd_set fds;
#endif
	while (running) {
#ifdef ENABLE_READLINE
		FD_ZERO(&fds);
		FD_SET(fileno(rl_instream), &fds);
		int r = select(FD_SETSIZE, &fds, NULL, NULL, NULL);
		if (r < 0 && errno != EINTR) {
			rl_callback_handler_remove();
			break;
		}
		if (sigwinch_received) {
			rl_resize_terminal();
			sigwinch_received = false;
		}
		if (r < 0)
			continue;
		if (FD_ISSET(fileno(rl_instream), &fds))
			rl_callback_read_char();
#else
		printf("> ");
		const char *line = fgets(buf, 2048, stdin);
		if (line == NULL)
			break;
		size_t len = strlen(buf);
		if (len == 0)
			continue;
		buf[len-1] = 0;
		last_status_ok = run(line);
#endif
	}

	expr_destroy(e);
	return !last_status_ok;
}
