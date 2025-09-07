/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */
#pragma once

#include <vector>
#include <string>
#include <memory>
#include <map>

enum JoinType {
    INNER_JOIN, LEFT_JOIN, RIGHT_JOIN, FULL_JOIN, SEMI_JOIN
};
namespace ast {

    enum SvType {
        SV_TYPE_INT, SV_TYPE_FLOAT, SV_TYPE_STRING, SV_TYPE_BOOL
    };

    enum SvCompOp {
        SV_OP_EQ, SV_OP_NE, SV_OP_LT, SV_OP_GT, SV_OP_LE, SV_OP_GE,
        SV_OP_ADD, SV_OP_SUB, SV_OP_MUL, SV_OP_DIV, SV_OP_NEG
    };

    enum AggFuncType {
        COUNT_ALL, COUNT, MAX, MIN, SUM, AVG
    };

    enum OrderByDir {
        OrderBy_DEFAULT,
        OrderBy_ASC,
        OrderBy_DESC
    };

    enum SetKnobType {
        EnableNestLoop, EnableSortMerge
    };

// Base class for tree nodes
    struct TreeNode {
        virtual ~TreeNode() = default;  // enable polymorphism
    };

    struct Help : public TreeNode {
    };

    struct ShowTables : public TreeNode {
    };

    struct TxnBegin : public TreeNode {
    };

    struct TxnCommit : public TreeNode {
    };

    struct TxnAbort : public TreeNode {
    };

    struct TxnRollback : public TreeNode {
    };

    struct CreateCheckpoint : public TreeNode {
    };

    struct TypeLen : public TreeNode {
        SvType type;
        int len;

        TypeLen(SvType type_, int len_) : type(type_), len(len_) {}
    };

    struct Field : public TreeNode {
    };

    struct ColDef : public Field {
        std::string col_name;
        std::shared_ptr<TypeLen> type_len;

        ColDef(std::string col_name_, std::shared_ptr<TypeLen> type_len_) :
                col_name(std::move(col_name_)), type_len(std::move(type_len_)) {}
    };

    struct CreateTable : public TreeNode {
        std::string tab_name;
        std::vector<std::shared_ptr<Field>> fields;

        CreateTable(std::string tab_name_, std::vector<std::shared_ptr<Field>> fields_) :
                tab_name(std::move(tab_name_)), fields(std::move(fields_)) {}
    };

    struct DropTable : public TreeNode {
        std::string tab_name;

        DropTable(std::string tab_name_) : tab_name(std::move(tab_name_)) {}
    };

    struct DescTable : public TreeNode {
        std::string tab_name;

        DescTable(std::string tab_name_) : tab_name(std::move(tab_name_)) {}
    };

    struct CreateIndex : public TreeNode {
        std::string tab_name;
        std::vector<std::string> col_names;

        CreateIndex(std::string tab_name_, std::vector<std::string> col_names_) :
                tab_name(std::move(tab_name_)), col_names(std::move(col_names_)) {}
    };

    struct DropIndex : public TreeNode {
        std::string tab_name;
        std::vector<std::string> col_names;

        DropIndex(std::string tab_name_, std::vector<std::string> col_names_) :
                tab_name(std::move(tab_name_)), col_names(std::move(col_names_)) {}
    };

    struct ShowIndex : public TreeNode {
        std::string tab_name;

        ShowIndex(std::string tab_name_) : tab_name(std::move(tab_name_)) {
        }
    };

    struct Expr : public TreeNode {
    };

    struct Value : public Expr {
    };

    struct IntLit : public Value {
        int val;

        IntLit(int val_) : val(val_) {}
    };

    struct FloatLit : public Value {
        float val;

        FloatLit(float val_) : val(val_) {}
    };

    struct StringLit : public Value {
        std::string val;

        StringLit(std::string val_) : val(std::move(val_)) {}
    };

    struct BoolLit : public Value {
        bool val;

        BoolLit(bool val_) : val(val_) {}
    };

    struct Col : public Expr {
        std::string tab_name;
        std::string col_name;
        bool isAgg;
        AggFuncType aggType;
        std::string alias;

        Col(std::string tab_name_, std::string col_name_) :
                tab_name(std::move(tab_name_)), col_name(std::move(col_name_)) {
            isAgg = false;
            alias = "";
        }

        Col(std::string tab_name_, std::string col_name_, AggFuncType type_) :
                tab_name(std::move(tab_name_)), col_name(std::move(col_name_)) {
            isAgg = true;
            alias = "";
            if (col_name_ == "*") {
                aggType = AggFuncType::COUNT_ALL;
            } else {
                aggType = type_;
            }
        }

        Col(std::string tab_name_, std::string col_name_, std::string alias_) :
                tab_name(std::move(tab_name_)), col_name(std::move(col_name_)), alias(std::move(alias_)) {
            isAgg = false;
        }

        Col(std::string tab_name_, std::string col_name_, AggFuncType type_, std::string alias_) :
                tab_name(std::move(tab_name_)), col_name(std::move(col_name_)), alias(std::move(alias_)) {
            isAgg = true;
            if (col_name_ == "*" ) {
                aggType = AggFuncType::COUNT_ALL;
            } else {
                aggType = type_;
            }
        }
    };

    struct AggreCol : public Col
    {
        AggFuncType aggre_type;
        AggreCol(std::shared_ptr<Col> col_, AggFuncType aggre_type_)
                : Col(col_->tab_name, col_->col_name),
                  aggre_type(aggre_type_)
        {}
        AggreCol(std::shared_ptr<Col> col_, AggFuncType aggre_type_, std::string alias_)
                : Col(col_->tab_name, col_->col_name, alias_),
                  aggre_type(aggre_type_)
        {}
    };


    struct SetClause : public TreeNode {
        std::string col_name;
        std::shared_ptr<Value> val;
        std::shared_ptr<Expr> expr;
        bool is_expr;

        //2t增量更新add
        bool isAdd;

        // Constructor for value assignment
        SetClause(std::string col_name_, std::shared_ptr<Value> val_) :
                col_name(std::move(col_name_)), val(std::move(val_)), is_expr(false) {}
        
        // Constructor for expression assignment
        SetClause(std::string col_name_, std::shared_ptr<Expr> expr_) :
                col_name(std::move(col_name_)), expr(std::move(expr_)), is_expr(true) {}
    };

    struct BinaryExpr : public Expr {
        std::shared_ptr<Expr> lhs;
        SvCompOp op;
        std::shared_ptr<Expr> rhs;

        BinaryExpr(std::shared_ptr<Col> lhs_, SvCompOp op_, std::shared_ptr<Expr> rhs_) :
                lhs(std::move(lhs_)), op(op_), rhs(std::move(rhs_)) {}
        
        BinaryExpr(std::shared_ptr<Expr> lhs_, SvCompOp op_, std::shared_ptr<Expr> rhs_) :
                lhs(std::move(lhs_)), op(op_), rhs(std::move(rhs_)) {}
    };

    struct UnaryExpr : public Expr {
        SvCompOp op;
        std::shared_ptr<Expr> operand;

        UnaryExpr(SvCompOp op_, std::shared_ptr<Expr> operand_) :
                op(op_), operand(std::move(operand_)) {}
    };

    struct InsertStmt : public TreeNode {
        std::string tab_name;
        std::vector<std::shared_ptr<Value>> vals;

        InsertStmt(std::string tab_name_, std::vector<std::shared_ptr<Value>> vals_) :
                tab_name(std::move(tab_name_)), vals(std::move(vals_)) {}
    };

    struct DeleteStmt : public TreeNode {
        std::string tab_name;
        std::vector<std::shared_ptr<BinaryExpr>> conds;

        DeleteStmt(std::string tab_name_, std::vector<std::shared_ptr<BinaryExpr>> conds_) :
                tab_name(std::move(tab_name_)), conds(std::move(conds_)) {}
    };

    struct UpdateStmt : public TreeNode {
        std::string tab_name;
        std::vector<std::shared_ptr<SetClause>> set_clauses;
        std::vector<std::shared_ptr<BinaryExpr>> conds;

        UpdateStmt(std::string tab_name_,
                   std::vector<std::shared_ptr<SetClause>> set_clauses_,
                   std::vector<std::shared_ptr<BinaryExpr>> conds_) :
                tab_name(std::move(tab_name_)), set_clauses(std::move(set_clauses_)), conds(std::move(conds_)) {}
    };

    struct JoinExpr : public TreeNode {
        std::string left;
        std::string right;
        std::vector<std::shared_ptr<BinaryExpr>> conds;
        JoinType type;

        JoinExpr(std::string left_, std::string right_,
                 std::vector<std::shared_ptr<BinaryExpr>> conds_, JoinType type_) :
                left(std::move(left_)), right(std::move(right_)), conds(std::move(conds_)), type(type_) {}
    };

    struct GroupByClause : public TreeNode {
        std::vector<std::shared_ptr<Col>> group_by_cols;

        GroupByClause(std::vector<std::shared_ptr<Col>> group_by_cols_) : group_by_cols(std::move(group_by_cols_)) {}
    };

    struct HavingClause : public TreeNode {
        std::vector<std::shared_ptr<BinaryExpr>> conds;

        HavingClause(std::vector<std::shared_ptr<BinaryExpr>> conds_) : conds(std::move(conds_)) {}
    };

    struct OrderByItem : public TreeNode {
        std::shared_ptr<Col> col;
        OrderByDir orderby_dir;

        OrderByItem(std::shared_ptr<Col> col_, OrderByDir orderby_dir_) :
                col(std::move(col_)), orderby_dir(orderby_dir_) {}
    };

    struct OrderBy : public TreeNode {
        std::vector<std::shared_ptr<OrderByItem>> order_items;

        OrderBy(std::vector<std::shared_ptr<OrderByItem>> order_items_) :
                order_items(std::move(order_items_)) {}
    };

    struct SelectStmt : public TreeNode {
        std::vector<std::shared_ptr<Col>> cols;
        std::vector<std::string> tabs;
        std::map<std::string, std::string> table_aliases; // alias -> table_name mapping
        std::vector<std::shared_ptr<BinaryExpr>> conds;
        std::vector<std::shared_ptr<JoinExpr>> jointree;
        std::shared_ptr<GroupByClause> group_by_clause;
        std::shared_ptr<HavingClause> having_clause;
        std::shared_ptr<OrderBy> order;
        int limit_count;

        bool has_sort{false};
        bool has_group_by{false};
        bool has_having{false};
        bool has_subquery{false};
        bool has_limit{false};


        SelectStmt(
                std::vector<std::shared_ptr<Col>> cols_,
                std::vector<std::string> tabs_,
                std::vector<std::shared_ptr<BinaryExpr>> conds_,
                std::shared_ptr<GroupByClause> group_by_Clause_,
                std::shared_ptr<HavingClause> having_clause_,
                std::shared_ptr<OrderBy> order_,
                int limit_count_ = -1
        )
                : cols(std::move(cols_)),
                  tabs(std::move(tabs_)),
                  conds(std::move(conds_)),
                  group_by_clause(std::move(group_by_Clause_)),
                  having_clause(std::move(having_clause_)),
                  order(std::move(order_)),
                  limit_count(limit_count_) {
            has_sort = (bool) order;
            has_group_by = (bool) group_by_clause;
            has_having = (bool) having_clause;
            has_limit = (limit_count > 0);
        }
    };

    struct ExplainStmt : public TreeNode {
        std::shared_ptr<SelectStmt> select_stmt;

        ExplainStmt(std::shared_ptr<SelectStmt> select_stmt_) :
                select_stmt(std::move(select_stmt_)) {}
    };

// set enable_nestloop
    struct SetStmt : public TreeNode {
        SetKnobType set_knob_type_;
        bool bool_val_;

        SetStmt(SetKnobType &type, bool bool_value) :
                set_knob_type_(type), bool_val_(bool_value) {}
    };

// Semantic value
    struct SemValue {
        int                      sv_int;
        float                    sv_float;
        std::string              sv_str;
        bool                     sv_bool;
        OrderByDir               sv_orderby_dir;
        std::vector<std::string> sv_strs;

        std::shared_ptr<TreeNode> sv_node;

        SvCompOp sv_comp_op;

        std::shared_ptr<TypeLen> sv_type_len;

        std::shared_ptr<Field>              sv_field;
        std::vector<std::shared_ptr<Field>> sv_fields;

        std::shared_ptr<Expr> sv_expr;

        std::shared_ptr<Value>              sv_val;
        std::vector<std::shared_ptr<Value>> sv_vals;

        std::shared_ptr<Col>              sv_col;
        std::vector<std::shared_ptr<Col>> sv_cols;

        std::shared_ptr<SetClause>              sv_set_clause;
        std::vector<std::shared_ptr<SetClause>> sv_set_clauses;

        std::shared_ptr<BinaryExpr>                      sv_cond;
        std::vector<std::shared_ptr<BinaryExpr>>         sv_conds;

        std::shared_ptr<OrderBy> sv_orderby;
        std::shared_ptr<OrderByItem> sv_orderby_item;
        std::vector<std::shared_ptr<OrderByItem>> sv_orderby_items;

        SetKnobType sv_setKnobType;

        AggFuncType                            sv_agg_type;
        std::shared_ptr<AggreCol>              sv_aggre;
        std::vector<std::shared_ptr<AggreCol>> sv_aggres;
        std::shared_ptr<GroupByClause>         sv_group_by_Clause;
        std::shared_ptr<HavingClause>          sv_having_clause;

        std::shared_ptr<JoinExpr>              sv_join;
        std::vector<std::shared_ptr<JoinExpr>> sv_joins;

        struct {
            std::vector<std::string> tables;
            std::vector<std::shared_ptr<JoinExpr>> joins;
            std::map<std::string, std::string> table_aliases; // alias -> table_name mapping
        } sv_table_list;

    };

    extern std::shared_ptr<ast::TreeNode> parse_tree;

}
//        int sv_int;
//        float sv_float;
//        std::string sv_str;
//        bool sv_bool;
//        OrderByDir sv_orderby_dir;
//        std::vector<std::string> sv_strs;
//
//        std::shared_ptr<TreeNode> sv_node;
//
//        SvCompOp sv_comp_op;
//
//        AggregateType sv_agg_type;
//        AggFuncType sv_agg_func_type;
//
//        std::shared_ptr<TypeLen> sv_type_len;
//
//        std::shared_ptr<Field> sv_field;
//        std::vector<std::shared_ptr<Field>> sv_fields;
//
//        std::shared_ptr<Expr> sv_expr;
//        std::vector<std::shared_ptr<Expr>> sv_exprs;
//
//        std::shared_ptr<Value> sv_val;
//        std::vector<std::shared_ptr<Value>> sv_vals;
//
//        std::shared_ptr<Col> sv_col;
//        std::vector<std::shared_ptr<Col>> sv_cols;
//
//        std::shared_ptr<SetClause> sv_set_clause;
//        std::vector<std::shared_ptr<SetClause>> sv_set_clauses;
//
//        std::shared_ptr<BinaryExpr> sv_cond;
//        std::vector<std::shared_ptr<BinaryExpr>> sv_conds;
//
//        std::shared_ptr<OrderBy> sv_orderby;
//        std::shared_ptr<GroupBy> sv_group_by;
//
//        SetKnobType sv_setKnobType;

#define YYSTYPE ast::SemValue
