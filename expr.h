#ifndef __EXPR_H__
#define __EXPR_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct _Expr Expr;

typedef struct {
	size_t start, end;
	const char *err;
} ExprError;

typedef enum {
	ExprArgTypeStr,
	ExprArgTypeNum,
} ExprArgType;

typedef union {
	char *Str;
	double Num;
} ExprArg;

typedef struct {
	const char *name;
	const char *description;
	double (*func)(Expr *e, ExprArg *args);
	const char **arg_names;
	ExprArgType *arg_types;
	size_t n_args;
} ExprBuiltinFunc;

typedef struct {
	const char *name;
	const char *description;
	double val;
} ExprBuiltinVar;

extern ExprBuiltinFunc *expr_builtin_funcs;
extern const size_t     expr_n_builtin_funcs;
extern ExprBuiltinVar  *expr_builtin_vars;
extern const size_t     expr_n_builtin_vars;

Expr *expr_new();
void expr_destroy(Expr *e);
ExprError expr_set(Expr *e, const char *expr) __attribute__((warn_unused_result));
ExprError expr_eval(Expr *e, double *out_res) __attribute__((warn_unused_result));
void expr_set_var(Expr *e, const char *name, double val);
bool expr_get_var(Expr *e, const char *name, double *out); /* Returns false if not present */
void expr_set_func(Expr *e, const char *name, double (*func)(Expr *e, ExprArg *args), ExprArgType *arg_types, size_t n_args);
void expr_set_userdata(Expr *e, void *userdata);
void *expr_get_userdata(Expr *e);

#endif /* __EXPR_H__ */
