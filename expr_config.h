#ifndef EXPR_INCLUDE_CONFIG
#error expr_config.h should not be imported by any files other than expr.c
#endif

static double fn_sqrt(Expr *e, ExprArg *args)  {return sqrt(args[0].Num);                  }
static double fn_cbrt(Expr *e, ExprArg *args)  {return cbrt(args[0].Num);                  }
static double fn_pow(Expr *e, ExprArg *args)   {return pow(args[0].Num, args[1].Num);      }
static double fn_exp(Expr *e, ExprArg *args)   {return exp(args[0].Num);                   }
static double fn_ln(Expr *e, ExprArg *args)    {return log(args[0].Num);                   }
static double fn_log(Expr *e, ExprArg *args)   {return log(args[1].Num) / log(args[0].Num);}
static double fn_mod(Expr *e, ExprArg *args)   {return fmod(args[0].Num, args[1].Num);     }
static double fn_round(Expr *e, ExprArg *args) {return round(args[0].Num);                 }
static double fn_floor(Expr *e, ExprArg *args) {return floor(args[0].Num);                 }
static double fn_ceil(Expr *e, ExprArg *args)  {return ceil(args[0].Num);                  }
static double fn_sin(Expr *e, ExprArg *args)   {return sin(args[0].Num);                   }
static double fn_cos(Expr *e, ExprArg *args)   {return cos(args[0].Num);                   }
static double fn_tan(Expr *e, ExprArg *args)   {return tan(args[0].Num);                   }
static double fn_asin(Expr *e, ExprArg *args)  {return asin(args[0].Num);                  }
static double fn_acos(Expr *e, ExprArg *args)  {return acos(args[0].Num);                  }
static double fn_atan(Expr *e, ExprArg *args)  {return atan(args[0].Num);                  }
static double fn_sinh(Expr *e, ExprArg *args)  {return sinh(args[0].Num);                  }
static double fn_cosh(Expr *e, ExprArg *args)  {return cosh(args[0].Num);                  }
static double fn_tanh(Expr *e, ExprArg *args)  {return tanh(args[0].Num);                  }
static double fn_asinh(Expr *e, ExprArg *args) {return asinh(args[0].Num);                 }
static double fn_acosh(Expr *e, ExprArg *args) {return acosh(args[0].Num);                 }
static double fn_atanh(Expr *e, ExprArg *args) {return atanh(args[0].Num);                 }
static double fn_abs(Expr *e, ExprArg *args)   {return fabs(args[0].Num);                  }
static double fn_hypot(Expr *e, ExprArg *args) {return hypot(args[0].Num, args[1].Num);    }
static double fn_polar(Expr *e, ExprArg *args) {return atan2(args[1].Num, args[0].Num);    }
static double fn_max(Expr *e, ExprArg *args)   {return fmax(args[0].Num, args[1].Num);     }
static double fn_min(Expr *e, ExprArg *args)   {return fmin(args[0].Num, args[1].Num);     }
static double fn_rad(Expr *e, ExprArg *args)   {return args[0].Num / M_PI * 180.0;         }
static double fn_deg(Expr *e, ExprArg *args)   {return args[0].Num / 180.0 * M_PI;         }

static double fn_set(Expr *e, ExprArg *args)   {
	expr_set_var(e, args[0].Str, args[1].Num);
	return args[1].Num;
}

static ExprArgType arg_types_n[]  = {ExprArgTypeNum                };
static ExprArgType arg_types_nn[] = {ExprArgTypeNum, ExprArgTypeNum};
static ExprArgType arg_types_sn[] = {ExprArgTypeStr, ExprArgTypeNum};

static const char *arg_names_x[]        = {"x"            };
static const char *arg_names_xy[]       = {"x",    "y"    };
static const char *arg_names_nx[]       = {"n",    "x"    };
static const char *arg_names_name_val[] = {"name", "value"};

static ExprBuiltinFunc _builtin_funcs[] = {
	{"sqrt",  "square root of x",                 fn_sqrt,  arg_names_x,         arg_types_n,  1},
	{"cbrt",  "cube root of x",                   fn_cbrt,  arg_names_x,         arg_types_n,  1},
	{"pow",   "x^y",                              fn_pow,   arg_names_xy,        arg_types_nn, 2},
	{"exp",   "e^x",                              fn_exp,   arg_names_x,         arg_types_n,  1},
	{"ln",    "natural log (base e) of x",        fn_ln,    arg_names_x,         arg_types_n,  1},
	{"log",   "log (base n) of x",                fn_log,   arg_names_nx,        arg_types_nn, 2},
	{"mod",   "x%y",                              fn_mod,   arg_names_xy,        arg_types_nn, 2},
	{"round", "closest integer to x",             fn_round, arg_names_x,         arg_types_n,  1},
	{"floor", "greatest integer less than x",     fn_floor, arg_names_x,         arg_types_n,  1},
	{"ceil",  "smallest integer grater than x",   fn_ceil,  arg_names_x,         arg_types_n,  1},
	{"sin",   "sine of x",                        fn_sin,   arg_names_x,         arg_types_n,  1},
	{"cos",   "cosine of x",                      fn_cos,   arg_names_x,         arg_types_n,  1},
	{"tan",   "tangent of x",                     fn_tan,   arg_names_x,         arg_types_n,  1},
	{"asin",  "inverse sine of x",                fn_asin,  arg_names_x,         arg_types_n,  1},
	{"acos",  "inverse cosine of x",              fn_acos,  arg_names_x,         arg_types_n,  1},
	{"atan",  "inverse tangent of x",             fn_atan,  arg_names_x,         arg_types_n,  1},
	{"sinh",  "hyperbolic sine of x",             fn_sinh,  arg_names_x,         arg_types_n,  1},
	{"cosh",  "hyperbolic cosine of x",           fn_cosh,  arg_names_x,         arg_types_n,  1},
	{"tanh",  "hyperbolic tangent of x",          fn_tanh,  arg_names_x,         arg_types_n,  1},
	{"asinh", "inverse hyperbolic sine of x",     fn_asinh, arg_names_x,         arg_types_n,  1},
	{"acosh", "inverse hyperbolic cosine of x",   fn_acosh, arg_names_x,         arg_types_n,  1},
	{"atanh", "inverse hyperbolic tangent of x",  fn_atanh, arg_names_x,         arg_types_n,  1},
	{"abs",   "absolute value of x",              fn_abs,   arg_names_x,         arg_types_n,  1},
	{"hypot", "sqrt(x^2+y^2)",                    fn_hypot, arg_names_xy,        arg_types_nn, 2},
	{"polar", "polar coordinates to radians",     fn_polar, arg_names_xy,        arg_types_nn, 2},
	{"max",   "the greater value of x and y",     fn_max,   arg_names_xy,        arg_types_nn, 2},
	{"min",   "the smaller value of x and y",     fn_min,   arg_names_xy,        arg_types_nn, 2},
	{"rad",   "x (radians) to degrees",           fn_rad,   arg_names_x,         arg_types_n,  1},
	{"deg",   "x (degrees) to radians",           fn_deg,   arg_names_x,         arg_types_n,  1},

	{"set",   "(re-)set the value of a variable", fn_set,   arg_names_name_val,  arg_types_sn, 2},
};

static ExprBuiltinVar _builtin_vars[] = {
	{"pi",  "π",                                  M_PI                  },
	{"tau", "τ = 2π",                             2.0 * M_PI            },
	{"e",   "Euler's number",                     M_E                   },
	{"phi", "golden ratio",                       1.61803398874989484820},
	{"h",   "Planck constant (Js)",               6.62607015e-34        },
	{"NA",  "Avogadro constant (1/mol)",          6.02214076e23         },
	{"c",   "speed of light (m/s)",               299792458             },
	{"g",   "gravitational acceleration (m/s^2)", 9.80665               },
	{"G",   "gravitational constant (N(m/kg)^2)", 6.673889e-11          },
	{"k",   "Boltzmann constant (J/K)",           1.380649e-23          },
};

#undef EXPR_INCLUDE_CONFIG
