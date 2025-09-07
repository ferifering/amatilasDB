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

#ifndef YY_YY_ROOT_DB2025_AMATILASDB_SRC_PARSER_YACC_TAB_H_INCLUDED
# define YY_YY_ROOT_DB2025_AMATILASDB_SRC_PARSER_YACC_TAB_H_INCLUDED
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
    SHOW = 258,                    /* SHOW  */
    TABLES = 259,                  /* TABLES  */
    CREATE = 260,                  /* CREATE  */
    TABLE = 261,                   /* TABLE  */
    DROP = 262,                    /* DROP  */
    DESC = 263,                    /* DESC  */
    INSERT = 264,                  /* INSERT  */
    INTO = 265,                    /* INTO  */
    VALUES = 266,                  /* VALUES  */
    DELETE = 267,                  /* DELETE  */
    FROM = 268,                    /* FROM  */
    ASC = 269,                     /* ASC  */
    DESC_ORDER = 270,              /* DESC_ORDER  */
    ORDER = 271,                   /* ORDER  */
    BY = 272,                      /* BY  */
    IN = 273,                      /* IN  */
    LIMIT = 274,                   /* LIMIT  */
    WHERE = 275,                   /* WHERE  */
    UPDATE = 276,                  /* UPDATE  */
    SET = 277,                     /* SET  */
    SELECT = 278,                  /* SELECT  */
    INT = 279,                     /* INT  */
    CHAR = 280,                    /* CHAR  */
    FLOAT = 281,                   /* FLOAT  */
    INDEX = 282,                   /* INDEX  */
    AND = 283,                     /* AND  */
    JOIN = 284,                    /* JOIN  */
    SEMI = 285,                    /* SEMI  */
    ON = 286,                      /* ON  */
    EXIT = 287,                    /* EXIT  */
    HELP = 288,                    /* HELP  */
    TXN_BEGIN = 289,               /* TXN_BEGIN  */
    TXN_COMMIT = 290,              /* TXN_COMMIT  */
    TXN_ABORT = 291,               /* TXN_ABORT  */
    TXN_ROLLBACK = 292,            /* TXN_ROLLBACK  */
    ORDER_BY = 293,                /* ORDER_BY  */
    ENABLE_NESTLOOP = 294,         /* ENABLE_NESTLOOP  */
    ENABLE_SORTMERGE = 295,        /* ENABLE_SORTMERGE  */
    STATIC_CHECKPOINT = 296,       /* STATIC_CHECKPOINT  */
    EXPLAIN = 297,                 /* EXPLAIN  */
    UMINUS = 298,                  /* UMINUS  */
    AVG = 299,                     /* AVG  */
    SUM = 300,                     /* SUM  */
    COUNT = 301,                   /* COUNT  */
    MAX = 302,                     /* MAX  */
    MIN = 303,                     /* MIN  */
    AS = 304,                      /* AS  */
    GROUP = 305,                   /* GROUP  */
    HAVING = 306,                  /* HAVING  */
    LEQ = 307,                     /* LEQ  */
    NEQ = 308,                     /* NEQ  */
    GEQ = 309,                     /* GEQ  */
    T_EOF = 310,                   /* T_EOF  */
    IDENTIFIER = 311,              /* IDENTIFIER  */
    VALUE_STRING = 312,            /* VALUE_STRING  */
    VALUE_INT = 313,               /* VALUE_INT  */
    VALUE_FLOAT = 314,             /* VALUE_FLOAT  */
    VALUE_BOOL = 315               /* VALUE_BOOL  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif




int yyparse (void);


#endif /* !YY_YY_ROOT_DB2025_AMATILASDB_SRC_PARSER_YACC_TAB_H_INCLUDED  */
