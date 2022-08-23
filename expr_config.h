#ifndef EXPR_INCLUDE_CONFIG
#error expr_config.h should not be imported by any files other than expr.c
#endif

static double fn_sqrt(double *args)  {return sqrt(args[0]);              }
static double fn_cbrt(double *args)  {return cbrt(args[0]);              }
static double fn_pow(double *args)   {return pow(args[0], args[1]);      }
static double fn_exp(double *args)   {return exp(args[0]);               }
static double fn_ln(double *args)    {return log(args[0]);               }
static double fn_log(double *args)   {return log(args[1]) / log(args[0]);}
static double fn_mod(double *args)   {return fmod(args[0], args[1]);     }
static double fn_round(double *args) {return round(args[0]);             }
static double fn_floor(double *args) {return floor(args[0]);             }
static double fn_ceil(double *args)  {return ceil(args[0]);              }
static double fn_sin(double *args)   {return sin(args[0]);               }
static double fn_cos(double *args)   {return cos(args[0]);               }
static double fn_tan(double *args)   {return tan(args[0]);               }
static double fn_asin(double *args)  {return asin(args[0]);              }
static double fn_acos(double *args)  {return acos(args[0]);              }
static double fn_atan(double *args)  {return atan(args[0]);              }
static double fn_sinh(double *args)  {return sinh(args[0]);              }
static double fn_cosh(double *args)  {return cosh(args[0]);              }
static double fn_tanh(double *args)  {return tanh(args[0]);              }
static double fn_asinh(double *args) {return asinh(args[0]);             }
static double fn_acosh(double *args) {return acosh(args[0]);             }
static double fn_atanh(double *args) {return atanh(args[0]);             }
static double fn_abs(double *args)   {return fabs(args[0]);              }
static double fn_hypot(double *args) {return hypot(args[0], args[1]);    }
static double fn_polar(double *args) {return atan2(args[1], args[0]);    }
static double fn_max(double *args)   {return fmax(args[0], args[1]);     }
static double fn_min(double *args)   {return fmin(args[0], args[1]);     }
static double fn_rad(double *args)   {return args[0] / M_PI * 180.0;     }
static double fn_deg(double *args)   {return args[0] / 180.0 * M_PI;     }

static const char *arg_names_x[]  = {"x"};
static const char *arg_names_xy[] = {"x", "y"};
static const char *arg_names_nx[] = {"n", "x"};

static ExprBuiltinFunc _builtin_funcs[] = {
	{"sqrt",  "square root of x",                 fn_sqrt,  arg_names_x,  1},
	{"cbrt",  "cube root of x",                   fn_cbrt,  arg_names_x,  1},
	{"pow",   "x^y",                              fn_pow,   arg_names_xy, 2},
	{"exp",   "e^x",                              fn_exp,   arg_names_x,  1},
	{"ln",    "natural log (base e) of x",        fn_ln,    arg_names_x,  1},
	{"log",   "log (base n) of x",                fn_log,   arg_names_nx, 2},
	{"mod",   "x%y",                              fn_mod,   arg_names_xy, 2},
	{"round", "closest integer to x",             fn_round, arg_names_x,  1},
	{"floor", "greatest integer less than x",     fn_floor, arg_names_x,  1},
	{"ceil",  "smallest integer grater than x",   fn_ceil,  arg_names_x,  1},
	{"sin",   "sine of x",                        fn_sin,   arg_names_x,  1},
	{"cos",   "cosine of x",                      fn_cos,   arg_names_x,  1},
	{"tan",   "tangent of x",                     fn_tan,   arg_names_x,  1},
	{"asin",  "inverse sine of x",                fn_asin,  arg_names_x,  1},
	{"acos",  "inverse cosine of x",              fn_acos,  arg_names_x,  1},
	{"atan",  "inverse tangent of x",             fn_atan,  arg_names_x,  1},
	{"sinh",  "hyperbolic sine of x",             fn_sinh,  arg_names_x,  1},
	{"cosh",  "hyperbolic cosine of x",           fn_cosh,  arg_names_x,  1},
	{"tanh",  "hyperbolic tangent of x",          fn_tanh,  arg_names_x,  1},
	{"asinh", "inverse hyperbolic sine of x",     fn_asinh, arg_names_x,  1},
	{"acosh", "inverse hyperbolic cosine of x",   fn_acosh, arg_names_x,  1},
	{"atanh", "inverse hyperbolic tangent of x",  fn_atanh, arg_names_x,  1},
	{"abs",   "absolute value of x",              fn_abs,   arg_names_x,  1},
	{"hypot", "sqrt(x^2+y^2)",                    fn_hypot, arg_names_xy, 2},
	{"polar", "polar coordinates to radians",     fn_polar, arg_names_xy, 2},
	{"max",   "the greater value of x and y",     fn_max,   arg_names_xy, 2},
	{"min",   "the smaller value of x and y",     fn_min,   arg_names_xy, 2},
	{"rad",   "x (radians) to degrees",           fn_rad,   arg_names_x,  1},
	{"deg",   "x (degrees) to radians",           fn_deg,   arg_names_x,  1},
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
