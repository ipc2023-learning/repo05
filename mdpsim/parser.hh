/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

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

#line 207 "parser.hh"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
