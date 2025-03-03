
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse         ops_pddlparse
#define yylex           ops_pddllex
#define yyerror         ops_pddlerror
#define yylval          ops_pddllval
#define yychar          ops_pddlchar
#define yydebug         ops_pddldebug
#define yynerrs         ops_pddlnerrs


/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 19 "scan-ops_pddl.y"


#define YYMAXDEPTH 1000000 
#define YY_NO_UNPUT

#ifndef __SUN__
#define YYSTACK_USE_ALLOCA FALSE
#endif


#include <stdio.h>
#include <string.h> 
#include "ff.h"
#include "memory.h"
#include "parse.h"


#define yyin ops_pddlin
#define yytext ops_pddltext

#ifndef SCAN_ERR
#define SCAN_ERR
#define DOMDEF_EXPECTED            0
#define DOMAIN_EXPECTED            1
#define DOMNAME_EXPECTED           2
#define LBRACKET_EXPECTED          3
#define RBRACKET_EXPECTED          4
#define DOMDEFS_EXPECTED           5
#define REQUIREM_EXPECTED          6
#define TYPEDLIST_EXPECTED         7
#define LITERAL_EXPECTED           8
#define PRECONDDEF_UNCORRECT       9
#define TYPEDEF_EXPECTED          10
#define CONSTLIST_EXPECTED        11
#define PREDDEF_EXPECTED          12 
#define NAME_EXPECTED             13
#define VARIABLE_EXPECTED         14
#define ACTIONFUNCTOR_EXPECTED    15
#define ATOM_FORMULA_EXPECTED     16
#define EFFECT_DEF_EXPECTED       17
#define NEG_FORMULA_EXPECTED      18
#define NOT_SUPPORTED             19
#define ACTION                    20
#define DERIVED_PRED_EXPECTED     21
#endif


#define NAME_STR "name\0"
#define VARIABLE_STR "variable\0"
#define STANDARD_TYPE "OBJECT\0"
 

static char *serrmsg[] = {
  "domain definition expected",
  "'domain' expected",
  "domain name expected",
  "'(' expected",
  "')' expected",
  "additional domain definitions expected",
  "requirements (e.g. ':STRIPS') expected",
  "typed list of <%s> expected",
  "literal expected",
  "uncorrect precondition definition",
  "type definition expected",
  "list of constants expected",
  "predicate definition expected",
  "<name> expected",
  "<variable> expected",
  "action functor expected",
  "atomic formula expected",
  "effect definition expected",
  "negated atomic formula expected",
  "requirement %s not supported by this IPP version",  
  "action definition is not correct",
  "derived predicate definition is not correct",
  NULL
};


//void opserr( int errno, char *par );


static int sact_err;
static char *sact_err_par = NULL;
static PlOperator *scur_op = NULL;
static PlOperator *der_op = NULL;
static Bool sis_negated = FALSE;

 int yylex(void);
 int yyerror(char *msg); 

/* 
 * call	bison -pops -bscan-ops scan-ops.y
 */

void opserr( int errno, char *par )
{

  sact_err = errno;

  if ( sact_err_par ) {
    free(sact_err_par);
  }
  if ( par ) {
    sact_err_par = new_Token(strlen(par)+1);
    strcpy(sact_err_par, par);
  } else {
    sact_err_par = NULL;
  }

}
  
int supported( char *str )

{
  
  int i;
  
/*sositituito per pddl2 (negative-precontions oltre a negation*/
  /*  char * sup[] = { ":STRIPS", ":NEGATION", ":EQUALITY",":TYPING", 
		   ":CONDITIONAL-EFFECTS", ":DISJUNCTIVE-PRECONDITIONS", 
		   ":EXISTENTIAL-PRECONDITIONS", ":UNIVERSAL-PRECONDITIONS", 
		   ":QUANTIFIED-PRECONDITIONS", ":ADL",
		   NULL };     */
  char * sup[] = { ":STRIPS", ":NEGATIVE-PRECONDITIONS",":NEGATION",
		     ":EQUALITY",":TYPING", 
		   ":CONDITIONAL-EFFECTS", ":DISJUNCTIVE-PRECONDITIONS", 
		   ":EXISTENTIAL-PRECONDITIONS", ":UNIVERSAL-PRECONDITIONS", 
		   ":QUANTIFIED-PRECONDITIONS", ":ADL", ":DERIVED",
		   NULL };    

  return 1;
  for (i=0; NULL != sup[i]; i++) {
    if ( SAME == strcmp(sup[i], str) ) {
      return TRUE;
    }
  }
  
  return FALSE;

}


 


/* Line 189 of yacc.c  */
#line 228 "scan-ops_pddl.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     DEFINE_TOK = 258,
     DOMAIN_TOK = 259,
     REQUIREMENTS_TOK = 260,
     TYPES_TOK = 261,
     EITHER_TOK = 262,
     CONSTANTS_TOK = 263,
     PREDICATES_TOK = 264,
     FUNCTIONS_TOK = 265,
     DURATIVE_ACTION_TOK = 266,
     CONDITION_TOK = 267,
     DURATION_TOK = 268,
     DURATION_VAR_TOK = 269,
     AT_START = 270,
     AT_END = 271,
     OVER_ALL = 272,
     INCREASE_TOK = 273,
     DECREASE_TOK = 274,
     TIME_TOK = 275,
     GREATER_OR_EQUAL_TOK = 276,
     LESS_THAN_OR_EQUAL_TOK = 277,
     INTVAL = 278,
     FLOATVAL = 279,
     ASSIGN_TOK = 280,
     SCALE_UP_TOK = 281,
     SCALE_DOWN_TOK = 282,
     PLUS_TOK = 283,
     MINUS_TOK = 284,
     MUL_TOK = 285,
     DIV_TOK = 286,
     EQUAL_TOK = 287,
     GREATER_TOK = 288,
     LESS_THAN_TOK = 289,
     ACTION_TOK = 290,
     VARS_TOK = 291,
     DERIVED_TOK = 292,
     PRECONDITION_TOK = 293,
     PARAMETERS_TOK = 294,
     EFFECT_TOK = 295,
     AND_TOK = 296,
     NOT_TOK = 297,
     WHEN_TOK = 298,
     FORALL_TOK = 299,
     IMPLY_TOK = 300,
     OR_TOK = 301,
     EXISTS_TOK = 302,
     NAME = 303,
     VARIABLE = 304,
     OPEN_PAREN = 305,
     CLOSE_PAREN = 306,
     UMINUS = 307
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 169 "scan-ops_pddl.y"


  char string[MAX_LENGTH];
  char *pstring;
  PlNode *pPlNode;
  FactList *pFactList;
  TokenList *pTokenList;
  TypedList *pTypedList;

    int ival;
    float fval;



/* Line 214 of yacc.c  */
#line 331 "scan-ops_pddl.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 343 "scan-ops_pddl.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

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
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
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
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   398

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  53
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  79
/* YYNRULES -- Number of rules.  */
#define YYNRULES  174
/* YYNRULES -- Number of states.  */
#define YYNSTATES  392

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   307

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
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
      45,    46,    47,    48,    49,    50,    51,    52
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,     8,    14,    19,    21,    24,
      27,    30,    33,    36,    39,    42,    45,    46,    52,    53,
      54,    61,    62,    68,    69,    70,    77,    78,    87,    88,
      89,    97,    98,    99,   103,   104,   110,   111,   117,   118,
     119,   128,   129,   134,   135,   141,   142,   147,   148,   153,
     155,   160,   165,   170,   176,   184,   192,   194,   200,   202,
     207,   213,   219,   225,   231,   233,   235,   240,   242,   244,
     246,   248,   250,   252,   257,   263,   269,   275,   281,   283,
     285,   287,   289,   291,   293,   295,   297,   299,   300,   303,
     305,   310,   318,   324,   330,   331,   334,   339,   341,   346,
     347,   350,   352,   354,   356,   359,   361,   363,   364,   370,
     374,   377,   378,   384,   388,   391,   392,   393,   402,   403,
     409,   410,   415,   416,   421,   422,   427,   428,   435,   440,
     442,   447,   449,   452,   457,   462,   467,   468,   471,   476,
     478,   486,   492,   498,   503,   508,   513,   518,   524,   530,
     536,   542,   547,   549,   551,   553,   555,   557,   559,   565,
     571,   573,   578,   580,   582,   585,   591,   596,   601,   603,
     605,   607,   609,   611,   614
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      54,     0,    -1,    -1,    55,    56,    -1,    -1,    50,     3,
      58,    57,    59,    -1,    50,     4,    48,    51,    -1,    51,
      -1,    69,    59,    -1,    76,    59,    -1,    74,    59,    -1,
      78,    59,    -1,    60,    59,    -1,   105,    59,    -1,    64,
      59,    -1,   112,    59,    -1,    -1,    50,     9,    62,    61,
      51,    -1,    -1,    -1,    50,    48,   104,    51,    63,    62,
      -1,    -1,    50,    10,    66,    65,    51,    -1,    -1,    -1,
      50,    48,   104,    51,    67,    66,    -1,    -1,    50,    48,
     104,    51,    29,    48,    68,    66,    -1,    -1,    -1,    50,
       5,    70,    48,    71,    72,    51,    -1,    -1,    -1,    48,
      73,    72,    -1,    -1,    50,     6,    75,   103,    51,    -1,
      -1,    50,     8,    77,   103,    51,    -1,    -1,    -1,    50,
      35,    79,    48,    80,    81,    82,    51,    -1,    -1,    39,
      50,   104,    51,    -1,    -1,    36,    50,   104,    51,    82,
      -1,    -1,    38,    85,    83,    82,    -1,    -1,    40,    95,
      84,    82,    -1,    97,    -1,    50,    41,    94,    51,    -1,
      50,    46,    94,    51,    -1,    50,    42,    85,    51,    -1,
      50,    45,    85,    85,    51,    -1,    50,    47,    50,   104,
      51,    85,    51,    -1,    50,    44,    50,   104,    51,    85,
      51,    -1,    86,    -1,    50,    89,    87,    87,    51,    -1,
      90,    -1,    50,    29,    87,    51,    -1,    50,    29,    87,
      87,    51,    -1,    50,    28,    87,    87,    51,    -1,    50,
      30,    87,    87,    51,    -1,    50,    31,    87,    87,    51,
      -1,    88,    -1,    91,    -1,    50,    92,    99,    51,    -1,
      92,    -1,    33,    -1,    34,    -1,    32,    -1,    21,    -1,
      22,    -1,    50,    29,    90,    51,    -1,    50,    29,    90,
      90,    51,    -1,    50,    28,    90,    90,    51,    -1,    50,
      30,    90,    90,    51,    -1,    50,    31,    90,    90,    51,
      -1,    91,    -1,    23,    -1,    24,    -1,    48,    -1,    25,
      -1,    26,    -1,    27,    -1,    18,    -1,    19,    -1,    -1,
      85,    94,    -1,    97,    -1,    50,    41,    96,    51,    -1,
      50,    44,    50,   104,    51,    95,    51,    -1,    50,    43,
      85,    95,    51,    -1,    50,    93,    88,    87,    51,    -1,
      -1,    95,    96,    -1,    50,    42,    98,    51,    -1,    98,
      -1,    50,   102,    99,    51,    -1,    -1,   100,    99,    -1,
      48,    -1,    49,    -1,    48,    -1,    48,   101,    -1,    48,
      -1,    32,    -1,    -1,    48,   131,   101,    51,   103,    -1,
      48,   130,   103,    -1,    48,   103,    -1,    -1,    49,   131,
     101,    51,   104,    -1,    49,   130,   104,    -1,    49,   104,
      -1,    -1,    -1,    50,    11,   106,    48,   107,    81,   108,
      51,    -1,    -1,    36,    50,   104,    51,   108,    -1,    -1,
      13,   125,   109,   108,    -1,    -1,    12,   115,   110,   108,
      -1,    -1,    40,   119,   111,   108,    -1,    -1,    50,    37,
     113,   114,    85,    51,    -1,    50,   102,   104,    51,    -1,
     117,    -1,    50,    41,   116,    51,    -1,   117,    -1,   117,
     116,    -1,    50,    15,    85,    51,    -1,    50,    16,    85,
      51,    -1,    50,    17,    85,    51,    -1,    -1,   119,   118,
      -1,    50,    41,   118,    51,    -1,   120,    -1,    50,    44,
      50,   104,    51,   119,    51,    -1,    50,    43,   115,   119,
      51,    -1,    50,    93,    88,   122,    51,    -1,    50,    15,
      95,    51,    -1,    50,    16,    95,    51,    -1,    50,    15,
     121,    51,    -1,    50,    16,   121,    51,    -1,    50,    18,
      88,   124,    51,    -1,    50,    19,    88,   124,    51,    -1,
      50,    93,    88,   122,    51,    -1,    50,   123,   122,   122,
      51,    -1,    50,    29,   122,    51,    -1,    14,    -1,    87,
      -1,    28,    -1,    29,    -1,    30,    -1,    31,    -1,    50,
      30,    87,    20,    51,    -1,    50,    30,    20,    87,    51,
      -1,    20,    -1,    50,    41,   126,    51,    -1,   127,    -1,
     127,    -1,   127,   126,    -1,    50,   128,    14,   129,    51,
      -1,    50,    15,   127,    51,    -1,    50,    16,   127,    51,
      -1,    22,    -1,    21,    -1,    32,    -1,    90,    -1,    87,
      -1,    29,    48,    -1,    29,    50,     7,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   294,   294,   294,   305,   304,   320,   330,   332,   334,
     336,   338,   340,   343,   345,   349,   357,   356,   366,   369,
     368,   399,   398,   408,   411,   410,   436,   435,   465,   469,
     464,   480,   484,   483,   497,   496,   510,   509,   525,   532,
     524,   546,   550,   563,   566,   585,   584,   591,   590,   605,
     618,   624,   630,   636,   646,   661,   677,   688,   699,   705,
     715,   727,   740,   752,   764,   771,   782,   789,   799,   804,
     809,   814,   819,   826,   833,   840,   847,   854,   861,   869,
     879,   891,   935,   940,   945,   950,   955,   966,   970,   982,
     995,  1001,  1016,  1031,  1045,  1049,  1061,  1067,  1076,  1088,
    1090,  1101,  1107,  1117,  1124,  1135,  1141,  1152,  1154,  1167,
    1178,  1198,  1200,  1212,  1226,  1245,  1249,  1244,  1259,  1262,
    1281,  1280,  1289,  1288,  1295,  1294,  1305,  1304,  1341,  1369,
    1374,  1382,  1387,  1395,  1401,  1407,  1419,  1423,  1430,  1436,
    1442,  1456,  1471,  1496,  1502,  1508,  1514,  1520,  1530,  1542,
    1557,  1564,  1574,  1583,  1588,  1598,  1608,  1618,  1631,  1643,
    1656,  1667,  1673,  1681,  1686,  1694,  1709,  1714,  1721,  1730,
    1739,  1750,  1752,  1756,  1764
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "DEFINE_TOK", "DOMAIN_TOK",
  "REQUIREMENTS_TOK", "TYPES_TOK", "EITHER_TOK", "CONSTANTS_TOK",
  "PREDICATES_TOK", "FUNCTIONS_TOK", "DURATIVE_ACTION_TOK",
  "CONDITION_TOK", "DURATION_TOK", "DURATION_VAR_TOK", "AT_START",
  "AT_END", "OVER_ALL", "INCREASE_TOK", "DECREASE_TOK", "TIME_TOK",
  "GREATER_OR_EQUAL_TOK", "LESS_THAN_OR_EQUAL_TOK", "INTVAL", "FLOATVAL",
  "ASSIGN_TOK", "SCALE_UP_TOK", "SCALE_DOWN_TOK", "PLUS_TOK", "MINUS_TOK",
  "MUL_TOK", "DIV_TOK", "EQUAL_TOK", "GREATER_TOK", "LESS_THAN_TOK",
  "ACTION_TOK", "VARS_TOK", "DERIVED_TOK", "PRECONDITION_TOK",
  "PARAMETERS_TOK", "EFFECT_TOK", "AND_TOK", "NOT_TOK", "WHEN_TOK",
  "FORALL_TOK", "IMPLY_TOK", "OR_TOK", "EXISTS_TOK", "NAME", "VARIABLE",
  "OPEN_PAREN", "CLOSE_PAREN", "UMINUS", "$accept", "file", "$@1",
  "domain_definition", "$@2", "domain_name", "optional_domain_defs",
  "predicates_def", "$@3", "predicates_list", "$@4", "functions_def",
  "$@5", "functions_list", "$@6", "$@7", "require_def", "$@8", "$@9",
  "require_key_star", "$@10", "types_def", "$@11", "constants_def", "$@12",
  "action_def", "$@13", "$@14", "param_def", "action_def_body", "$@15",
  "$@16", "adl_goal_description", "f_comp", "f_exp", "f_head",
  "binary_comp", "num_exp", "number", "function_symbol", "assign_op",
  "adl_goal_description_star", "adl_effect", "adl_effect_star",
  "literal_term", "atomic_formula_term", "term_star", "term", "name_plus",
  "predicate", "typed_list_name", "typed_list_variable",
  "durative_action_def", "$@17", "$@18", "durative_action_def_body",
  "$@19", "$@20", "$@21", "derived_def", "$@22", "derived_predicate_def",
  "da_adl_goal_description", "timed_adl_goal_description_plus",
  "timed_adl_goal_description", "da_adl_effect_star", "da_adl_effect",
  "timed_adl_effect", "f_assign_da", "f_exp_da", "binary_op", "f_exp_t",
  "duration_constraint", "simple_duration_constraint_plus",
  "simple_duration_constraint", "d_op", "d_value", "type", "either", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    53,    55,    54,    57,    56,    58,    59,    59,    59,
      59,    59,    59,    59,    59,    59,    61,    60,    62,    63,
      62,    65,    64,    66,    67,    66,    68,    66,    70,    71,
      69,    72,    73,    72,    75,    74,    77,    76,    79,    80,
      78,    81,    81,    82,    82,    83,    82,    84,    82,    85,
      85,    85,    85,    85,    85,    85,    85,    86,    87,    87,
      87,    87,    87,    87,    87,    87,    88,    88,    89,    89,
      89,    89,    89,    90,    90,    90,    90,    90,    90,    91,
      91,    92,    93,    93,    93,    93,    93,    94,    94,    95,
      95,    95,    95,    95,    96,    96,    97,    97,    98,    99,
      99,   100,   100,   101,   101,   102,   102,   103,   103,   103,
     103,   104,   104,   104,   104,   106,   107,   105,   108,   108,
     109,   108,   110,   108,   111,   108,   113,   112,   114,   115,
     115,   116,   116,   117,   117,   117,   118,   118,   119,   119,
     119,   119,   119,   120,   120,   120,   120,   120,   120,   121,
     122,   122,   122,   122,   123,   123,   123,   123,   124,   124,
     124,   125,   125,   126,   126,   127,   127,   127,   128,   128,
     128,   129,   129,   130,   131
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     0,     5,     4,     1,     2,     2,
       2,     2,     2,     2,     2,     2,     0,     5,     0,     0,
       6,     0,     5,     0,     0,     6,     0,     8,     0,     0,
       7,     0,     0,     3,     0,     5,     0,     5,     0,     0,
       8,     0,     4,     0,     5,     0,     4,     0,     4,     1,
       4,     4,     4,     5,     7,     7,     1,     5,     1,     4,
       5,     5,     5,     5,     1,     1,     4,     1,     1,     1,
       1,     1,     1,     4,     5,     5,     5,     5,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     0,     2,     1,
       4,     7,     5,     5,     0,     2,     4,     1,     4,     0,
       2,     1,     1,     1,     2,     1,     1,     0,     5,     3,
       2,     0,     5,     3,     2,     0,     0,     8,     0,     5,
       0,     4,     0,     4,     0,     4,     0,     6,     4,     1,
       4,     1,     2,     4,     4,     4,     0,     2,     4,     1,
       7,     5,     5,     4,     4,     4,     4,     5,     5,     5,
       5,     4,     1,     1,     1,     1,     1,     1,     5,     5,
       1,     4,     1,     1,     2,     5,     4,     4,     1,     1,
       1,     1,     1,     2,     3
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     0,     1,     0,     3,     0,     0,     4,     0,
       0,     0,     0,     7,     5,     0,     0,     0,     0,     0,
       0,     0,     0,     6,    28,    34,    36,    18,    23,   115,
      38,   126,    12,    14,     8,    10,     9,    11,    13,    15,
       0,   107,   107,     0,    16,     0,    21,     0,     0,     0,
      29,   107,     0,     0,   111,     0,   111,     0,   116,    39,
       0,     0,    31,     0,   110,   107,     0,    35,    37,   111,
       0,    17,     0,    22,    41,    41,   106,   105,   111,     0,
       0,    56,    49,    97,    32,     0,   173,     0,   109,   103,
       0,   114,   111,     0,    19,    24,     0,   118,    43,     0,
      71,    72,    70,    68,    69,    87,     0,     0,     0,    87,
       0,     0,    99,   127,    31,    30,   174,   104,   107,   113,
       0,    18,     0,    23,   111,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   128,    87,     0,     0,     0,   111,
       0,     0,   111,    79,    80,    81,     0,     0,    64,    58,
      65,    67,   101,   102,     0,    99,    33,   108,   111,    20,
      26,    25,     0,     0,   122,   129,     0,   120,   162,   111,
       0,   124,   139,   117,   111,    45,     0,    47,    89,    40,
      88,    50,    52,    96,     0,     0,    51,     0,     0,     0,
       0,     0,    99,     0,    98,   100,   112,    23,    42,     0,
       0,     0,     0,   118,     0,     0,   169,   168,   170,     0,
       0,   118,     0,     0,     0,     0,     0,    82,    83,    84,
     136,     0,     0,     0,   118,     0,    43,    85,    86,    94,
       0,     0,     0,     0,    43,     0,    53,     0,     0,    58,
       0,    58,     0,    58,     0,    58,     0,    57,    27,     0,
       0,     0,     0,     0,   131,   123,     0,     0,     0,     0,
     163,     0,   121,   118,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   136,     0,   111,     0,   125,    43,    46,
      94,     0,     0,     0,     0,   111,     0,    48,     0,     0,
       0,     0,     0,    78,    59,     0,    73,     0,     0,     0,
       0,     0,    66,   133,   134,   135,   130,   132,   166,   167,
     161,   164,   172,    58,     0,   119,     0,   143,   145,   144,
     146,   160,     0,     0,     0,   138,   137,     0,     0,   152,
       0,   153,     0,    44,    95,    90,     0,     0,     0,    55,
      54,    61,     0,     0,     0,     0,    75,    60,    74,    62,
      76,    63,    77,   165,     0,     0,   147,   148,   141,     0,
     154,     0,   156,   157,     0,   142,    92,     0,    93,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   149,     0,     0,   140,   151,     0,    91,   159,
     158,   150
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     5,    10,     8,    14,    15,    55,    44,
     121,    16,    57,    46,   123,   197,    17,    40,    62,    85,
     114,    18,    41,    19,    42,    20,    48,    75,    97,   133,
     226,   234,   135,    81,   331,   148,   111,   149,   150,   151,
     223,   136,   280,   281,    82,    83,   154,   155,    90,   112,
      52,    70,    21,    47,    74,   129,   211,   203,   224,    22,
      49,    61,   164,   253,   165,   272,   273,   172,   266,   332,
     364,   323,   167,   259,   260,   210,   314,    65,    66
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -207
static const yytype_int16 yypact[] =
{
    -207,    30,   -17,  -207,    61,  -207,    37,   122,  -207,    42,
     107,    53,   202,  -207,  -207,   107,   107,   107,   107,   107,
     107,   107,   107,  -207,  -207,  -207,  -207,    88,    92,  -207,
    -207,  -207,  -207,  -207,  -207,  -207,  -207,  -207,  -207,  -207,
      63,    99,    99,   113,  -207,   118,  -207,   126,   128,   127,
    -207,     6,   142,   143,   130,   144,   130,   146,  -207,  -207,
     -10,   137,   150,    -5,  -207,    99,   152,  -207,  -207,     3,
     151,  -207,   155,  -207,   162,   162,  -207,  -207,   130,   272,
     158,  -207,  -207,  -207,  -207,   165,  -207,   210,  -207,   152,
     167,  -207,   130,   152,  -207,   190,   172,    57,    15,   178,
    -207,  -207,    52,  -207,  -207,   137,   137,   173,   137,   137,
     180,   136,     8,  -207,   150,  -207,  -207,  -207,    99,  -207,
     181,    88,   187,    92,   130,   191,   192,   198,   208,   214,
     209,   137,   211,   215,  -207,   137,   217,   232,   236,   130,
     137,   237,   130,  -207,  -207,  -207,   197,   136,  -207,  -207,
    -207,  -207,  -207,  -207,   238,     8,  -207,  -207,   130,  -207,
    -207,  -207,   239,   100,  -207,  -207,   199,  -207,  -207,   130,
     259,  -207,  -207,  -207,   130,  -207,   228,  -207,  -207,  -207,
    -207,  -207,  -207,  -207,   240,   244,  -207,   245,   136,   136,
     136,   136,     8,   246,  -207,  -207,  -207,    92,  -207,   137,
     137,   137,   242,    57,   248,   248,  -207,  -207,  -207,   248,
     287,    57,   256,   258,   258,   106,   106,  -207,  -207,  -207,
     208,   191,   260,   106,    57,   261,    15,  -207,  -207,   211,
     265,   137,   279,   106,    15,   137,  -207,   137,   136,    25,
      89,   101,   136,    25,   136,    25,   280,  -207,  -207,   281,
     282,   283,    51,   284,   242,  -207,   241,   285,   286,   288,
     248,   136,  -207,    57,   228,   289,   290,   291,   292,   263,
     -13,   -13,   293,   208,   208,   130,    86,  -207,    15,  -207,
     211,   294,   -10,   236,   211,   130,   136,  -207,   295,   296,
     297,    91,   298,  -207,  -207,   299,  -207,   300,   301,   302,
     303,   304,  -207,  -207,  -207,  -207,  -207,  -207,  -207,  -207,
    -207,  -207,  -207,  -207,   305,  -207,   106,  -207,  -207,  -207,
    -207,  -207,   308,   307,   309,  -207,  -207,   310,   311,  -207,
     251,  -207,   312,  -207,  -207,  -207,   313,   314,   315,  -207,
    -207,  -207,    25,    25,    25,    25,  -207,  -207,  -207,  -207,
    -207,  -207,  -207,  -207,    86,    82,  -207,  -207,  -207,   208,
     136,    86,   136,   136,    86,  -207,  -207,   211,  -207,    25,
     101,    25,    25,   315,   316,   136,   337,   317,    89,   318,
      86,   319,  -207,   320,   321,  -207,  -207,   322,  -207,  -207,
    -207,  -207
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -207,  -207,  -207,  -207,  -207,  -207,   306,  -207,  -207,   188,
    -207,  -207,  -207,  -108,  -207,  -207,  -207,  -207,  -207,   216,
    -207,  -207,  -207,  -207,  -207,  -207,  -207,  -207,   323,  -206,
    -207,  -207,   -32,  -207,  -111,  -202,  -207,  -180,  -199,  -134,
    -170,   -85,  -129,    79,  -131,  -102,  -132,  -207,   -31,   324,
     -26,   -51,  -207,  -207,  -207,  -177,  -207,  -207,  -207,  -207,
    -207,  -207,   153,   121,  -183,   103,  -126,  -207,   163,  -176,
    -207,   108,  -207,   120,  -109,  -207,  -207,   325,   326
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -107
static const yytype_int16 yytable[] =
{
     147,   178,   171,   177,   138,    72,   233,   321,   239,   241,
     243,   245,   192,   270,   271,   161,    53,   168,    91,   254,
     279,   276,    76,   195,   141,    64,   255,    99,   287,    80,
       3,   286,    63,     4,   262,    63,   193,   322,    77,    88,
     293,   119,   293,    86,   293,    87,   293,   277,   143,   144,
     180,   130,    69,   131,    51,   132,   152,   153,   117,   292,
     246,   297,   120,   299,     6,   301,   199,   200,   201,   125,
     126,   254,   333,   162,   137,   291,   140,   238,   240,   242,
     244,   313,   178,   178,   265,   267,   315,     7,   184,   248,
      11,   187,   157,   127,   316,   257,   258,   128,   178,   175,
     329,  -106,   375,  -106,    23,   143,   144,   196,   185,   143,
     144,    50,   143,   144,   354,   199,   200,   201,   212,   342,
     343,   344,   345,   225,   143,   144,     9,   290,   283,   295,
     145,   298,   146,   300,   145,   192,   330,   145,    43,   146,
     294,   202,    45,   293,   293,   293,   293,    51,   327,   178,
     312,   291,   296,   178,   145,   336,   269,    12,    13,   143,
     144,    54,   369,   370,   371,   372,    56,   249,   250,   251,
     293,   293,   293,   293,    58,   338,    59,    60,   374,    69,
     239,   241,   243,   245,   145,   379,   146,    79,   380,   292,
     297,   299,   301,    67,    68,    71,   192,    73,    84,   284,
      89,    96,    94,   288,   387,   289,    95,    24,    25,   113,
      26,    27,    28,    29,   204,   205,   115,   116,   118,   122,
     206,   207,   124,   139,   328,   188,   189,   190,   191,   134,
     142,   208,   158,   377,   337,   160,   178,    30,   381,    31,
     209,   163,   166,   373,   376,   145,   227,   228,   169,   238,
     378,   242,   244,   217,   218,   219,   204,   205,   170,   174,
      76,   176,   206,   207,   383,   173,   179,   295,   181,   229,
     230,   231,   232,   208,   213,   214,    77,   215,   216,   360,
     361,   362,   363,   182,   217,   218,   219,   183,   186,   194,
     198,   235,   252,   100,   101,   236,   237,   247,   256,   145,
     220,   261,   221,   222,   102,   103,   104,   263,   264,   159,
     275,   145,   278,   105,   106,   282,   107,   108,   109,   110,
      77,    32,    33,    34,    35,    36,    37,    38,    39,   285,
     156,   302,   303,   304,   305,   306,   308,   309,   355,   310,
     317,   318,   319,   320,   325,   335,   339,   340,   341,   346,
     347,   348,   349,   350,   351,   352,   353,   384,   356,   334,
     357,   358,   359,   365,   366,   367,   368,   382,   385,   386,
     388,   389,   390,   391,   274,   307,   326,   268,     0,   324,
     311,     0,     0,     0,    78,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    92,    93,     0,     0,    98
};

static const yytype_int16 yycheck[] =
{
     111,   132,   128,   132,   106,    56,   176,    20,   188,   189,
     190,   191,   146,   215,   216,   123,    42,   126,    69,   202,
     226,   223,    32,   155,   109,    51,   203,    78,   234,    61,
       0,   233,    29,    50,   211,    29,   147,    50,    48,    65,
     239,    92,   241,    48,   243,    50,   245,   224,    23,    24,
     135,    36,    49,    38,    48,    40,    48,    49,    89,   239,
     192,   241,    93,   243,     3,   245,    15,    16,    17,    12,
      13,   254,   278,   124,   106,    50,   108,   188,   189,   190,
     191,   261,   213,   214,   213,   214,   263,    50,   139,   197,
      48,   142,   118,    36,   264,   204,   205,    40,   229,   131,
      14,    49,    20,    51,    51,    23,    24,   158,   140,    23,
      24,    48,    23,    24,   316,    15,    16,    17,   169,    28,
      29,    30,    31,   174,    23,    24,     4,   238,   230,   240,
      48,   242,    50,   244,    48,   269,    50,    48,    50,    50,
      51,    41,    50,   342,   343,   344,   345,    48,   274,   280,
     261,    50,    51,   284,    48,   284,    50,    50,    51,    23,
      24,    48,   342,   343,   344,   345,    48,   199,   200,   201,
     369,   370,   371,   372,    48,   286,    48,    50,   354,    49,
     360,   361,   362,   363,    48,   361,    50,    50,   364,   369,
     370,   371,   372,    51,    51,    51,   330,    51,    48,   231,
      48,    39,    51,   235,   380,   237,    51,     5,     6,    51,
       8,     9,    10,    11,    15,    16,    51,     7,    51,    29,
      21,    22,    50,    50,   275,    28,    29,    30,    31,    51,
      50,    32,    51,   359,   285,    48,   367,    35,   367,    37,
      41,    50,    50,   354,   355,    48,    18,    19,    50,   360,
     361,   362,   363,    25,    26,    27,    15,    16,    50,    50,
      32,    50,    21,    22,   375,    51,    51,   378,    51,    41,
      42,    43,    44,    32,    15,    16,    48,    18,    19,    28,
      29,    30,    31,    51,    25,    26,    27,    51,    51,    51,
      51,    51,    50,    21,    22,    51,    51,    51,    50,    48,
      41,    14,    43,    44,    32,    33,    34,    51,    50,   121,
      50,    48,    51,    41,    42,    50,    44,    45,    46,    47,
      48,    15,    16,    17,    18,    19,    20,    21,    22,    50,
     114,    51,    51,    51,    51,    51,    51,    51,    30,    51,
      51,    51,    51,    51,    51,    51,    51,    51,    51,    51,
      51,    51,    51,    51,    51,    51,    51,    20,    51,   280,
      51,    51,    51,    51,    51,    51,    51,    51,    51,    51,
      51,    51,    51,    51,   221,   254,   273,   214,    -1,   271,
     260,    -1,    -1,    -1,    60,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    69,    69,    -1,    -1,    75
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    54,    55,     0,    50,    56,     3,    50,    58,     4,
      57,    48,    50,    51,    59,    60,    64,    69,    74,    76,
      78,   105,   112,    51,     5,     6,     8,     9,    10,    11,
      35,    37,    59,    59,    59,    59,    59,    59,    59,    59,
      70,    75,    77,    50,    62,    50,    66,   106,    79,   113,
      48,    48,   103,   103,    48,    61,    48,    65,    48,    48,
      50,   114,    71,    29,   103,   130,   131,    51,    51,    49,
     104,    51,   104,    51,   107,    80,    32,    48,   102,    50,
      85,    86,    97,    98,    48,    72,    48,    50,   103,    48,
     101,   104,   130,   131,    51,    51,    39,    81,    81,   104,
      21,    22,    32,    33,    34,    41,    42,    44,    45,    46,
      47,    89,   102,    51,    73,    51,     7,   101,    51,   104,
     101,    63,    29,    67,    50,    12,    13,    36,    40,   108,
      36,    38,    40,    82,    51,    85,    94,    85,    98,    50,
      85,    94,    50,    23,    24,    48,    50,    87,    88,    90,
      91,    92,    48,    49,    99,   100,    72,   103,    51,    62,
      48,    66,   104,    50,   115,   117,    50,   125,   127,    50,
      50,   119,   120,    51,    50,    85,    50,    95,    97,    51,
      94,    51,    51,    51,   104,    85,    51,   104,    28,    29,
      30,    31,    92,    87,    51,    99,   104,    68,    51,    15,
      16,    17,    41,   110,    15,    16,    21,    22,    32,    41,
     128,   109,   104,    15,    16,    18,    19,    25,    26,    27,
      41,    43,    44,    93,   111,   104,    83,    18,    19,    41,
      42,    43,    44,    93,    84,    51,    51,    51,    87,    90,
      87,    90,    87,    90,    87,    90,    99,    51,    66,    85,
      85,    85,    50,   116,   117,   108,    50,   127,   127,   126,
     127,    14,   108,    51,    50,    95,   121,    95,   121,    50,
      88,    88,   118,   119,   115,    50,    88,   108,    51,    82,
      95,    96,    50,    98,    85,    50,    88,    82,    85,    85,
      87,    50,    90,    91,    51,    87,    51,    90,    87,    90,
      87,    90,    51,    51,    51,    51,    51,   116,    51,    51,
      51,   126,    87,    90,   129,   108,    93,    51,    51,    51,
      51,    20,    50,   124,   124,    51,   118,   119,   104,    14,
      50,    87,   122,    82,    96,    51,    95,   104,    87,    51,
      51,    51,    28,    29,    30,    31,    51,    51,    51,    51,
      51,    51,    51,    51,    88,    30,    51,    51,    51,    51,
      28,    29,    30,    31,   123,    51,    51,    51,    51,    90,
      90,    90,    90,    87,   122,    20,    87,   119,    87,   122,
     122,    95,    51,    87,    20,    51,    51,   122,    51,    51,
      51,    51
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
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



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

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
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
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
      if (yyn == 0 || yyn == YYTABLE_NINF)
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

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

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
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1455 of yacc.c  */
#line 294 "scan-ops_pddl.y"
    { 
  opserr( DOMDEF_EXPECTED, NULL );
;}
    break;

  case 4:

/* Line 1455 of yacc.c  */
#line 305 "scan-ops_pddl.y"
    { 
;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 308 "scan-ops_pddl.y"
    {
  static int once=0;
  if ( gcmd_line.display_info >= 1 && once==0) {
    printf(" domain '%s' defined", gdomain_name);
    once=1;
  }
;}
    break;

  case 6:

/* Line 1455 of yacc.c  */
#line 321 "scan-ops_pddl.y"
    { 
  gdomain_name = new_Token( strlen((yyvsp[(3) - (4)].string))+1 );
  strcpy( gdomain_name, (yyvsp[(3) - (4)].string));
;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 357 "scan-ops_pddl.y"
    {
;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 360 "scan-ops_pddl.y"
    { 
;}
    break;

  case 18:

/* Line 1455 of yacc.c  */
#line 366 "scan-ops_pddl.y"
    {;}
    break;

  case 19:

/* Line 1455 of yacc.c  */
#line 369 "scan-ops_pddl.y"
    {

  TypedListList *tll;

  if ( gparse_predicates ) {
    tll = gparse_predicates;
    while ( tll->next ) {
      tll = tll->next;
    }
    tll->next = new_TypedListList();
    tll = tll->next;
  } else {
    tll = new_TypedListList();
    gparse_predicates = tll;
  }

  tll->predicate = new_Token( strlen( (yyvsp[(2) - (4)].string) ) + 1);
  strcpy( tll->predicate, (yyvsp[(2) - (4)].string) );

  tll->args = (yyvsp[(3) - (4)].pTypedList);

;}
    break;

  case 21:

/* Line 1455 of yacc.c  */
#line 399 "scan-ops_pddl.y"
    {
;}
    break;

  case 22:

/* Line 1455 of yacc.c  */
#line 402 "scan-ops_pddl.y"
    { 
;}
    break;

  case 23:

/* Line 1455 of yacc.c  */
#line 408 "scan-ops_pddl.y"
    {;}
    break;

  case 24:

/* Line 1455 of yacc.c  */
#line 411 "scan-ops_pddl.y"
    {

  TypedListList *tll;

  if ( gparse_functions ) {
    tll = gparse_functions;
    while ( tll->next ) {
      tll = tll->next;
    }
    tll->next = new_TypedListList();
    tll = tll->next;
  } else {
    tll = new_TypedListList();
    gparse_functions = tll;
  }

  tll->predicate = new_Token( strlen( (yyvsp[(2) - (4)].string) ) + 1);
  strcpy( tll->predicate, (yyvsp[(2) - (4)].string) );

  tll->args = (yyvsp[(3) - (4)].pTypedList);

;}
    break;

  case 26:

/* Line 1455 of yacc.c  */
#line 436 "scan-ops_pddl.y"
    {

  TypedListList *tll;

  if ( gparse_functions ) {
    tll = gparse_functions;
    while ( tll->next ) {
      tll = tll->next;
    }
    tll->next = new_TypedListList();
    tll = tll->next;
  } else {
    tll = new_TypedListList();
    gparse_functions = tll;
  }

  tll->predicate = new_Token( strlen( (yyvsp[(2) - (6)].string) ) + 1);
  strcpy( tll->predicate, (yyvsp[(2) - (6)].string) );

  tll->args = (yyvsp[(3) - (6)].pTypedList);

;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 465 "scan-ops_pddl.y"
    {
  opserr( REQUIREM_EXPECTED, NULL );
;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 469 "scan-ops_pddl.y"
    { 
  if ( !supported( (yyvsp[(4) - (4)].string) ) ) {
    opserr( NOT_SUPPORTED, (yyvsp[(4) - (4)].string) );
    yyerror(NULL);
  }
;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 484 "scan-ops_pddl.y"
    { 
  if ( !supported( (yyvsp[(1) - (1)].string) ) ) {
    opserr( NOT_SUPPORTED, (yyvsp[(1) - (1)].string) );
    yyerror(NULL);
  }
;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 497 "scan-ops_pddl.y"
    { 
  opserr( TYPEDEF_EXPECTED, NULL ); 
;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 501 "scan-ops_pddl.y"
    {
  gparse_types = (yyvsp[(4) - (5)].pTypedList);
;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 510 "scan-ops_pddl.y"
    { 
  opserr( CONSTLIST_EXPECTED, NULL ); 
;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 514 "scan-ops_pddl.y"
    {
  gparse_constants = (yyvsp[(4) - (5)].pTypedList);
;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 525 "scan-ops_pddl.y"
    { 
#if YYDEBUG != 0
  printf("\n\nin action_def rule\n\n\n"); 
#endif
  opserr( ACTION, NULL );
;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 532 "scan-ops_pddl.y"
    { 
  scur_op = new_PlOperator( (yyvsp[(4) - (4)].string) );
;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 536 "scan-ops_pddl.y"
    {
  scur_op->next = gloaded_ops;
  gloaded_ops = scur_op; 
;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 546 "scan-ops_pddl.y"
    { 
  scur_op->params = NULL; 
;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 551 "scan-ops_pddl.y"
    {
  TypedList *tl;
  scur_op->parse_params = (yyvsp[(3) - (4)].pTypedList);
  for (tl = scur_op->parse_params; tl; tl = tl->next) {
    /* to be able to distinguish params from :VARS 
     */
    scur_op->number_of_real_params++;
  }
;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 567 "scan-ops_pddl.y"
    {
  TypedList *tl = NULL;

  /* add vars as parameters 
   */
  if ( scur_op->parse_params ) {
    for( tl = scur_op->parse_params; tl->next; tl = tl->next ) {
      /* empty, get to the end of list 
       */
    }
    tl->next = (yyvsp[(3) - (5)].pTypedList);
    tl = tl->next;
  } else {
    scur_op->parse_params = (yyvsp[(3) - (5)].pTypedList);
  }
;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 585 "scan-ops_pddl.y"
    { 
  scur_op->preconds = (yyvsp[(2) - (2)].pPlNode); 
;}
    break;

  case 47:

/* Line 1455 of yacc.c  */
#line 591 "scan-ops_pddl.y"
    { 
  scur_op->effects = (yyvsp[(2) - (2)].pPlNode); 
;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 606 "scan-ops_pddl.y"
    { 
  if ( sis_negated ) {
    (yyval.pPlNode) = new_PlNode(NOT);
    (yyval.pPlNode)->sons = new_PlNode(ATOM);
    (yyval.pPlNode)->sons->atom = (yyvsp[(1) - (1)].pTokenList);
    sis_negated = FALSE;
  } else {
    (yyval.pPlNode) = new_PlNode(ATOM);
    (yyval.pPlNode)->atom = (yyvsp[(1) - (1)].pTokenList);
  }
;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 619 "scan-ops_pddl.y"
    { 
  (yyval.pPlNode) = new_PlNode(AND);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 625 "scan-ops_pddl.y"
    { 
  (yyval.pPlNode) = new_PlNode(OR);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 631 "scan-ops_pddl.y"
    { 
  (yyval.pPlNode) = new_PlNode(NOT);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 637 "scan-ops_pddl.y"
    { 
  PlNode *np = new_PlNode(NOT);
  np->sons = (yyvsp[(3) - (5)].pPlNode);
  np->next = (yyvsp[(4) - (5)].pPlNode);

  (yyval.pPlNode) = new_PlNode(OR);
  (yyval.pPlNode)->sons = np;
;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 649 "scan-ops_pddl.y"
    { 

  PlNode *pln;

  pln = new_PlNode(EX);
  pln->parse_vars = (yyvsp[(4) - (7)].pTypedList);

  (yyval.pPlNode) = pln;
  pln->sons = (yyvsp[(6) - (7)].pPlNode);

;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 664 "scan-ops_pddl.y"
    { 

  PlNode *pln;

  pln = new_PlNode(ALL);
  pln->parse_vars = (yyvsp[(4) - (7)].pTypedList);

  (yyval.pPlNode) = pln;
  pln->sons = (yyvsp[(6) - (7)].pPlNode);

;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 689 "scan-ops_pddl.y"
    { 
  (yyval.pPlNode) = new_PlNode(BIN_COMP);
  (yyval.pPlNode)->sons = (yyvsp[(2) - (5)].pPlNode);
  (yyval.pPlNode)->sons->sons= (yyvsp[(3) - (5)].pPlNode);
  (yyval.pPlNode)->sons->sons->next= (yyvsp[(4) - (5)].pPlNode);
;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 700 "scan-ops_pddl.y"
    {
       (yyval.pPlNode)=new_PlNode(NUM_EXP);
       (yyval.pPlNode)->sons = (yyvsp[(1) - (1)].pPlNode);
;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 706 "scan-ops_pddl.y"
    { 
  PlNode *pln;

  (yyval.pPlNode)=new_PlNode(F_EXP);
  pln=new_PlNode(UMINUS_CONN);
  (yyval.pPlNode)->sons = pln;
  (yyval.pPlNode)->sons->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 716 "scan-ops_pddl.y"
    { 
  PlNode *pln;

  (yyval.pPlNode)=new_PlNode(F_EXP);
  pln=new_PlNode(MINUS_CONN); 
  (yyval.pPlNode)->sons = pln;
  (yyval.pPlNode)->sons->sons = (yyvsp[(3) - (5)].pPlNode);
  (yyval.pPlNode)->sons->sons->next = (yyvsp[(4) - (5)].pPlNode);

;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 728 "scan-ops_pddl.y"
    { 
  PlNode *pln;

  (yyval.pPlNode)=new_PlNode(F_EXP);
  pln=new_PlNode(PLUS_CONN); 
  (yyval.pPlNode)->sons = pln;
  (yyval.pPlNode)->sons->sons = (yyvsp[(3) - (5)].pPlNode);
  (yyval.pPlNode)->sons->sons->next = (yyvsp[(4) - (5)].pPlNode);


;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 741 "scan-ops_pddl.y"
    { 
  PlNode *pln;

  (yyval.pPlNode)=new_PlNode(F_EXP);
  pln=new_PlNode(MUL_CONN); 
  (yyval.pPlNode)->sons = pln;
  (yyval.pPlNode)->sons->sons = (yyvsp[(3) - (5)].pPlNode);
  (yyval.pPlNode)->sons->sons->next = (yyvsp[(4) - (5)].pPlNode);

;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 753 "scan-ops_pddl.y"
    { 
  PlNode *pln;

  (yyval.pPlNode)=new_PlNode(F_EXP);
  pln=new_PlNode(DIV_CONN); 
  (yyval.pPlNode)->sons = pln;
  (yyval.pPlNode)->sons->sons = (yyvsp[(3) - (5)].pPlNode);
  (yyval.pPlNode)->sons->sons->next = (yyvsp[(4) - (5)].pPlNode);

;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 765 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(FN_HEAD);
  (yyval.pPlNode)->atom = (yyvsp[(1) - (1)].pTokenList);
;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 772 "scan-ops_pddl.y"
    {
    (yyval.pPlNode)=new_PlNode(ATOM);
    (yyval.pPlNode)->atom = (yyvsp[(1) - (1)].pTokenList);
;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 783 "scan-ops_pddl.y"
    { 
  (yyval.pTokenList) = new_TokenList();
  (yyval.pTokenList)->item = (yyvsp[(2) - (4)].pstring);
  (yyval.pTokenList)->next = (yyvsp[(3) - (4)].pTokenList);
;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 790 "scan-ops_pddl.y"
    { 
  (yyval.pTokenList) = new_TokenList();
  (yyval.pTokenList)->item = (yyvsp[(1) - (1)].pstring);
;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 800 "scan-ops_pddl.y"
    {
  (yyval.pPlNode)=new_PlNode(GREATER_THAN_CONN);
;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 805 "scan-ops_pddl.y"
    {
  (yyval.pPlNode)=new_PlNode(LESS_THAN_CONN);
;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 810 "scan-ops_pddl.y"
    {
  (yyval.pPlNode)=new_PlNode(EQUAL_CONN);
;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 815 "scan-ops_pddl.y"
    {
  (yyval.pPlNode)=new_PlNode(GREATER_OR_EQUAL_CONN);
;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 820 "scan-ops_pddl.y"
    {
  (yyval.pPlNode)=new_PlNode(LESS_THAN_OR_EQUAL_CONN);
;}
    break;

  case 73:

/* Line 1455 of yacc.c  */
#line 827 "scan-ops_pddl.y"
    {
  (yyval.pPlNode)=new_PlNode(UMINUS_CONN);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
   
;}
    break;

  case 74:

/* Line 1455 of yacc.c  */
#line 834 "scan-ops_pddl.y"
    {
  (yyval.pPlNode)=new_PlNode(MINUS_CONN);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (5)].pPlNode);
  (yyval.pPlNode)->sons->next = (yyvsp[(4) - (5)].pPlNode);
;}
    break;

  case 75:

/* Line 1455 of yacc.c  */
#line 841 "scan-ops_pddl.y"
    {
  (yyval.pPlNode)=new_PlNode(PLUS_CONN);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (5)].pPlNode);
  (yyval.pPlNode)->sons->next = (yyvsp[(4) - (5)].pPlNode);
;}
    break;

  case 76:

/* Line 1455 of yacc.c  */
#line 848 "scan-ops_pddl.y"
    {
  (yyval.pPlNode)=new_PlNode(MUL_CONN);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (5)].pPlNode);
  (yyval.pPlNode)->sons->next = (yyvsp[(4) - (5)].pPlNode);
;}
    break;

  case 77:

/* Line 1455 of yacc.c  */
#line 855 "scan-ops_pddl.y"
    {
  (yyval.pPlNode)=new_PlNode(DIV_CONN);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (5)].pPlNode);
  (yyval.pPlNode)->sons->next = (yyvsp[(4) - (5)].pPlNode);
;}
    break;

  case 78:

/* Line 1455 of yacc.c  */
#line 862 "scan-ops_pddl.y"
    {
    (yyval.pPlNode)=new_PlNode(ATOM);
    (yyval.pPlNode)->atom = (yyvsp[(1) - (1)].pTokenList);
;}
    break;

  case 79:

/* Line 1455 of yacc.c  */
#line 870 "scan-ops_pddl.y"
    {
  Token t;
  t = new_Token( strlen((yyvsp[(1) - (1)].string))+1 );
  strcpy (t, (yyvsp[(1) - (1)].string));
  (yyval.pTokenList) = new_TokenList();
  (yyval.pTokenList)->item = t;

;}
    break;

  case 80:

/* Line 1455 of yacc.c  */
#line 880 "scan-ops_pddl.y"
    {
  Token t;
  t = new_Token( strlen((yyvsp[(1) - (1)].string))+1 );
  strcpy (t, (yyvsp[(1) - (1)].string));
  (yyval.pTokenList) = new_TokenList();
  (yyval.pTokenList)->item = t;

;}
    break;

  case 81:

/* Line 1455 of yacc.c  */
#line 892 "scan-ops_pddl.y"
    { 
  (yyval.pstring) = new_Token( strlen((yyvsp[(1) - (1)].string))+1 );
  strcpy( (yyval.pstring), (yyvsp[(1) - (1)].string) );
;}
    break;

  case 82:

/* Line 1455 of yacc.c  */
#line 936 "scan-ops_pddl.y"
    {
  (yyval.pPlNode)=new_PlNode(ASSIGN_CONN);
;}
    break;

  case 83:

/* Line 1455 of yacc.c  */
#line 941 "scan-ops_pddl.y"
    {
  (yyval.pPlNode)=new_PlNode(SCALE_UP_CONN);
;}
    break;

  case 84:

/* Line 1455 of yacc.c  */
#line 946 "scan-ops_pddl.y"
    {
  (yyval.pPlNode)=new_PlNode(SCALE_DOWN_CONN);
;}
    break;

  case 85:

/* Line 1455 of yacc.c  */
#line 951 "scan-ops_pddl.y"
    {
  (yyval.pPlNode)=new_PlNode(INCREASE_CONN);
;}
    break;

  case 86:

/* Line 1455 of yacc.c  */
#line 956 "scan-ops_pddl.y"
    {
  (yyval.pPlNode)=new_PlNode(DECREASE_CONN);
;}
    break;

  case 87:

/* Line 1455 of yacc.c  */
#line 966 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = NULL;
;}
    break;

  case 88:

/* Line 1455 of yacc.c  */
#line 971 "scan-ops_pddl.y"
    {
  (yyvsp[(1) - (2)].pPlNode)->next = (yyvsp[(2) - (2)].pPlNode);
  (yyval.pPlNode) = (yyvsp[(1) - (2)].pPlNode);
;}
    break;

  case 89:

/* Line 1455 of yacc.c  */
#line 983 "scan-ops_pddl.y"
    { 
  if ( sis_negated ) {
    (yyval.pPlNode) = new_PlNode(NOT);
    (yyval.pPlNode)->sons = new_PlNode(ATOM);
    (yyval.pPlNode)->sons->atom = (yyvsp[(1) - (1)].pTokenList);
    sis_negated = FALSE;
  } else {
    (yyval.pPlNode) = new_PlNode(ATOM);
    (yyval.pPlNode)->atom = (yyvsp[(1) - (1)].pTokenList);
  }
;}
    break;

  case 90:

/* Line 1455 of yacc.c  */
#line 996 "scan-ops_pddl.y"
    { 
  (yyval.pPlNode) = new_PlNode(AND);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 91:

/* Line 1455 of yacc.c  */
#line 1004 "scan-ops_pddl.y"
    { 

  PlNode *pln;

  pln = new_PlNode(ALL);
  pln->parse_vars = (yyvsp[(4) - (7)].pTypedList);

  (yyval.pPlNode) = pln;
  pln->sons = (yyvsp[(6) - (7)].pPlNode);

;}
    break;

  case 92:

/* Line 1455 of yacc.c  */
#line 1017 "scan-ops_pddl.y"
    {
  /* This will be conditional effects in FF representation, but here
   * a formula like (WHEN p q) will be saved as:
   *  [WHEN]
   *  [sons]
   *   /  \
   * [p]  [q]
   * That means, the first son is p, and the second one is q. 
   */
  (yyval.pPlNode) = new_PlNode(WHEN);
  (yyvsp[(3) - (5)].pPlNode)->next = (yyvsp[(4) - (5)].pPlNode);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (5)].pPlNode);
;}
    break;

  case 93:

/* Line 1455 of yacc.c  */
#line 1032 "scan-ops_pddl.y"
    {
  PlNode* pln;
  (yyval.pPlNode)=(yyvsp[(2) - (5)].pPlNode);
  pln = new_PlNode(FN_HEAD);
  (yyval.pPlNode)->sons = pln;
  (yyval.pPlNode)->sons->atom = (yyvsp[(3) - (5)].pTokenList);
  (yyval.pPlNode)->sons->next = (yyvsp[(4) - (5)].pPlNode);
;}
    break;

  case 94:

/* Line 1455 of yacc.c  */
#line 1045 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = NULL;
;}
    break;

  case 95:

/* Line 1455 of yacc.c  */
#line 1050 "scan-ops_pddl.y"
    {
  (yyvsp[(1) - (2)].pPlNode)->next = (yyvsp[(2) - (2)].pPlNode);
  (yyval.pPlNode) = (yyvsp[(1) - (2)].pPlNode);
;}
    break;

  case 96:

/* Line 1455 of yacc.c  */
#line 1062 "scan-ops_pddl.y"
    { 
  (yyval.pTokenList) = (yyvsp[(3) - (4)].pTokenList);
  sis_negated = TRUE;
;}
    break;

  case 97:

/* Line 1455 of yacc.c  */
#line 1068 "scan-ops_pddl.y"
    {
  (yyval.pTokenList) = (yyvsp[(1) - (1)].pTokenList);
;}
    break;

  case 98:

/* Line 1455 of yacc.c  */
#line 1077 "scan-ops_pddl.y"
    { 
  (yyval.pTokenList) = new_TokenList();
  (yyval.pTokenList)->item = (yyvsp[(2) - (4)].pstring);
  (yyval.pTokenList)->next = (yyvsp[(3) - (4)].pTokenList);
;}
    break;

  case 99:

/* Line 1455 of yacc.c  */
#line 1088 "scan-ops_pddl.y"
    { (yyval.pTokenList) = NULL; ;}
    break;

  case 100:

/* Line 1455 of yacc.c  */
#line 1091 "scan-ops_pddl.y"
    {
  (yyval.pTokenList) = new_TokenList();
  (yyval.pTokenList)->item = (yyvsp[(1) - (2)].pstring);
  (yyval.pTokenList)->next = (yyvsp[(2) - (2)].pTokenList);
;}
    break;

  case 101:

/* Line 1455 of yacc.c  */
#line 1102 "scan-ops_pddl.y"
    { 
  (yyval.pstring) = new_Token( strlen((yyvsp[(1) - (1)].string))+1 );
  strcpy( (yyval.pstring), (yyvsp[(1) - (1)].string) );
;}
    break;

  case 102:

/* Line 1455 of yacc.c  */
#line 1108 "scan-ops_pddl.y"
    { 
  (yyval.pstring) = new_Token( strlen((yyvsp[(1) - (1)].string))+1 );
  strcpy( (yyval.pstring), (yyvsp[(1) - (1)].string) );
;}
    break;

  case 103:

/* Line 1455 of yacc.c  */
#line 1118 "scan-ops_pddl.y"
    {
  (yyval.pTokenList) = new_TokenList();
  (yyval.pTokenList)->item = new_Token( strlen((yyvsp[(1) - (1)].string))+1 );
  strcpy( (yyval.pTokenList)->item, (yyvsp[(1) - (1)].string) );
;}
    break;

  case 104:

/* Line 1455 of yacc.c  */
#line 1125 "scan-ops_pddl.y"
    {
  (yyval.pTokenList) = new_TokenList();
  (yyval.pTokenList)->item = new_Token( strlen((yyvsp[(1) - (2)].string))+1 );
  strcpy( (yyval.pTokenList)->item, (yyvsp[(1) - (2)].string) );
  (yyval.pTokenList)->next = (yyvsp[(2) - (2)].pTokenList);
;}
    break;

  case 105:

/* Line 1455 of yacc.c  */
#line 1136 "scan-ops_pddl.y"
    { 
  (yyval.pstring) = new_Token( strlen((yyvsp[(1) - (1)].string))+1 );
  strcpy( (yyval.pstring), (yyvsp[(1) - (1)].string) );
;}
    break;

  case 106:

/* Line 1455 of yacc.c  */
#line 1142 "scan-ops_pddl.y"
    { 
  (yyval.pstring) = new_Token( strlen(EQ_STR)+1 );
  strcpy( (yyval.pstring), EQ_STR );
;}
    break;

  case 107:

/* Line 1455 of yacc.c  */
#line 1152 "scan-ops_pddl.y"
    { (yyval.pTypedList) = NULL; ;}
    break;

  case 108:

/* Line 1455 of yacc.c  */
#line 1158 "scan-ops_pddl.y"
    { 

  (yyval.pTypedList) = new_TypedList();
  (yyval.pTypedList)->name = new_Token( strlen((yyvsp[(1) - (5)].string))+1 );
  strcpy( (yyval.pTypedList)->name, (yyvsp[(1) - (5)].string) );
  (yyval.pTypedList)->type = (yyvsp[(3) - (5)].pTokenList);
  (yyval.pTypedList)->next = (yyvsp[(5) - (5)].pTypedList);
;}
    break;

  case 109:

/* Line 1455 of yacc.c  */
#line 1168 "scan-ops_pddl.y"
    {
  (yyval.pTypedList) = new_TypedList();
  (yyval.pTypedList)->name = new_Token( strlen((yyvsp[(1) - (3)].string))+1 );
  strcpy( (yyval.pTypedList)->name, (yyvsp[(1) - (3)].string) );
  (yyval.pTypedList)->type = new_TokenList();
  (yyval.pTypedList)->type->item = new_Token( strlen((yyvsp[(2) - (3)].pstring))+1 );
  strcpy( (yyval.pTypedList)->type->item, (yyvsp[(2) - (3)].pstring) );
  (yyval.pTypedList)->next = (yyvsp[(3) - (3)].pTypedList);
;}
    break;

  case 110:

/* Line 1455 of yacc.c  */
#line 1179 "scan-ops_pddl.y"
    {
  (yyval.pTypedList) = new_TypedList();
  (yyval.pTypedList)->name = new_Token( strlen((yyvsp[(1) - (2)].string))+1 );
  strcpy( (yyval.pTypedList)->name, (yyvsp[(1) - (2)].string) );
  if ( (yyvsp[(2) - (2)].pTypedList) ) {/* another element (already typed) is following */
    (yyval.pTypedList)->type = copy_TokenList( (yyvsp[(2) - (2)].pTypedList)->type );
  } else {/* no further element - it must be an untyped list */
    (yyval.pTypedList)->type = new_TokenList();
    (yyval.pTypedList)->type->item = new_Token( strlen(STANDARD_TYPE)+1 );
    strcpy( (yyval.pTypedList)->type->item, STANDARD_TYPE );
  }
  (yyval.pTypedList)->next = (yyvsp[(2) - (2)].pTypedList);
;}
    break;

  case 111:

/* Line 1455 of yacc.c  */
#line 1198 "scan-ops_pddl.y"
    { (yyval.pTypedList) = NULL; ;}
    break;

  case 112:

/* Line 1455 of yacc.c  */
#line 1204 "scan-ops_pddl.y"
    {
  (yyval.pTypedList) = new_TypedList();
  (yyval.pTypedList)->name = new_Token( strlen((yyvsp[(1) - (5)].string))+1 );
  strcpy( (yyval.pTypedList)->name, (yyvsp[(1) - (5)].string) );
  (yyval.pTypedList)->type = (yyvsp[(3) - (5)].pTokenList);
  (yyval.pTypedList)->next = (yyvsp[(5) - (5)].pTypedList);
;}
    break;

  case 113:

/* Line 1455 of yacc.c  */
#line 1216 "scan-ops_pddl.y"
    {
  (yyval.pTypedList) = new_TypedList();
  (yyval.pTypedList)->name = new_Token( strlen((yyvsp[(1) - (3)].string))+1 );
  strcpy( (yyval.pTypedList)->name, (yyvsp[(1) - (3)].string) );
  (yyval.pTypedList)->type = new_TokenList();
  (yyval.pTypedList)->type->item = new_Token( strlen((yyvsp[(2) - (3)].pstring))+1 );
  strcpy( (yyval.pTypedList)->type->item, (yyvsp[(2) - (3)].pstring) );
  (yyval.pTypedList)->next = (yyvsp[(3) - (3)].pTypedList);
;}
    break;

  case 114:

/* Line 1455 of yacc.c  */
#line 1227 "scan-ops_pddl.y"
    {
  (yyval.pTypedList) = new_TypedList();
  (yyval.pTypedList)->name = new_Token( strlen((yyvsp[(1) - (2)].string))+1 );
  strcpy( (yyval.pTypedList)->name, (yyvsp[(1) - (2)].string) );
  if ( (yyvsp[(2) - (2)].pTypedList) ) {/* another element (already typed) is following */
    (yyval.pTypedList)->type = copy_TokenList( (yyvsp[(2) - (2)].pTypedList)->type );
  } else {/* no further element - it must be an untyped list */
    (yyval.pTypedList)->type = new_TokenList();
    (yyval.pTypedList)->type->item = new_Token( strlen(STANDARD_TYPE)+1 );
    strcpy( (yyval.pTypedList)->type->item, STANDARD_TYPE );
  }
  (yyval.pTypedList)->next = (yyvsp[(2) - (2)].pTypedList);
;}
    break;

  case 115:

/* Line 1455 of yacc.c  */
#line 1245 "scan-ops_pddl.y"
    { 
  opserr( ACTION, NULL ); 
;}
    break;

  case 116:

/* Line 1455 of yacc.c  */
#line 1249 "scan-ops_pddl.y"
    {
  scur_op = new_PlOperator( (yyvsp[(4) - (4)].string) );
;}
    break;

  case 117:

/* Line 1455 of yacc.c  */
#line 1253 "scan-ops_pddl.y"
    {
  scur_op->next = gloaded_ops;
  gloaded_ops = scur_op;
;}
    break;

  case 119:

/* Line 1455 of yacc.c  */
#line 1263 "scan-ops_pddl.y"
    {
  TypedList *tl = NULL;

  /* add vars as parameters
   */
  if ( scur_op->parse_params ) {
    for( tl = scur_op->parse_params; tl->next; tl = tl->next ) {
      /* empty, get to the end of list
       */
    }
    tl->next = (yyvsp[(3) - (5)].pTypedList);
    tl = tl->next;
  } else {
    scur_op->parse_params = (yyvsp[(3) - (5)].pTypedList);
  }
;}
    break;

  case 120:

/* Line 1455 of yacc.c  */
#line 1281 "scan-ops_pddl.y"
    {

  scur_op->duration = (yyvsp[(2) - (2)].pPlNode);

;}
    break;

  case 122:

/* Line 1455 of yacc.c  */
#line 1289 "scan-ops_pddl.y"
    {
  scur_op->preconds = (yyvsp[(2) - (2)].pPlNode);
;}
    break;

  case 124:

/* Line 1455 of yacc.c  */
#line 1295 "scan-ops_pddl.y"
    {
  scur_op->effects = (yyvsp[(2) - (2)].pPlNode);
;}
    break;

  case 126:

/* Line 1455 of yacc.c  */
#line 1305 "scan-ops_pddl.y"
    {
  opserr(DERIVED_PRED_EXPECTED, NULL);
;}
    break;

  case 127:

/* Line 1455 of yacc.c  */
#line 1309 "scan-ops_pddl.y"
    {
  der_op = new_PlOperator((yyvsp[(4) - (6)].pPlNode) -> atom -> item);
  der_op -> parse_params = (yyvsp[(4) - (6)].pPlNode) -> parse_vars;
  (yyvsp[(4) - (6)].pPlNode) -> parse_vars = NULL;
  der_op -> effects  = (yyvsp[(4) - (6)].pPlNode);
  der_op -> preconds = (yyvsp[(5) - (6)].pPlNode);
  der_op -> next = gderived_predicates;
  gderived_predicates = der_op;
  gnum_derived_predicates++;
;}
    break;

  case 128:

/* Line 1455 of yacc.c  */
#line 1342 "scan-ops_pddl.y"
    {
  PlNode *pln;
  TokenList *a;
  TypedList *t;
  pln = new_PlNode(ATOM);
  pln -> atom = new_TokenList();
  pln -> atom -> item = (yyvsp[(2) - (4)].pstring);
  pln -> parse_vars = (yyvsp[(3) - (4)].pTypedList);
  for (a = pln -> atom, t = (yyvsp[(3) - (4)].pTypedList); t; t = t -> next) {
    a -> next = new_TokenList();
    a = a -> next;
    a -> item = (char *) calloc(strlen(t -> name) + 1 ,sizeof(char));
    strcpy(a -> item, t -> name);
  }

  (yyval.pPlNode) = pln;
;}
    break;

  case 129:

/* Line 1455 of yacc.c  */
#line 1370 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = (yyvsp[(1) - (1)].pPlNode);
;}
    break;

  case 130:

/* Line 1455 of yacc.c  */
#line 1375 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(AND);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 131:

/* Line 1455 of yacc.c  */
#line 1383 "scan-ops_pddl.y"
    {
  (yyval.pPlNode)=(yyvsp[(1) - (1)].pPlNode);
;}
    break;

  case 132:

/* Line 1455 of yacc.c  */
#line 1388 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = (yyvsp[(1) - (2)].pPlNode);
  (yyval.pPlNode)->next = (yyvsp[(2) - (2)].pPlNode);
;}
    break;

  case 133:

/* Line 1455 of yacc.c  */
#line 1396 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(AT_START_CONN);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 134:

/* Line 1455 of yacc.c  */
#line 1402 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(AT_END_CONN);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 135:

/* Line 1455 of yacc.c  */
#line 1408 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(OVER_ALL_CONN);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 136:

/* Line 1455 of yacc.c  */
#line 1419 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = NULL;
;}
    break;

  case 137:

/* Line 1455 of yacc.c  */
#line 1424 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = (yyvsp[(1) - (2)].pPlNode);
  (yyval.pPlNode)->next = (yyvsp[(2) - (2)].pPlNode);
;}
    break;

  case 138:

/* Line 1455 of yacc.c  */
#line 1431 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(AND);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 139:

/* Line 1455 of yacc.c  */
#line 1437 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = (yyvsp[(1) - (1)].pPlNode);
;}
    break;

  case 140:

/* Line 1455 of yacc.c  */
#line 1445 "scan-ops_pddl.y"
    {
  PlNode *pln;

  pln = new_PlNode(ALL);
  pln->parse_vars = (yyvsp[(4) - (7)].pTypedList);

  (yyval.pPlNode) = pln;
  pln->sons = (yyvsp[(6) - (7)].pPlNode);
;}
    break;

  case 141:

/* Line 1455 of yacc.c  */
#line 1457 "scan-ops_pddl.y"
    {
  /* This will be conditional effects in FF representation, but here
   * a formula like (WHEN p q) will be saved as:
   *  [WHEN]
   *  [sons]
   *   /  \
   * [p]  [q]
   * That means, the first son is p, and the second one is q.
   */
  (yyval.pPlNode) = new_PlNode(WHEN);
  (yyvsp[(3) - (5)].pPlNode)->next = (yyvsp[(4) - (5)].pPlNode);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (5)].pPlNode);
;}
    break;

  case 142:

/* Line 1455 of yacc.c  */
#line 1472 "scan-ops_pddl.y"
    {
  PlNode* pln;
  (yyval.pPlNode)=(yyvsp[(2) - (5)].pPlNode);
  pln = new_PlNode(FN_HEAD);
  (yyval.pPlNode)->sons = pln;
  (yyval.pPlNode)->sons->atom = (yyvsp[(3) - (5)].pTokenList);
  (yyval.pPlNode)->sons->next = (yyvsp[(4) - (5)].pPlNode);
;}
    break;

  case 143:

/* Line 1455 of yacc.c  */
#line 1497 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(AT_START_CONN);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 144:

/* Line 1455 of yacc.c  */
#line 1503 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(AT_END_CONN);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 145:

/* Line 1455 of yacc.c  */
#line 1509 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(AT_START_CONN);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 146:

/* Line 1455 of yacc.c  */
#line 1515 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(AT_END_CONN);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 147:

/* Line 1455 of yacc.c  */
#line 1521 "scan-ops_pddl.y"
    {
  PlNode *tmp;
  tmp = new_PlNode(INCREASE_CONN);
  tmp->sons = new_PlNode(FN_HEAD);
  tmp->sons->atom = (yyvsp[(3) - (5)].pTokenList);
  tmp->sons->next = (yyvsp[(4) - (5)].pPlNode);
  (yyval.pPlNode) = tmp;
;}
    break;

  case 148:

/* Line 1455 of yacc.c  */
#line 1531 "scan-ops_pddl.y"
    {
  PlNode *tmp;
  tmp = new_PlNode(DECREASE_CONN);
  tmp->sons = new_PlNode(FN_HEAD);
  tmp->sons->atom = (yyvsp[(3) - (5)].pTokenList);
  tmp->sons->next = (yyvsp[(4) - (5)].pPlNode);
  (yyval.pPlNode) = tmp;
;}
    break;

  case 149:

/* Line 1455 of yacc.c  */
#line 1543 "scan-ops_pddl.y"
    {

  PlNode* pln;
  (yyval.pPlNode)=(yyvsp[(2) - (5)].pPlNode);
  pln = new_PlNode(FN_HEAD);
  (yyval.pPlNode)->sons = pln;
  (yyval.pPlNode)->sons->atom = (yyvsp[(3) - (5)].pTokenList);
  (yyval.pPlNode)->sons->next = (yyvsp[(4) - (5)].pPlNode);

;}
    break;

  case 150:

/* Line 1455 of yacc.c  */
#line 1558 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = (yyvsp[(2) - (5)].pPlNode);
  (yyval.pPlNode)->sons->sons = (yyvsp[(3) - (5)].pPlNode);
  (yyval.pPlNode)->sons->sons->next = (yyvsp[(4) - (5)].pPlNode);
;}
    break;

  case 151:

/* Line 1455 of yacc.c  */
#line 1565 "scan-ops_pddl.y"
    {
  PlNode *pln;

  (yyval.pPlNode)=new_PlNode(F_EXP);
  pln=new_PlNode(UMINUS_CONN);
  (yyval.pPlNode)->sons = pln;
  (yyval.pPlNode)->sons->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 152:

/* Line 1455 of yacc.c  */
#line 1575 "scan-ops_pddl.y"
    {
  PlNode *pln;

  (yyval.pPlNode)=new_PlNode(F_EXP);
  pln=new_PlNode(DURATION_VAR_ATOM);
  (yyval.pPlNode)->sons = pln;
;}
    break;

  case 154:

/* Line 1455 of yacc.c  */
#line 1589 "scan-ops_pddl.y"
    {
  PlNode *pln;

  (yyval.pPlNode)=new_PlNode(F_EXP);
  pln=new_PlNode(PLUS_CONN);
  (yyval.pPlNode)->sons = pln;

;}
    break;

  case 155:

/* Line 1455 of yacc.c  */
#line 1599 "scan-ops_pddl.y"
    {
  PlNode *pln;

  (yyval.pPlNode)=new_PlNode(F_EXP);
  pln=new_PlNode(MINUS_CONN);
  (yyval.pPlNode)->sons = pln;

;}
    break;

  case 156:

/* Line 1455 of yacc.c  */
#line 1609 "scan-ops_pddl.y"
    {
  PlNode *pln;

  (yyval.pPlNode)=new_PlNode(F_EXP);
  pln=new_PlNode(MUL_CONN);
  (yyval.pPlNode)->sons = pln;

;}
    break;

  case 157:

/* Line 1455 of yacc.c  */
#line 1619 "scan-ops_pddl.y"
    {
  PlNode *pln;

  (yyval.pPlNode)=new_PlNode(F_EXP);
  pln=new_PlNode(DIV_CONN);
  (yyval.pPlNode)->sons = pln;

;}
    break;

  case 158:

/* Line 1455 of yacc.c  */
#line 1632 "scan-ops_pddl.y"
    {
  PlNode *pln;

  (yyval.pPlNode) = new_PlNode(F_EXP_T);
  pln = new_PlNode(MUL_CONN);
  (yyval.pPlNode)->sons = pln;
  (yyval.pPlNode)->sons->sons = (yyvsp[(3) - (5)].pPlNode);
  (yyval.pPlNode)->sons->sons->next = (yyvsp[(4) - (5)].pPlNode);  

;}
    break;

  case 159:

/* Line 1455 of yacc.c  */
#line 1644 "scan-ops_pddl.y"
    {
  PlNode *pln, *pln2;

  (yyval.pPlNode) = new_PlNode(F_EXP_T);
  pln2 = new_PlNode(TIME_VAR);
  pln=new_PlNode(MUL_CONN); 
  (yyval.pPlNode)->sons = pln;
  (yyval.pPlNode)->sons->sons = pln2;
  (yyval.pPlNode)->sons->sons->next = (yyvsp[(4) - (5)].pPlNode);  

;}
    break;

  case 160:

/* Line 1455 of yacc.c  */
#line 1657 "scan-ops_pddl.y"
    {
  PlNode *pln;

  pln = new_PlNode(TIME_VAR);
  (yyval.pPlNode) = pln;
;}
    break;

  case 161:

/* Line 1455 of yacc.c  */
#line 1668 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(AND);
  (yyval.pPlNode)->sons = (yyvsp[(3) - (4)].pPlNode);
;}
    break;

  case 162:

/* Line 1455 of yacc.c  */
#line 1674 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(AND);
  (yyval.pPlNode)->sons = (yyvsp[(1) - (1)].pPlNode);
;}
    break;

  case 163:

/* Line 1455 of yacc.c  */
#line 1682 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = (yyvsp[(1) - (1)].pPlNode);
;}
    break;

  case 164:

/* Line 1455 of yacc.c  */
#line 1687 "scan-ops_pddl.y"
    {
  (yyval.pPlNode)=(yyvsp[(1) - (2)].pPlNode);
  (yyval.pPlNode)->next = (yyvsp[(2) - (2)].pPlNode);
;}
    break;

  case 165:

/* Line 1455 of yacc.c  */
#line 1699 "scan-ops_pddl.y"
    {
  PlNode *pln;

  (yyval.pPlNode) = (yyvsp[(2) - (5)].pPlNode);
  pln = new_PlNode(DURATION_VAR_ATOM);
  (yyval.pPlNode)->sons->sons = pln;
  (yyval.pPlNode)->sons->sons->next = (yyvsp[(4) - (5)].pPlNode);
;}
    break;

  case 166:

/* Line 1455 of yacc.c  */
#line 1710 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(AND);
;}
    break;

  case 167:

/* Line 1455 of yacc.c  */
#line 1715 "scan-ops_pddl.y"
    {
  (yyval.pPlNode) = new_PlNode(AND);
;}
    break;

  case 168:

/* Line 1455 of yacc.c  */
#line 1722 "scan-ops_pddl.y"
    {
  PlNode *pln;

  (yyval.pPlNode)=new_PlNode(DURATION_CONSTRAINT_CONN);
  pln=new_PlNode(LESS_THAN_OR_EQUAL_CONN); 
  (yyval.pPlNode)->sons = pln;
;}
    break;

  case 169:

/* Line 1455 of yacc.c  */
#line 1731 "scan-ops_pddl.y"
    {
  PlNode *pln;

  (yyval.pPlNode)=new_PlNode(DURATION_CONSTRAINT_CONN);
  pln=new_PlNode(GREATER_OR_EQUAL_CONN); 
  (yyval.pPlNode)->sons = pln;
;}
    break;

  case 170:

/* Line 1455 of yacc.c  */
#line 1740 "scan-ops_pddl.y"
    {
  PlNode *pln;

  (yyval.pPlNode)=new_PlNode(DURATION_CONSTRAINT_CONN);
  pln=new_PlNode(EQUAL_CONN); 
  (yyval.pPlNode)->sons = pln;
;}
    break;

  case 173:

/* Line 1455 of yacc.c  */
#line 1757 "scan-ops_pddl.y"
    {
  (yyval.pstring) = new_Token(strlen((yyvsp[(2) - (2)].string)) + 1);
  strcpy((yyval.pstring), (yyvsp[(2) - (2)].string));
;}
    break;



/* Line 1455 of yacc.c  */
#line 3618 "scan-ops_pddl.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
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

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
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
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
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
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 1767 "scan-ops_pddl.y"

#include "lex.ops_pddl.c"


/**********************************************************************
 * Functions
 **********************************************************************/



int yyerror( char *msg )

{

  if (msg)
    printf("\n%s", msg);

  fprintf(stderr, "\n%s: syntax error in line %d, '%s':\n", 
	  gact_filename, lineno, yytext);

  if ( NULL != sact_err_par ) {
    fprintf(stderr, "%s %s\n", serrmsg[sact_err], sact_err_par);
  } else {
    fprintf(stderr, "%s\n", serrmsg[sact_err]);
  }

  fflush(stdout);
  exit( 1 );

  return 0;

}



void load_ops_file( char *filename )

{

  FILE * fp;/* pointer to input files */
  char tmp[MAX_LENGTH] = "";

  gbracket_count = 0;

   
  /* open operator file 
   */
  if( ( fp = fopen( filename, "r" ) ) == NULL ) {
    sprintf(tmp, "\n Can't find operator file: %s\n\n", filename );
    perror(tmp);
    exit( 1 );
  }

  gact_filename = filename;
  lineno = 1; 
  yyin = fp;

  yyparse();

  fclose( fp );/* and close file again */

}



