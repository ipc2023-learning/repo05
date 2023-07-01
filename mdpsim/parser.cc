/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 20 "parser.yy"

#include <config.h>
#include "problems.h"
#include "domains.h"
#include "actions.h"
#include "effects.h"
#include "formulas.h"
#include "expressions.h"
#include "functions.h"
#include "predicates.h"
#include "terms.h"
#include "types.h"
#include "rational.h"
#include <iostream>
#include <map>
#include <string>
#include <stdexcept>
#include <typeinfo>


/* Workaround for bug in Bison 1.35 that disables stack growth. */
#define YYLTYPE_IS_TRIVIAL 1


/*
 * Context of free variables.
 */
struct Context {
  void push_frame() {
    frames_.push_back(VariableMap());
  }

  void pop_frame() {
    frames_.pop_back();
  }

  void insert(const std::string& name, const Variable& v) {
    frames_.back().insert(std::make_pair(name, v));
  }

  const Variable* shallow_find(const std::string& name) const {
    VariableMap::const_iterator vi = frames_.back().find(name);
    if (vi != frames_.back().end()) {
      return &(*vi).second;
    } else {
      return 0;
    }
  }

  const Variable* find(const std::string& name) const {
    for (std::vector<VariableMap>::const_reverse_iterator fi =
           frames_.rbegin(); fi != frames_.rend(); fi++) {
      VariableMap::const_iterator vi = (*fi).find(name);
      if (vi != (*fi).end()) {
        return &(*vi).second;
      }
    }
    return 0;
  }

private:
  struct VariableMap : public std::map<std::string, Variable> {
  };

  std::vector<VariableMap> frames_;
};


/* The lexer. */
extern int yylex();
/* Current line number. */
extern size_t line_number;
/* Name of current file. */
extern std::string current_file;
/* Level of warnings. */
extern int warning_level;

/* Whether the last parsing attempt succeeded. */
static bool success = true;
/* Current domain. */
static Domain* domain;
/* Domains. */
static std::map<std::string, Domain*> domains;
/* Pointer to problem being parsed, or 0 if no problem is being parsed. */
static Problem* problem;
/* Current requirements. */
static Requirements* requirements;
/* The reward function, if rewards are required. */
static Function reward_function(-1);
/* Predicate being parsed. */
static const Predicate* predicate;
/* Whether predicate declaration is repeated. */
static bool repeated_predicate;
/* Function being parsed. */
static const Function* function;
/* Whether function declaration is repeated. */
static bool repeated_function;
/* Pointer to action being parsed, or 0 if no action is being parsed. */
static ActionSchema* action;
/* Current variable context. */
static Context context;
/* Predicate for atomic state formula being parsed. */
static const Predicate* atom_predicate;
/* Whether the predicate of the currently parsed atom was undeclared. */
static bool undeclared_atom_predicate;
/* Whether parsing effect fluents. */
static bool effect_fluent;
/* Whether parsing metric fluent. */
static bool metric_fluent;
/* Function for fluent being parsed. */
static const Function* fluent_function;
/* Whether the function of the currently parsed fluent was undeclared. */
static bool undeclared_fluent_function;
/* Paramerers for atomic state formula or fluent being parsed. */
static TermList term_parameters;
/* Quantified variables for effect or formula being parsed. */
static TermList quantified;
/* Most recently parsed term for equality formula. */
static Term eq_term(0);
/* Most recently parsed expression for equality formula. */
static const Expression* eq_expr;
/* The first term for equality formula. */
static Term first_eq_term(0);
/* The first expression for equality formula. */
static const Expression* first_eq_expr;
/* Kind of name map being parsed. */
static enum { TYPE_KIND, CONSTANT_KIND, OBJECT_KIND, VOID_KIND } name_kind;

/* Outputs an error message. */
static void yyerror(const std::string& s);
/* Outputs a warning message. */
static void yywarning(const std::string& s);
/* Creates an empty domain with the given name. */
static void make_domain(const std::string* name);
/* Creates an empty problem with the given name. */
static void make_problem(const std::string* name,
                         const std::string* domain_name);
/* Adds :typing to the requirements. */
static void require_typing();
/* Adds :fluents to the requirements. */
static void require_fluents();
/* Adds :disjunctive-preconditions to the requirements. */
static void require_disjunction();
/* Adds :conditional-effects to the requirements. */
static void require_conditional_effects();
/* Returns a simple type with the given name. */
static const Type& make_type(const std::string* name);
/* Returns the union of the given types. */
static Type make_type(const TypeSet& types);
/* Returns a simple term with the given name. */
static Term make_term(const std::string* name);
/* Creates a predicate with the given name. */
static void make_predicate(const std::string* name);
/* Creates a function with the given name. */
static void make_function(const std::string* name);
/* Creates an action with the given name. */
static void make_action(const std::string* name);
/* Adds the current action to the current domain. */
static void add_action();
/* Prepares for the parsing of a universally quantified effect. */
static void prepare_forall_effect();
/* Creates a universally quantified effect. */
static const Effect* make_forall_effect(const Effect& effect);
/* Adds an outcome to the given probabilistic effect. */
static void add_outcome(std::vector<std::pair<Rational, const Effect*> >& os,
                        const Rational* p, const Effect& effect);
/* Creates a probabilistic effect. */
static const Effect*
make_prob_effect(const std::vector<std::pair<Rational, const Effect*> >* os);
/* Creates an add effect. */
static const Effect* make_add_effect(const Atom& atom);
/* Creates a delete effect. */
static const Effect* make_delete_effect(const Atom& atom);
/* Creates an assign update effect. */
static const Effect* make_assign_effect(const Fluent& fluent,
                                        const Expression& expr);
/* Creates a scale-up update effect. */
static const Effect* make_scale_up_effect(const Fluent& fluent,
                                          const Expression& expr);
/* Creates a scale-down update effect. */
static const Effect* make_scale_down_effect(const Fluent& fluent,
                                            const Expression& expr);
/* Creates an increase update effect. */
static const Effect* make_increase_effect(const Fluent& fluent,
                                          const Expression& expr);
/* Creates a decrease update effect. */
static const Effect* make_decrease_effect(const Fluent& fluent,
                                          const Expression& expr);
/* Adds types, constants, or objects to the current domain or problem. */
static void add_names(const std::vector<const std::string*>* names,
                      const Type& type);
/* Adds variables to the current variable list. */
static void add_variables(const std::vector<const std::string*>* names,
                          const Type& type);
/* Prepares for the parsing of an atomic state formula. */
static void prepare_atom(const std::string* name);
/* Prepares for the parsing of a fluent. */
static void prepare_fluent(const std::string* name);
/* Adds a term with the given name to the current atomic state formula. */
static void add_term(const std::string* name);
/* Creates the atomic formula just parsed. */
static const Atom* make_atom();
/* Creates the fluent just parsed. */
static const Fluent* make_fluent();
/* Creates a subtraction. */
static const Expression* make_subtraction(const Expression& term,
                                          const Expression* opt_term);
/* Creates an atom or fluent for the given name to be used in an
   equality formula. */
static void make_eq_name(const std::string* name);
/* Creates an equality formula. */
static const StateFormula* make_equality();
/* Creates a negated formula. */
static const StateFormula* make_negation(const StateFormula& negand);
/* Creates an implication. */
static const StateFormula* make_implication(const StateFormula& f1,
                                            const StateFormula& f2);
/* Prepares for the parsing of an existentially quantified formula. */
static void prepare_exists();
/* Prepares for the parsing of a universally quantified formula. */
static void prepare_forall();
/* Creates an existentially quantified formula. */
static const StateFormula* make_exists(const StateFormula& body);
/* Creates a universally quantified formula. */
static const StateFormula* make_forall(const StateFormula& body);
/* Sets the goal reward for the current problem. */
static void set_goal_reward(const Expression& goal_reward);
/* Sets the default metric for the current problem. */
static void set_default_metric();

#line 302 "parser.cc"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    DEFINE = 258,                  /* DEFINE  */
    DOMAIN_TOKEN = 259,            /* DOMAIN_TOKEN  */
    PROBLEM = 260,                 /* PROBLEM  */
    REQUIREMENTS = 261,            /* REQUIREMENTS  */
    TYPES = 262,                   /* TYPES  */
    CONSTANTS = 263,               /* CONSTANTS  */
    PREDICATES = 264,              /* PREDICATES  */
    FUNCTIONS = 265,               /* FUNCTIONS  */
    STRIPS = 266,                  /* STRIPS  */
    TYPING = 267,                  /* TYPING  */
    NEGATIVE_PRECONDITIONS = 268,  /* NEGATIVE_PRECONDITIONS  */
    DISJUNCTIVE_PRECONDITIONS = 269, /* DISJUNCTIVE_PRECONDITIONS  */
    EQUALITY = 270,                /* EQUALITY  */
    EXISTENTIAL_PRECONDITIONS = 271, /* EXISTENTIAL_PRECONDITIONS  */
    UNIVERSAL_PRECONDITIONS = 272, /* UNIVERSAL_PRECONDITIONS  */
    QUANTIFIED_PRECONDITIONS = 273, /* QUANTIFIED_PRECONDITIONS  */
    CONDITIONAL_EFFECTS = 274,     /* CONDITIONAL_EFFECTS  */
    FLUENTS = 275,                 /* FLUENTS  */
    ADL = 276,                     /* ADL  */
    DURATIVE_ACTIONS = 277,        /* DURATIVE_ACTIONS  */
    DURATION_INEQUALITIES = 278,   /* DURATION_INEQUALITIES  */
    CONTINUOUS_EFFECTS = 279,      /* CONTINUOUS_EFFECTS  */
    PROBABILISTIC_EFFECTS = 280,   /* PROBABILISTIC_EFFECTS  */
    REWARDS = 281,                 /* REWARDS  */
    MDP = 282,                     /* MDP  */
    ACTION = 283,                  /* ACTION  */
    PARAMETERS = 284,              /* PARAMETERS  */
    PRECONDITION = 285,            /* PRECONDITION  */
    EFFECT = 286,                  /* EFFECT  */
    PDOMAIN = 287,                 /* PDOMAIN  */
    OBJECTS = 288,                 /* OBJECTS  */
    INIT = 289,                    /* INIT  */
    GOAL = 290,                    /* GOAL  */
    GOAL_REWARD = 291,             /* GOAL_REWARD  */
    METRIC = 292,                  /* METRIC  */
    TOTAL_TIME = 293,              /* TOTAL_TIME  */
    GOAL_ACHIEVED = 294,           /* GOAL_ACHIEVED  */
    WHEN = 295,                    /* WHEN  */
    NOT = 296,                     /* NOT  */
    AND = 297,                     /* AND  */
    OR = 298,                      /* OR  */
    IMPLY = 299,                   /* IMPLY  */
    EXISTS = 300,                  /* EXISTS  */
    FORALL = 301,                  /* FORALL  */
    PROBABILISTIC = 302,           /* PROBABILISTIC  */
    ASSIGN = 303,                  /* ASSIGN  */
    SCALE_UP = 304,                /* SCALE_UP  */
    SCALE_DOWN = 305,              /* SCALE_DOWN  */
    INCREASE = 306,                /* INCREASE  */
    DECREASE = 307,                /* DECREASE  */
    MINIMIZE = 308,                /* MINIMIZE  */
    MAXIMIZE = 309,                /* MAXIMIZE  */
    NUMBER_TOKEN = 310,            /* NUMBER_TOKEN  */
    OBJECT_TOKEN = 311,            /* OBJECT_TOKEN  */
    EITHER = 312,                  /* EITHER  */
    LE = 313,                      /* LE  */
    GE = 314,                      /* GE  */
    NAME = 315,                    /* NAME  */
    VARIABLE = 316,                /* VARIABLE  */
    NUMBER = 317,                  /* NUMBER  */
    ILLEGAL_TOKEN = 318            /* ILLEGAL_TOKEN  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define DEFINE 258
#define DOMAIN_TOKEN 259
#define PROBLEM 260
#define REQUIREMENTS 261
#define TYPES 262
#define CONSTANTS 263
#define PREDICATES 264
#define FUNCTIONS 265
#define STRIPS 266
#define TYPING 267
#define NEGATIVE_PRECONDITIONS 268
#define DISJUNCTIVE_PRECONDITIONS 269
#define EQUALITY 270
#define EXISTENTIAL_PRECONDITIONS 271
#define UNIVERSAL_PRECONDITIONS 272
#define QUANTIFIED_PRECONDITIONS 273
#define CONDITIONAL_EFFECTS 274
#define FLUENTS 275
#define ADL 276
#define DURATIVE_ACTIONS 277
#define DURATION_INEQUALITIES 278
#define CONTINUOUS_EFFECTS 279
#define PROBABILISTIC_EFFECTS 280
#define REWARDS 281
#define MDP 282
#define ACTION 283
#define PARAMETERS 284
#define PRECONDITION 285
#define EFFECT 286
#define PDOMAIN 287
#define OBJECTS 288
#define INIT 289
#define GOAL 290
#define GOAL_REWARD 291
#define METRIC 292
#define TOTAL_TIME 293
#define GOAL_ACHIEVED 294
#define WHEN 295
#define NOT 296
#define AND 297
#define OR 298
#define IMPLY 299
#define EXISTS 300
#define FORALL 301
#define PROBABILISTIC 302
#define ASSIGN 303
#define SCALE_UP 304
#define SCALE_DOWN 305
#define INCREASE 306
#define DECREASE 307
#define MINIMIZE 308
#define MAXIMIZE 309
#define NUMBER_TOKEN 310
#define OBJECT_TOKEN 311
#define EITHER 312
#define LE 313
#define GE 314
#define NAME 315
#define VARIABLE 316
#define NUMBER 317
#define ILLEGAL_TOKEN 318

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 266 "parser.yy"

  const Effect* effect;
  std::vector<std::pair<Rational, const Effect*> >* outcomes;
  const StateFormula* formula;
  const Atom* atom;
  const Expression* expr;
  const Fluent* fluent;
  const Type* type;
  TypeSet* types;
  const std::string* str;
  std::vector<const std::string*>* strs;
  const Rational* num;

#line 495 "parser.cc"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_DEFINE = 3,                     /* DEFINE  */
  YYSYMBOL_DOMAIN_TOKEN = 4,               /* DOMAIN_TOKEN  */
  YYSYMBOL_PROBLEM = 5,                    /* PROBLEM  */
  YYSYMBOL_REQUIREMENTS = 6,               /* REQUIREMENTS  */
  YYSYMBOL_TYPES = 7,                      /* TYPES  */
  YYSYMBOL_CONSTANTS = 8,                  /* CONSTANTS  */
  YYSYMBOL_PREDICATES = 9,                 /* PREDICATES  */
  YYSYMBOL_FUNCTIONS = 10,                 /* FUNCTIONS  */
  YYSYMBOL_STRIPS = 11,                    /* STRIPS  */
  YYSYMBOL_TYPING = 12,                    /* TYPING  */
  YYSYMBOL_NEGATIVE_PRECONDITIONS = 13,    /* NEGATIVE_PRECONDITIONS  */
  YYSYMBOL_DISJUNCTIVE_PRECONDITIONS = 14, /* DISJUNCTIVE_PRECONDITIONS  */
  YYSYMBOL_EQUALITY = 15,                  /* EQUALITY  */
  YYSYMBOL_EXISTENTIAL_PRECONDITIONS = 16, /* EXISTENTIAL_PRECONDITIONS  */
  YYSYMBOL_UNIVERSAL_PRECONDITIONS = 17,   /* UNIVERSAL_PRECONDITIONS  */
  YYSYMBOL_QUANTIFIED_PRECONDITIONS = 18,  /* QUANTIFIED_PRECONDITIONS  */
  YYSYMBOL_CONDITIONAL_EFFECTS = 19,       /* CONDITIONAL_EFFECTS  */
  YYSYMBOL_FLUENTS = 20,                   /* FLUENTS  */
  YYSYMBOL_ADL = 21,                       /* ADL  */
  YYSYMBOL_DURATIVE_ACTIONS = 22,          /* DURATIVE_ACTIONS  */
  YYSYMBOL_DURATION_INEQUALITIES = 23,     /* DURATION_INEQUALITIES  */
  YYSYMBOL_CONTINUOUS_EFFECTS = 24,        /* CONTINUOUS_EFFECTS  */
  YYSYMBOL_PROBABILISTIC_EFFECTS = 25,     /* PROBABILISTIC_EFFECTS  */
  YYSYMBOL_REWARDS = 26,                   /* REWARDS  */
  YYSYMBOL_MDP = 27,                       /* MDP  */
  YYSYMBOL_ACTION = 28,                    /* ACTION  */
  YYSYMBOL_PARAMETERS = 29,                /* PARAMETERS  */
  YYSYMBOL_PRECONDITION = 30,              /* PRECONDITION  */
  YYSYMBOL_EFFECT = 31,                    /* EFFECT  */
  YYSYMBOL_PDOMAIN = 32,                   /* PDOMAIN  */
  YYSYMBOL_OBJECTS = 33,                   /* OBJECTS  */
  YYSYMBOL_INIT = 34,                      /* INIT  */
  YYSYMBOL_GOAL = 35,                      /* GOAL  */
  YYSYMBOL_GOAL_REWARD = 36,               /* GOAL_REWARD  */
  YYSYMBOL_METRIC = 37,                    /* METRIC  */
  YYSYMBOL_TOTAL_TIME = 38,                /* TOTAL_TIME  */
  YYSYMBOL_GOAL_ACHIEVED = 39,             /* GOAL_ACHIEVED  */
  YYSYMBOL_WHEN = 40,                      /* WHEN  */
  YYSYMBOL_NOT = 41,                       /* NOT  */
  YYSYMBOL_AND = 42,                       /* AND  */
  YYSYMBOL_OR = 43,                        /* OR  */
  YYSYMBOL_IMPLY = 44,                     /* IMPLY  */
  YYSYMBOL_EXISTS = 45,                    /* EXISTS  */
  YYSYMBOL_FORALL = 46,                    /* FORALL  */
  YYSYMBOL_PROBABILISTIC = 47,             /* PROBABILISTIC  */
  YYSYMBOL_ASSIGN = 48,                    /* ASSIGN  */
  YYSYMBOL_SCALE_UP = 49,                  /* SCALE_UP  */
  YYSYMBOL_SCALE_DOWN = 50,                /* SCALE_DOWN  */
  YYSYMBOL_INCREASE = 51,                  /* INCREASE  */
  YYSYMBOL_DECREASE = 52,                  /* DECREASE  */
  YYSYMBOL_MINIMIZE = 53,                  /* MINIMIZE  */
  YYSYMBOL_MAXIMIZE = 54,                  /* MAXIMIZE  */
  YYSYMBOL_NUMBER_TOKEN = 55,              /* NUMBER_TOKEN  */
  YYSYMBOL_OBJECT_TOKEN = 56,              /* OBJECT_TOKEN  */
  YYSYMBOL_EITHER = 57,                    /* EITHER  */
  YYSYMBOL_LE = 58,                        /* LE  */
  YYSYMBOL_GE = 59,                        /* GE  */
  YYSYMBOL_NAME = 60,                      /* NAME  */
  YYSYMBOL_VARIABLE = 61,                  /* VARIABLE  */
  YYSYMBOL_NUMBER = 62,                    /* NUMBER  */
  YYSYMBOL_ILLEGAL_TOKEN = 63,             /* ILLEGAL_TOKEN  */
  YYSYMBOL_64_ = 64,                       /* '('  */
  YYSYMBOL_65_ = 65,                       /* ')'  */
  YYSYMBOL_66_ = 66,                       /* '-'  */
  YYSYMBOL_67_ = 67,                       /* '='  */
  YYSYMBOL_68_ = 68,                       /* '<'  */
  YYSYMBOL_69_ = 69,                       /* '>'  */
  YYSYMBOL_70_ = 70,                       /* '+'  */
  YYSYMBOL_71_ = 71,                       /* '*'  */
  YYSYMBOL_72_ = 72,                       /* '/'  */
  YYSYMBOL_YYACCEPT = 73,                  /* $accept  */
  YYSYMBOL_file = 74,                      /* file  */
  YYSYMBOL_75_1 = 75,                      /* $@1  */
  YYSYMBOL_domains_and_problems = 76,      /* domains_and_problems  */
  YYSYMBOL_domain_def = 77,                /* domain_def  */
  YYSYMBOL_78_2 = 78,                      /* $@2  */
  YYSYMBOL_domain_body = 79,               /* domain_body  */
  YYSYMBOL_domain_body2 = 80,              /* domain_body2  */
  YYSYMBOL_domain_body3 = 81,              /* domain_body3  */
  YYSYMBOL_domain_body4 = 82,              /* domain_body4  */
  YYSYMBOL_domain_body5 = 83,              /* domain_body5  */
  YYSYMBOL_domain_body6 = 84,              /* domain_body6  */
  YYSYMBOL_domain_body7 = 85,              /* domain_body7  */
  YYSYMBOL_domain_body8 = 86,              /* domain_body8  */
  YYSYMBOL_domain_body9 = 87,              /* domain_body9  */
  YYSYMBOL_structure_defs = 88,            /* structure_defs  */
  YYSYMBOL_structure_def = 89,             /* structure_def  */
  YYSYMBOL_require_def = 90,               /* require_def  */
  YYSYMBOL_require_keys = 91,              /* require_keys  */
  YYSYMBOL_require_key = 92,               /* require_key  */
  YYSYMBOL_types_def = 93,                 /* types_def  */
  YYSYMBOL_94_3 = 94,                      /* $@3  */
  YYSYMBOL_constants_def = 95,             /* constants_def  */
  YYSYMBOL_96_4 = 96,                      /* $@4  */
  YYSYMBOL_predicates_def = 97,            /* predicates_def  */
  YYSYMBOL_functions_def = 98,             /* functions_def  */
  YYSYMBOL_99_5 = 99,                      /* $@5  */
  YYSYMBOL_predicate_decls = 100,          /* predicate_decls  */
  YYSYMBOL_predicate_decl = 101,           /* predicate_decl  */
  YYSYMBOL_102_6 = 102,                    /* $@6  */
  YYSYMBOL_function_decls = 103,           /* function_decls  */
  YYSYMBOL_function_decl_seq = 104,        /* function_decl_seq  */
  YYSYMBOL_function_type_spec = 105,       /* function_type_spec  */
  YYSYMBOL_106_7 = 106,                    /* $@7  */
  YYSYMBOL_function_decl = 107,            /* function_decl  */
  YYSYMBOL_108_8 = 108,                    /* $@8  */
  YYSYMBOL_action_def = 109,               /* action_def  */
  YYSYMBOL_110_9 = 110,                    /* $@9  */
  YYSYMBOL_parameters = 111,               /* parameters  */
  YYSYMBOL_action_body = 112,              /* action_body  */
  YYSYMBOL_action_body2 = 113,             /* action_body2  */
  YYSYMBOL_precondition = 114,             /* precondition  */
  YYSYMBOL_effect = 115,                   /* effect  */
  YYSYMBOL_eff_formula = 116,              /* eff_formula  */
  YYSYMBOL_117_10 = 117,                   /* $@10  */
  YYSYMBOL_118_11 = 118,                   /* $@11  */
  YYSYMBOL_eff_formulas = 119,             /* eff_formulas  */
  YYSYMBOL_prob_effs = 120,                /* prob_effs  */
  YYSYMBOL_probability = 121,              /* probability  */
  YYSYMBOL_p_effect = 122,                 /* p_effect  */
  YYSYMBOL_123_12 = 123,                   /* $@12  */
  YYSYMBOL_124_13 = 124,                   /* $@13  */
  YYSYMBOL_125_14 = 125,                   /* $@14  */
  YYSYMBOL_126_15 = 126,                   /* $@15  */
  YYSYMBOL_127_16 = 127,                   /* $@16  */
  YYSYMBOL_problem_def = 128,              /* problem_def  */
  YYSYMBOL_129_17 = 129,                   /* $@17  */
  YYSYMBOL_problem_body = 130,             /* problem_body  */
  YYSYMBOL_problem_body2 = 131,            /* problem_body2  */
  YYSYMBOL_problem_body3 = 132,            /* problem_body3  */
  YYSYMBOL_object_decl = 133,              /* object_decl  */
  YYSYMBOL_134_18 = 134,                   /* $@18  */
  YYSYMBOL_init = 135,                     /* init  */
  YYSYMBOL_init_elements = 136,            /* init_elements  */
  YYSYMBOL_init_element = 137,             /* init_element  */
  YYSYMBOL_prob_inits = 138,               /* prob_inits  */
  YYSYMBOL_simple_init = 139,              /* simple_init  */
  YYSYMBOL_one_inits = 140,                /* one_inits  */
  YYSYMBOL_one_init = 141,                 /* one_init  */
  YYSYMBOL_value = 142,                    /* value  */
  YYSYMBOL_goal_spec = 143,                /* goal_spec  */
  YYSYMBOL_goal_reward = 144,              /* goal_reward  */
  YYSYMBOL_metric_spec = 145,              /* metric_spec  */
  YYSYMBOL_146_19 = 146,                   /* $@19  */
  YYSYMBOL_147_20 = 147,                   /* $@20  */
  YYSYMBOL_formula = 148,                  /* formula  */
  YYSYMBOL_149_21 = 149,                   /* $@21  */
  YYSYMBOL_150_22 = 150,                   /* $@22  */
  YYSYMBOL_151_23 = 151,                   /* $@23  */
  YYSYMBOL_152_24 = 152,                   /* $@24  */
  YYSYMBOL_153_25 = 153,                   /* $@25  */
  YYSYMBOL_154_26 = 154,                   /* $@26  */
  YYSYMBOL_155_27 = 155,                   /* $@27  */
  YYSYMBOL_156_28 = 156,                   /* $@28  */
  YYSYMBOL_conjuncts = 157,                /* conjuncts  */
  YYSYMBOL_disjuncts = 158,                /* disjuncts  */
  YYSYMBOL_atomic_term_formula = 159,      /* atomic_term_formula  */
  YYSYMBOL_160_29 = 160,                   /* $@29  */
  YYSYMBOL_atomic_name_formula = 161,      /* atomic_name_formula  */
  YYSYMBOL_162_30 = 162,                   /* $@30  */
  YYSYMBOL_f_exp = 163,                    /* f_exp  */
  YYSYMBOL_term_or_f_exp = 164,            /* term_or_f_exp  */
  YYSYMBOL_165_31 = 165,                   /* $@31  */
  YYSYMBOL_166_32 = 166,                   /* $@32  */
  YYSYMBOL_167_33 = 167,                   /* $@33  */
  YYSYMBOL_168_34 = 168,                   /* $@34  */
  YYSYMBOL_169_35 = 169,                   /* $@35  */
  YYSYMBOL_opt_f_exp = 170,                /* opt_f_exp  */
  YYSYMBOL_f_head = 171,                   /* f_head  */
  YYSYMBOL_172_36 = 172,                   /* $@36  */
  YYSYMBOL_ground_f_exp = 173,             /* ground_f_exp  */
  YYSYMBOL_opt_ground_f_exp = 174,         /* opt_ground_f_exp  */
  YYSYMBOL_ground_f_head = 175,            /* ground_f_head  */
  YYSYMBOL_176_37 = 176,                   /* $@37  */
  YYSYMBOL_terms = 177,                    /* terms  */
  YYSYMBOL_names = 178,                    /* names  */
  YYSYMBOL_variables = 179,                /* variables  */
  YYSYMBOL_180_38 = 180,                   /* $@38  */
  YYSYMBOL_variable_seq = 181,             /* variable_seq  */
  YYSYMBOL_typed_names = 182,              /* typed_names  */
  YYSYMBOL_183_39 = 183,                   /* $@39  */
  YYSYMBOL_name_seq = 184,                 /* name_seq  */
  YYSYMBOL_type_spec = 185,                /* type_spec  */
  YYSYMBOL_type = 186,                     /* type  */
  YYSYMBOL_187_40 = 187,                   /* $@40  */
  YYSYMBOL_188_41 = 188,                   /* $@41  */
  YYSYMBOL_types = 189,                    /* types  */
  YYSYMBOL_function_type = 190,            /* function_type  */
  YYSYMBOL_define = 191,                   /* define  */
  YYSYMBOL_domain = 192,                   /* domain  */
  YYSYMBOL_problem = 193,                  /* problem  */
  YYSYMBOL_when = 194,                     /* when  */
  YYSYMBOL_not = 195,                      /* not  */
  YYSYMBOL_and = 196,                      /* and  */
  YYSYMBOL_or = 197,                       /* or  */
  YYSYMBOL_imply = 198,                    /* imply  */
  YYSYMBOL_exists = 199,                   /* exists  */
  YYSYMBOL_forall = 200,                   /* forall  */
  YYSYMBOL_probabilistic = 201,            /* probabilistic  */
  YYSYMBOL_assign = 202,                   /* assign  */
  YYSYMBOL_scale_up = 203,                 /* scale_up  */
  YYSYMBOL_scale_down = 204,               /* scale_down  */
  YYSYMBOL_increase = 205,                 /* increase  */
  YYSYMBOL_decrease = 206,                 /* decrease  */
  YYSYMBOL_minimize = 207,                 /* minimize  */
  YYSYMBOL_maximize = 208,                 /* maximize  */
  YYSYMBOL_number = 209,                   /* number  */
  YYSYMBOL_object = 210,                   /* object  */
  YYSYMBOL_either = 211,                   /* either  */
  YYSYMBOL_type_name = 212,                /* type_name  */
  YYSYMBOL_predicate = 213,                /* predicate  */
  YYSYMBOL_function = 214,                 /* function  */
  YYSYMBOL_name = 215,                     /* name  */
  YYSYMBOL_variable = 216                  /* variable  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1177

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  73
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  144
/* YYNRULES -- Number of rules.  */
#define YYNRULES  309
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  512

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   318


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      64,    65,    71,    70,     2,    66,     2,    72,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      68,    67,    69,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   299,   299,   299,   303,   304,   305,   312,   312,   316,
     317,   318,   319,   322,   323,   324,   327,   328,   329,   330,
     331,   332,   333,   336,   337,   338,   339,   340,   343,   344,
     345,   346,   347,   350,   351,   352,   353,   354,   357,   358,
     359,   362,   363,   364,   367,   368,   369,   372,   373,   376,
     379,   382,   383,   386,   387,   388,   390,   392,   393,   395,
     397,   399,   400,   401,   402,   407,   412,   417,   421,   426,
     433,   433,   437,   437,   441,   444,   444,   451,   452,   455,
     455,   459,   460,   461,   464,   465,   468,   468,   471,   471,
     479,   479,   483,   484,   487,   488,   491,   492,   495,   498,
     505,   506,   507,   507,   509,   509,   511,   515,   516,   519,
     524,   528,   531,   532,   533,   533,   535,   535,   537,   537,
     539,   539,   541,   541,   550,   549,   554,   555,   558,   559,
     562,   563,   566,   566,   570,   573,   574,   577,   578,   580,
     584,   589,   593,   594,   597,   598,   601,   602,   606,   609,
     610,   613,   614,   618,   619,   619,   621,   621,   628,   630,
     629,   632,   632,   634,   634,   636,   636,   638,   638,   640,
     641,   642,   642,   643,   644,   644,   646,   646,   650,   651,
     654,   655,   658,   658,   660,   663,   663,   665,   672,   673,
     674,   675,   676,   677,   680,   682,   682,   684,   684,   686,
     686,   688,   688,   690,   690,   692,   693,   696,   697,   700,
     700,   701,   704,   705,   707,   709,   711,   713,   714,   716,
     718,   720,   724,   725,   728,   728,   730,   737,   738,   739,
     742,   743,   746,   747,   748,   748,   752,   753,   756,   757,
     758,   758,   761,   762,   765,   768,   769,   769,   770,   770,
     773,   774,   775,   776,   779,   786,   789,   792,   795,   798,
     801,   804,   807,   810,   813,   816,   819,   822,   825,   828,
     831,   834,   837,   840,   843,   846,   849,   849,   849,   850,
     851,   851,   852,   855,   856,   856,   859,   862,   862,   862,
     863,   863,   863,   864,   864,   864,   864,   864,   864,   864,
     864,   865,   865,   865,   865,   865,   866,   866,   867,   870
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "DEFINE",
  "DOMAIN_TOKEN", "PROBLEM", "REQUIREMENTS", "TYPES", "CONSTANTS",
  "PREDICATES", "FUNCTIONS", "STRIPS", "TYPING", "NEGATIVE_PRECONDITIONS",
  "DISJUNCTIVE_PRECONDITIONS", "EQUALITY", "EXISTENTIAL_PRECONDITIONS",
  "UNIVERSAL_PRECONDITIONS", "QUANTIFIED_PRECONDITIONS",
  "CONDITIONAL_EFFECTS", "FLUENTS", "ADL", "DURATIVE_ACTIONS",
  "DURATION_INEQUALITIES", "CONTINUOUS_EFFECTS", "PROBABILISTIC_EFFECTS",
  "REWARDS", "MDP", "ACTION", "PARAMETERS", "PRECONDITION", "EFFECT",
  "PDOMAIN", "OBJECTS", "INIT", "GOAL", "GOAL_REWARD", "METRIC",
  "TOTAL_TIME", "GOAL_ACHIEVED", "WHEN", "NOT", "AND", "OR", "IMPLY",
  "EXISTS", "FORALL", "PROBABILISTIC", "ASSIGN", "SCALE_UP", "SCALE_DOWN",
  "INCREASE", "DECREASE", "MINIMIZE", "MAXIMIZE", "NUMBER_TOKEN",
  "OBJECT_TOKEN", "EITHER", "LE", "GE", "NAME", "VARIABLE", "NUMBER",
  "ILLEGAL_TOKEN", "'('", "')'", "'-'", "'='", "'<'", "'>'", "'+'", "'*'",
  "'/'", "$accept", "file", "$@1", "domains_and_problems", "domain_def",
  "$@2", "domain_body", "domain_body2", "domain_body3", "domain_body4",
  "domain_body5", "domain_body6", "domain_body7", "domain_body8",
  "domain_body9", "structure_defs", "structure_def", "require_def",
  "require_keys", "require_key", "types_def", "$@3", "constants_def",
  "$@4", "predicates_def", "functions_def", "$@5", "predicate_decls",
  "predicate_decl", "$@6", "function_decls", "function_decl_seq",
  "function_type_spec", "$@7", "function_decl", "$@8", "action_def", "$@9",
  "parameters", "action_body", "action_body2", "precondition", "effect",
  "eff_formula", "$@10", "$@11", "eff_formulas", "prob_effs",
  "probability", "p_effect", "$@12", "$@13", "$@14", "$@15", "$@16",
  "problem_def", "$@17", "problem_body", "problem_body2", "problem_body3",
  "object_decl", "$@18", "init", "init_elements", "init_element",
  "prob_inits", "simple_init", "one_inits", "one_init", "value",
  "goal_spec", "goal_reward", "metric_spec", "$@19", "$@20", "formula",
  "$@21", "$@22", "$@23", "$@24", "$@25", "$@26", "$@27", "$@28",
  "conjuncts", "disjuncts", "atomic_term_formula", "$@29",
  "atomic_name_formula", "$@30", "f_exp", "term_or_f_exp", "$@31", "$@32",
  "$@33", "$@34", "$@35", "opt_f_exp", "f_head", "$@36", "ground_f_exp",
  "opt_ground_f_exp", "ground_f_head", "$@37", "terms", "names",
  "variables", "$@38", "variable_seq", "typed_names", "$@39", "name_seq",
  "type_spec", "type", "$@40", "$@41", "types", "function_type", "define",
  "domain", "problem", "when", "not", "and", "or", "imply", "exists",
  "forall", "probabilistic", "assign", "scale_up", "scale_down",
  "increase", "decrease", "minimize", "maximize", "number", "object",
  "either", "type_name", "predicate", "function", "name", "variable", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-379)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-249)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -379,    29,  -379,  -379,   -10,    65,  -379,  -379,  -379,    -2,
     113,  -379,  -379,  1031,  1031,  -379,  -379,  -379,  -379,  -379,
    -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,
    -379,  -379,  -379,  -379,  -379,  -379,  -379,    11,    31,  -379,
      68,    74,   119,   208,    99,  -379,  -379,   116,  -379,   123,
     133,   143,   155,   166,  -379,  1031,   995,  -379,  -379,  -379,
    -379,  1031,  -379,   149,  -379,   120,  -379,   148,  -379,    22,
    -379,   116,   175,   176,    30,  -379,   116,   175,   177,     8,
    -379,   116,   176,   177,   125,  -379,  -379,  -379,  -379,  -379,
    -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,
    -379,  -379,  1112,  -379,  1031,  1031,   146,   178,  -379,    77,
    -379,   116,   116,    15,  -379,   116,   116,  -379,    27,  -379,
     116,   116,  -379,  -379,  -379,  -379,  -379,   147,   547,  -379,
     179,   286,  -379,  -379,  1031,   180,    59,  -379,   217,   116,
     116,   116,   185,  -379,   -11,  -379,  -379,  -379,  -379,  -379,
    -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,
    -379,  -379,  -379,   178,  -379,   186,   144,    64,   194,   195,
    -379,  -379,   199,   200,  -379,  -379,  -379,  -379,  1062,   201,
    -379,  1031,   198,   198,   211,  -379,   198,   941,   973,   203,
    -379,   239,  -379,  -379,  -379,   941,   172,   126,  -379,  -379,
     132,  -379,   110,  -379,  -379,   216,  -379,  -379,   209,    16,
    -379,   210,  -379,  -379,  -379,   212,   515,  -379,  -379,  -379,
    1057,  -379,  -379,  -379,  -379,  -379,  1031,   252,   215,  -379,
    -379,  -379,  -379,  -379,   402,  -379,  -379,  -379,  -379,  -379,
    -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,   821,  -379,
    -379,   941,  -379,  -379,   941,  -379,  -379,  -379,  -379,  -379,
    -379,  -379,  -379,  -379,  -379,  -379,   985,  -379,  -379,   214,
    -379,  -379,  -379,  -379,  -379,   218,    18,  -379,  -379,  -379,
    -379,   222,   791,   791,   228,  -379,  -379,   198,   853,   853,
    -379,   425,  -379,  -379,  -379,   853,   853,   229,   445,  -379,
     941,   223,   231,  -379,   941,   286,   233,   722,   232,  -379,
      41,   973,   883,   883,   883,   883,   883,  -379,   915,   214,
    -379,   192,  -379,  -379,  -379,  -379,  -379,   344,   234,  -379,
    -379,   245,  -379,  -379,  -379,  -379,  -379,   483,   853,  -379,
    -379,   853,  -379,  -379,  -379,  -379,  -379,   821,   853,   853,
    -379,  -379,  -379,   750,   246,   198,   198,   574,   973,  -379,
    -379,  -379,   198,  -379,   973,  -379,  1031,   853,   853,   853,
     853,   853,  1031,   242,   999,    49,  -379,   791,   248,   249,
     791,   791,   791,   791,  -379,  -379,  -379,   853,   853,   853,
     853,  -379,   250,   253,   853,   853,   853,   853,  -379,   254,
     255,   257,  -379,  -379,  -379,   258,   260,  -379,  -379,  -379,
     261,   262,  -379,   263,   264,   268,   270,   271,   272,   167,
    -379,  -379,  -379,  -379,   999,   664,   273,  -379,  -379,   791,
     791,   791,   791,  -379,   853,   853,   853,   853,  -379,  -379,
    -379,   853,   853,   853,   853,   605,  -379,  -379,  -379,   941,
     941,  -379,   973,  -379,  -379,  -379,  -379,  -379,  -379,   915,
    -379,  -379,  -379,  -379,   281,  -379,   290,   291,   292,   293,
     692,  -379,   295,   296,   297,   298,   633,   299,   300,   301,
     302,  -379,   303,   304,   309,   313,   763,   339,  -379,  -379,
    -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,
    -379,  -379,  -379,  -379,  -379,  -379,  -379,   337,   139,  -379,
    -379,  -379
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       2,     0,     4,     1,     3,     0,     5,     6,   255,     0,
       0,   256,   257,     0,     0,   287,   288,   289,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   290,   291,   292,   308,     0,     0,     7,
       0,     9,     0,     0,     0,    12,    15,    22,    47,    10,
      13,    16,    17,    18,    49,     0,     0,    70,    72,    77,
      75,     0,     8,     0,    48,     0,    11,     0,    14,     0,
      19,    27,    23,    24,     0,    20,    32,    28,    29,     0,
      21,    37,    33,    34,     0,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,     0,    51,   238,   238,     0,    81,    90,     0,
      25,    40,    38,     0,    26,    43,    41,    30,     0,    31,
      46,    44,    35,    36,   124,    50,    52,     0,   239,   242,
       0,     0,    74,    78,     0,     0,    82,    84,    92,    39,
      42,    45,   153,    71,   246,   240,   243,    73,   276,   277,
     278,   280,   281,   285,   284,   279,   282,   283,    79,    88,
     286,    76,    86,    81,    85,     0,    96,     0,   153,     0,
     127,   129,   153,   153,   131,   150,   274,   244,     0,     0,
     245,   238,   232,   232,     0,    83,   232,     0,     0,     0,
      95,    96,    97,   132,   135,     0,     0,     0,   126,   125,
       0,   128,     0,   130,   247,     0,   241,   309,     0,   233,
     236,     0,   273,    87,   254,     0,     0,    98,   158,   184,
       0,    99,   100,   112,    91,    94,   238,     0,     0,   271,
     272,   156,   154,   275,     0,    80,   234,   237,    89,    93,
     259,   260,   261,   262,   263,   264,   163,   165,     0,   161,
     167,     0,   178,   171,     0,   174,   176,   182,   258,   265,
     266,   267,   268,   269,   270,   104,     0,   107,   102,     0,
     114,   116,   118,   120,   122,     0,     0,   134,   136,   137,
     187,   153,     0,     0,     0,   250,   251,   232,     0,     0,
     194,     0,   159,   205,   206,     0,     0,     0,     0,   180,
       0,     0,     0,   227,     0,     0,     0,     0,     0,   111,
       0,     0,     0,     0,     0,     0,     0,   133,     0,     0,
     185,     0,   149,   151,   219,   221,   212,     0,     0,   217,
     226,     0,   249,   252,   253,   235,   188,     0,     0,   193,
     211,     0,   197,   195,   199,   201,   203,     0,     0,     0,
     169,   170,   179,     0,     0,   232,   232,     0,     0,   113,
     101,   108,   232,   106,     0,   109,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   230,     0,     0,     0,
       0,     0,     0,     0,   224,   157,   155,     0,     0,     0,
       0,   209,     0,     0,     0,     0,     0,     0,   227,     0,
       0,     0,   172,   181,   173,     0,     0,   183,   228,   229,
       0,     0,   110,     0,     0,     0,     0,     0,     0,     0,
     140,   142,   146,   139,     0,     0,     0,   218,   220,   222,
       0,     0,     0,   230,   207,     0,     0,     0,   227,   164,
     166,   207,     0,     0,     0,     0,   160,   162,   168,     0,
       0,   105,     0,   115,   117,   119,   121,   123,   138,     0,
     144,   141,   186,   231,   153,   223,     0,     0,     0,     0,
       0,   208,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   204,     0,     0,     0,     0,     0,     0,   152,   214,
     213,   215,   216,   225,   190,   189,   191,   192,   210,   198,
     196,   200,   202,   175,   177,   103,   148,     0,     0,   143,
     145,   147
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -379,  -379,  -379,  -379,  -379,  -379,  -379,   354,   358,  -379,
    -379,  -379,   332,   329,   330,   -26,   -32,   275,  -379,   319,
    -379,  -379,   -19,  -379,    51,    69,  -379,  -379,  -379,  -379,
     259,  -379,  -379,  -379,   287,  -379,  -379,  -379,  -379,  -379,
     240,  -379,  -379,  -297,  -379,  -379,  -379,  -379,  -291,  -379,
    -379,  -379,  -379,  -379,  -379,  -379,  -379,  -379,   256,   265,
    -379,  -379,  -379,  -379,  -379,  -379,     9,  -379,   -52,  -379,
     266,  -379,  -273,  -379,  -379,  -188,  -379,  -379,  -379,  -379,
    -379,  -379,  -379,  -379,  -379,  -379,  -185,  -379,   213,  -379,
     -17,    88,  -379,  -379,  -379,  -379,  -379,    -5,  -112,  -379,
    -246,  -379,  -307,  -379,  -378,     5,  -174,  -379,  -379,   -87,
    -379,  -379,   235,  -379,  -379,  -379,  -379,  -379,  -379,  -379,
    -379,  -379,   221,  -214,  -379,  -379,  -379,   225,   170,  -379,
    -379,  -379,  -379,  -379,  -379,  -379,  -379,  -204,  -379,  -165,
    -127,  -129,   -13,  -207
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,     2,     4,     6,    41,    44,    45,    46,    70,
      75,    80,   110,   114,   119,    47,    48,    49,   102,   103,
      50,   104,    51,   105,    52,    53,   107,   106,   133,   182,
     135,   136,   163,   184,   137,   183,    54,   138,   166,   189,
     190,   191,   192,   221,   308,   304,   307,   310,   311,   222,
     312,   313,   314,   315,   316,     7,   142,   169,   170,   171,
     172,   226,   173,   227,   278,   375,   420,   486,   421,   507,
     174,   322,   175,   283,   282,   217,   347,   295,   288,   289,
     296,   299,   301,   302,   298,   353,   218,   303,   422,   376,
     471,   292,   395,   394,   396,   397,   398,   472,   339,   438,
     328,   466,   329,   433,   357,   425,   208,   287,   209,   127,
     181,   128,   145,   177,   178,   179,   284,   213,     9,    13,
      14,   265,   251,   252,   253,   254,   255,   256,   269,   270,
     271,   272,   273,   274,   231,   232,   214,   180,   234,   157,
     219,   340,   160,   210
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      37,    38,   237,   223,   158,   159,   267,   228,   323,   211,
     361,   373,   215,   204,   365,    64,    58,    59,   130,   364,
     445,   148,   149,   150,    59,    71,    76,    81,   374,     3,
     285,    59,    60,    77,    82,    58,    61,   331,    58,    64,
      60,   294,    84,    61,    64,   176,   111,   115,   108,    64,
      61,   111,   120,  -248,     5,    61,   115,   120,    61,   121,
     476,   410,    10,   297,   121,   259,   300,   412,     8,   286,
      56,   151,   152,   153,   154,   155,    39,   207,   156,    64,
     333,   306,   144,    64,   424,   318,   139,    60,    64,   257,
     140,   129,   129,   257,   206,   141,    40,   193,   194,   195,
     280,   196,    72,   309,    83,    61,   363,    64,    64,    64,
     352,   309,   354,   335,   423,   146,   358,    11,    12,   334,
      73,    78,   223,   134,   116,   162,   223,    57,    58,    59,
      60,   426,    42,   116,   429,   430,   431,   432,    43,   275,
     294,   112,   148,   149,   150,   195,   112,   196,    61,   320,
     409,    55,   485,   330,   330,   484,    58,    59,    60,   193,
     194,   195,   346,   196,    62,   403,   194,   195,   129,   196,
     148,   149,   150,   223,   187,   188,    61,    61,   257,   223,
      63,   405,   406,   465,   467,   468,   469,    65,   411,   330,
     124,   488,   151,   152,   153,   154,   155,    67,   384,   156,
     367,   368,   369,   370,   371,   460,   459,    69,   391,   241,
     131,   132,   143,   129,    56,    57,    58,    59,    60,    74,
     151,   152,   153,   154,   155,   229,   230,   156,   377,   196,
      79,   148,   149,   150,   459,   293,    61,   391,   409,   109,
     113,   118,   134,   384,   147,   161,   165,   280,   330,   167,
     186,   330,   330,   330,   330,   148,   149,   150,   197,   207,
     199,   482,   483,   200,   202,   205,   212,   223,   224,   409,
     188,   338,   341,   233,   235,   238,   309,   239,   348,   349,
     281,   151,   152,   317,   176,   155,   321,   355,   156,   148,
     149,   150,   320,   332,   350,   356,   362,   280,   359,   385,
     330,   330,   330,   330,   418,   151,   152,   153,   154,   155,
     386,   404,   156,   427,   428,   439,   276,   277,   440,   446,
     447,   392,   448,   449,   393,   450,   451,   452,   453,   454,
     330,   400,   401,   455,   293,   456,   457,   458,   464,   151,
     152,   153,   154,   155,   408,   487,   156,    15,    16,    17,
     413,   414,   415,   416,   417,   489,   490,   491,   492,   280,
     494,   495,   496,   497,   499,   500,   501,   502,   503,   504,
     434,   435,   436,   437,   505,   506,   196,   441,   442,   443,
     444,   320,   378,   379,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,   511,    66,    36,   148,   149,   150,    68,   117,
     380,   122,   463,   123,   381,   382,   383,   168,   473,   474,
     475,   126,   185,   164,   198,   478,   479,   480,    15,    16,
      17,   225,   408,   461,   510,   399,   477,   201,   470,   203,
     279,   266,     0,     0,   236,   268,   319,     0,   148,   149,
     150,     0,     0,     0,     0,   151,   152,   463,   176,   155,
       0,     0,   156,   408,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,     0,     0,    36,    15,    16,    17,     0,
       0,   342,     0,     0,     0,   343,   344,   345,   151,   152,
     153,   154,   155,     0,     0,   156,     0,     0,     0,   216,
     351,     0,     0,     0,     0,     0,     0,     0,   148,   149,
     150,     0,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,     0,     0,    36,     0,     0,     0,     0,     0,   387,
      15,    16,    17,   388,   389,   390,   240,   241,   242,   243,
     244,   245,     0,     0,     0,     0,     0,     0,   151,   152,
     153,   154,   155,   246,   247,   156,     0,    15,    16,    17,
       0,     0,   248,   249,   250,     0,     0,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,     0,     0,    36,    15,    16,
      17,     0,     0,   144,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,     0,     0,    36,   207,    15,    16,    17,   407,
       0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,     0,     0,    36,   207,    15,    16,    17,
     481,     0,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,     0,     0,    36,   207,    15,    16,    17,   498,     0,
       0,     0,     0,     0,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,     0,     0,    36,   148,   149,   150,     0,   462,
       0,     0,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
       0,     0,    36,   148,   149,   150,     0,   493,     0,     0,
       0,     0,     0,     0,     0,     0,   148,   149,   150,     0,
       0,     0,     0,     0,     0,   151,   152,   153,   154,   155,
       0,     0,   156,     0,     0,     0,   220,   360,     0,     0,
       0,     0,     0,     0,    15,    16,    17,     0,     0,     0,
       0,     0,     0,   151,   152,   153,   154,   155,     0,     0,
     156,     0,     0,     0,   216,   402,   151,   152,   153,   154,
     155,     0,     0,   156,    15,    16,    17,   508,   509,   324,
     325,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,     0,
       0,    36,     0,   326,     0,   327,    15,    16,    17,     0,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,     0,
       0,    36,   207,   290,     0,   291,    15,    16,    17,     0,
       0,     0,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,     0,     0,    36,     0,   336,     0,   337,    15,    16,
      17,     0,     0,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,     0,     0,    36,   148,   149,   150,   366,     0,     0,
       0,     0,     0,     0,     0,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,     0,     0,    36,   148,   149,   150,   372,
       0,     0,     0,     0,     0,     0,     0,     0,   148,   149,
     150,     0,     0,     0,   151,   152,   153,   154,   155,     0,
       0,   156,   148,   149,   150,   216,    85,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,     0,     0,     0,   151,   152,   153,   154,
     155,     0,     0,   156,    15,    16,    17,   220,   151,   152,
     153,   154,   155,     0,     0,   156,     0,     0,     0,   305,
       0,     0,   151,   152,   153,   154,   155,     0,     0,   156,
     148,   149,   150,   419,     0,   148,   149,   150,     0,     0,
       0,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,     0,
       0,    36,     0,     0,     0,     0,     0,   258,   240,   241,
       0,     0,     0,   245,   259,   260,   261,   262,   263,   264,
     151,   152,   153,   154,   155,   151,   152,   156,     0,   155,
       0,     0,   156,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   125
};

static const yytype_int16 yycheck[] =
{
      13,    14,   209,   188,   131,   134,   220,   195,   281,   183,
     307,   318,   186,   178,   311,    47,     8,     9,   105,   310,
     398,     3,     4,     5,     9,    51,    52,    53,   319,     0,
     234,     9,    10,    52,    53,     8,    28,   283,     8,    71,
      10,   248,    55,    28,    76,    56,    72,    73,    61,    81,
      28,    77,    78,    64,    64,    28,    82,    83,    28,    78,
     438,   358,    64,   251,    83,    47,   254,   364,     3,   234,
       6,    53,    54,    55,    56,    57,    65,    61,    60,   111,
     284,   266,    66,   115,   375,    67,   112,    10,   120,   216,
     116,   104,   105,   220,   181,   121,    65,    33,    34,    35,
     227,    37,    51,    62,    53,    28,    65,   139,   140,   141,
     298,    62,   300,   287,    65,   128,   304,     4,     5,   284,
      51,    52,   307,    64,    73,    66,   311,     7,     8,     9,
      10,   377,    64,    82,   380,   381,   382,   383,    64,   226,
     347,    72,     3,     4,     5,    35,    77,    37,    28,   276,
     357,    32,   459,   282,   283,   452,     8,     9,    10,    33,
      34,    35,   291,    37,    65,   353,    34,    35,   181,    37,
       3,     4,     5,   358,    30,    31,    28,    28,   305,   364,
      64,   355,   356,   429,   430,   431,   432,    64,   362,   318,
      65,   464,    53,    54,    55,    56,    57,    64,   327,    60,
     312,   313,   314,   315,   316,   419,    67,    64,   337,    42,
      64,    65,    65,   226,     6,     7,     8,     9,    10,    64,
      53,    54,    55,    56,    57,    53,    54,    60,    36,    37,
      64,     3,     4,     5,    67,   248,    28,   366,   445,    64,
      64,    64,    64,   372,    65,    65,    29,   374,   377,    64,
      64,   380,   381,   382,   383,     3,     4,     5,    64,    61,
      65,   449,   450,    64,    64,    64,    55,   452,    65,   476,
      31,   288,   289,    57,    65,    65,    62,    65,   295,   296,
      65,    53,    54,    65,    56,    57,    64,    64,    60,     3,
       4,     5,   419,    65,    65,    64,    64,   424,    65,    65,
     429,   430,   431,   432,    62,    53,    54,    55,    56,    57,
      65,    65,    60,    65,    65,    65,    64,    65,    65,    65,
      65,   338,    65,    65,   341,    65,    65,    65,    65,    65,
     459,   348,   349,    65,   347,    65,    65,    65,    65,    53,
      54,    55,    56,    57,   357,    64,    60,     3,     4,     5,
     367,   368,   369,   370,   371,    65,    65,    65,    65,   486,
      65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
     387,   388,   389,   390,    65,    62,    37,   394,   395,   396,
     397,   508,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    65,    49,    60,     3,     4,     5,    50,    77,
      66,    82,   425,    83,    70,    71,    72,   142,   435,   436,
     437,   102,   163,   136,   168,   442,   443,   444,     3,     4,
       5,   191,   445,   424,   486,   347,   441,   172,   433,   173,
     227,   220,    -1,    -1,   209,   220,   276,    -1,     3,     4,
       5,    -1,    -1,    -1,    -1,    53,    54,   470,    56,    57,
      -1,    -1,    60,   476,    -1,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    -1,    -1,    60,     3,     4,     5,    -1,
      -1,    66,    -1,    -1,    -1,    70,    71,    72,    53,    54,
      55,    56,    57,    -1,    -1,    60,    -1,    -1,    -1,    64,
      65,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
       5,    -1,    -1,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    -1,    -1,    60,    -1,    -1,    -1,    -1,    -1,    66,
       3,     4,     5,    70,    71,    72,    41,    42,    43,    44,
      45,    46,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,
      55,    56,    57,    58,    59,    60,    -1,     3,     4,     5,
      -1,    -1,    67,    68,    69,    -1,    -1,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    -1,    -1,    60,     3,     4,
       5,    -1,    -1,    66,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    -1,    -1,    60,    61,     3,     4,     5,    65,
      -1,    -1,    -1,    -1,    -1,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    -1,    -1,    60,    61,     3,     4,     5,
      65,    -1,    -1,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    -1,    -1,    60,    61,     3,     4,     5,    65,    -1,
      -1,    -1,    -1,    -1,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    -1,    -1,    60,     3,     4,     5,    -1,    65,
      -1,    -1,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      -1,    -1,    60,     3,     4,     5,    -1,    65,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,    -1,
      -1,    -1,    -1,    -1,    -1,    53,    54,    55,    56,    57,
      -1,    -1,    60,    -1,    -1,    -1,    64,    65,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,    -1,    -1,    -1,
      -1,    -1,    -1,    53,    54,    55,    56,    57,    -1,    -1,
      60,    -1,    -1,    -1,    64,    65,    53,    54,    55,    56,
      57,    -1,    -1,    60,     3,     4,     5,    64,    65,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    -1,
      -1,    60,    -1,    62,    -1,    64,     3,     4,     5,    -1,
      -1,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    -1,
      -1,    60,    61,    62,    -1,    64,     3,     4,     5,    -1,
      -1,    -1,    -1,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    -1,    -1,    60,    -1,    62,    -1,    64,     3,     4,
       5,    -1,    -1,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    -1,    -1,    60,     3,     4,     5,    64,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    -1,    -1,    60,     3,     4,     5,    64,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
       5,    -1,    -1,    -1,    53,    54,    55,    56,    57,    -1,
      -1,    60,     3,     4,     5,    64,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    -1,    -1,    -1,    53,    54,    55,    56,
      57,    -1,    -1,    60,     3,     4,     5,    64,    53,    54,
      55,    56,    57,    -1,    -1,    60,    -1,    -1,    -1,    64,
      -1,    -1,    53,    54,    55,    56,    57,    -1,    -1,    60,
       3,     4,     5,    64,    -1,     3,     4,     5,    -1,    -1,
      -1,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    -1,
      -1,    60,    -1,    -1,    -1,    -1,    -1,    40,    41,    42,
      -1,    -1,    -1,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    53,    54,    60,    -1,    57,
      -1,    -1,    60,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    65
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    74,    75,     0,    76,    64,    77,   128,     3,   191,
      64,     4,     5,   192,   193,     3,     4,     5,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    60,   215,   215,    65,
      65,    78,    64,    64,    79,    80,    81,    88,    89,    90,
      93,    95,    97,    98,   109,    32,     6,     7,     8,     9,
      10,    28,    65,    64,    89,    64,    80,    64,    81,    64,
      82,    88,    97,    98,    64,    83,    88,    95,    98,    64,
      84,    88,    95,    97,   215,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    91,    92,    94,    96,   100,    99,   215,    64,
      85,    88,    98,    64,    86,    88,    97,    85,    64,    87,
      88,    95,    86,    87,    65,    65,    92,   182,   184,   215,
     182,    64,    65,   101,    64,   103,   104,   107,   110,    88,
      88,    88,   129,    65,    66,   185,   215,    65,     3,     4,
       5,    53,    54,    55,    56,    57,    60,   212,   213,   214,
     215,    65,    66,   105,   107,    29,   111,    64,    90,   130,
     131,   132,   133,   135,   143,   145,    56,   186,   187,   188,
     210,   183,   102,   108,   106,   103,    64,    30,    31,   112,
     113,   114,   115,    33,    34,    35,    37,    64,   131,    65,
      64,   132,    64,   143,   212,    64,   182,    61,   179,   181,
     216,   179,    55,   190,   209,   179,    64,   148,   159,   213,
      64,   116,   122,   159,    65,   113,   134,   136,   148,    53,
      54,   207,   208,    57,   211,    65,   185,   216,    65,    65,
      41,    42,    43,    44,    45,    46,    58,    59,    67,    68,
      69,   195,   196,   197,   198,   199,   200,   213,    40,    47,
      48,    49,    50,    51,    52,   194,   195,   196,   200,   201,
     202,   203,   204,   205,   206,   182,    64,    65,   137,   161,
     213,    65,   147,   146,   189,   210,   212,   180,   151,   152,
      62,    64,   164,   215,   216,   150,   153,   148,   157,   154,
     148,   155,   156,   160,   118,    64,   159,   119,   117,    62,
     120,   121,   123,   124,   125,   126,   127,    65,    67,   201,
     213,    64,   144,   145,    38,    39,    62,    64,   173,   175,
     214,   173,    65,   210,   212,   179,    62,    64,   163,   171,
     214,   163,    66,    70,    71,    72,   214,   149,   163,   163,
      65,    65,   148,   158,   148,    64,    64,   177,   148,    65,
      65,   116,    64,    65,   121,   116,    64,   171,   171,   171,
     171,   171,    64,   175,   121,   138,   162,    36,    38,    39,
      66,    70,    71,    72,   214,    65,    65,    66,    70,    71,
      72,   214,   163,   163,   166,   165,   167,   168,   169,   164,
     163,   163,    65,   148,    65,   179,   179,    65,   215,   216,
     116,   179,   116,   163,   163,   163,   163,   163,    62,    64,
     139,   141,   161,    65,   121,   178,   173,    65,    65,   173,
     173,   173,   173,   176,   163,   163,   163,   163,   172,    65,
      65,   163,   163,   163,   163,   177,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    65,    67,
     196,   139,    65,   215,    65,   173,   174,   173,   173,   173,
     178,   163,   170,   163,   163,   163,   177,   170,   163,   163,
     163,    65,   148,   148,   116,   175,   140,    64,   145,    65,
      65,    65,    65,    65,    65,    65,    65,    65,    65,    65,
      65,    65,    65,    65,    65,    65,    62,   142,    64,    65,
     141,    65
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    73,    75,    74,    76,    76,    76,    78,    77,    79,
      79,    79,    79,    80,    80,    80,    81,    81,    81,    81,
      81,    81,    81,    82,    82,    82,    82,    82,    83,    83,
      83,    83,    83,    84,    84,    84,    84,    84,    85,    85,
      85,    86,    86,    86,    87,    87,    87,    88,    88,    89,
      90,    91,    91,    92,    92,    92,    92,    92,    92,    92,
      92,    92,    92,    92,    92,    92,    92,    92,    92,    92,
      94,    93,    96,    95,    97,    99,    98,   100,   100,   102,
     101,   103,   103,   103,   104,   104,   106,   105,   108,   107,
     110,   109,   111,   111,   112,   112,   113,   113,   114,   115,
     116,   116,   117,   116,   118,   116,   116,   119,   119,   120,
     120,   121,   122,   122,   123,   122,   124,   122,   125,   122,
     126,   122,   127,   122,   129,   128,   130,   130,   131,   131,
     132,   132,   134,   133,   135,   136,   136,   137,   137,   137,
     138,   138,   139,   139,   140,   140,   141,   141,   142,   143,
     143,   144,   144,   145,   146,   145,   147,   145,   148,   149,
     148,   150,   148,   151,   148,   152,   148,   153,   148,   148,
     148,   154,   148,   148,   155,   148,   156,   148,   157,   157,
     158,   158,   160,   159,   159,   162,   161,   161,   163,   163,
     163,   163,   163,   163,   164,   165,   164,   166,   164,   167,
     164,   168,   164,   169,   164,   164,   164,   170,   170,   172,
     171,   171,   173,   173,   173,   173,   173,   173,   173,   173,
     173,   173,   174,   174,   176,   175,   175,   177,   177,   177,
     178,   178,   179,   179,   180,   179,   181,   181,   182,   182,
     183,   182,   184,   184,   185,   186,   187,   186,   188,   186,
     189,   189,   189,   189,   190,   191,   192,   193,   194,   195,
     196,   197,   198,   199,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   210,   211,   212,   212,   212,   212,
     212,   212,   212,   213,   213,   213,   214,   215,   215,   215,
     215,   215,   215,   215,   215,   215,   215,   215,   215,   215,
     215,   215,   215,   215,   215,   215,   215,   215,   215,   216
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     2,     0,     2,     2,     0,     9,     0,
       1,     2,     1,     1,     2,     1,     1,     1,     1,     2,
       2,     2,     1,     1,     1,     2,     2,     1,     1,     1,
       2,     2,     1,     1,     1,     2,     2,     1,     1,     2,
       1,     1,     2,     1,     1,     2,     1,     1,     2,     1,
       4,     1,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     5,     0,     5,     4,     0,     5,     0,     2,     0,
       5,     0,     1,     3,     1,     2,     0,     3,     0,     5,
       0,     7,     0,     4,     2,     1,     0,     1,     2,     2,
       1,     4,     0,     8,     0,     6,     4,     0,     2,     2,
       3,     1,     1,     4,     0,     6,     0,     6,     0,     6,
       0,     6,     0,     6,     0,    13,     2,     1,     2,     1,
       2,     1,     0,     5,     4,     0,     2,     1,     5,     4,
       2,     3,     1,     4,     0,     2,     1,     5,     1,     5,
       1,     1,     5,     0,     0,     6,     0,     6,     1,     0,
       6,     0,     6,     0,     6,     0,     6,     0,     6,     4,
       4,     0,     5,     5,     0,     8,     0,     8,     0,     2,
       0,     2,     0,     5,     1,     0,     5,     1,     1,     5,
       5,     5,     5,     1,     1,     0,     6,     0,     6,     0,
       6,     0,     6,     0,     5,     1,     1,     0,     1,     0,
       5,     1,     1,     5,     5,     5,     5,     1,     3,     1,
       3,     1,     0,     1,     0,     5,     1,     0,     2,     2,
       0,     2,     0,     1,     0,     4,     1,     2,     0,     1,
       0,     4,     1,     2,     2,     1,     0,     2,     0,     5,
       1,     1,     2,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* $@1: %empty  */
#line 299 "parser.yy"
       { success = true; line_number = 1; }
#line 2225 "parser.cc"
    break;

  case 3: /* file: $@1 domains_and_problems  */
#line 300 "parser.yy"
         { if (!success) YYERROR; }
#line 2231 "parser.cc"
    break;

  case 7: /* $@2: %empty  */
#line 312 "parser.yy"
                                            { make_domain((yyvsp[-1].str)); }
#line 2237 "parser.cc"
    break;

  case 53: /* require_key: STRIPS  */
#line 386 "parser.yy"
                     { requirements->strips = true; }
#line 2243 "parser.cc"
    break;

  case 54: /* require_key: TYPING  */
#line 387 "parser.yy"
                     { requirements->typing = true; }
#line 2249 "parser.cc"
    break;

  case 55: /* require_key: NEGATIVE_PRECONDITIONS  */
#line 389 "parser.yy"
                { requirements->negative_preconditions = true; }
#line 2255 "parser.cc"
    break;

  case 56: /* require_key: DISJUNCTIVE_PRECONDITIONS  */
#line 391 "parser.yy"
                { requirements->disjunctive_preconditions = true; }
#line 2261 "parser.cc"
    break;

  case 57: /* require_key: EQUALITY  */
#line 392 "parser.yy"
                       { requirements->equality = true; }
#line 2267 "parser.cc"
    break;

  case 58: /* require_key: EXISTENTIAL_PRECONDITIONS  */
#line 394 "parser.yy"
                { requirements->existential_preconditions = true; }
#line 2273 "parser.cc"
    break;

  case 59: /* require_key: UNIVERSAL_PRECONDITIONS  */
#line 396 "parser.yy"
                { requirements->universal_preconditions = true; }
#line 2279 "parser.cc"
    break;

  case 60: /* require_key: QUANTIFIED_PRECONDITIONS  */
#line 398 "parser.yy"
                { requirements->quantified_preconditions(); }
#line 2285 "parser.cc"
    break;

  case 61: /* require_key: CONDITIONAL_EFFECTS  */
#line 399 "parser.yy"
                                  { requirements->conditional_effects = true; }
#line 2291 "parser.cc"
    break;

  case 62: /* require_key: FLUENTS  */
#line 400 "parser.yy"
                      { requirements->fluents = true; }
#line 2297 "parser.cc"
    break;

  case 63: /* require_key: ADL  */
#line 401 "parser.yy"
                  { requirements->adl(); }
#line 2303 "parser.cc"
    break;

  case 64: /* require_key: DURATIVE_ACTIONS  */
#line 403 "parser.yy"
                {
                  throw std::runtime_error("`:durative-actions'"
                                           " not supported");
                }
#line 2312 "parser.cc"
    break;

  case 65: /* require_key: DURATION_INEQUALITIES  */
#line 408 "parser.yy"
                {
                  throw std::runtime_error("`:duration-inequalities'"
                                           " not supported");
                }
#line 2321 "parser.cc"
    break;

  case 66: /* require_key: CONTINUOUS_EFFECTS  */
#line 413 "parser.yy"
                {
                  throw std::runtime_error("`:continuous-effects'"
                                           " not supported");
                }
#line 2330 "parser.cc"
    break;

  case 67: /* require_key: PROBABILISTIC_EFFECTS  */
#line 418 "parser.yy"
                {
                  requirements->probabilistic_effects = true;
                }
#line 2338 "parser.cc"
    break;

  case 68: /* require_key: REWARDS  */
#line 422 "parser.yy"
                {
                  requirements->rewards = true;
                  reward_function = domain->functions().add_function("reward");
                }
#line 2347 "parser.cc"
    break;

  case 69: /* require_key: MDP  */
#line 427 "parser.yy"
                {
                  requirements->mdp();
                  reward_function = domain->functions().add_function("reward");
                }
#line 2356 "parser.cc"
    break;

  case 70: /* $@3: %empty  */
#line 433 "parser.yy"
                      { require_typing(); name_kind = TYPE_KIND; }
#line 2362 "parser.cc"
    break;

  case 71: /* types_def: '(' TYPES $@3 typed_names ')'  */
#line 434 "parser.yy"
                              { name_kind = VOID_KIND; }
#line 2368 "parser.cc"
    break;

  case 72: /* $@4: %empty  */
#line 437 "parser.yy"
                              { name_kind = CONSTANT_KIND; }
#line 2374 "parser.cc"
    break;

  case 73: /* constants_def: '(' CONSTANTS $@4 typed_names ')'  */
#line 438 "parser.yy"
                  { name_kind = VOID_KIND; }
#line 2380 "parser.cc"
    break;

  case 75: /* $@5: %empty  */
#line 444 "parser.yy"
                              { require_fluents(); }
#line 2386 "parser.cc"
    break;

  case 79: /* $@6: %empty  */
#line 455 "parser.yy"
                               { make_predicate((yyvsp[0].str)); }
#line 2392 "parser.cc"
    break;

  case 80: /* predicate_decl: '(' predicate $@6 variables ')'  */
#line 456 "parser.yy"
                   { predicate = 0; }
#line 2398 "parser.cc"
    break;

  case 86: /* $@7: %empty  */
#line 468 "parser.yy"
                         { require_typing(); }
#line 2404 "parser.cc"
    break;

  case 88: /* $@8: %empty  */
#line 471 "parser.yy"
                             { make_function((yyvsp[0].str)); }
#line 2410 "parser.cc"
    break;

  case 89: /* function_decl: '(' function $@8 variables ')'  */
#line 472 "parser.yy"
                  { function = 0; }
#line 2416 "parser.cc"
    break;

  case 90: /* $@9: %empty  */
#line 479 "parser.yy"
                             { make_action((yyvsp[0].str)); }
#line 2422 "parser.cc"
    break;

  case 91: /* action_def: '(' ACTION name $@9 parameters action_body ')'  */
#line 480 "parser.yy"
                                          { add_action(); }
#line 2428 "parser.cc"
    break;

  case 98: /* precondition: PRECONDITION formula  */
#line 495 "parser.yy"
                                    { action->set_precondition(*(yyvsp[0].formula)); }
#line 2434 "parser.cc"
    break;

  case 99: /* effect: EFFECT eff_formula  */
#line 498 "parser.yy"
                            { action->set_effect(*(yyvsp[0].effect)); }
#line 2440 "parser.cc"
    break;

  case 101: /* eff_formula: '(' and eff_formulas ')'  */
#line 506 "parser.yy"
                                       { (yyval.effect) = (yyvsp[-1].effect); }
#line 2446 "parser.cc"
    break;

  case 102: /* $@10: %empty  */
#line 507 "parser.yy"
                         { prepare_forall_effect(); }
#line 2452 "parser.cc"
    break;

  case 103: /* eff_formula: '(' forall $@10 '(' variables ')' eff_formula ')'  */
#line 508 "parser.yy"
                                { (yyval.effect) = make_forall_effect(*(yyvsp[-1].effect)); }
#line 2458 "parser.cc"
    break;

  case 104: /* $@11: %empty  */
#line 509 "parser.yy"
                       { require_conditional_effects(); }
#line 2464 "parser.cc"
    break;

  case 105: /* eff_formula: '(' when $@11 formula eff_formula ')'  */
#line 510 "parser.yy"
                                { (yyval.effect) = &ConditionalEffect::make(*(yyvsp[-2].formula), *(yyvsp[-1].effect)); }
#line 2470 "parser.cc"
    break;

  case 106: /* eff_formula: '(' probabilistic prob_effs ')'  */
#line 512 "parser.yy"
                { (yyval.effect) = make_prob_effect((yyvsp[-1].outcomes)); }
#line 2476 "parser.cc"
    break;

  case 107: /* eff_formulas: %empty  */
#line 515 "parser.yy"
                           { (yyval.effect) = &Effect::EMPTY; }
#line 2482 "parser.cc"
    break;

  case 108: /* eff_formulas: eff_formulas eff_formula  */
#line 516 "parser.yy"
                                        { (yyval.effect) = &(*(yyvsp[-1].effect) && *(yyvsp[0].effect)); }
#line 2488 "parser.cc"
    break;

  case 109: /* prob_effs: probability eff_formula  */
#line 520 "parser.yy"
              {
                (yyval.outcomes) = new std::vector<std::pair<Rational, const Effect*> >();
                add_outcome(*(yyval.outcomes), (yyvsp[-1].num), *(yyvsp[0].effect));
              }
#line 2497 "parser.cc"
    break;

  case 110: /* prob_effs: prob_effs probability eff_formula  */
#line 525 "parser.yy"
              { (yyval.outcomes) = (yyvsp[-2].outcomes); add_outcome(*(yyval.outcomes), (yyvsp[-1].num), *(yyvsp[0].effect)); }
#line 2503 "parser.cc"
    break;

  case 112: /* p_effect: atomic_term_formula  */
#line 531 "parser.yy"
                               { (yyval.effect) = make_add_effect(*(yyvsp[0].atom)); }
#line 2509 "parser.cc"
    break;

  case 113: /* p_effect: '(' not atomic_term_formula ')'  */
#line 532 "parser.yy"
                                           { (yyval.effect) = make_delete_effect(*(yyvsp[-1].atom)); }
#line 2515 "parser.cc"
    break;

  case 114: /* $@12: %empty  */
#line 533 "parser.yy"
                      { effect_fluent = true; }
#line 2521 "parser.cc"
    break;

  case 115: /* p_effect: '(' assign $@12 f_head f_exp ')'  */
#line 534 "parser.yy"
             { (yyval.effect) = make_assign_effect(*(yyvsp[-2].fluent), *(yyvsp[-1].expr)); }
#line 2527 "parser.cc"
    break;

  case 116: /* $@13: %empty  */
#line 535 "parser.yy"
                        { effect_fluent = true; }
#line 2533 "parser.cc"
    break;

  case 117: /* p_effect: '(' scale_up $@13 f_head f_exp ')'  */
#line 536 "parser.yy"
             { (yyval.effect) = make_scale_up_effect(*(yyvsp[-2].fluent), *(yyvsp[-1].expr)); }
#line 2539 "parser.cc"
    break;

  case 118: /* $@14: %empty  */
#line 537 "parser.yy"
                          { effect_fluent = true; }
#line 2545 "parser.cc"
    break;

  case 119: /* p_effect: '(' scale_down $@14 f_head f_exp ')'  */
#line 538 "parser.yy"
             { (yyval.effect) = make_scale_down_effect(*(yyvsp[-2].fluent), *(yyvsp[-1].expr)); }
#line 2551 "parser.cc"
    break;

  case 120: /* $@15: %empty  */
#line 539 "parser.yy"
                        { effect_fluent = true; }
#line 2557 "parser.cc"
    break;

  case 121: /* p_effect: '(' increase $@15 f_head f_exp ')'  */
#line 540 "parser.yy"
             { (yyval.effect) = make_increase_effect(*(yyvsp[-2].fluent), *(yyvsp[-1].expr)); }
#line 2563 "parser.cc"
    break;

  case 122: /* $@16: %empty  */
#line 541 "parser.yy"
                        { effect_fluent = true; }
#line 2569 "parser.cc"
    break;

  case 123: /* p_effect: '(' decrease $@16 f_head f_exp ')'  */
#line 542 "parser.yy"
             { (yyval.effect) = make_decrease_effect(*(yyvsp[-2].fluent), *(yyvsp[-1].expr)); }
#line 2575 "parser.cc"
    break;

  case 124: /* $@17: %empty  */
#line 550 "parser.yy"
                { make_problem((yyvsp[-5].str), (yyvsp[-1].str)); }
#line 2581 "parser.cc"
    break;

  case 125: /* problem_def: '(' define '(' problem name ')' '(' PDOMAIN name ')' $@17 problem_body ')'  */
#line 551 "parser.yy"
                { problem->instantiate(); delete requirements; }
#line 2587 "parser.cc"
    break;

  case 132: /* $@18: %empty  */
#line 566 "parser.yy"
                          { name_kind = OBJECT_KIND; }
#line 2593 "parser.cc"
    break;

  case 133: /* object_decl: '(' OBJECTS $@18 typed_names ')'  */
#line 567 "parser.yy"
                { name_kind = VOID_KIND; }
#line 2599 "parser.cc"
    break;

  case 137: /* init_element: atomic_name_formula  */
#line 577 "parser.yy"
                                   { problem->add_init_atom(*(yyvsp[0].atom)); }
#line 2605 "parser.cc"
    break;

  case 138: /* init_element: '(' '=' ground_f_head NUMBER ')'  */
#line 579 "parser.yy"
                 { problem->add_init_value(*(yyvsp[-2].fluent), *(yyvsp[-1].num)); delete (yyvsp[-1].num); }
#line 2611 "parser.cc"
    break;

  case 139: /* init_element: '(' probabilistic prob_inits ')'  */
#line 581 "parser.yy"
                 { problem->add_init_effect(*make_prob_effect((yyvsp[-1].outcomes))); }
#line 2617 "parser.cc"
    break;

  case 140: /* prob_inits: probability simple_init  */
#line 585 "parser.yy"
               {
                 (yyval.outcomes) = new std::vector<std::pair<Rational, const Effect*> >();
                 add_outcome(*(yyval.outcomes), (yyvsp[-1].num), *(yyvsp[0].effect));
               }
#line 2626 "parser.cc"
    break;

  case 141: /* prob_inits: prob_inits probability simple_init  */
#line 590 "parser.yy"
               { (yyval.outcomes) = (yyvsp[-2].outcomes); add_outcome(*(yyval.outcomes), (yyvsp[-1].num), *(yyvsp[0].effect)); }
#line 2632 "parser.cc"
    break;

  case 143: /* simple_init: '(' and one_inits ')'  */
#line 594 "parser.yy"
                                    { (yyval.effect) = (yyvsp[-1].effect); }
#line 2638 "parser.cc"
    break;

  case 144: /* one_inits: %empty  */
#line 597 "parser.yy"
                        { (yyval.effect) = &Effect::EMPTY; }
#line 2644 "parser.cc"
    break;

  case 145: /* one_inits: one_inits one_init  */
#line 598 "parser.yy"
                               { (yyval.effect) = &(*(yyvsp[-1].effect) && *(yyvsp[0].effect)); }
#line 2650 "parser.cc"
    break;

  case 146: /* one_init: atomic_name_formula  */
#line 601 "parser.yy"
                               { (yyval.effect) = make_add_effect(*(yyvsp[0].atom)); }
#line 2656 "parser.cc"
    break;

  case 147: /* one_init: '(' '=' ground_f_head value ')'  */
#line 603 "parser.yy"
             { (yyval.effect) = make_assign_effect(*(yyvsp[-2].fluent), *(yyvsp[-1].expr)); }
#line 2662 "parser.cc"
    break;

  case 148: /* value: NUMBER  */
#line 606 "parser.yy"
               { (yyval.expr) = new Value(*(yyvsp[0].num)); delete (yyvsp[0].num); }
#line 2668 "parser.cc"
    break;

  case 149: /* goal_spec: '(' GOAL formula ')' goal_reward  */
#line 609 "parser.yy"
                                             { problem->set_goal(*(yyvsp[-2].formula)); }
#line 2674 "parser.cc"
    break;

  case 152: /* goal_reward: '(' GOAL_REWARD ground_f_exp ')' metric_spec  */
#line 615 "parser.yy"
                { set_goal_reward(*(yyvsp[-2].expr)); }
#line 2680 "parser.cc"
    break;

  case 153: /* metric_spec: %empty  */
#line 618 "parser.yy"
                          { set_default_metric(); }
#line 2686 "parser.cc"
    break;

  case 154: /* $@19: %empty  */
#line 619 "parser.yy"
                                  { metric_fluent = true; }
#line 2692 "parser.cc"
    break;

  case 155: /* metric_spec: '(' METRIC maximize $@19 ground_f_exp ')'  */
#line 620 "parser.yy"
                { problem->set_metric(*(yyvsp[-1].expr)); metric_fluent = false; }
#line 2698 "parser.cc"
    break;

  case 156: /* $@20: %empty  */
#line 621 "parser.yy"
                                  { metric_fluent = true; }
#line 2704 "parser.cc"
    break;

  case 157: /* metric_spec: '(' METRIC minimize $@20 ground_f_exp ')'  */
#line 622 "parser.yy"
                { problem->set_metric(*(yyvsp[-1].expr), true); metric_fluent = false; }
#line 2710 "parser.cc"
    break;

  case 158: /* formula: atomic_term_formula  */
#line 628 "parser.yy"
                              { (yyval.formula) = (yyvsp[0].atom); }
#line 2716 "parser.cc"
    break;

  case 159: /* $@21: %empty  */
#line 630 "parser.yy"
            { first_eq_term = eq_term; first_eq_expr = eq_expr; }
#line 2722 "parser.cc"
    break;

  case 160: /* formula: '(' '=' term_or_f_exp $@21 term_or_f_exp ')'  */
#line 631 "parser.yy"
                              { (yyval.formula) = make_equality(); }
#line 2728 "parser.cc"
    break;

  case 161: /* $@22: %empty  */
#line 632 "parser.yy"
                  { require_fluents(); }
#line 2734 "parser.cc"
    break;

  case 162: /* formula: '(' '<' $@22 f_exp f_exp ')'  */
#line 633 "parser.yy"
            { (yyval.formula) = &LessThan::make(*(yyvsp[-2].expr), *(yyvsp[-1].expr)); }
#line 2740 "parser.cc"
    break;

  case 163: /* $@23: %empty  */
#line 634 "parser.yy"
                 { require_fluents(); }
#line 2746 "parser.cc"
    break;

  case 164: /* formula: '(' LE $@23 f_exp f_exp ')'  */
#line 635 "parser.yy"
            { (yyval.formula) = &LessThanOrEqualTo::make(*(yyvsp[-2].expr), *(yyvsp[-1].expr)); }
#line 2752 "parser.cc"
    break;

  case 165: /* $@24: %empty  */
#line 636 "parser.yy"
                 { require_fluents(); }
#line 2758 "parser.cc"
    break;

  case 166: /* formula: '(' GE $@24 f_exp f_exp ')'  */
#line 637 "parser.yy"
            { (yyval.formula) = &GreaterThanOrEqualTo::make(*(yyvsp[-2].expr), *(yyvsp[-1].expr)); }
#line 2764 "parser.cc"
    break;

  case 167: /* $@25: %empty  */
#line 638 "parser.yy"
                  { require_fluents(); }
#line 2770 "parser.cc"
    break;

  case 168: /* formula: '(' '>' $@25 f_exp f_exp ')'  */
#line 639 "parser.yy"
            { (yyval.formula) = &GreaterThan::make(*(yyvsp[-2].expr), *(yyvsp[-1].expr)); }
#line 2776 "parser.cc"
    break;

  case 169: /* formula: '(' not formula ')'  */
#line 640 "parser.yy"
                              { (yyval.formula) = make_negation(*(yyvsp[-1].formula)); }
#line 2782 "parser.cc"
    break;

  case 170: /* formula: '(' and conjuncts ')'  */
#line 641 "parser.yy"
                                { (yyval.formula) = (yyvsp[-1].formula); }
#line 2788 "parser.cc"
    break;

  case 171: /* $@26: %empty  */
#line 642 "parser.yy"
                 { require_disjunction(); }
#line 2794 "parser.cc"
    break;

  case 172: /* formula: '(' or $@26 disjuncts ')'  */
#line 642 "parser.yy"
                                                          { (yyval.formula) = (yyvsp[-1].formula); }
#line 2800 "parser.cc"
    break;

  case 173: /* formula: '(' imply formula formula ')'  */
#line 643 "parser.yy"
                                        { (yyval.formula) = make_implication(*(yyvsp[-2].formula), *(yyvsp[-1].formula)); }
#line 2806 "parser.cc"
    break;

  case 174: /* $@27: %empty  */
#line 644 "parser.yy"
                     { prepare_exists(); }
#line 2812 "parser.cc"
    break;

  case 175: /* formula: '(' exists $@27 '(' variables ')' formula ')'  */
#line 645 "parser.yy"
            { (yyval.formula) = make_exists(*(yyvsp[-1].formula)); }
#line 2818 "parser.cc"
    break;

  case 176: /* $@28: %empty  */
#line 646 "parser.yy"
                     { prepare_forall(); }
#line 2824 "parser.cc"
    break;

  case 177: /* formula: '(' forall $@28 '(' variables ')' formula ')'  */
#line 647 "parser.yy"
            { (yyval.formula) = make_forall(*(yyvsp[-1].formula)); }
#line 2830 "parser.cc"
    break;

  case 178: /* conjuncts: %empty  */
#line 650 "parser.yy"
                        { (yyval.formula) = &StateFormula::TRUE; }
#line 2836 "parser.cc"
    break;

  case 179: /* conjuncts: conjuncts formula  */
#line 651 "parser.yy"
                              { (yyval.formula) = &(*(yyvsp[-1].formula) && *(yyvsp[0].formula)); }
#line 2842 "parser.cc"
    break;

  case 180: /* disjuncts: %empty  */
#line 654 "parser.yy"
                        { (yyval.formula) = &StateFormula::FALSE; }
#line 2848 "parser.cc"
    break;

  case 181: /* disjuncts: disjuncts formula  */
#line 655 "parser.yy"
                              { (yyval.formula) = &(*(yyvsp[-1].formula) || *(yyvsp[0].formula)); }
#line 2854 "parser.cc"
    break;

  case 182: /* $@29: %empty  */
#line 658 "parser.yy"
                                    { prepare_atom((yyvsp[0].str)); }
#line 2860 "parser.cc"
    break;

  case 183: /* atomic_term_formula: '(' predicate $@29 terms ')'  */
#line 659 "parser.yy"
                        { (yyval.atom) = make_atom(); }
#line 2866 "parser.cc"
    break;

  case 184: /* atomic_term_formula: predicate  */
#line 660 "parser.yy"
                                { prepare_atom((yyvsp[0].str)); (yyval.atom) = make_atom(); }
#line 2872 "parser.cc"
    break;

  case 185: /* $@30: %empty  */
#line 663 "parser.yy"
                                    { prepare_atom((yyvsp[0].str)); }
#line 2878 "parser.cc"
    break;

  case 186: /* atomic_name_formula: '(' predicate $@30 names ')'  */
#line 664 "parser.yy"
                        { (yyval.atom) = make_atom(); }
#line 2884 "parser.cc"
    break;

  case 187: /* atomic_name_formula: predicate  */
#line 665 "parser.yy"
                                { prepare_atom((yyvsp[0].str)); (yyval.atom) = make_atom(); }
#line 2890 "parser.cc"
    break;

  case 188: /* f_exp: NUMBER  */
#line 672 "parser.yy"
               { (yyval.expr) = new Value(*(yyvsp[0].num)); delete (yyvsp[0].num); }
#line 2896 "parser.cc"
    break;

  case 189: /* f_exp: '(' '+' f_exp f_exp ')'  */
#line 673 "parser.yy"
                                { (yyval.expr) = &Addition::make(*(yyvsp[-2].expr), *(yyvsp[-1].expr)); }
#line 2902 "parser.cc"
    break;

  case 190: /* f_exp: '(' '-' f_exp opt_f_exp ')'  */
#line 674 "parser.yy"
                                    { (yyval.expr) = make_subtraction(*(yyvsp[-2].expr), (yyvsp[-1].expr)); }
#line 2908 "parser.cc"
    break;

  case 191: /* f_exp: '(' '*' f_exp f_exp ')'  */
#line 675 "parser.yy"
                                { (yyval.expr) = &Multiplication::make(*(yyvsp[-2].expr), *(yyvsp[-1].expr)); }
#line 2914 "parser.cc"
    break;

  case 192: /* f_exp: '(' '/' f_exp f_exp ')'  */
#line 676 "parser.yy"
                                { (yyval.expr) = &Division::make(*(yyvsp[-2].expr), *(yyvsp[-1].expr)); }
#line 2920 "parser.cc"
    break;

  case 193: /* f_exp: f_head  */
#line 677 "parser.yy"
               { (yyval.expr) = (yyvsp[0].fluent); }
#line 2926 "parser.cc"
    break;

  case 194: /* term_or_f_exp: NUMBER  */
#line 681 "parser.yy"
                  { require_fluents(); eq_expr = new Value(*(yyvsp[0].num)); delete (yyvsp[0].num); }
#line 2932 "parser.cc"
    break;

  case 195: /* $@31: %empty  */
#line 682 "parser.yy"
                        { require_fluents(); }
#line 2938 "parser.cc"
    break;

  case 196: /* term_or_f_exp: '(' '+' $@31 f_exp f_exp ')'  */
#line 683 "parser.yy"
                  { eq_expr = &Addition::make(*(yyvsp[-2].expr), *(yyvsp[-1].expr)); }
#line 2944 "parser.cc"
    break;

  case 197: /* $@32: %empty  */
#line 684 "parser.yy"
                        { require_fluents(); }
#line 2950 "parser.cc"
    break;

  case 198: /* term_or_f_exp: '(' '-' $@32 f_exp opt_f_exp ')'  */
#line 685 "parser.yy"
                  { eq_expr = make_subtraction(*(yyvsp[-2].expr), (yyvsp[-1].expr)); }
#line 2956 "parser.cc"
    break;

  case 199: /* $@33: %empty  */
#line 686 "parser.yy"
                        { require_fluents(); }
#line 2962 "parser.cc"
    break;

  case 200: /* term_or_f_exp: '(' '*' $@33 f_exp f_exp ')'  */
#line 687 "parser.yy"
                  { eq_expr = &Multiplication::make(*(yyvsp[-2].expr), *(yyvsp[-1].expr)); }
#line 2968 "parser.cc"
    break;

  case 201: /* $@34: %empty  */
#line 688 "parser.yy"
                        { require_fluents(); }
#line 2974 "parser.cc"
    break;

  case 202: /* term_or_f_exp: '(' '/' $@34 f_exp f_exp ')'  */
#line 689 "parser.yy"
                  { eq_expr = &Division::make(*(yyvsp[-2].expr), *(yyvsp[-1].expr)); }
#line 2980 "parser.cc"
    break;

  case 203: /* $@35: %empty  */
#line 690 "parser.yy"
                             { require_fluents(); prepare_fluent((yyvsp[0].str)); }
#line 2986 "parser.cc"
    break;

  case 204: /* term_or_f_exp: '(' function $@35 terms ')'  */
#line 691 "parser.yy"
                            { eq_expr = make_fluent(); }
#line 2992 "parser.cc"
    break;

  case 205: /* term_or_f_exp: name  */
#line 692 "parser.yy"
                     { make_eq_name((yyvsp[0].str)); }
#line 2998 "parser.cc"
    break;

  case 206: /* term_or_f_exp: variable  */
#line 693 "parser.yy"
                         { eq_term = make_term((yyvsp[0].str)); eq_expr = 0; }
#line 3004 "parser.cc"
    break;

  case 207: /* opt_f_exp: %empty  */
#line 696 "parser.yy"
                        { (yyval.expr) = 0; }
#line 3010 "parser.cc"
    break;

  case 209: /* $@36: %empty  */
#line 700 "parser.yy"
                      { prepare_fluent((yyvsp[0].str)); }
#line 3016 "parser.cc"
    break;

  case 210: /* f_head: '(' function $@36 terms ')'  */
#line 700 "parser.yy"
                                                        { (yyval.fluent) = make_fluent(); }
#line 3022 "parser.cc"
    break;

  case 211: /* f_head: function  */
#line 701 "parser.yy"
                  { prepare_fluent((yyvsp[0].str)); (yyval.fluent) = make_fluent(); }
#line 3028 "parser.cc"
    break;

  case 212: /* ground_f_exp: NUMBER  */
#line 704 "parser.yy"
                      { (yyval.expr) = new Value(*(yyvsp[0].num)); delete (yyvsp[0].num); }
#line 3034 "parser.cc"
    break;

  case 213: /* ground_f_exp: '(' '+' ground_f_exp ground_f_exp ')'  */
#line 706 "parser.yy"
                 { (yyval.expr) = &Addition::make(*(yyvsp[-2].expr), *(yyvsp[-1].expr)); }
#line 3040 "parser.cc"
    break;

  case 214: /* ground_f_exp: '(' '-' ground_f_exp opt_ground_f_exp ')'  */
#line 708 "parser.yy"
                 { (yyval.expr) = make_subtraction(*(yyvsp[-2].expr), (yyvsp[-1].expr)); }
#line 3046 "parser.cc"
    break;

  case 215: /* ground_f_exp: '(' '*' ground_f_exp ground_f_exp ')'  */
#line 710 "parser.yy"
                 { (yyval.expr) = &Multiplication::make(*(yyvsp[-2].expr), *(yyvsp[-1].expr)); }
#line 3052 "parser.cc"
    break;

  case 216: /* ground_f_exp: '(' '/' ground_f_exp ground_f_exp ')'  */
#line 712 "parser.yy"
                 { (yyval.expr) = &Division::make(*(yyvsp[-2].expr), *(yyvsp[-1].expr)); }
#line 3058 "parser.cc"
    break;

  case 217: /* ground_f_exp: ground_f_head  */
#line 713 "parser.yy"
                             { (yyval.expr) = (yyvsp[0].fluent); }
#line 3064 "parser.cc"
    break;

  case 218: /* ground_f_exp: '(' TOTAL_TIME ')'  */
#line 715 "parser.yy"
                 { (yyval.expr) = &Fluent::make(domain->total_time(), TermList()); }
#line 3070 "parser.cc"
    break;

  case 219: /* ground_f_exp: TOTAL_TIME  */
#line 717 "parser.yy"
                 { (yyval.expr) = &Fluent::make(domain->total_time(), TermList()); }
#line 3076 "parser.cc"
    break;

  case 220: /* ground_f_exp: '(' GOAL_ACHIEVED ')'  */
#line 719 "parser.yy"
                 { (yyval.expr) = &Fluent::make(domain->goal_achieved(), TermList()); }
#line 3082 "parser.cc"
    break;

  case 221: /* ground_f_exp: GOAL_ACHIEVED  */
#line 721 "parser.yy"
                 { (yyval.expr) = &Fluent::make(domain->goal_achieved(), TermList()); }
#line 3088 "parser.cc"
    break;

  case 222: /* opt_ground_f_exp: %empty  */
#line 724 "parser.yy"
                               { (yyval.expr) = 0; }
#line 3094 "parser.cc"
    break;

  case 224: /* $@37: %empty  */
#line 728 "parser.yy"
                             { prepare_fluent((yyvsp[0].str)); }
#line 3100 "parser.cc"
    break;

  case 225: /* ground_f_head: '(' function $@37 names ')'  */
#line 729 "parser.yy"
                  { (yyval.fluent) = make_fluent(); }
#line 3106 "parser.cc"
    break;

  case 226: /* ground_f_head: function  */
#line 730 "parser.yy"
                         { prepare_fluent((yyvsp[0].str)); (yyval.fluent) = make_fluent(); }
#line 3112 "parser.cc"
    break;

  case 228: /* terms: terms name  */
#line 738 "parser.yy"
                   { add_term((yyvsp[0].str)); }
#line 3118 "parser.cc"
    break;

  case 229: /* terms: terms variable  */
#line 739 "parser.yy"
                       { add_term((yyvsp[0].str)); }
#line 3124 "parser.cc"
    break;

  case 231: /* names: names name  */
#line 743 "parser.yy"
                   { add_term((yyvsp[0].str)); }
#line 3130 "parser.cc"
    break;

  case 233: /* variables: variable_seq  */
#line 747 "parser.yy"
                         { add_variables((yyvsp[0].strs), TypeTable::OBJECT); }
#line 3136 "parser.cc"
    break;

  case 234: /* $@38: %empty  */
#line 748 "parser.yy"
                                   { add_variables((yyvsp[-1].strs), *(yyvsp[0].type)); delete (yyvsp[0].type); }
#line 3142 "parser.cc"
    break;

  case 236: /* variable_seq: variable  */
#line 752 "parser.yy"
                        { (yyval.strs) = new std::vector<const std::string*>(1, (yyvsp[0].str)); }
#line 3148 "parser.cc"
    break;

  case 237: /* variable_seq: variable_seq variable  */
#line 753 "parser.yy"
                                     { (yyval.strs) = (yyvsp[-1].strs); (yyval.strs)->push_back((yyvsp[0].str)); }
#line 3154 "parser.cc"
    break;

  case 239: /* typed_names: name_seq  */
#line 757 "parser.yy"
                       { add_names((yyvsp[0].strs), TypeTable::OBJECT); }
#line 3160 "parser.cc"
    break;

  case 240: /* $@39: %empty  */
#line 758 "parser.yy"
                                 { add_names((yyvsp[-1].strs), *(yyvsp[0].type)); delete (yyvsp[0].type); }
#line 3166 "parser.cc"
    break;

  case 242: /* name_seq: name  */
#line 761 "parser.yy"
                { (yyval.strs) = new std::vector<const std::string*>(1, (yyvsp[0].str)); }
#line 3172 "parser.cc"
    break;

  case 243: /* name_seq: name_seq name  */
#line 762 "parser.yy"
                         { (yyval.strs) = (yyvsp[-1].strs); (yyval.strs)->push_back((yyvsp[0].str)); }
#line 3178 "parser.cc"
    break;

  case 244: /* type_spec: '-' type  */
#line 765 "parser.yy"
                     { (yyval.type) = (yyvsp[0].type); }
#line 3184 "parser.cc"
    break;

  case 245: /* type: object  */
#line 768 "parser.yy"
              { (yyval.type) = new Type(TypeTable::OBJECT); }
#line 3190 "parser.cc"
    break;

  case 246: /* $@40: %empty  */
#line 769 "parser.yy"
       { require_typing(); }
#line 3196 "parser.cc"
    break;

  case 247: /* type: $@40 type_name  */
#line 769 "parser.yy"
                                       { (yyval.type) = new Type(make_type((yyvsp[0].str))); }
#line 3202 "parser.cc"
    break;

  case 248: /* $@41: %empty  */
#line 770 "parser.yy"
       { require_typing(); }
#line 3208 "parser.cc"
    break;

  case 249: /* type: $@41 '(' either types ')'  */
#line 770 "parser.yy"
                                                  { (yyval.type) = new Type(make_type(*(yyvsp[-1].types))); delete (yyvsp[-1].types); }
#line 3214 "parser.cc"
    break;

  case 250: /* types: object  */
#line 773 "parser.yy"
               { (yyval.types) = new TypeSet(); }
#line 3220 "parser.cc"
    break;

  case 251: /* types: type_name  */
#line 774 "parser.yy"
                  { (yyval.types) = new TypeSet(); (yyval.types)->insert(make_type((yyvsp[0].str))); }
#line 3226 "parser.cc"
    break;

  case 252: /* types: types object  */
#line 775 "parser.yy"
                     { (yyval.types) = (yyvsp[-1].types); }
#line 3232 "parser.cc"
    break;

  case 253: /* types: types type_name  */
#line 776 "parser.yy"
                        { (yyval.types) = (yyvsp[-1].types); (yyval.types)->insert(make_type((yyvsp[0].str))); }
#line 3238 "parser.cc"
    break;

  case 255: /* define: DEFINE  */
#line 786 "parser.yy"
                { delete (yyvsp[0].str); }
#line 3244 "parser.cc"
    break;

  case 256: /* domain: DOMAIN_TOKEN  */
#line 789 "parser.yy"
                      { delete (yyvsp[0].str); }
#line 3250 "parser.cc"
    break;

  case 257: /* problem: PROBLEM  */
#line 792 "parser.yy"
                  { delete (yyvsp[0].str); }
#line 3256 "parser.cc"
    break;

  case 258: /* when: WHEN  */
#line 795 "parser.yy"
            { delete (yyvsp[0].str); }
#line 3262 "parser.cc"
    break;

  case 259: /* not: NOT  */
#line 798 "parser.yy"
          { delete (yyvsp[0].str); }
#line 3268 "parser.cc"
    break;

  case 260: /* and: AND  */
#line 801 "parser.yy"
          { delete (yyvsp[0].str); }
#line 3274 "parser.cc"
    break;

  case 261: /* or: OR  */
#line 804 "parser.yy"
        { delete (yyvsp[0].str); }
#line 3280 "parser.cc"
    break;

  case 262: /* imply: IMPLY  */
#line 807 "parser.yy"
              { delete (yyvsp[0].str); }
#line 3286 "parser.cc"
    break;

  case 263: /* exists: EXISTS  */
#line 810 "parser.yy"
                { delete (yyvsp[0].str); }
#line 3292 "parser.cc"
    break;

  case 264: /* forall: FORALL  */
#line 813 "parser.yy"
                { delete (yyvsp[0].str); }
#line 3298 "parser.cc"
    break;

  case 265: /* probabilistic: PROBABILISTIC  */
#line 816 "parser.yy"
                              { delete (yyvsp[0].str); }
#line 3304 "parser.cc"
    break;

  case 266: /* assign: ASSIGN  */
#line 819 "parser.yy"
                { delete (yyvsp[0].str); }
#line 3310 "parser.cc"
    break;

  case 267: /* scale_up: SCALE_UP  */
#line 822 "parser.yy"
                    { delete (yyvsp[0].str); }
#line 3316 "parser.cc"
    break;

  case 268: /* scale_down: SCALE_DOWN  */
#line 825 "parser.yy"
                        { delete (yyvsp[0].str); }
#line 3322 "parser.cc"
    break;

  case 269: /* increase: INCREASE  */
#line 828 "parser.yy"
                    { delete (yyvsp[0].str); }
#line 3328 "parser.cc"
    break;

  case 270: /* decrease: DECREASE  */
#line 831 "parser.yy"
                    { delete (yyvsp[0].str); }
#line 3334 "parser.cc"
    break;

  case 271: /* minimize: MINIMIZE  */
#line 834 "parser.yy"
                    { delete (yyvsp[0].str); }
#line 3340 "parser.cc"
    break;

  case 272: /* maximize: MAXIMIZE  */
#line 837 "parser.yy"
                    { delete (yyvsp[0].str); }
#line 3346 "parser.cc"
    break;

  case 273: /* number: NUMBER_TOKEN  */
#line 840 "parser.yy"
                      { delete (yyvsp[0].str); }
#line 3352 "parser.cc"
    break;

  case 274: /* object: OBJECT_TOKEN  */
#line 843 "parser.yy"
                      { delete (yyvsp[0].str); }
#line 3358 "parser.cc"
    break;

  case 275: /* either: EITHER  */
#line 846 "parser.yy"
                { delete (yyvsp[0].str); }
#line 3364 "parser.cc"
    break;


#line 3368 "parser.cc"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 873 "parser.yy"


/* Outputs an error message. */
static void yyerror(const std::string& s) {
  std::cerr << PACKAGE ":" << current_file << ':' << line_number << ": " << s
            << std::endl;
  success = false;
}


/* Outputs a warning. */
static void yywarning(const std::string& s) {
  if (warning_level > 0) {
    std::cerr << PACKAGE ":" << current_file << ':' << line_number << ": " << s
              << std::endl;
    if (warning_level > 1) {
      success = false;
    }
  }
}


/* Creates an empty domain with the given name. */
static void make_domain(const std::string* name) {
  domain = new Domain(*name);
  domains[*name] = domain;
  requirements = &domain->requirements;
  problem = 0;
  delete name;
}


/* Creates an empty problem with the given name. */
static void make_problem(const std::string* name,
                         const std::string* domain_name) {
  std::map<std::string, Domain*>::const_iterator di =
    domains.find(*domain_name);
  if (di != domains.end()) {
    domain = (*di).second;
  } else {
    domain = new Domain(*domain_name);
    domains[*domain_name] = domain;
    yyerror("undeclared domain `" + *domain_name + "' used");
  }
  requirements = new Requirements(domain->requirements);
  problem = new Problem(*name, *domain);
  const Fluent& total_time_fluent =
    Fluent::make(domain->total_time(), TermList());
  const Update* total_time_update =
    new Assign(total_time_fluent, *new Value(0));
  problem->add_init_effect(UpdateEffect::make(*total_time_update));
  const Fluent& goal_achieved_fluent =
    Fluent::make(domain->goal_achieved(), TermList());
  const Update* goal_achieved_update =
    new Assign(goal_achieved_fluent, *new Value(0));
  problem->add_init_effect(UpdateEffect::make(*goal_achieved_update));
  if (requirements->rewards) {
    reward_function = *domain->functions().find_function("reward");
    const Fluent& reward_fluent = Fluent::make(reward_function, TermList());
    const Update* reward_update = new Assign(reward_fluent, *new Value(0));
    problem->add_init_effect(UpdateEffect::make(*reward_update));
  }
  delete name;
  delete domain_name;
}


/* Adds :typing to the requirements. */
static void require_typing() {
  if (!requirements->typing) {
    yywarning("assuming `:typing' requirement");
    requirements->typing = true;
  }
}


/* Adds :fluents to the requirements. */
static void require_fluents() {
  if (!requirements->fluents) {
    yywarning("assuming `:fluents' requirement");
    requirements->fluents = true;
  }
}


/* Adds :disjunctive-preconditions to the requirements. */
static void require_disjunction() {
  if (!requirements->disjunctive_preconditions) {
    yywarning("assuming `:disjunctive-preconditions' requirement");
    requirements->disjunctive_preconditions = true;
  }
}


/* Adds :conditional-effects to the requirements. */
static void require_conditional_effects() {
  if (!requirements->conditional_effects) {
    yywarning("assuming `:conditional-effects' requirement");
    requirements->conditional_effects = true;
  }
}


/* Returns a simple type with the given name. */
static const Type& make_type(const std::string* name) {
  const Type* t = domain->types().find_type(*name);
  if (t == 0) {
    t = &domain->types().add_type(*name);
    if (name_kind != TYPE_KIND) {
      yywarning("implicit declaration of type `" + *name + "'");
    }
  }
  delete name;
  return *t;
}


/* Returns the union of the given types. */
static Type make_type(const TypeSet& types) {
  return TypeTable::union_type(types);
}


/* Returns a simple term with the given name. */
static Term make_term(const std::string* name) {
  if ((*name)[0] == '?') {
    const Variable* vp = context.find(*name);
    if (vp != 0) {
      delete name;
      return *vp;
    } else {
      Variable v = TermTable::add_variable(TypeTable::OBJECT);
      context.insert(*name, v);
      yyerror("free variable `" + *name + "' used");
      delete name;
      return v;
    }
  } else {
    TermTable& terms = (problem != 0) ? problem->terms() : domain->terms();
    const Object* o = terms.find_object(*name);
    if (o == 0) {
      size_t n = term_parameters.size();
      if (atom_predicate != 0
          && PredicateTable::parameters(*atom_predicate).size() > n) {
        const Type& t = PredicateTable::parameters(*atom_predicate)[n];
        o = &terms.add_object(*name, t);
      } else {
        o = &terms.add_object(*name, TypeTable::OBJECT);
      }
      yywarning("implicit declaration of object `" + *name + "'");
    }
    delete name;
    return *o;
  }
}


/* Creates a predicate with the given name. */
static void make_predicate(const std::string* name) {
  predicate = domain->predicates().find_predicate(*name);
  if (predicate == 0) {
    repeated_predicate = false;
    predicate = &domain->predicates().add_predicate(*name);
  } else {
    repeated_predicate = true;
    yywarning("ignoring repeated declaration of predicate `" + *name + "'");
  }
  delete name;
}


/* Creates a function with the given name. */
static void make_function(const std::string* name) {
  repeated_function = false;
  function = domain->functions().find_function(*name);
  if (function == 0) {
    function = &domain->functions().add_function(*name);
  } else {
    repeated_function = true;
    if (requirements->rewards && *name == "reward") {
      yywarning("ignoring declaration of reserved function `reward'");
    } else if (*name == "total-time" || *name == "goal-achieved") {
      yywarning("ignoring declaration of reserved function `" + *name + "'");
    } else {
      yywarning("ignoring repeated declaration of function `" + *name + "'");
    }
  }
  delete name;
}


/* Creates an action with the given name. */
static void make_action(const std::string* name) {
  context.push_frame();
  action = new ActionSchema(*name);
  delete name;
}


/* Adds the current action to the current domain. */
static void add_action() {
  context.pop_frame();
  if (domain->find_action(action->name()) == 0) {
    domain->add_action(*action);
  } else {
    yywarning("ignoring repeated declaration of action `"
              + action->name() + "'");
    delete action;
  }
  action = 0;
}


/* Prepares for the parsing of a universally quantified effect. */
static void prepare_forall_effect() {
  if (!requirements->conditional_effects) {
    yywarning("assuming `:conditional-effects' requirement");
    requirements->conditional_effects = true;
  }
  context.push_frame();
  quantified.push_back(Term(0));
}


/* Creates a universally quantified effect. */
static const Effect* make_forall_effect(const Effect& effect) {
  context.pop_frame();
  size_t n = quantified.size() - 1;
  size_t m = n;
  while (quantified[n].variable()) {
    n--;
  }
  VariableList parameters;
  for (size_t i = n + 1; i <= m; i++) {
    parameters.push_back(quantified[i].as_variable());
  }
  quantified.resize(n, Term(0));
  return &QuantifiedEffect::make(parameters, effect);
}


/* Adds an outcome to the given probabilistic effect. */
static void add_outcome(std::vector<std::pair<Rational, const Effect*> >& os,
                        const Rational* p, const Effect& effect) {
  if (!requirements->probabilistic_effects) {
    yywarning("assuming `:probabilistic-effects' requirement");
    requirements->probabilistic_effects = true;
  }
  if (*p < 0 || *p > 1) {
    yyerror("outcome probability needs to be in the interval [0,1]");
  }
  os.push_back(std::make_pair(*p, &effect));
  delete p;
}


/* Creates a probabilistic effect. */
static const Effect*
make_prob_effect(const std::vector<std::pair<Rational, const Effect*> >* os) {
  Rational psum = 0;
  for (size_t i = 0; i < os->size(); i++) {
    psum = psum + (*os)[i].first;
  }
  if (psum > 1) {
    yyerror("effect outcome probabilities add up to more than 1");
    delete os;
    return &Effect::EMPTY;
  } else {
    const Effect& peff = ProbabilisticEffect::make(*os);
    delete os;
    return &peff;
  }
}


/* Creates an add effect. */
static const Effect* make_add_effect(const Atom& atom) {
  PredicateTable::make_dynamic(atom.predicate());
  return new AddEffect(atom);
}


/* Creates a delete effect. */
static const Effect* make_delete_effect(const Atom& atom) {
  PredicateTable::make_dynamic(atom.predicate());
  return new DeleteEffect(atom);
}


/* Creates an assign update effect. */
static const Effect* make_assign_effect(const Fluent& fluent,
                                        const Expression& expr) {
  if (requirements->rewards && fluent.function() == reward_function) {
    yyerror("only constant reward increments/decrements allowed");
  } else {
    require_fluents();
  }
  effect_fluent = false;
  FunctionTable::make_dynamic(fluent.function());
  return &UpdateEffect::make(*new Assign(fluent, expr));
}


/* Creates a scale-up update effect. */
static const Effect* make_scale_up_effect(const Fluent& fluent,
                                          const Expression& expr) {
  if (requirements->rewards && fluent.function() == reward_function) {
    yyerror("only constant reward increments/decrements allowed");
  } else {
    require_fluents();
  }
  effect_fluent = false;
  FunctionTable::make_dynamic(fluent.function());
  return &UpdateEffect::make(*new ScaleUp(fluent, expr));
}


/* Creates a scale-down update effect. */
static const Effect* make_scale_down_effect(const Fluent& fluent,
                                            const Expression& expr) {
  if (requirements->rewards && fluent.function() == reward_function) {
    yyerror("only constant reward increments/decrements allowed");
  } else {
    require_fluents();
  }
  effect_fluent = false;
  FunctionTable::make_dynamic(fluent.function());
  return &UpdateEffect::make(*new ScaleDown(fluent, expr));
}


/* Creates an increase update effect. */
static const Effect* make_increase_effect(const Fluent& fluent,
                                          const Expression& expr) {
  if (requirements->rewards && fluent.function() == reward_function) {
    if (typeid(expr) != typeid(Value)) {
      yyerror("only constant reward increments/decrements allowed");
    }
  } else {
    require_fluents();
  }
  effect_fluent = false;
  FunctionTable::make_dynamic(fluent.function());
  return &UpdateEffect::make(*new Increase(fluent, expr));
}


/* Creates a decrease update effect. */
static const Effect* make_decrease_effect(const Fluent& fluent,
                                          const Expression& expr) {
  if (requirements->rewards && fluent.function() == reward_function) {
    if (typeid(expr) != typeid(Value)) {
      yyerror("only constant reward increments/decrements allowed");
    }
  } else {
    require_fluents();
  }
  effect_fluent = false;
  FunctionTable::make_dynamic(fluent.function());
  return &UpdateEffect::make(*new Decrease(fluent, expr));
}


/* Adds types, constants, or objects to the current domain or problem. */
static void add_names(const std::vector<const std::string*>* names,
                      const Type& type) {
  for (std::vector<const std::string*>::const_iterator si = names->begin();
       si != names->end(); si++) {
    const std::string* s = *si;
    if (name_kind == TYPE_KIND) {
      if (*s == TypeTable::OBJECT_NAME) {
        yywarning("ignoring declaration of reserved type `object'");
      } else if (*s == TypeTable::NUMBER_NAME) {
        yywarning("ignoring declaration of reserved type `number'");
      } else {
        const Type* t = domain->types().find_type(*s);
        if (t == 0) {
          t = &domain->types().add_type(*s);
        }
        if (!TypeTable::add_supertype(*t, type)) {
          yyerror("cyclic type hierarchy");
        }
      }
    } else if (name_kind == CONSTANT_KIND) {
      const Object* o = domain->terms().find_object(*s);
      if (o == 0) {
        domain->terms().add_object(*s, type);
      } else {
        TypeSet components;
        TypeTable::components(components, TermTable::type(*o));
        components.insert(type);
        TermTable::set_type(*o, make_type(components));
      }
    } else { /* name_kind == OBJECT_KIND */
      if (domain->terms().find_object(*s) != 0) {
        yywarning("ignoring declaration of object `" + *s
                  + "' previously declared as constant");
      } else {
        const Object* o = problem->terms().find_object(*s);
        if (o == 0) {
          problem->terms().add_object(*s, type);
        } else {
          TypeSet components;
          TypeTable::components(components, TermTable::type(*o));
          components.insert(type);
          TermTable::set_type(*o, make_type(components));
        }
      }
    }
    delete s;
  }
  delete names;
}


/* Adds variables to the current variable list. */
static void add_variables(const std::vector<const std::string*>* names,
                          const Type& type) {
  for (std::vector<const std::string*>::const_iterator si = names->begin();
       si != names->end(); si++) {
    const std::string* s = *si;
    if (predicate != 0) {
      if (!repeated_predicate) {
        PredicateTable::add_parameter(*predicate, type);
      }
    } else if (function != 0) {
      if (!repeated_function) {
        FunctionTable::add_parameter(*function, type);
      }
    } else {
      if (context.shallow_find(*s) != 0) {
        yyerror("repetition of parameter `" + *s + "'");
      } else if (context.find(*s) != 0) {
        yywarning("shadowing parameter `" + *s + "'");
      }
      Variable var = TermTable::add_variable(type);
      context.insert(*s, var);
      if (!quantified.empty()) {
        quantified.push_back(var);
      } else { /* action != 0 */
        action->add_parameter(var);
      }
    }
    delete s;
  }
  delete names;
}


/* Prepares for the parsing of an atomic state formula. */
static void prepare_atom(const std::string* name) {
  atom_predicate = domain->predicates().find_predicate(*name);
  if (atom_predicate == 0) {
    atom_predicate = &domain->predicates().add_predicate(*name);
    undeclared_atom_predicate = true;
    if (problem != 0) {
      yywarning("undeclared predicate `" + *name + "' used");
    } else {
      yywarning("implicit declaration of predicate `" + *name + "'");
    }
  } else {
    undeclared_atom_predicate = false;
  }
  term_parameters.clear();
  delete name;
}


/* Prepares for the parsing of a fluent. */
static void prepare_fluent(const std::string* name) {
  fluent_function = domain->functions().find_function(*name);
  if (fluent_function == 0) {
    fluent_function = &domain->functions().add_function(*name);
    undeclared_fluent_function = true;
    if (problem != 0) {
      yywarning("undeclared function `" + *name + "' used");
    } else {
      yywarning("implicit declaration of function `" + *name + "'");
    }
  } else {
    undeclared_fluent_function = false;
  }
  if (requirements->rewards && *name == "reward") {
    if (!effect_fluent && !metric_fluent) {
      yyerror("reserved function `reward' not allowed here");
    }
  } else if ((*name == "total-time" || *name == "goal-achieved")) {
    if (!metric_fluent) {
      yyerror("reserved function `" + *name + "' not allowed here");
    }
  } else {
    require_fluents();
  }
  term_parameters.clear();
  delete name;
}


/* Adds a term with the given name to the current atomic state formula. */
static void add_term(const std::string* name) {
  const Term& term = make_term(name);
  if (atom_predicate != 0) {
    size_t n = term_parameters.size();
    if (undeclared_atom_predicate) {
      PredicateTable::add_parameter(*atom_predicate, TermTable::type(term));
    } else {
      const TypeList& params = PredicateTable::parameters(*atom_predicate);
      if (params.size() > n
          && !TypeTable::subtype(TermTable::type(term), params[n])) {
        yyerror("type mismatch");
      }
    }
  } else if (fluent_function != 0) {
    size_t n = term_parameters.size();
    if (undeclared_fluent_function) {
      FunctionTable::add_parameter(*fluent_function, TermTable::type(term));
    } else {
      const TypeList& params = FunctionTable::parameters(*fluent_function);
      if (params.size() > n
          && !TypeTable::subtype(TermTable::type(term), params[n])) {
        yyerror("type mismatch");
      }
    }
  }
  term_parameters.push_back(term);
}


/* Creates the atomic formula just parsed. */
static const Atom* make_atom() {
  size_t n = term_parameters.size();
  if (PredicateTable::parameters(*atom_predicate).size() < n) {
    yyerror("too many parameters passed to predicate `"
            + PredicateTable::name(*atom_predicate) + "'");
  } else if (PredicateTable::parameters(*atom_predicate).size() > n) {
    yyerror("too few parameters passed to predicate `"
            + PredicateTable::name(*atom_predicate) + "'");
  }
  const Atom& atom = Atom::make(*atom_predicate, term_parameters);
  atom_predicate = 0;
  return &atom;
}


/* Creates the fluent just parsed. */
static const Fluent* make_fluent() {
  size_t n = term_parameters.size();
  if (FunctionTable::parameters(*fluent_function).size() < n) {
    yyerror("too many parameters passed to function `"
            + FunctionTable::name(*fluent_function) + "'");
  } else if (FunctionTable::parameters(*fluent_function).size() > n) {
    yyerror("too few parameters passed to function `"
            + FunctionTable::name(*fluent_function) + "'");
  }
  const Fluent& fluent = Fluent::make(*fluent_function, term_parameters);
  fluent_function = 0;
  return &fluent;
}


/* Creates a subtraction. */
static const Expression* make_subtraction(const Expression& term,
                                          const Expression* opt_term) {
  if (opt_term != 0) {
    return &Subtraction::make(term, *opt_term);
  } else {
    return &Subtraction::make(*new Value(0), term);
  }
}


/* Creates an atom or fluent for the given name to be used in an
   equality formula. */
static void make_eq_name(const std::string* name) {
  const Function* f = domain->functions().find_function(*name);
  if (f != 0) {
    prepare_fluent(name);
    eq_expr = make_fluent();
  } else {
    /* Assume this is a term. */
    eq_term = make_term(name);
    eq_expr = 0;
  }
}


/* Creates an equality formula. */
static const StateFormula* make_equality() {
  if (!requirements->equality) {
    yywarning("assuming `:equality' requirement");
    requirements->equality = true;
  }
  if (first_eq_expr != 0 && eq_expr != 0) {
    return &EqualTo::make(*first_eq_expr, *eq_expr);
  } else if (first_eq_expr == 0 && eq_expr == 0) {
    if (TypeTable::subtype(TermTable::type(first_eq_term),
                           TermTable::type(eq_term))
        || TypeTable::subtype(TermTable::type(eq_term),
                              TermTable::type(first_eq_term))) {
      return &Equality::make(first_eq_term, eq_term);
    } else {
      return &StateFormula::FALSE;
    }
  } else {
    yyerror("comparison of term and numeric expression");
    return &StateFormula::FALSE;
  }
}


/* Creates a negated formula. */
static const StateFormula* make_negation(const StateFormula& negand) {
  if (typeid(negand) == typeid(Atom)) {
    if (!requirements->negative_preconditions) {
      yywarning("assuming `:negative-preconditions' requirement");
      requirements->negative_preconditions = true;
    }
  } else if (typeid(negand) != typeid(Equality)
             && dynamic_cast<const Comparison*>(&negand) != 0) {
    require_disjunction();
  }
  return &Negation::make(negand);
}


/* Creates an implication. */
static const StateFormula* make_implication(const StateFormula& f1,
                                            const StateFormula& f2) {
  require_disjunction();
  return &(Negation::make(f1) || f2);
}


/* Prepares for the parsing of an existentially quantified formula. */
static void prepare_exists() {
  if (!requirements->existential_preconditions) {
    yywarning("assuming `:existential-preconditions' requirement");
    requirements->existential_preconditions = true;
  }
  context.push_frame();
  quantified.push_back(Term(0));
}


/* Prepares for the parsing of a universally quantified formula. */
static void prepare_forall() {
  if (!requirements->universal_preconditions) {
    yywarning("assuming `:universal-preconditions' requirement");
    requirements->universal_preconditions = true;
  }
  context.push_frame();
  quantified.push_back(Term(0));
}


/* Creates an existentially quantified formula. */
static const StateFormula* make_exists(const StateFormula& body) {
  context.pop_frame();
  size_t m = quantified.size() - 1;
  size_t n = m;
  while (quantified[n].variable()) {
    n--;
  }
  if (n < m) {
    VariableList parameters;
    for (size_t i = n + 1; i <= m; i++) {
      parameters.push_back(quantified[i].as_variable());
    }
    quantified.resize(n, Term(0));
    return &Exists::make(parameters, body);
  } else {
    quantified.pop_back();
    return &body;
  }
}


/* Creates a universally quantified formula. */
static const StateFormula* make_forall(const StateFormula& body) {
  context.pop_frame();
  size_t m = quantified.size() - 1;
  size_t n = m;
  while (quantified[n].variable()) {
    n--;
  }
  if (n < m) {
    VariableList parameters;
    for (size_t i = n + 1; i <= m; i++) {
      parameters.push_back(quantified[i].as_variable());
    }
    quantified.resize(n, Term(0));
    return &Forall::make(parameters, body);
  } else {
    quantified.pop_back();
    return &body;
  }
}


/* Sets the goal reward for the current problem. */
void set_goal_reward(const Expression& goal_reward) {
  if (!requirements->rewards) {
    yyerror("goal reward only allowed with the `:rewards' requirement");
  } else {
    FunctionTable::make_dynamic(reward_function);
    const Fluent& reward_fluent = Fluent::make(reward_function, TermList());
    problem->set_goal_reward(*new Increase(reward_fluent, goal_reward));
  }
}


/* Sets the default metric for the current problem. */
static void set_default_metric() {
  if (requirements->rewards) {
    problem->set_metric(Fluent::make(reward_function, TermList()));
  } else if (requirements->probabilistic_effects) {
    problem->set_metric(Fluent::make(domain->goal_achieved(), TermList()));
  } else {
    problem->set_metric(Fluent::make(domain->total_time(), TermList()), true);
  }
}
