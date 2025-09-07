/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "analyze.h"

/**
 * @description: 分析器，进行语义分析和查询重写，需要检查不符合语义规定的部分
 * @param {shared_ptr<ast::TreeNode>} parse parser生成的结果集
 * @return {shared_ptr<Query>} Query
 */
std::shared_ptr<Query> Analyze::do_analyze(std::shared_ptr<ast::TreeNode> parse) {
    std::shared_ptr<Query> query = std::make_shared<Query>();
    if (auto x = std::dynamic_pointer_cast<ast::SelectStmt>(parse)) {
        // 处理表名
        query->tables = std::move(x->tabs);
        // 处理表别名
        query->table_aliases = std::move(x->table_aliases);
        /** TODO: 检查表是否存在 */

        check_table_exist(query->tables);

        // 检查是否有SEMI JOIN
        bool has_semi_join = false;
        std::string left_table_name;
        for (const auto& join_expr : x->jointree) {
            if (join_expr->type == SEMI_JOIN) {
                has_semi_join = true;
                left_table_name = join_expr->left;
                break;
            }
        }

        // 处理target list，再target list中添加上表名，例如 a.id
        for (auto &sv_sel_col: x->cols) {
            TabCol sel_col = {.tab_name = sv_sel_col->tab_name, .col_name = sv_sel_col->col_name};
            query->cols.push_back(sel_col);

        }

        std::vector<ColMeta> all_cols;
        if (has_semi_join) {
            get_all_cols({left_table_name}, all_cols);
        } else {
            get_all_cols(query->tables, all_cols);
        }

        if (query->cols.empty()) {
            // select all columns
            for (auto &col: all_cols) {
                TabCol sel_col = {.tab_name = col.tab_name, .col_name = col.name};
                query->cols.push_back(sel_col);
            }
        } else {
            // infer table name from column name
            for (size_t i = 0; i < query->cols.size(); ++i) {
                auto &sel_col = query->cols[i];
                auto &ast_col = x->cols[i];
                // 跳过COUNT(*)的列检查
                if (!(ast_col->isAgg && ast_col->aggType == ast::AggFuncType::COUNT_ALL)) {
                    if (has_semi_join) {
                        if (!sel_col.tab_name.empty() && sel_col.tab_name != left_table_name) {
                            throw RMDBError("Semi Join can only select columns from left table: " + left_table_name);
                        }
                    }
                    sel_col = check_column(all_cols, sel_col); // 列元数据校验
                }
            }
        }
        // 处理where条件
        get_clause(x->conds, query->conds);
        check_clause(query->tables, query->conds);
    } else if (auto x = std::dynamic_pointer_cast<ast::UpdateStmt>(parse)) {
        // 2t 实现update语法分析///////////////////////////


        /** TODO: */
        // 拿到表名
        query->tables = {x->tab_name};

        // 拿到元数据
        std::vector<ColMeta> all_cols;
        get_all_cols(query->tables, all_cols);
        if (all_cols.empty()) {
            throw ColumnNotFoundError(query->tables[0] + " has no columns defined");
        }

        // 没有where条件时，选所有列
        // 优化后（合并为范围for循环）
        if (query->conds.empty()) {
            for (auto &col: all_cols) {
                query->cols.push_back({.tab_name = col.tab_name,
                                              .col_name = col.name});
            }
            // 验证check_column
            for (auto &col: query->cols) {
                col = check_column(all_cols, col);
            }
        }

        // 拿到set的值
        for (auto &set: x->set_clauses) {
            if (set->is_expr) {
                // 表达式赋值
                auto expr_node = convert_ast_expr_to_expr_node(set->expr, query->tables);
                SetClause set_item{
                        {.tab_name = x->tab_name, .col_name = set->col_name},
                        expr_node};
                query->set_clauses.push_back(set_item);
            } else {
                // 简单值赋值
                SetClause set_item{
                        {.tab_name = x->tab_name, .col_name = set->col_name},
                        // 转换为内部 Value 类型
                        convert_sv_value(set->val),
                        set->isAdd};
                query->set_clauses.push_back(set_item);
            }
        }

        // where条件
        get_clause(x->conds, query->conds);
        // 确认查询条件有效
        check_clause(query->tables, query->conds);
        ///////////////////////////////////////////////////////////////

    } else if (auto x = std::dynamic_pointer_cast<ast::DeleteStmt>(parse)) {
        // 处理where条件
        get_clause(x->conds, query->conds);
        check_clause({x->tab_name}, query->conds);
    } else if (auto x = std::dynamic_pointer_cast<ast::InsertStmt>(parse)) {
        //         // 处理insert 的values值
        // for (auto &sv_val: x->vals) {
        //     query->values.push_back(convert_sv_value(sv_val));
        // }

        //2t 使得类型你不同仍可操作
        // Get table metadata
        TabMeta &tab = sm_manager_->db_.get_table(x->tab_name);
        // Check value count
        if (x->vals.size() != tab.cols.size()) {
            throw IncompatibleCountError("Column count doesn't match value count");
        }
        //处理insert 的values值
        for (size_t i = 0; i < x->vals.size(); i++) {
            auto &col = tab.cols[i];
            auto val = convert_sv_value(x->vals[i]);
            if (col.type == TYPE_FLOAT && val.type == TYPE_INT) {

                val.set_float(static_cast<float>(val.get_int()));

                val.type = TYPE_FLOAT;
            }
            else if (col.type == TYPE_INT && val.type == TYPE_FLOAT) {
             //TODO: 字段是int 但是值是float 则将float转为int

            }
            if (col.type != val.type) {
                throw IncompatibleTypeError(coltype2str(col.type), coltype2str(val.type));
            }

            query->values.push_back(val);


        }
    } else {
        // do nothing
    }
    query->parse = std::move(parse);
    return query;
}

// 2t 检查表是否存在
void Analyze::check_table_exist(const std::vector<std::string> tabs) {
    for (auto &tab_name: tabs) {
        // 看看当前打开的表集合中是否有这个表
        if (!sm_manager_->fhs_.count(tab_name)) {
            throw TableNotFoundError("表不存在在" + tab_name);
        }
    }
}

TabCol Analyze::check_column(const std::vector<ColMeta> &all_cols, TabCol target) {
    // 跳过COUNT(*)的列检查
    if (target.col_name == "*") {
        return target;
    }
    
    if (target.tab_name.empty()) {
        // Table name not specified, infer table name from column name
        std::string tab_name;
        for (auto &col: all_cols) {
            if (col.name == target.col_name) {
                if (!tab_name.empty()) {
                    throw AmbiguousColumnError(target.col_name);
                }
                tab_name = col.tab_name;
            }
        }
        if (tab_name.empty()) {
            throw ColumnNotFoundError(target.col_name);
        }
        target.tab_name = tab_name;
    } else {
        // t2
        /** TODO: Make sure target column exists */
        // 标记是否找到
        bool found = false;

        for (auto &col: all_cols) {
            if (col.tab_name == target.tab_name && col.name == target.col_name) {
                // 找到匹配的列，退出循环
                found = true;
                break;
            }
        }

        if (!found) {
            throw ColumnNotFoundError("column不存在 " + target.tab_name + "." + target.col_name);
        }
    }
    return target;
}

void Analyze::get_all_cols(const std::vector<std::string> &tab_names, std::vector<ColMeta> &all_cols) {
    for (auto &sel_tab_name: tab_names) {
        // 这里db_不能写成get_db(), 注意要传指针
        const auto &sel_tab_cols = sm_manager_->db_.get_table(sel_tab_name).cols;
        all_cols.insert(all_cols.end(), sel_tab_cols.begin(), sel_tab_cols.end());
    }
}

void Analyze::get_clause(const std::vector<std::shared_ptr<ast::BinaryExpr>> &sv_conds, std::vector<Condition> &conds) {
    conds.clear();
    for (auto &expr: sv_conds) {
        auto lhs_col = std::dynamic_pointer_cast<ast::Col>(expr->lhs);
        if (!lhs_col) {
            continue;
        }
        
        // 检查WHERE子句中是否使用了聚合函数
        if (lhs_col->isAgg) {
            throw RMDBError("Aggregate functions are not allowed in WHERE clause");
        }
        
        Condition cond;
        cond.lhs_col = {.tab_name = lhs_col->tab_name, .col_name = lhs_col->col_name};
        cond.op = convert_sv_comp_op(expr->op);
        if (auto rhs_val = std::dynamic_pointer_cast<ast::Value>(expr->rhs)) {
            cond.is_rhs_val = true;
            cond.rhs_val = convert_sv_value(rhs_val);
        } else if (auto rhs_col = std::dynamic_pointer_cast<ast::Col>(expr->rhs)) {
            // 检查右侧是否也是聚合函数
            if (rhs_col->isAgg) {
                throw RMDBError("Aggregate functions are not allowed in WHERE clause");
            }
            cond.is_rhs_val = false;
            cond.rhs_col = {.tab_name = rhs_col->tab_name, .col_name = rhs_col->col_name};
        }
        conds.push_back(cond);
    }
}

void Analyze::check_clause(const std::vector<std::string> &tab_names, std::vector<Condition> &conds) {
    // auto all_cols = get_all_cols(tab_names);
    std::vector<ColMeta> all_cols;
    get_all_cols(tab_names, all_cols);
    // Get raw values in where clause
    for (auto &cond: conds) {
        // Infer table name from column name
        cond.lhs_col = check_column(all_cols, cond.lhs_col);
        if (!cond.is_rhs_val) {
            cond.rhs_col = check_column(all_cols, cond.rhs_col);
        }
        TabMeta &lhs_tab = sm_manager_->db_.get_table(cond.lhs_col.tab_name);
        auto lhs_col = lhs_tab.get_col(cond.lhs_col.col_name);
        ColType lhs_type = lhs_col->type;
        ColType rhs_type;
        if (cond.is_rhs_val) {
            cond.rhs_val.init_raw(lhs_col->len);
            rhs_type = cond.rhs_val.type;
        } else {
            TabMeta &rhs_tab = sm_manager_->db_.get_table(cond.rhs_col.tab_name);
            auto rhs_col = rhs_tab.get_col(cond.rhs_col.col_name);
            rhs_type = rhs_col->type;
        }

        // 2t
        // 如果判断where 1 = 1.0 左右两边类型不一致
        if (lhs_type != rhs_type) {
            // 处理: int和float也要能比较 整数的float转int 当右操作数是值且类型为float时
            if (lhs_type == TYPE_INT && rhs_type == TYPE_FLOAT && cond.is_rhs_val) {
                // 检查值是否为整数
                float rhs_float = cond.rhs_val.get_float();
                if (rhs_float == static_cast<float>(static_cast<int>(rhs_float))) {
                    // 转换为int类型，更新右值
                    cond.rhs_val.set_int(static_cast<int>(rhs_float));
                    rhs_type = TYPE_INT;
                } else {
                    throw IncompatibleTypeError(coltype2str(lhs_type), coltype2str(rhs_type));
                }
            } else if (lhs_type == TYPE_FLOAT && rhs_type == TYPE_INT && cond.is_rhs_val) {

                cond.rhs_val.set_float(static_cast<float>(cond.rhs_val.get_int()));
                rhs_type = TYPE_FLOAT;

            } else {
                // 再不一致就抛异常了
                throw IncompatibleTypeError(coltype2str(lhs_type), coltype2str(rhs_type));
            }
        }
        //

    }

}


Value Analyze::convert_sv_value(const std::shared_ptr<ast::Value> &sv_val) {
    Value val;
    if (auto int_lit = std::dynamic_pointer_cast<ast::IntLit>(sv_val)) {
        val.set_int(int_lit->val);
    } else if (auto float_lit = std::dynamic_pointer_cast<ast::FloatLit>(sv_val)) {
        val.set_float(float_lit->val);
    } else if (auto str_lit = std::dynamic_pointer_cast<ast::StringLit>(sv_val)) {
        val.set_str(str_lit->val);
    } else {
        throw InternalError("Unexpected sv value type");
    }
    return val;
}

CompOp Analyze::convert_sv_comp_op(ast::SvCompOp op) {
    std::map<ast::SvCompOp, CompOp> m = {
            {ast::SV_OP_EQ, OP_EQ},
            {ast::SV_OP_NE, OP_NE},
            {ast::SV_OP_LT, OP_LT},
            {ast::SV_OP_GT, OP_GT},
            {ast::SV_OP_LE, OP_LE},
            {ast::SV_OP_GE, OP_GE},
    };
    return m.at(op);
}

std::shared_ptr<ExprNode> Analyze::convert_ast_expr_to_expr_node(const std::shared_ptr<ast::Expr> &ast_expr, const std::vector<std::string> &table_names) {
    if (auto value = std::dynamic_pointer_cast<ast::Value>(ast_expr)) {
        // 常量值
        Value val = convert_sv_value(value);
        return ExprNode::make_const(val);
    } else if (auto col = std::dynamic_pointer_cast<ast::Col>(ast_expr)) {
        // 列引用
        TabCol tab_col{col->tab_name, col->col_name};
        
        // 如果表名为空，需要推断
        if (tab_col.tab_name.empty()) {
            std::vector<ColMeta> all_cols;
            get_all_cols(table_names, all_cols);
            tab_col = check_column(all_cols, tab_col);
        }
        
        return ExprNode::make_col(tab_col);
    } else if (auto binary_expr = std::dynamic_pointer_cast<ast::BinaryExpr>(ast_expr)) {
        // 二元表达式
        auto left = convert_ast_expr_to_expr_node(binary_expr->lhs, table_names);
        auto right = convert_ast_expr_to_expr_node(binary_expr->rhs, table_names);
        
        ExprOp op;
        switch (binary_expr->op) {
            case ast::SV_OP_ADD:
                op = EXPR_ADD;
                break;
            case ast::SV_OP_SUB:
                op = EXPR_SUB;
                break;
            case ast::SV_OP_MUL:
                op = EXPR_MUL;
                break;
            case ast::SV_OP_DIV:
                op = EXPR_DIV;
                break;
            default:
                throw InternalError("Unsupported binary operator in expression");
        }
        
        return ExprNode::make_binary(op, left, right);
    } else if (auto unary_expr = std::dynamic_pointer_cast<ast::UnaryExpr>(ast_expr)) {
        // 一元表达式（负号）
        auto operand = convert_ast_expr_to_expr_node(unary_expr->operand, table_names);
        
        if (unary_expr->op == ast::SV_OP_NEG) {
            // 创建一个常量0和操作数相减来实现负号
            Value zero;
            zero.set_int(0);
            auto zero_node = ExprNode::make_const(zero);
            return ExprNode::make_binary(EXPR_SUB, zero_node, operand);
        } else {
            throw InternalError("Unsupported unary operator in expression");
        }
    } else {
        throw InternalError("Unsupported expression type");
    }
}
