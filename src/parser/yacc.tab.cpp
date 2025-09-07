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
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "/root/db2025-amatilasdb/src/parser/yacc.y"

#include "ast.h"
#include "yacc.tab.h"
#include <iostream>
#include <memory>

int yylex(YYSTYPE *yylval, YYLTYPE *yylloc);

void yyerror(YYLTYPE *locp, const char* s) {
    std::cerr << "Parser Error at line " << locp->first_line << " column " << locp->first_column << ": " << s << std::endl;
}

using namespace ast;

#line 86 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"

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

#include "yacc.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_SHOW = 3,                       /* SHOW  */
  YYSYMBOL_TABLES = 4,                     /* TABLES  */
  YYSYMBOL_CREATE = 5,                     /* CREATE  */
  YYSYMBOL_TABLE = 6,                      /* TABLE  */
  YYSYMBOL_DROP = 7,                       /* DROP  */
  YYSYMBOL_DESC = 8,                       /* DESC  */
  YYSYMBOL_INSERT = 9,                     /* INSERT  */
  YYSYMBOL_INTO = 10,                      /* INTO  */
  YYSYMBOL_VALUES = 11,                    /* VALUES  */
  YYSYMBOL_DELETE = 12,                    /* DELETE  */
  YYSYMBOL_FROM = 13,                      /* FROM  */
  YYSYMBOL_ASC = 14,                       /* ASC  */
  YYSYMBOL_DESC_ORDER = 15,                /* DESC_ORDER  */
  YYSYMBOL_ORDER = 16,                     /* ORDER  */
  YYSYMBOL_BY = 17,                        /* BY  */
  YYSYMBOL_IN = 18,                        /* IN  */
  YYSYMBOL_LIMIT = 19,                     /* LIMIT  */
  YYSYMBOL_WHERE = 20,                     /* WHERE  */
  YYSYMBOL_UPDATE = 21,                    /* UPDATE  */
  YYSYMBOL_SET = 22,                       /* SET  */
  YYSYMBOL_SELECT = 23,                    /* SELECT  */
  YYSYMBOL_INT = 24,                       /* INT  */
  YYSYMBOL_CHAR = 25,                      /* CHAR  */
  YYSYMBOL_FLOAT = 26,                     /* FLOAT  */
  YYSYMBOL_INDEX = 27,                     /* INDEX  */
  YYSYMBOL_AND = 28,                       /* AND  */
  YYSYMBOL_JOIN = 29,                      /* JOIN  */
  YYSYMBOL_SEMI = 30,                      /* SEMI  */
  YYSYMBOL_ON = 31,                        /* ON  */
  YYSYMBOL_EXIT = 32,                      /* EXIT  */
  YYSYMBOL_HELP = 33,                      /* HELP  */
  YYSYMBOL_TXN_BEGIN = 34,                 /* TXN_BEGIN  */
  YYSYMBOL_TXN_COMMIT = 35,                /* TXN_COMMIT  */
  YYSYMBOL_TXN_ABORT = 36,                 /* TXN_ABORT  */
  YYSYMBOL_TXN_ROLLBACK = 37,              /* TXN_ROLLBACK  */
  YYSYMBOL_ORDER_BY = 38,                  /* ORDER_BY  */
  YYSYMBOL_ENABLE_NESTLOOP = 39,           /* ENABLE_NESTLOOP  */
  YYSYMBOL_ENABLE_SORTMERGE = 40,          /* ENABLE_SORTMERGE  */
  YYSYMBOL_STATIC_CHECKPOINT = 41,         /* STATIC_CHECKPOINT  */
  YYSYMBOL_EXPLAIN = 42,                   /* EXPLAIN  */
  YYSYMBOL_43_ = 43,                       /* '+'  */
  YYSYMBOL_44_ = 44,                       /* '-'  */
  YYSYMBOL_45_ = 45,                       /* '*'  */
  YYSYMBOL_46_ = 46,                       /* '/'  */
  YYSYMBOL_UMINUS = 47,                    /* UMINUS  */
  YYSYMBOL_AVG = 48,                       /* AVG  */
  YYSYMBOL_SUM = 49,                       /* SUM  */
  YYSYMBOL_COUNT = 50,                     /* COUNT  */
  YYSYMBOL_MAX = 51,                       /* MAX  */
  YYSYMBOL_MIN = 52,                       /* MIN  */
  YYSYMBOL_AS = 53,                        /* AS  */
  YYSYMBOL_GROUP = 54,                     /* GROUP  */
  YYSYMBOL_HAVING = 55,                    /* HAVING  */
  YYSYMBOL_LEQ = 56,                       /* LEQ  */
  YYSYMBOL_NEQ = 57,                       /* NEQ  */
  YYSYMBOL_GEQ = 58,                       /* GEQ  */
  YYSYMBOL_T_EOF = 59,                     /* T_EOF  */
  YYSYMBOL_IDENTIFIER = 60,                /* IDENTIFIER  */
  YYSYMBOL_VALUE_STRING = 61,              /* VALUE_STRING  */
  YYSYMBOL_VALUE_INT = 62,                 /* VALUE_INT  */
  YYSYMBOL_VALUE_FLOAT = 63,               /* VALUE_FLOAT  */
  YYSYMBOL_VALUE_BOOL = 64,                /* VALUE_BOOL  */
  YYSYMBOL_65_ = 65,                       /* ';'  */
  YYSYMBOL_66_ = 66,                       /* '='  */
  YYSYMBOL_67_ = 67,                       /* '('  */
  YYSYMBOL_68_ = 68,                       /* ')'  */
  YYSYMBOL_69_ = 69,                       /* ','  */
  YYSYMBOL_70_ = 70,                       /* '.'  */
  YYSYMBOL_71_ = 71,                       /* '<'  */
  YYSYMBOL_72_ = 72,                       /* '>'  */
  YYSYMBOL_YYACCEPT = 73,                  /* $accept  */
  YYSYMBOL_start = 74,                     /* start  */
  YYSYMBOL_stmt = 75,                      /* stmt  */
  YYSYMBOL_txnStmt = 76,                   /* txnStmt  */
  YYSYMBOL_dbStmt = 77,                    /* dbStmt  */
  YYSYMBOL_setStmt = 78,                   /* setStmt  */
  YYSYMBOL_ddl = 79,                       /* ddl  */
  YYSYMBOL_dml = 80,                       /* dml  */
  YYSYMBOL_fieldList = 81,                 /* fieldList  */
  YYSYMBOL_colNameList = 82,               /* colNameList  */
  YYSYMBOL_field = 83,                     /* field  */
  YYSYMBOL_type = 84,                      /* type  */
  YYSYMBOL_valueList = 85,                 /* valueList  */
  YYSYMBOL_value = 86,                     /* value  */
  YYSYMBOL_condition = 87,                 /* condition  */
  YYSYMBOL_optGroupClause = 88,            /* optGroupClause  */
  YYSYMBOL_GroupColList = 89,              /* GroupColList  */
  YYSYMBOL_optHavingClause = 90,           /* optHavingClause  */
  YYSYMBOL_havingConditions = 91,          /* havingConditions  */
  YYSYMBOL_optWhereClause = 92,            /* optWhereClause  */
  YYSYMBOL_whereClause = 93,               /* whereClause  */
  YYSYMBOL_col = 94,                       /* col  */
  YYSYMBOL_agg_type = 95,                  /* agg_type  */
  YYSYMBOL_colList = 96,                   /* colList  */
  YYSYMBOL_op = 97,                        /* op  */
  YYSYMBOL_expr = 98,                      /* expr  */
  YYSYMBOL_setClauses = 99,                /* setClauses  */
  YYSYMBOL_setClause = 100,                /* setClause  */
  YYSYMBOL_selector = 101,                 /* selector  */
  YYSYMBOL_tableList = 102,                /* tableList  */
  YYSYMBOL_opt_order_clause = 103,         /* opt_order_clause  */
  YYSYMBOL_order_list = 104,               /* order_list  */
  YYSYMBOL_order_item = 105,               /* order_item  */
  YYSYMBOL_opt_asc_desc = 106,             /* opt_asc_desc  */
  YYSYMBOL_opt_limit_clause = 107,         /* opt_limit_clause  */
  YYSYMBOL_set_knob_type = 108,            /* set_knob_type  */
  YYSYMBOL_tbName = 109,                   /* tbName  */
  YYSYMBOL_colName = 110                   /* colName  */
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
typedef yytype_uint8 yy_state_t;

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

#if 1

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
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE) \
             + YYSIZEOF (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

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
#define YYFINAL  54
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   227

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  73
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  38
/* YYNRULES -- Number of rules.  */
#define YYNRULES  111
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  217

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   315


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
      67,    68,    45,    43,    69,    44,    70,    46,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    65,
      71,    66,    72,     2,     2,     2,     2,     2,     2,     2,
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
      35,    36,    37,    38,    39,    40,    41,    42,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    86,    86,    91,    96,   101,   109,   110,   111,   112,
     113,   117,   121,   125,   129,   133,   140,   147,   154,   158,
     162,   166,   170,   174,   181,   185,   189,   193,   200,   210,
     214,   221,   225,   232,   239,   243,   247,   254,   258,   265,
     269,   273,   277,   284,   288,   295,   297,   304,   308,   315,
     317,   324,   329,   336,   337,   344,   348,   355,   359,   363,
     367,   371,   375,   379,   383,   387,   391,   399,   403,   407,
     411,   415,   423,   427,   434,   438,   442,   446,   450,   454,
     461,   465,   469,   473,   477,   481,   485,   489,   496,   500,
     507,   511,   518,   522,   529,   535,   542,   550,   561,   565,
     569,   573,   580,   587,   588,   589,   593,   597,   601,   602,
     605,   607
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "SHOW", "TABLES",
  "CREATE", "TABLE", "DROP", "DESC", "INSERT", "INTO", "VALUES", "DELETE",
  "FROM", "ASC", "DESC_ORDER", "ORDER", "BY", "IN", "LIMIT", "WHERE",
  "UPDATE", "SET", "SELECT", "INT", "CHAR", "FLOAT", "INDEX", "AND",
  "JOIN", "SEMI", "ON", "EXIT", "HELP", "TXN_BEGIN", "TXN_COMMIT",
  "TXN_ABORT", "TXN_ROLLBACK", "ORDER_BY", "ENABLE_NESTLOOP",
  "ENABLE_SORTMERGE", "STATIC_CHECKPOINT", "EXPLAIN", "'+'", "'-'", "'*'",
  "'/'", "UMINUS", "AVG", "SUM", "COUNT", "MAX", "MIN", "AS", "GROUP",
  "HAVING", "LEQ", "NEQ", "GEQ", "T_EOF", "IDENTIFIER", "VALUE_STRING",
  "VALUE_INT", "VALUE_FLOAT", "VALUE_BOOL", "';'", "'='", "'('", "')'",
  "','", "'.'", "'<'", "'>'", "$accept", "start", "stmt", "txnStmt",
  "dbStmt", "setStmt", "ddl", "dml", "fieldList", "colNameList", "field",
  "type", "valueList", "value", "condition", "optGroupClause",
  "GroupColList", "optHavingClause", "havingConditions", "optWhereClause",
  "whereClause", "col", "agg_type", "colList", "op", "expr", "setClauses",
  "setClause", "selector", "tableList", "opt_order_clause", "order_list",
  "order_item", "opt_asc_desc", "opt_limit_clause", "set_knob_type",
  "tbName", "colName", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-113)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-111)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      86,     2,     9,     3,    -6,    -1,   -40,   -40,     8,    92,
    -113,  -113,  -113,  -113,  -113,  -113,    14,  -113,    32,   -16,
    -113,  -113,  -113,  -113,  -113,  -113,    43,   -40,   -40,  -113,
     -40,   -40,   -40,   -40,  -113,  -113,    36,  -113,  -113,    -3,
    -113,  -113,  -113,  -113,  -113,  -113,    17,  -113,    29,    -8,
      77,    24,    47,    92,  -113,  -113,   -40,    39,    45,  -113,
      50,    99,   112,    95,    96,   -29,   115,   -40,    95,    95,
     146,  -113,    95,    95,    95,   101,   133,  -113,  -113,   -15,
    -113,   104,  -113,   105,   106,   110,  -113,    23,  -113,   121,
    -113,   -40,   -34,  -113,    59,    57,  -113,    70,    41,   133,
    -113,  -113,  -113,  -113,   133,  -113,  -113,   152,    58,    90,
      95,  -113,   133,   134,    95,   135,   -40,   157,   -40,   144,
      95,    23,  -113,    95,  -113,   122,  -113,  -113,  -113,    95,
    -113,    81,  -113,  -113,  -113,    -4,   133,  -113,  -113,  -113,
    -113,  -113,  -113,   133,   133,   133,   133,   133,   133,  -113,
    -113,    27,    95,   131,    95,   170,   -40,  -113,   185,   148,
    -113,   144,  -113,   142,  -113,  -113,    41,  -113,  -113,    27,
     108,   108,  -113,  -113,    27,  -113,   154,  -113,   133,   174,
     115,   133,   192,   148,   141,  -113,    95,  -113,   133,   145,
    -113,  -113,   182,   194,   193,   192,  -113,  -113,  -113,   115,
     133,   115,   151,  -113,   193,  -113,  -113,   143,   147,  -113,
    -113,  -113,  -113,  -113,  -113,   115,  -113
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       4,     3,    11,    12,    13,    14,     0,     5,     0,     0,
       9,     6,    10,     7,     8,    16,     0,     0,     0,    15,
       0,     0,     0,     0,   110,    20,     0,   108,   109,     0,
      92,    71,    67,    68,    70,    69,   111,    72,     0,    93,
       0,     0,    58,     0,     1,     2,     0,     0,     0,    19,
       0,     0,    53,     0,     0,     0,     0,     0,     0,     0,
       0,    23,     0,     0,     0,     0,     0,    25,   111,    53,
      88,     0,    17,     0,     0,     0,    73,    53,    94,    57,
      63,     0,     0,    29,     0,     0,    31,     0,     0,     0,
      41,    39,    40,    42,     0,    80,    55,    54,    81,     0,
       0,    26,     0,    61,     0,    59,     0,     0,     0,    45,
       0,    53,    18,     0,    34,     0,    36,    33,    21,     0,
      22,     0,    37,    81,    86,     0,     0,    78,    77,    79,
      74,    75,    76,     0,     0,     0,     0,     0,     0,    89,
      80,    91,     0,     0,     0,     0,     0,    95,     0,    49,
      62,    45,    30,     0,    32,    24,     0,    87,    56,    43,
      82,    83,    84,    85,    44,    66,    60,    64,     0,     0,
       0,     0,    99,    49,     0,    38,     0,    96,     0,    46,
      47,    51,    50,     0,   107,    99,    35,    65,    97,     0,
       0,     0,     0,    27,   107,    48,    52,   105,    98,   100,
     106,    28,   103,   104,   102,     0,   101
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -113,  -113,  -113,  -113,  -113,  -113,  -113,  -113,  -113,   149,
      94,  -113,  -113,   -91,  -112,    54,  -113,    35,  -113,   -76,
    -113,    -9,  -113,  -113,   111,   -66,  -113,   109,   168,   136,
      30,  -113,     7,  -113,    20,  -113,    -5,   -55
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,    18,    19,    20,    21,    22,    23,    24,    92,    95,
      93,   127,   131,   105,   106,   159,   189,   182,   192,    77,
     107,   133,    48,    49,   143,   109,    79,    80,    50,    87,
     194,   208,   209,   214,   203,    39,    51,    52
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      47,    35,    36,   111,    32,    76,    25,   132,    81,    30,
      85,   119,    33,    89,    90,    27,    83,    94,    96,    96,
      34,   150,    57,    58,   168,    59,    60,    61,    62,    26,
      31,    46,    54,   134,   122,   123,    28,    53,   135,   144,
     145,   146,   147,    76,    47,   161,   151,    37,    38,    55,
      29,    71,   116,   117,   110,    81,    56,    86,    63,   153,
      84,    66,    88,    64,   167,   160,   187,   108,    94,   191,
     144,   145,   146,   147,   164,   185,   198,   169,   170,   171,
     172,   173,   174,   124,   125,   126,    88,  -110,   206,     1,
      67,     2,   118,     3,    68,     4,    65,   175,     5,   177,
      69,     6,   100,   101,   102,   103,    72,     7,     8,     9,
      75,   155,    73,   157,   137,   138,   139,    74,    10,    11,
      12,    13,    14,    15,   140,   128,   129,   108,    16,   141,
     142,   197,    76,   144,   145,   146,   147,    40,   130,   129,
      41,    42,    43,    44,    45,    17,   137,   138,   139,   165,
     166,   179,    46,   146,   147,    78,   140,   212,   213,    91,
      82,   141,   142,    41,    42,    43,    44,    45,    98,   108,
     112,   190,   108,   113,   120,    46,   114,    99,   115,   108,
     136,    41,    42,    43,    44,    45,   156,   152,   154,   163,
     205,   108,   207,    46,   100,   101,   102,   103,   158,   176,
     104,   178,   180,   181,   184,   188,   207,   186,   193,   196,
     200,   201,   202,   210,   199,   183,   215,   162,   195,   149,
     148,    70,   216,    97,   211,   204,     0,   121
};

static const yytype_int16 yycheck[] =
{
       9,     6,     7,    79,    10,    20,     4,    98,    63,     6,
      65,    87,    13,    68,    69,     6,    45,    72,    73,    74,
      60,   112,    27,    28,   136,    30,    31,    32,    33,    27,
      27,    60,     0,    99,    68,    69,    27,    23,   104,    43,
      44,    45,    46,    20,    53,   121,   112,    39,    40,    65,
      41,    56,    29,    30,    69,   110,    13,    66,    22,   114,
      65,    69,    67,    66,    68,   120,   178,    76,   123,   181,
      43,    44,    45,    46,   129,   166,   188,   143,   144,   145,
     146,   147,   148,    24,    25,    26,    91,    70,   200,     3,
      13,     5,    69,     7,    70,     9,    67,   152,    12,   154,
      53,    15,    61,    62,    63,    64,    67,    21,    22,    23,
      11,   116,    67,   118,    56,    57,    58,    67,    32,    33,
      34,    35,    36,    37,    66,    68,    69,   136,    42,    71,
      72,   186,    20,    43,    44,    45,    46,    45,    68,    69,
      48,    49,    50,    51,    52,    59,    56,    57,    58,    68,
      69,   156,    60,    45,    46,    60,    66,    14,    15,    13,
      64,    71,    72,    48,    49,    50,    51,    52,    67,   178,
      66,   180,   181,    68,    53,    60,    70,    44,    68,   188,
      28,    48,    49,    50,    51,    52,    29,    53,    53,    67,
     199,   200,   201,    60,    61,    62,    63,    64,    54,    68,
      67,    31,    17,    55,    62,    31,   215,    53,    16,    68,
      28,    17,    19,    62,    69,   161,    69,   123,   183,   110,
     109,    53,   215,    74,   204,   195,    -1,    91
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     5,     7,     9,    12,    15,    21,    22,    23,
      32,    33,    34,    35,    36,    37,    42,    59,    74,    75,
      76,    77,    78,    79,    80,     4,    27,     6,    27,    41,
       6,    27,    10,    13,    60,   109,   109,    39,    40,   108,
      45,    48,    49,    50,    51,    52,    60,    94,    95,    96,
     101,   109,   110,    23,     0,    65,    13,   109,   109,   109,
     109,   109,   109,    22,    66,    67,    69,    13,    70,    53,
     101,   109,    67,    67,    67,    11,    20,    92,    60,    99,
     100,   110,    64,    45,   109,   110,    94,   102,   109,   110,
     110,    13,    81,    83,   110,    82,   110,    82,    67,    44,
      61,    62,    63,    64,    67,    86,    87,    93,    94,    98,
      69,    92,    66,    68,    70,    68,    29,    30,    69,    92,
      53,   102,    68,    69,    24,    25,    26,    84,    68,    69,
      68,    85,    86,    94,    98,    98,    28,    56,    57,    58,
      66,    71,    72,    97,    43,    44,    45,    46,    97,   100,
      86,    98,    53,   110,    53,   109,    29,   109,    54,    88,
     110,    92,    83,    67,   110,    68,    69,    68,    87,    98,
      98,    98,    98,    98,    98,   110,    68,   110,    31,   109,
      17,    55,    90,    88,    62,    86,    53,    87,    31,    89,
      94,    87,    91,    16,   103,    90,    68,   110,    87,    69,
      28,    17,    19,   107,   103,    94,    87,    94,   104,   105,
      62,   107,    14,    15,   106,    69,   105
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    73,    74,    74,    74,    74,    75,    75,    75,    75,
      75,    76,    76,    76,    76,    76,    77,    78,    79,    79,
      79,    79,    79,    79,    80,    80,    80,    80,    80,    81,
      81,    82,    82,    83,    84,    84,    84,    85,    85,    86,
      86,    86,    86,    87,    87,    88,    88,    89,    89,    90,
      90,    91,    91,    92,    92,    93,    93,    94,    94,    94,
      94,    94,    94,    94,    94,    94,    94,    95,    95,    95,
      95,    95,    96,    96,    97,    97,    97,    97,    97,    97,
      98,    98,    98,    98,    98,    98,    98,    98,    99,    99,
     100,   100,   101,   101,   102,   102,   102,   102,   103,   103,
     104,   104,   105,   106,   106,   106,   107,   107,   108,   108,
     109,   110
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     2,     2,     4,     6,     3,
       2,     6,     6,     4,     7,     4,     5,     9,    10,     1,
       3,     1,     3,     2,     1,     4,     1,     1,     3,     1,
       1,     1,     1,     3,     3,     0,     3,     1,     3,     0,
       2,     1,     3,     0,     2,     1,     3,     3,     1,     4,
       6,     4,     5,     3,     6,     8,     6,     1,     1,     1,
       1,     1,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     3,     3,     3,     2,     3,     1,     3,
       3,     3,     1,     1,     1,     3,     5,     6,     3,     0,
       1,     3,     2,     1,     1,     0,     2,     0,     1,     1,
       1,     1
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
        yyerror (&yylloc, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF

/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


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


/* YYLOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

# ifndef YYLOCATION_PRINT

#  if defined YY_LOCATION_PRINT

   /* Temporary convenience wrapper in case some people defined the
      undocumented and private YY_LOCATION_PRINT macros.  */
#   define YYLOCATION_PRINT(File, Loc)  YY_LOCATION_PRINT(File, *(Loc))

#  elif defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static int
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  int res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
}

#   define YYLOCATION_PRINT  yy_location_print_

    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT(File, Loc)  YYLOCATION_PRINT(File, &(Loc))

#  else

#   define YYLOCATION_PRINT(File, Loc) ((void) 0)
    /* Temporary convenience wrapper in case some people defined the
       undocumented and private YY_LOCATION_PRINT macros.  */
#   define YY_LOCATION_PRINT  YYLOCATION_PRINT

#  endif
# endif /* !defined YYLOCATION_PRINT */


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Location); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (yylocationp);
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
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  YYLOCATION_PRINT (yyo, yylocationp);
  YYFPRINTF (yyo, ": ");
  yy_symbol_value_print (yyo, yykind, yyvaluep, yylocationp);
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp,
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
                       &yyvsp[(yyi + 1) - (yynrhs)],
                       &(yylsp[(yyi + 1) - (yynrhs)]));
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule); \
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


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
  YYLTYPE *yylloc;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
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
            else
              goto append;

          append:
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

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, YYLTYPE *yylocationp)
{
  YY_USE (yyvaluep);
  YY_USE (yylocationp);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}






/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
/* Lookahead token kind.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs = 0;

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

    /* The location stack: array, bottom, top.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls = yylsa;
    YYLTYPE *yylsp = yyls;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

  /* The locations where the error started and ended.  */
  YYLTYPE yyerror_range[3];

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  yylsp[0] = yylloc;
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
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yyls1, yysize * YYSIZEOF (*yylsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
        yyls = yyls1;
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
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

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
      yychar = yylex (&yylval, &yylloc);
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
      yyerror_range[1] = yylloc;
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
  *++yylsp = yylloc;

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

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* start: stmt ';'  */
#line 87 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        parse_tree = (yyvsp[-1].sv_node);
        YYACCEPT;
    }
#line 1735 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 3: /* start: HELP  */
#line 92 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        parse_tree = std::make_shared<Help>();
        YYACCEPT;
    }
#line 1744 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 4: /* start: EXIT  */
#line 97 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        parse_tree = nullptr;
        YYACCEPT;
    }
#line 1753 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 5: /* start: T_EOF  */
#line 102 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        parse_tree = nullptr;
        YYACCEPT;
    }
#line 1762 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 11: /* txnStmt: TXN_BEGIN  */
#line 118 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<TxnBegin>();
    }
#line 1770 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 12: /* txnStmt: TXN_COMMIT  */
#line 122 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<TxnCommit>();
    }
#line 1778 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 13: /* txnStmt: TXN_ABORT  */
#line 126 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<TxnAbort>();
    }
#line 1786 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 14: /* txnStmt: TXN_ROLLBACK  */
#line 130 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<TxnRollback>();
    }
#line 1794 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 15: /* txnStmt: CREATE STATIC_CHECKPOINT  */
#line 134 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<CreateCheckpoint>();
    }
#line 1802 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 16: /* dbStmt: SHOW TABLES  */
#line 141 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<ShowTables>();
    }
#line 1810 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 17: /* setStmt: SET set_knob_type '=' VALUE_BOOL  */
#line 148 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<SetStmt>((yyvsp[-2].sv_setKnobType), (yyvsp[0].sv_bool));
    }
#line 1818 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 18: /* ddl: CREATE TABLE tbName '(' fieldList ')'  */
#line 155 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<CreateTable>((yyvsp[-3].sv_str), (yyvsp[-1].sv_fields));
    }
#line 1826 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 19: /* ddl: DROP TABLE tbName  */
#line 159 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<DropTable>((yyvsp[0].sv_str));
    }
#line 1834 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 20: /* ddl: DESC_ORDER tbName  */
#line 163 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<DescTable>((yyvsp[0].sv_str));
    }
#line 1842 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 21: /* ddl: CREATE INDEX tbName '(' colNameList ')'  */
#line 167 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<CreateIndex>((yyvsp[-3].sv_str), (yyvsp[-1].sv_strs));
    }
#line 1850 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 22: /* ddl: DROP INDEX tbName '(' colNameList ')'  */
#line 171 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<DropIndex>((yyvsp[-3].sv_str), (yyvsp[-1].sv_strs));
    }
#line 1858 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 23: /* ddl: SHOW INDEX FROM tbName  */
#line 175 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<ShowIndex>((yyvsp[0].sv_str));
    }
#line 1866 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 24: /* dml: INSERT INTO tbName VALUES '(' valueList ')'  */
#line 182 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<InsertStmt>((yyvsp[-4].sv_str), (yyvsp[-1].sv_vals));
    }
#line 1874 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 25: /* dml: DELETE FROM tbName optWhereClause  */
#line 186 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<DeleteStmt>((yyvsp[-1].sv_str), (yyvsp[0].sv_conds));
    }
#line 1882 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 26: /* dml: UPDATE tbName SET setClauses optWhereClause  */
#line 190 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_node) = std::make_shared<UpdateStmt>((yyvsp[-3].sv_str), (yyvsp[-1].sv_set_clauses), (yyvsp[0].sv_conds));
    }
#line 1890 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 27: /* dml: SELECT selector FROM tableList optWhereClause optGroupClause optHavingClause opt_order_clause opt_limit_clause  */
#line 194 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        auto select_stmt = std::make_shared<SelectStmt>((yyvsp[-7].sv_cols), (yyvsp[-5].sv_table_list).tables, (yyvsp[-4].sv_conds), (yyvsp[-3].sv_group_by_Clause), (yyvsp[-2].sv_having_clause), (yyvsp[-1].sv_orderby), (yyvsp[0].sv_int));
        select_stmt->jointree = (yyvsp[-5].sv_table_list).joins;
        select_stmt->table_aliases = (yyvsp[-5].sv_table_list).table_aliases;
        (yyval.sv_node) = select_stmt;
    }
#line 1901 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 28: /* dml: EXPLAIN SELECT selector FROM tableList optWhereClause optGroupClause optHavingClause opt_order_clause opt_limit_clause  */
#line 201 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        auto select_stmt = std::make_shared<SelectStmt>((yyvsp[-7].sv_cols), (yyvsp[-5].sv_table_list).tables, (yyvsp[-4].sv_conds), (yyvsp[-3].sv_group_by_Clause), (yyvsp[-2].sv_having_clause), (yyvsp[-1].sv_orderby), (yyvsp[0].sv_int));
        select_stmt->jointree = (yyvsp[-5].sv_table_list).joins;
        select_stmt->table_aliases = (yyvsp[-5].sv_table_list).table_aliases;
        (yyval.sv_node) = std::make_shared<ExplainStmt>(select_stmt);
    }
#line 1912 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 29: /* fieldList: field  */
#line 211 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_fields) = std::vector<std::shared_ptr<Field>>{(yyvsp[0].sv_field)};
    }
#line 1920 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 30: /* fieldList: fieldList ',' field  */
#line 215 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_fields).push_back((yyvsp[0].sv_field));
    }
#line 1928 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 31: /* colNameList: colName  */
#line 222 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_strs) = std::vector<std::string>{(yyvsp[0].sv_str)};
    }
#line 1936 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 32: /* colNameList: colNameList ',' colName  */
#line 226 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_strs).push_back((yyvsp[0].sv_str));
    }
#line 1944 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 33: /* field: colName type  */
#line 233 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_field) = std::make_shared<ColDef>((yyvsp[-1].sv_str), (yyvsp[0].sv_type_len));
    }
#line 1952 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 34: /* type: INT  */
#line 240 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_type_len) = std::make_shared<TypeLen>(SV_TYPE_INT, sizeof(int));
    }
#line 1960 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 35: /* type: CHAR '(' VALUE_INT ')'  */
#line 244 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_type_len) = std::make_shared<TypeLen>(SV_TYPE_STRING, (yyvsp[-1].sv_int));
    }
#line 1968 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 36: /* type: FLOAT  */
#line 248 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_type_len) = std::make_shared<TypeLen>(SV_TYPE_FLOAT, sizeof(float));
    }
#line 1976 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 37: /* valueList: value  */
#line 255 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_vals) = std::vector<std::shared_ptr<Value>>{(yyvsp[0].sv_val)};
    }
#line 1984 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 38: /* valueList: valueList ',' value  */
#line 259 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_vals).push_back((yyvsp[0].sv_val));
    }
#line 1992 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 39: /* value: VALUE_INT  */
#line 266 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<IntLit>((yyvsp[0].sv_int));
    }
#line 2000 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 40: /* value: VALUE_FLOAT  */
#line 270 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<FloatLit>((yyvsp[0].sv_float));
    }
#line 2008 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 41: /* value: VALUE_STRING  */
#line 274 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<StringLit>((yyvsp[0].sv_str));
    }
#line 2016 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 42: /* value: VALUE_BOOL  */
#line 278 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_val) = std::make_shared<BoolLit>((yyvsp[0].sv_bool));
    }
#line 2024 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 43: /* condition: col op expr  */
#line 285 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_cond) = std::make_shared<BinaryExpr>((yyvsp[-2].sv_col), (yyvsp[-1].sv_comp_op), (yyvsp[0].sv_expr));
    }
#line 2032 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 44: /* condition: expr op expr  */
#line 289 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_cond) = std::make_shared<BinaryExpr>((yyvsp[-2].sv_expr), (yyvsp[-1].sv_comp_op), (yyvsp[0].sv_expr));
    }
#line 2040 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 45: /* optGroupClause: %empty  */
#line 295 "/root/db2025-amatilasdb/src/parser/yacc.y"
                  { (yyval.sv_group_by_Clause) = nullptr; }
#line 2046 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 46: /* optGroupClause: GROUP BY GroupColList  */
#line 298 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
       (yyval.sv_group_by_Clause) = std::make_shared<GroupByClause>((yyvsp[0].sv_cols));
    }
#line 2054 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 47: /* GroupColList: col  */
#line 305 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_cols) = std::vector<std::shared_ptr<Col>>{(yyvsp[0].sv_col)};
    }
#line 2062 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 48: /* GroupColList: GroupColList ',' col  */
#line 309 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_cols).push_back((yyvsp[0].sv_col));
    }
#line 2070 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 49: /* optHavingClause: %empty  */
#line 315 "/root/db2025-amatilasdb/src/parser/yacc.y"
                  { (yyval.sv_having_clause) = nullptr; }
#line 2076 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 50: /* optHavingClause: HAVING havingConditions  */
#line 318 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_having_clause) = std::make_shared<HavingClause>((yyvsp[0].sv_conds));
    }
#line 2084 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 51: /* havingConditions: condition  */
#line 325 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_conds) = std::vector<std::shared_ptr<BinaryExpr>>{(yyvsp[0].sv_cond)};
    }
#line 2092 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 52: /* havingConditions: havingConditions AND condition  */
#line 330 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_conds).push_back((yyvsp[0].sv_cond));
    }
#line 2100 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 53: /* optWhereClause: %empty  */
#line 336 "/root/db2025-amatilasdb/src/parser/yacc.y"
                      { /* ignore*/ }
#line 2106 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 54: /* optWhereClause: WHERE whereClause  */
#line 338 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_conds) = (yyvsp[0].sv_conds);
    }
#line 2114 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 55: /* whereClause: condition  */
#line 345 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_conds) = std::vector<std::shared_ptr<BinaryExpr>>{(yyvsp[0].sv_cond)};
    }
#line 2122 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 56: /* whereClause: whereClause AND condition  */
#line 349 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_conds).push_back((yyvsp[0].sv_cond));
    }
#line 2130 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 57: /* col: tbName '.' colName  */
#line 356 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_col) = std::make_shared<Col>((yyvsp[-2].sv_str), (yyvsp[0].sv_str));
    }
#line 2138 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 58: /* col: colName  */
#line 360 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_col) = std::make_shared<Col>("", (yyvsp[0].sv_str));
    }
#line 2146 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 59: /* col: agg_type '(' colName ')'  */
#line 364 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_col) = std::make_shared<Col>("", (yyvsp[-1].sv_str), (yyvsp[-3].sv_agg_type));
    }
#line 2154 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 60: /* col: agg_type '(' tbName '.' colName ')'  */
#line 368 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_col) = std::make_shared<Col>("", (yyvsp[-1].sv_str), (yyvsp[-5].sv_agg_type));
    }
#line 2162 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 61: /* col: agg_type '(' '*' ')'  */
#line 372 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_col) = std::make_shared<Col>("", "*", (yyvsp[-3].sv_agg_type));
    }
#line 2170 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 62: /* col: tbName '.' colName AS colName  */
#line 376 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_col) = std::make_shared<Col>((yyvsp[-4].sv_str), (yyvsp[-2].sv_str), (yyvsp[0].sv_str));
    }
#line 2178 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 63: /* col: colName AS colName  */
#line 380 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_col) = std::make_shared<Col>("", (yyvsp[-2].sv_str), (yyvsp[0].sv_str));
    }
#line 2186 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 64: /* col: agg_type '(' colName ')' AS colName  */
#line 384 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_col) = std::make_shared<Col>("", (yyvsp[-3].sv_str), (yyvsp[-5].sv_agg_type), (yyvsp[0].sv_str));
    }
#line 2194 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 65: /* col: agg_type '(' tbName '.' colName ')' AS colName  */
#line 388 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_col) = std::make_shared<Col>((yyvsp[-5].sv_str), (yyvsp[-3].sv_str), (yyvsp[-7].sv_agg_type), (yyvsp[0].sv_str));
    }
#line 2202 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 66: /* col: agg_type '(' '*' ')' AS colName  */
#line 392 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_col) = std::make_shared<Col>("", "*", (yyvsp[-5].sv_agg_type), (yyvsp[0].sv_str));
    }
#line 2210 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 67: /* agg_type: SUM  */
#line 400 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_agg_type) = AggFuncType::SUM;
    }
#line 2218 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 68: /* agg_type: COUNT  */
#line 404 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_agg_type) = AggFuncType::COUNT;
    }
#line 2226 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 69: /* agg_type: MIN  */
#line 408 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_agg_type) = AggFuncType::MIN;
    }
#line 2234 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 70: /* agg_type: MAX  */
#line 412 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_agg_type) = AggFuncType::MAX;
    }
#line 2242 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 71: /* agg_type: AVG  */
#line 416 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_agg_type) = AggFuncType::AVG;
    }
#line 2250 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 72: /* colList: col  */
#line 424 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_cols) = std::vector<std::shared_ptr<Col>>{(yyvsp[0].sv_col)};
    }
#line 2258 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 73: /* colList: colList ',' col  */
#line 428 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_cols).push_back((yyvsp[0].sv_col));
    }
#line 2266 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 74: /* op: '='  */
#line 435 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_EQ;
    }
#line 2274 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 75: /* op: '<'  */
#line 439 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_LT;
    }
#line 2282 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 76: /* op: '>'  */
#line 443 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_GT;
    }
#line 2290 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 77: /* op: NEQ  */
#line 447 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_NE;
    }
#line 2298 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 78: /* op: LEQ  */
#line 451 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_LE;
    }
#line 2306 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 79: /* op: GEQ  */
#line 455 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_comp_op) = SV_OP_GE;
    }
#line 2314 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 80: /* expr: value  */
#line 462 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::static_pointer_cast<Expr>((yyvsp[0].sv_val));
    }
#line 2322 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 81: /* expr: col  */
#line 466 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::static_pointer_cast<Expr>((yyvsp[0].sv_col));
    }
#line 2330 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 82: /* expr: expr '+' expr  */
#line 470 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::make_shared<BinaryExpr>((yyvsp[-2].sv_expr), SV_OP_ADD, (yyvsp[0].sv_expr));
    }
#line 2338 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 83: /* expr: expr '-' expr  */
#line 474 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::make_shared<BinaryExpr>((yyvsp[-2].sv_expr), SV_OP_SUB, (yyvsp[0].sv_expr));
    }
#line 2346 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 84: /* expr: expr '*' expr  */
#line 478 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::make_shared<BinaryExpr>((yyvsp[-2].sv_expr), SV_OP_MUL, (yyvsp[0].sv_expr));
    }
#line 2354 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 85: /* expr: expr '/' expr  */
#line 482 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::make_shared<BinaryExpr>((yyvsp[-2].sv_expr), SV_OP_DIV, (yyvsp[0].sv_expr));
    }
#line 2362 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 86: /* expr: '-' expr  */
#line 486 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = std::make_shared<UnaryExpr>(SV_OP_NEG, (yyvsp[0].sv_expr));
    }
#line 2370 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 87: /* expr: '(' expr ')'  */
#line 490 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_expr) = (yyvsp[-1].sv_expr);
    }
#line 2378 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 88: /* setClauses: setClause  */
#line 497 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_set_clauses) = std::vector<std::shared_ptr<SetClause>>{(yyvsp[0].sv_set_clause)};
    }
#line 2386 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 89: /* setClauses: setClauses ',' setClause  */
#line 501 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_set_clauses).push_back((yyvsp[0].sv_set_clause));
    }
#line 2394 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 90: /* setClause: colName '=' value  */
#line 508 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_set_clause) = std::make_shared<SetClause>((yyvsp[-2].sv_str), (yyvsp[0].sv_val));
    }
#line 2402 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 91: /* setClause: colName '=' expr  */
#line 512 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_set_clause) = std::make_shared<SetClause>((yyvsp[-2].sv_str), (yyvsp[0].sv_expr));
    }
#line 2410 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 92: /* selector: '*'  */
#line 519 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_cols) = {};
    }
#line 2418 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 93: /* selector: colList  */
#line 523 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_cols) = (yyvsp[0].sv_cols);
    }
#line 2426 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 94: /* tableList: tbName  */
#line 530 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_table_list).tables = std::vector<std::string>{(yyvsp[0].sv_str)};
        (yyval.sv_table_list).joins = std::vector<std::shared_ptr<JoinExpr>>{};
        (yyval.sv_table_list).table_aliases = std::map<std::string, std::string>{};
    }
#line 2436 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 95: /* tableList: tableList ',' tbName  */
#line 536 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_table_list).tables = (yyvsp[-2].sv_table_list).tables;
        (yyval.sv_table_list).tables.push_back((yyvsp[0].sv_str));
        (yyval.sv_table_list).joins = (yyvsp[-2].sv_table_list).joins;
        (yyval.sv_table_list).table_aliases = (yyvsp[-2].sv_table_list).table_aliases;
    }
#line 2447 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 96: /* tableList: tableList JOIN tbName ON condition  */
#line 543 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_table_list).tables = (yyvsp[-4].sv_table_list).tables;
        (yyval.sv_table_list).tables.push_back((yyvsp[-2].sv_str));
        (yyval.sv_table_list).joins = (yyvsp[-4].sv_table_list).joins;
        (yyval.sv_table_list).joins.push_back(std::make_shared<JoinExpr>((yyvsp[-4].sv_table_list).tables.back(), (yyvsp[-2].sv_str), std::vector<std::shared_ptr<BinaryExpr>>{(yyvsp[0].sv_cond)}, INNER_JOIN));
        (yyval.sv_table_list).table_aliases = (yyvsp[-4].sv_table_list).table_aliases;
    }
#line 2459 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 97: /* tableList: tableList SEMI JOIN tbName ON condition  */
#line 551 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_table_list).tables = (yyvsp[-5].sv_table_list).tables;
        (yyval.sv_table_list).tables.push_back((yyvsp[-2].sv_str));
        (yyval.sv_table_list).joins = (yyvsp[-5].sv_table_list).joins;
        (yyval.sv_table_list).joins.push_back(std::make_shared<JoinExpr>((yyvsp[-5].sv_table_list).tables.back(), (yyvsp[-2].sv_str), std::vector<std::shared_ptr<BinaryExpr>>{(yyvsp[0].sv_cond)}, SEMI_JOIN));
        (yyval.sv_table_list).table_aliases = (yyvsp[-5].sv_table_list).table_aliases;
    }
#line 2471 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 98: /* opt_order_clause: ORDER BY order_list  */
#line 562 "/root/db2025-amatilasdb/src/parser/yacc.y"
    { 
        (yyval.sv_orderby) = std::make_shared<OrderBy>((yyvsp[0].sv_orderby_items)); 
    }
#line 2479 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 99: /* opt_order_clause: %empty  */
#line 565 "/root/db2025-amatilasdb/src/parser/yacc.y"
                      { (yyval.sv_orderby) = nullptr; }
#line 2485 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 100: /* order_list: order_item  */
#line 570 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_orderby_items) = std::vector<std::shared_ptr<OrderByItem>>{(yyvsp[0].sv_orderby_item)};
    }
#line 2493 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 101: /* order_list: order_list ',' order_item  */
#line 574 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_orderby_items).push_back((yyvsp[0].sv_orderby_item));
    }
#line 2501 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 102: /* order_item: col opt_asc_desc  */
#line 581 "/root/db2025-amatilasdb/src/parser/yacc.y"
    { 
        (yyval.sv_orderby_item) = std::make_shared<OrderByItem>((yyvsp[-1].sv_col), (yyvsp[0].sv_orderby_dir));
    }
#line 2509 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 103: /* opt_asc_desc: ASC  */
#line 587 "/root/db2025-amatilasdb/src/parser/yacc.y"
                 { (yyval.sv_orderby_dir) = OrderBy_ASC;     }
#line 2515 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 104: /* opt_asc_desc: DESC_ORDER  */
#line 588 "/root/db2025-amatilasdb/src/parser/yacc.y"
                  { (yyval.sv_orderby_dir) = OrderBy_DESC;    }
#line 2521 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 105: /* opt_asc_desc: %empty  */
#line 589 "/root/db2025-amatilasdb/src/parser/yacc.y"
            { (yyval.sv_orderby_dir) = OrderBy_DEFAULT; }
#line 2527 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 106: /* opt_limit_clause: LIMIT VALUE_INT  */
#line 594 "/root/db2025-amatilasdb/src/parser/yacc.y"
    {
        (yyval.sv_int) = (yyvsp[0].sv_int);
    }
#line 2535 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 107: /* opt_limit_clause: %empty  */
#line 597 "/root/db2025-amatilasdb/src/parser/yacc.y"
                    { (yyval.sv_int) = -1; }
#line 2541 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 108: /* set_knob_type: ENABLE_NESTLOOP  */
#line 601 "/root/db2025-amatilasdb/src/parser/yacc.y"
                    { (yyval.sv_setKnobType) = EnableNestLoop; }
#line 2547 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;

  case 109: /* set_knob_type: ENABLE_SORTMERGE  */
#line 602 "/root/db2025-amatilasdb/src/parser/yacc.y"
                         { (yyval.sv_setKnobType) = EnableSortMerge; }
#line 2553 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"
    break;


#line 2557 "/root/db2025-amatilasdb/src/parser/yacc.tab.cpp"

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
  *++yylsp = yyloc;

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
      {
        yypcontext_t yyctx
          = {yyssp, yytoken, &yylloc};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (&yylloc, yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  yyerror_range[1] = yylloc;
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
                      yytoken, &yylval, &yylloc);
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

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, yylsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  ++yylsp;
  YYLLOC_DEFAULT (*yylsp, yyerror_range, 2);

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
  yyerror (&yylloc, YY_("memory exhausted"));
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
                  yytoken, &yylval, &yylloc);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, yylsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 608 "/root/db2025-amatilasdb/src/parser/yacc.y"

