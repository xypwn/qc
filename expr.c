#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "expr.h"

#define EXPR_INCLUDE_CONFIG
#include "expr_config.h"

/* Builtin funcs array */
const size_t expr_n_builtin_funcs = sizeof(_builtin_funcs) / sizeof(_builtin_funcs[0]);
ExprBuiltinFunc *expr_builtin_funcs = _builtin_funcs;

/* Builtin vars array */
const size_t expr_n_builtin_vars = sizeof(_builtin_vars) / sizeof(_builtin_vars[0]);
ExprBuiltinVar *expr_builtin_vars = _builtin_vars;

#define TRY(x) {ExprError _err = x; if (_err.err != NULL) return _err;}

typedef struct {
	size_t start, end;

	enum {
		TokNull,
		TokOp,
		TokNum,
		TokIdent,
	} kind;

	union {
		double Num;
		char Char;
		char *Str;
	};
} Tok;

typedef struct Var {
	char *name;
	double val;
} Var;

typedef struct Func {
	char *name;
	double (*func)(double *args);
	size_t n_args;
} Func;

struct _Expr {
	Tok *toks_working;
	size_t toks_working_len;
	Tok *toks;
	size_t toks_len;
	size_t toks_cap;

	Var *vars;
	size_t vars_len;
	size_t vars_cap;

	Func *funcs;
	size_t funcs_len;
	size_t funcs_cap;
};

static size_t smap_get_idx(void *smap, const char *key, size_t type_size, size_t cap);
static void *smap_get_for_setting(void **smap, const char *key, size_t type_size, size_t *len, size_t *cap);
static void del_toks(Expr *e, Tok *start, Tok *end);
static ExprError collapse(Expr *e, Tok *t) __attribute__((warn_unused_result));
static ExprError eval(Expr *e, Tok *t, double *out_res) __attribute__((warn_unused_result));
static uint32_t fnv1a32(const void *data, size_t n);
static Func get_func(Expr *e, const char *name);
static void push_tok(Expr *e, Tok t);
static ExprError tokenize(Expr *e, const char *expr) __attribute__((warn_unused_result));

const static uint8_t op_prec[256] = {
	['('] = 0, /* A precedence of 0 is reserved for delimiters. */
	[')'] = 0,
	[','] = 0,
	['+'] = 1,
	['-'] = 1,
	['*'] = 2,
	['/'] = 2,
	['^'] = 3,
};
#define OP_PREC(tok_char) (op_prec[(size_t)tok_char])

const static enum {
	OrderLtr,
	OrderRtl,
} op_order[256] = {
	['('] = OrderLtr,
	[')'] = OrderLtr,
	['+'] = OrderLtr,
	['-'] = OrderLtr,
	['*'] = OrderLtr,
	['/'] = OrderLtr,
	['^'] = OrderRtl,
};
#define OP_ORDER(tok_char) (op_order[(size_t)tok_char])

#define IS_NUM(c) (c >= '0' && c <= '9')
#define IS_ALPHA(c) ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
#define IS_SYMBOL(c) ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))

Expr *expr_new() {
	Expr *res = malloc(sizeof(Expr));
	*res = (Expr){0};
	for (size_t i = 0; i < expr_n_builtin_funcs; i++) {
		expr_set_func(res, expr_builtin_funcs[i].name, expr_builtin_funcs[i].func, expr_builtin_funcs[i].n_args);
	}
	for (size_t i = 0; i < expr_n_builtin_vars; i++) {
		expr_set_var(res, expr_builtin_vars[i].name, expr_builtin_vars[i].val);
	}
	return res;
}

void expr_destroy(Expr *e) {
	for (size_t i = 0; i < e->toks_len; i++) {
		if (e->toks[i].kind == TokIdent)
			free(e->toks[i].Str);
	}
	free(e->toks);
	free(e->toks_working);
	for (size_t i = 0; i < e->vars_cap; i++)
		free(e->vars[i].name);
	free(e->vars);
	for (size_t i = 0; i < e->funcs_cap; i++)
		free(e->funcs[i].name);
	free(e->funcs);
	free(e);
}

ExprError expr_set(Expr *e, const char *expr) {
	for (size_t i = 0; i < e->toks_len; i++) {
		if (e->toks[i].kind == TokIdent)
			free(e->toks[i].Str);
	}

	free(e->toks_working); e->toks_working = NULL;
	e->toks_working_len = 0;

	free(e->toks); e->toks = NULL;
	e->toks_len = 0;
	e->toks_cap = 0;

	TRY(tokenize(e, expr));
	return (ExprError){0};
}

ExprError expr_eval(Expr *e, double *out_res) {
	e->toks_working = realloc(e->toks_working, sizeof(Tok) * e->toks_len);
	memcpy(e->toks_working, e->toks, sizeof(Tok) * e->toks_len);
	e->toks_working_len = e->toks_len;
	TRY(eval(e, e->toks_working, out_res));
	return (ExprError){0};
}

static size_t smap_get_idx(void *smap, const char *key, size_t type_size, size_t cap) {
	size_t i = fnv1a32(key, strlen(key)) & (cap - 1);
	while (1) {
		void *i_ptr = (uint8_t*)smap + type_size * i;
		char *i_key = *((char**)i_ptr);
		if (i_key == NULL || strcmp(i_key, key) == 0) {
			return i;
		}
		i = (i + 1) % cap;
	}
}

static void *smap_get_for_setting(void **smap, const char *key, size_t type_size, size_t *len, size_t *cap) {
	if (*cap == 0 || (double)*len / (double)*cap >= 0.7) {
		size_t new_cap = *cap == 0 ? 16 : *cap * 2;
		void *new = malloc(type_size * new_cap);
		for (size_t i = 0; i < new_cap; i++)
			*((char**)((uint8_t*)new + type_size * i)) = NULL;
		for (size_t i = 0; i < *cap; i++) {
			void *i_ptr = (uint8_t*)*smap + type_size * i;
			char *i_key = *((char**)i_ptr);
			if (i_key != NULL) {
				void *ptr = (uint8_t*)new + type_size * smap_get_idx(new, i_key, type_size, new_cap);
				memcpy(ptr, i_ptr, type_size);
			}
		}
		free(*smap);
		*cap = new_cap;
		*smap = new;
	}

	void *ptr = (uint8_t*)*smap + type_size * smap_get_idx(*smap, key, type_size, *cap);
	char **keyptr = (char**)ptr;
	if (*keyptr == NULL) {
		*keyptr = strdup(key);
		(*len)++;
	}
	return ptr;
}

void expr_set_var(Expr *e, const char *name, double val) {
	Var *v = smap_get_for_setting((void**)&e->vars, name, sizeof(Var), &e->vars_len, &e->vars_cap);
	v->val = val;
}

bool expr_get_var(Expr *e, const char *name, double *out) {
	Var v = e->vars[smap_get_idx(e->vars, name, sizeof(Var), e->vars_cap)];
	*out = v.name == NULL ? NAN : v.val;
	return v.name != NULL;
}

void expr_set_func(Expr *e, const char *name, double (*func)(double *args), size_t n_args) {
	Func *v = smap_get_for_setting((void**)&e->funcs, name, sizeof(Func), &e->funcs_len, &e->funcs_cap);
	v->func = func;
	v->n_args = n_args;
}

static void del_toks(Expr *e, Tok *start, Tok *end) {
	memmove(start, end, (e->toks_working_len - (end - e->toks_working)) * sizeof(Tok));
	e->toks_working_len -= end - start;
}

static ExprError collapse(Expr *e, Tok *t) {
	/* Collapse factor. */
	if (t[1].kind == TokOp && t[1].Char == '-') {
		TRY(collapse(e, t + 1));
		if (t[2].kind != TokNum) {
			return (ExprError){.start = t[2].start, .end = t[2].end, .err = "invalid expression after minus factor"};
		}
		t[2].Num *= -1.0;
		del_toks(e, t + 1, t + 2);
	}

	/* Collapse parentheses. */
	if (t[1].kind == TokOp && t[1].Char == '(') {
		double res;
		TRY(eval(e, t + 1, &res));
		size_t i;
		for (i = 2; !(t[i].kind == TokOp && OP_PREC(t[i].Char) == 0); i++);
		del_toks(e, t + 2, t + i + 1);
		/* Put the newly evaluated value into place. */
		t[1].kind = TokNum;
		t[1].Num = res;
	}


	if (t[1].kind == TokIdent) {
		if (t + 2 < e->toks_working + e->toks_working_len && (t[2].kind == TokOp && t[2].Char == '(')) {
			/* Collapse function. */
			double arg_results[16];
			size_t arg_results_size = 0;

			t += 2;
			while (1) {
				if (arg_results_size < 16) {
					double res;
					TRY(eval(e, t, &res));
					arg_results[arg_results_size++] = res;
				}
				size_t i = 1;
				for (; !(t[i].kind == TokOp && OP_PREC(t[i].Char) == 0); i++);
				bool end = t[i].Char == ')';
				if (t[i].Char == ',')
					del_toks(e, t, t + i);
				else if (t[i].Char == ')')
					del_toks(e, t, t + i + 1);
				if (end)
					break;
			}
			t -= 2;

			Func func = get_func(e, t[1].Str);
			if (func.name == NULL)
				return (ExprError){.start = t[1].start, .end = t[1].end, .err = "unknown function"};
			if (arg_results_size != func.n_args)
				return (ExprError){.start = t[1].start, .end = t[1].end, .err = "invalid number of arguments to function"};

			t[1].kind = TokNum;
			t[1].Num = func.func(arg_results);
		} else {
			/* Collapse variable. */
			t[1].kind = TokNum;
			if (!expr_get_var(e, t[1].Str, &t[1].Num))
				return (ExprError){.start = t[1].start, .end = t[1].end, .err = "unknown variable"};
		}
	}
	return (ExprError){0};
}

static ExprError eval(Expr *e, Tok *t, double *out_res) {
	if (!(t[0].kind == TokOp && OP_PREC(t[0].Char) == 0)) {
		return (ExprError){.start = t[0].start, .end = t[0].end, .err = "expected delimiter at beginning of expression"};
	}

	while (1) {
		TRY(collapse(e, t));

		if (!(t[0].kind == TokOp && t[1].kind == TokNum && t[2].kind == TokOp)) {
			return (ExprError){.start = t[0].start, .end = t[1].end, .err = "invalid token order"};
		}

		const char curr_op = t[0].Char;
		const uint8_t curr_prec = OP_PREC(curr_op);

		const char next_op = t[2].Char;
		const uint8_t next_prec = OP_PREC(next_op);

		/* Delimiters have a precedence of 0; if we have a number between two delimiters, we're done. */
		if (curr_prec == 0 && next_prec == 0) {
			*out_res = t[1].Num;
			return (ExprError){0};
		}

		if (next_prec > curr_prec || (next_prec == curr_prec && OP_ORDER(curr_op) == OrderRtl)) {
			t += 2;
		} else if (next_prec < curr_prec || (next_prec == curr_prec && OP_ORDER(curr_op) == OrderLtr)) {
			double res;
			double lhs = t[-1].Num, rhs = t[1].Num;
			switch (curr_op) {
			case '+': res = lhs + rhs; break;
			case '-': res = lhs - rhs; break;
			case '*': res = lhs * rhs; break;
			case '/': res = lhs / rhs; break;
			case '^': res = pow(lhs, rhs); break;
			default:
				return (ExprError){.start = t[0].start, .end = t[0].end, .err = "invalid operator"};
			}

			t[1].Num = res;

			del_toks(e, t - 1, t + 1);

			t -= 2;
		}
	}
}

static uint32_t fnv1a32(const void *data, size_t n) {
	uint32_t res = 2166136261u;
	for (size_t i = 0; i < n; i++) {
		res ^= ((uint8_t*)data)[i];
		res *= 16777619u;
	}
	return res;
}

static Func get_func(Expr *e, const char *name) {
	return e->funcs[smap_get_idx(e->funcs, name, sizeof(Func), e->funcs_cap)];
}

static void push_tok(Expr *e, Tok t) {
	if (e->toks_len >= e->toks_cap) {
		size_t new_cap = e->toks_cap == 0 ? 16 : e->toks_cap * 2;
		e->toks = realloc(e->toks, sizeof(Tok) * new_cap);
		e->toks_cap = new_cap;
	}
	e->toks[e->toks_len++] = t;
}

static ExprError tokenize(Expr *e, const char *expr) {
	size_t start;

	push_tok(e, (Tok){.start = 0, .end = 1, .kind = TokOp, .Char = '('});

	/* We try to use the stack first for parenthesis tracking and only resort
	 * to heap allocation if the depth exceeds 16. */
	size_t paren_depth = 0;
	size_t parens_cap = 16;
	size_t *h_parens = NULL;
	size_t s_parens[16];
	size_t *parens = s_parens;

	Tok last;
	const char *curr = expr;
	for (char c = *curr; c != 0; c = *(++curr)) {
		if (e->toks_len > 0)
			last = e->toks[e->toks_len-1];
		else
			last = (Tok){.kind = TokNull};

		if (c == ' ')
			continue;

		if (IS_NUM(c) || c == '.') {
			bool dot_seen = c == '.';
			bool e_seen = false;
			char add_e_as_var = 0;
			char buf[32];
			start = curr - expr;
			buf[0] = c;
			size_t i = 1;
			while (i < 31 && (IS_NUM(curr[i]) || curr[i] == '.' || curr[i] == 'e' || curr[i] == 'E' || ((curr[i-1] == 'e' || curr[i-1] == 'E') && (curr[i] == '-' || curr[i] == '+')))) {
				if (curr[i] == '.') {
					if (dot_seen) {
						free(h_parens);
						return (ExprError){.start = start + i, .end = start + i, .err = "more than one dot in decimal number"};
					} else if (e_seen) {
						return (ExprError){.start = start + i, .end = start + i, .err = "decimal dot not allowed in exponent"};
					} else
						dot_seen = true;
				}
				if (curr[i] == 'e' || curr[i] == 'E') {
					if (e_seen) {
						free(h_parens);
						return (ExprError){.start = start + i, .end = start + i, .err = "more than one 'e' or 'E' in decimal number"};
					} else
						e_seen = true;
				}
				buf[i] = curr[i];
				i++;
			}

			if (i > 0 && (curr[i-1] == 'e' || curr[i-1] == 'E'))
				add_e_as_var = curr[i-1];

			curr += i - 1;

			if (add_e_as_var)
				buf[--i] = 0;
			else
				buf[i] = 0;

			char *endptr;
			double num = strtod(buf, &endptr);
			size_t endpos = endptr - buf;
			if (endpos != i) {
				free(h_parens);
				return (ExprError){.start = start + endpos, .end = start + endpos, .err = "error parsing number"};
			}

			if (last.kind == TokIdent || (last.kind == TokOp && last.Char == ')') || last.kind == TokNum)
				push_tok(e, (Tok){.start = last.end + 1, .end = last.end + 1, .kind = TokOp, .Char = '*'});

			push_tok(e, (Tok){.start = start, .end = curr - expr, .kind = TokNum, .Num = num});

			if (add_e_as_var) {
				char b[2] = { add_e_as_var, 0 };
				push_tok(e, (Tok){.start = curr - expr + 1, .end = curr - expr + 1, .kind = TokOp, .Char = '*'});
				push_tok(e, (Tok){.start = curr - expr - 1, .end = curr - expr - 1, .kind = TokIdent, .Str = strdup(b)});
			}
			continue;
		}

		if (IS_SYMBOL(c)) {
			start = curr - expr;
			char *buf = malloc(32);
			buf[0] = c;
			size_t i = 1;
			while (i < 31 && IS_SYMBOL(curr[i])) {
				buf[i] = curr[i];
				i++;
			}
			curr += i - 1;
			buf[i++] = 0;

			if (last.kind == TokIdent || (last.kind == TokOp && last.Char == ')') || last.kind == TokNum)
				push_tok(e, (Tok){.start = last.end + 1, .end = last.end + 1, .kind = TokOp, .Char = '*'});

			push_tok(e, (Tok){.start = start, .end = curr - expr, .kind = TokIdent, .Str = buf});
			continue;
		}

		if (c == '(') {
			if (paren_depth+1 > parens_cap) {
				bool h_n = h_parens == NULL;
				parens = h_parens = realloc(h_parens, sizeof(size_t) * (parens_cap *= 2));
				if (h_n) {
					memcpy(h_parens, s_parens, sizeof(size_t) * 16);
				}
			}
			parens[paren_depth++] = curr - expr;
		} else if (c == ')') {
			if (paren_depth == 0) {
				free(h_parens);
				return (ExprError){.start = curr - expr, .end = curr - expr, .err = "unmatched ')'"};
			}
			paren_depth--;
		}

		switch (c) {
		case '(':
		case ')':
		case ',':
		case '+':
		case '-':
		case '*':
		case '/':
		case '^': {
			if (c == '(' && ((last.kind == TokOp && last.Char == ')') || last.kind == TokNum))
				push_tok(e, (Tok){.start = last.end + 1, .end = last.end + 1, .kind = TokOp, .Char = '*'});
			push_tok(e, (Tok){.start = curr - expr, .end = curr - expr, .kind = TokOp, .Char = c});
			break;
		}
		default:
			free(h_parens);
			return (ExprError){.start = curr - expr, .end = curr - expr, .err = "unrecognized symbol"};
		}
	}

	if (paren_depth > 0) {
		size_t p = parens[paren_depth-1];
		free(h_parens);
		return (ExprError){.start = p, .end = p, .err = "unmatched '('"};
	}

	push_tok(e, (Tok){.start = curr - expr, .end = curr - expr, .kind = TokOp, .Char = ')'});

	free(h_parens);
	return (ExprError){0};
}
