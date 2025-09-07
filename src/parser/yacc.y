%{
#include "ast.h"
#include "yacc.tab.h"
#include <iostream>
#include <memory>

int yylex(YYSTYPE *yylval, YYLTYPE *yylloc);

void yyerror(YYLTYPE *locp, const char* s) {
    std::cerr << "Parser Error at line " << locp->first_line << " column " << locp->first_column << ": " << s << std::endl;
}

using namespace ast;
%}

// request a pure (reentrant) parser
%define api.pure full
// enable location in error handler
%locations
// enable verbose syntax error message
%define parse.error verbose

// keywords
%token SHOW TABLES CREATE TABLE DROP DESC INSERT INTO VALUES DELETE FROM ASC DESC_ORDER ORDER BY IN LIMIT
WHERE UPDATE SET SELECT INT CHAR FLOAT INDEX AND JOIN SEMI ON EXIT HELP TXN_BEGIN TXN_COMMIT TXN_ABORT TXN_ROLLBACK ORDER_BY ENABLE_NESTLOOP ENABLE_SORTMERGE STATIC_CHECKPOINT EXPLAIN

// arithmetic operators
%left '+' '-'
%left '*' '/'
%left UMINUS


//AGG OPS
%token AVG SUM COUNT MAX MIN
%token AS
//GROUP
%token GROUP HAVING

// non-keywords
%token LEQ NEQ GEQ T_EOF

// type-specific tokens
%token <sv_str> IDENTIFIER VALUE_STRING
%token <sv_int> VALUE_INT
%token <sv_float> VALUE_FLOAT
%token <sv_bool> VALUE_BOOL

// specify types for non-terminal symbol
%type <sv_node> stmt dbStmt ddl dml txnStmt setStmt
%type <sv_field> field
%type <sv_fields> fieldList
%type <sv_type_len> type
%type <sv_comp_op> op
%type <sv_expr> expr
%type <sv_val> value
%type <sv_vals> valueList
%type <sv_str> tbName colName
%type <sv_table_list> tableList
%type <sv_strs> colNameList
%type <sv_col> col
%type <sv_cols> colList selector
%type <sv_cols> group_by_list
%type <sv_set_clause> setClause
%type <sv_set_clauses> setClauses
%type <sv_cond> condition
%type <sv_conds> whereClause optWhereClause havingClause
%type <sv_orderby>  order_clause opt_order_clause
%type <sv_orderby_item> order_item
%type <sv_orderby_items> order_list
%type <sv_orderby_dir> opt_asc_desc
%type <sv_int> opt_limit_clause
%type <sv_setKnobType> set_knob_type

//AGG
%type <sv_agg_type> agg_type
//GROUP BY
%type <sv_group_by_Clause> optGroupClause
%type <sv_cols> GroupColList
//HAVING
%type <sv_having_clause> optHavingClause
%type <sv_conds> havingConditions


%%
start:
        stmt ';'
    {
        parse_tree = $1;
        YYACCEPT;
    }
    |   HELP
    {
        parse_tree = std::make_shared<Help>();
        YYACCEPT;
    }
    |   EXIT
    {
        parse_tree = nullptr;
        YYACCEPT;
    }
    |   T_EOF
    {
        parse_tree = nullptr;
        YYACCEPT;
    }
    ;

stmt:
        dbStmt
    |   ddl
    |   dml
    |   txnStmt
    |   setStmt
    ;

txnStmt:
        TXN_BEGIN
    {
        $$ = std::make_shared<TxnBegin>();
    }
    |   TXN_COMMIT
    {
        $$ = std::make_shared<TxnCommit>();
    }
    |   TXN_ABORT
    {
        $$ = std::make_shared<TxnAbort>();
    }
    | TXN_ROLLBACK
    {
        $$ = std::make_shared<TxnRollback>();
    }
    | CREATE STATIC_CHECKPOINT
    {
        $$ = std::make_shared<CreateCheckpoint>();
    }
    ;

dbStmt:
        SHOW TABLES
    {
        $$ = std::make_shared<ShowTables>();
    }
    ;

setStmt:
        SET set_knob_type '=' VALUE_BOOL
    {
        $$ = std::make_shared<SetStmt>($2, $4);
    }
    ;

ddl:
        CREATE TABLE tbName '(' fieldList ')'
    {
        $$ = std::make_shared<CreateTable>($3, $5);
    }
    |   DROP TABLE tbName
    {
        $$ = std::make_shared<DropTable>($3);
    }
    |   DESC_ORDER tbName
    {
        $$ = std::make_shared<DescTable>($2);
    }
    |   CREATE INDEX tbName '(' colNameList ')'
    {
        $$ = std::make_shared<CreateIndex>($3, $5);
    }
    |   DROP INDEX tbName '(' colNameList ')'
    {
        $$ = std::make_shared<DropIndex>($3, $5);
    }
    |   SHOW INDEX FROM tbName
    {
        $$ = std::make_shared<ShowIndex>($4);
    }
    ;

dml:
        INSERT INTO tbName VALUES '(' valueList ')'
    {
        $$ = std::make_shared<InsertStmt>($3, $6);
    }
    |   DELETE FROM tbName optWhereClause
    {
        $$ = std::make_shared<DeleteStmt>($3, $4);
    }
    |   UPDATE tbName SET setClauses optWhereClause
    {
        $$ = std::make_shared<UpdateStmt>($2, $4, $5);
    }
    |   SELECT selector FROM tableList optWhereClause optGroupClause optHavingClause opt_order_clause opt_limit_clause
    {
        auto select_stmt = std::make_shared<SelectStmt>($2, $4.tables, $5, $6, $7, $8, $9);
        select_stmt->jointree = $4.joins;
        select_stmt->table_aliases = $4.table_aliases;
        $$ = select_stmt;
    }
    |   EXPLAIN SELECT selector FROM tableList optWhereClause optGroupClause optHavingClause opt_order_clause opt_limit_clause
    {
        auto select_stmt = std::make_shared<SelectStmt>($3, $5.tables, $6, $7, $8, $9, $10);
        select_stmt->jointree = $5.joins;
        select_stmt->table_aliases = $5.table_aliases;
        $$ = std::make_shared<ExplainStmt>(select_stmt);
    }
    ;

fieldList:
        field
    {
        $$ = std::vector<std::shared_ptr<Field>>{$1};
    }
    |   fieldList ',' field
    {
        $$.push_back($3);
    }
    ;

colNameList:
        colName
    {
        $$ = std::vector<std::string>{$1};
    }
    | colNameList ',' colName
    {
        $$.push_back($3);
    }
    ;

field:
        colName type
    {
        $$ = std::make_shared<ColDef>($1, $2);
    }
    ;

type:
        INT
    {
        $$ = std::make_shared<TypeLen>(SV_TYPE_INT, sizeof(int));
    }
    |   CHAR '(' VALUE_INT ')'
    {
        $$ = std::make_shared<TypeLen>(SV_TYPE_STRING, $3);
    }
    |   FLOAT
    {
        $$ = std::make_shared<TypeLen>(SV_TYPE_FLOAT, sizeof(float));
    }
    ;

valueList:
        value
    {
        $$ = std::vector<std::shared_ptr<Value>>{$1};
    }
    |   valueList ',' value
    {
        $$.push_back($3);
    }
    ;

value:
        VALUE_INT
    {
        $$ = std::make_shared<IntLit>($1);
    }
    |   VALUE_FLOAT
    {
        $$ = std::make_shared<FloatLit>($1);
    }
    |   VALUE_STRING
    {
        $$ = std::make_shared<StringLit>($1);
    }
    |   VALUE_BOOL
    {
        $$ = std::make_shared<BoolLit>($1);
    }
    ;

condition:
        col op expr
    {
        $$ = std::make_shared<BinaryExpr>($1, $2, $3);
    }
    |   expr op expr
    {
        $$ = std::make_shared<BinaryExpr>($1, $2, $3);
    }
    ;

optGroupClause:
    /* epsilon */ { $$ = nullptr; }
    |
    GROUP BY GroupColList
    {
       $$ = std::make_shared<GroupByClause>($3);
    }
    ;

GroupColList:
    col
    {
        $$ = std::vector<std::shared_ptr<Col>>{$1};
    }
    | GroupColList ',' col
    {
        $$.push_back($3);
    }
    ;

optHavingClause:
    /* epsilon */ { $$ = nullptr; }
    |
    HAVING havingConditions
    {
        $$ = std::make_shared<HavingClause>($2);
    }
    ;

havingConditions:
    condition
    {
        $$ = std::vector<std::shared_ptr<BinaryExpr>>{$1};
    }
    |
    havingConditions AND condition
    {
        $$.push_back($3);
    }
    ;

optWhereClause:
        /* epsilon */ { /* ignore*/ }
    |   WHERE whereClause
    {
        $$ = $2;
    }
    ;

whereClause:
        condition 
    {
        $$ = std::vector<std::shared_ptr<BinaryExpr>>{$1};
    }
    |   whereClause AND condition
    {
        $$.push_back($3);
    }
    ;

col:
        tbName '.' colName
    {
        $$ = std::make_shared<Col>($1, $3);
    }
    |   colName
    {
        $$ = std::make_shared<Col>("", $1);
    }
    |   agg_type '(' colName ')'
    {
        $$ = std::make_shared<Col>("", $3, $1);
    }
    |   agg_type '(' tbName '.' colName ')'
    {
        $$ = std::make_shared<Col>("", $5, $1);
    }
    |   agg_type '(' '*' ')'
    {
        $$ = std::make_shared<Col>("", "*", $1);
    }
    |   tbName '.' colName AS colName
    {
        $$ = std::make_shared<Col>($1, $3, $5);
    }
    |   colName AS colName
    {
        $$ = std::make_shared<Col>("", $1, $3);
    }
    |   agg_type '(' colName ')' AS colName
    {
        $$ = std::make_shared<Col>("", $3, $1, $6);
    }
    |   agg_type '(' tbName '.' colName ')' AS colName
    {
        $$ = std::make_shared<Col>($3, $5, $1, $8);
    }
    |   agg_type '(' '*' ')' AS colName
    {
        $$ = std::make_shared<Col>("", "*", $1, $6);
    }
    ;

// 定义聚合函数的规则
agg_type:
    SUM
    {
        $$ = AggFuncType::SUM;
    }
    | COUNT
    {
        $$ = AggFuncType::COUNT;
    }
    | MIN
    {
        $$ = AggFuncType::MIN;
    }
    | MAX
    {
        $$ = AggFuncType::MAX;
    }
    | AVG
    {
        $$ = AggFuncType::AVG;
    }
    ;


colList:
        col
    {
        $$ = std::vector<std::shared_ptr<Col>>{$1};
    }
    |   colList ',' col
    {
        $$.push_back($3);
    }
    ;

op:
        '='
    {
        $$ = SV_OP_EQ;
    }
    |   '<'
    {
        $$ = SV_OP_LT;
    }
    |   '>'
    {
        $$ = SV_OP_GT;
    }
    |   NEQ
    {
        $$ = SV_OP_NE;
    }
    |   LEQ
    {
        $$ = SV_OP_LE;
    }
    |   GEQ
    {
        $$ = SV_OP_GE;
    }
    ;

expr:
        value
    {
        $$ = std::static_pointer_cast<Expr>($1);
    }
    |   col
    {
        $$ = std::static_pointer_cast<Expr>($1);
    }
    |   expr '+' expr
    {
        $$ = std::make_shared<BinaryExpr>($1, SV_OP_ADD, $3);
    }
    |   expr '-' expr
    {
        $$ = std::make_shared<BinaryExpr>($1, SV_OP_SUB, $3);
    }
    |   expr '*' expr
    {
        $$ = std::make_shared<BinaryExpr>($1, SV_OP_MUL, $3);
    }
    |   expr '/' expr
    {
        $$ = std::make_shared<BinaryExpr>($1, SV_OP_DIV, $3);
    }
    |   '-' expr %prec UMINUS
    {
        $$ = std::make_shared<UnaryExpr>(SV_OP_NEG, $2);
    }
    |   '(' expr ')'
    {
        $$ = $2;
    }
    ;

setClauses:
        setClause
    {
        $$ = std::vector<std::shared_ptr<SetClause>>{$1};
    }
    |   setClauses ',' setClause
    {
        $$.push_back($3);
    }
    ;

setClause:
        colName '=' value
    {
        $$ = std::make_shared<SetClause>($1, $3);
    }
    |   colName '=' expr
    {
        $$ = std::make_shared<SetClause>($1, $3);
    }
    ;

selector:
        '*'
    {
        $$ = {};
    }
    |   colList
    {
        $$ = $1;
    }
    ;

tableList:
        tbName
    {
        $$.tables = std::vector<std::string>{$1};
        $$.joins = std::vector<std::shared_ptr<JoinExpr>>{};
        $$.table_aliases = std::map<std::string, std::string>{};
    }
    |   tableList ',' tbName
    {
        $$.tables = $1.tables;
        $$.tables.push_back($3);
        $$.joins = $1.joins;
        $$.table_aliases = $1.table_aliases;
    }
    |   tableList JOIN tbName ON condition
    {
        $$.tables = $1.tables;
        $$.tables.push_back($3);
        $$.joins = $1.joins;
        $$.joins.push_back(std::make_shared<JoinExpr>($1.tables.back(), $3, std::vector<std::shared_ptr<BinaryExpr>>{$5}, INNER_JOIN));
        $$.table_aliases = $1.table_aliases;
    }
    |   tableList SEMI JOIN tbName ON condition
    {
        $$.tables = $1.tables;
        $$.tables.push_back($4);
        $$.joins = $1.joins;
        $$.joins.push_back(std::make_shared<JoinExpr>($1.tables.back(), $4, std::vector<std::shared_ptr<BinaryExpr>>{$6}, SEMI_JOIN));
        $$.table_aliases = $1.table_aliases;
    }
    ;

opt_order_clause:
    ORDER BY order_list      
    { 
        $$ = std::make_shared<OrderBy>($3); 
    }
    |   /* epsilon */ { $$ = nullptr; }
    ;

order_list:
    order_item
    {
        $$ = std::vector<std::shared_ptr<OrderByItem>>{$1};
    }
    | order_list ',' order_item
    {
        $$.push_back($3);
    }
    ;

order_item:
    col opt_asc_desc 
    { 
        $$ = std::make_shared<OrderByItem>($1, $2);
    }
    ;   

opt_asc_desc:
    ASC          { $$ = OrderBy_ASC;     }
    |  DESC_ORDER { $$ = OrderBy_DESC;    }
    |       { $$ = OrderBy_DEFAULT; }
    ;

opt_limit_clause:
    LIMIT VALUE_INT
    {
        $$ = $2;
    }
    | /* epsilon */ { $$ = -1; }
    ;    

set_knob_type:
    ENABLE_NESTLOOP { $$ = EnableNestLoop; }
    |   ENABLE_SORTMERGE { $$ = EnableSortMerge; }
    ;

tbName: IDENTIFIER;

colName: IDENTIFIER;
%%
