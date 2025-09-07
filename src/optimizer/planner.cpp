/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "planner.h"

#include <memory>
#include <iostream>
#include <set>

#include "execution/executor_delete.h"
#include "execution/executor_index_scan.h"
#include "execution/executor_insert.h"
#include "execution/executor_nestedloop_join.h"
#include "execution/executor_projection.h"
#include "execution/executor_seq_scan.h"
#include "execution/executor_update.h"
#include "execution/executor_aggregate.h"
#include "index/ix.h"
#include "record_printer.h"

bool Planner::get_index_cols(std::string tab_name, std::vector<Condition> curr_conds, std::vector<Condition>& ret_conds, std::vector<std::string>& index_col_names) {
    if(curr_conds.empty()) return false;
    index_col_names.clear();

    TabMeta& tab = sm_manager_->db_.get_table(tab_name);
    std::map<std::string, Condition> col_cond_map;

    for(auto& cond: curr_conds) {
        if(cond.op == OP_NE) continue;

        if(cond.lhs_col.tab_name == tab_name && cond.is_rhs_val) {
            col_cond_map[cond.lhs_col.col_name] = cond;
        }
        else if(cond.is_rhs_val == false && cond.rhs_col.tab_name == tab_name) {
            std::swap(cond.lhs_col, cond.rhs_col);
            std::map<CompOp, CompOp> swap_op = {
                    {OP_EQ, OP_EQ}, {OP_NE, OP_NE}, {OP_LT, OP_GT}, {OP_GT, OP_LT}, {OP_LE, OP_GE}, {OP_GE, OP_LE},
            };
            cond.op = swap_op.at(cond.op);
            col_cond_map[cond.lhs_col.col_name] = cond;
        }
    }

    for(auto& index: tab.indexes) {
        bool can_use_index = false;
        std::vector<std::string> matched_cols;

        for(size_t i = 0; i < index.cols.size(); i++) {
            std::string col_name = index.cols[i].name;

            if(col_cond_map.find(col_name) != col_cond_map.end()) {
                matched_cols.push_back(col_name);
                if(col_cond_map[col_name].op != OP_EQ) {
                    can_use_index = true;
                    break;
                }
            } else {
                if(i == 0) break;
                can_use_index = true;
                break;
            }

            if(i == index.cols.size() - 1) {
                can_use_index = true;
            }
        }

        if(can_use_index && !matched_cols.empty()) {
            for (const auto& index_col:index.cols){
                index_col_names.push_back(index_col.name);
                for (auto cond:curr_conds){
                    if (cond.lhs_col.col_name == index_col.name){
                        ret_conds.push_back(cond);
                    }
                }

            }
            return true;
        }
    }

    return false;
}


/**
 * @brief 表算子条件谓词生成
 *
 * @param conds 条件
 * @param tab_names 表名
 * @return std::vector<Condition>
 */
std::vector<Condition> pop_conds(std::vector<Condition> &conds, std::string tab_names) {
    // auto has_tab = [&](const std::string &tab_name) {
    //     return std::find(tab_names.begin(), tab_names.end(), tab_name) != tab_names.end();
    // };
    std::vector<Condition> solved_conds;
    auto it = conds.begin();
    while (it != conds.end()) {
        if ((tab_names.compare(it->lhs_col.tab_name) == 0 && it->is_rhs_val) || (it->lhs_col.tab_name.compare(it->rhs_col.tab_name) == 0)) {
            solved_conds.emplace_back(std::move(*it));
            it = conds.erase(it);
        } else {
            it++;
        }
    }
    return solved_conds;
}

int push_conds(Condition *cond, std::shared_ptr<Plan> plan)
{
    if(auto x = std::dynamic_pointer_cast<ScanPlan>(plan))
    {
        if(x->tab_name_.compare(cond->lhs_col.tab_name) == 0) {
            return 1;
        } else if(x->tab_name_.compare(cond->rhs_col.tab_name) == 0){
            return 2;
        } else {
            return 0;
        }
    }
    else if(auto x = std::dynamic_pointer_cast<JoinPlan>(plan))
    {
        int left_res = push_conds(cond, x->left_);
        // 条件已经下推到左子节点
        if(left_res == 3){
            return 3;
        }
        int right_res = push_conds(cond, x->right_);
        // 条件已经下推到右子节点
        if(right_res == 3){
            return 3;
        }
        // 左子节点或右子节点有一个没有匹配到条件的列
        if(left_res == 0 || right_res == 0) {
            return left_res + right_res;
        }
        // 左子节点匹配到条件的右边
        if(left_res == 2) {
            // 需要将左右两边的条件变换位置
            std::map<CompOp, CompOp> swap_op = {
                {OP_EQ, OP_EQ}, {OP_NE, OP_NE}, {OP_LT, OP_GT}, {OP_GT, OP_LT}, {OP_LE, OP_GE}, {OP_GE, OP_LE},
            };
            std::swap(cond->lhs_col, cond->rhs_col);
            cond->op = swap_op.at(cond->op);
        }
        x->conds_.emplace_back(std::move(*cond));
        return 3;
    }
    return false;
}

std::shared_ptr<Plan> pop_scan(int *scantbl, std::string table, std::vector<std::string> &joined_tables, 
                std::vector<std::shared_ptr<Plan>> plans)
{
    for (size_t i = 0; i < plans.size(); i++) {
        auto x = std::dynamic_pointer_cast<ScanPlan>(plans[i]);
        if(x->tab_name_.compare(table) == 0)
        {
            scantbl[i] = 1;
            joined_tables.emplace_back(x->tab_name_);
            return plans[i];
        }
    }
    return nullptr;
}


std::shared_ptr<Query> Planner::logical_optimization(std::shared_ptr<Query> query, Context *context)
{
    // 参数有效性检查
    if (!query || !query->parse) {
        throw RMDBError("Invalid query for logical optimization");
    }

    auto select_stmt = std::dynamic_pointer_cast<ast::SelectStmt>(query->parse);
    if (!select_stmt) {
        throw RMDBError("Query is not a SELECT statement");
    }

    // 1. 谓词下推优化
    query = optimize_predicate_pushdown(query, context);

    // 2. 常量折叠优化
    query = optimize_constant_folding(query, context);

    // 3. 冗余条件消除
    query = optimize_redundant_conditions(query, context);

    // 4. 投影下推优化
    query = optimize_projection_pushdown(query, context);

    // 5. 连接重排序优化
    query = optimize_join_reordering(query, context);

    return query;
}

std::shared_ptr<Plan> Planner::physical_optimization(std::shared_ptr<Query> query, Context *context)
{
    if (!query || !query->parse) {
        throw RMDBError("Invalid query for physical optimization");
    }

    // 生成基础的扫描和连接计划
    std::shared_ptr<Plan> plan = make_one_rel(query);

    // 物理优化策略

    // 1. 索引选择优化
    // 在make_one_rel中已经处理了基本的索引选择
    // 这里可以进一步优化索引使用策略

    // 2. 连接算法选择优化
    // 根据表大小、内存限制等选择最优的连接算法
    // 当前支持嵌套循环连接和排序合并连接

    // 3. 排序优化
    // 如果ORDER BY的列上有索引，可以利用索引避免排序
    // 如果连接操作已经产生了有序结果，可以避免重复排序
    // 排序处理已移至generate_select_plan函数中

    // 4. 并行化考虑
    // TODO: 在未来版本中可以考虑并行执行计划

    // 5. 内存使用优化
    // TODO: 根据可用内存调整执行策略

    return plan;
}



std::shared_ptr<Plan> Planner::make_one_rel(std::shared_ptr<Query> query)
{
    auto x = std::dynamic_pointer_cast<ast::SelectStmt>(query->parse);
    std::vector<std::string> tables = query->tables;

    bool has_semi_join = false;
    for (const auto& join_expr : x->jointree) {
        if (join_expr->type == SEMI_JOIN) {
            has_semi_join = true;
            break;
        }
    }

    if (has_semi_join) {
        return make_one_rel_with_jointree(query);
    }

    std::vector<Condition> join_conditions;
    std::map<std::string, std::vector<Condition>> table_conditions;
    
    for (auto &cond: query->conds) {
        bool is_single_table = true;
        std::string table_name = "";
        
        if (!cond.lhs_col.tab_name.empty()) {
            table_name = cond.lhs_col.tab_name;
        }
        
        if (!cond.is_rhs_val && !cond.rhs_col.tab_name.empty()) {
            if (table_name.empty()) {
                table_name = cond.rhs_col.tab_name;
            } else if (table_name != cond.rhs_col.tab_name) {
                is_single_table = false;
            }
        }
        
        if (is_single_table && !table_name.empty()) {
            table_conditions[table_name].push_back(cond);
        } else {
            join_conditions.push_back(cond);
        }
    }

    std::vector<std::shared_ptr<Plan>> table_scan_executors(tables.size());
    for (size_t i = 0; i < tables.size(); i++) {
        std::vector<Condition> curr_conds = table_conditions[tables[i]];
        std::vector<std::string> index_col_names;
        std::vector<Condition> ret_conds;
        bool index_exist = get_index_cols(tables[i], curr_conds, ret_conds,index_col_names);
        if (!index_exist) {  // 该表没有索引
            index_col_names.clear();
            table_scan_executors[i] =
                std::make_shared<ScanPlan>(T_SeqScan, sm_manager_, tables[i], curr_conds, index_col_names);
        } else {  // 存在索引
            table_scan_executors[i] =
                std::make_shared<ScanPlan>(T_IndexScan, sm_manager_, tables[i], ret_conds, index_col_names);
        }
        //std::cout<<table_scan_executors[i]->tag<<'\n';
    }
    // 只有一个表，不需要join。
    if(tables.size() == 1)
    {
        return table_scan_executors[0];
    }
    auto conds = std::move(join_conditions);
    std::shared_ptr<Plan> table_join_executors;

    int scantbl[tables.size()];
    for(size_t i = 0; i < tables.size(); i++)
    {
        scantbl[i] = -1;
    }
    // 假设在ast中已经添加了jointree，这里需要修改的逻辑是，先处理jointree，然后再考虑剩下的部分
    if(conds.size() >= 1)
    {
        // 有连接条件

        // 根据连接条件，生成第一层join
        std::vector<std::string> joined_tables(tables.size());
        auto it = conds.begin();
        while (it != conds.end()) {
            std::shared_ptr<Plan> left , right;
            left = pop_scan(scantbl, it->lhs_col.tab_name, joined_tables, table_scan_executors);
            right = pop_scan(scantbl, it->rhs_col.tab_name, joined_tables, table_scan_executors);
            std::vector<Condition> join_conds{*it};
            //建立join
            // 判断使用哪种join方式
            if(enable_nestedloop_join && enable_sortmerge_join) {
                // 默认nested loop join
                table_join_executors = std::make_shared<JoinPlan>(T_NestLoop, std::move(left), std::move(right), join_conds);
            } else if(enable_nestedloop_join) {
                table_join_executors = std::make_shared<JoinPlan>(T_NestLoop, std::move(left), std::move(right), join_conds);
            } else if(enable_sortmerge_join) {
                table_join_executors = std::make_shared<JoinPlan>(T_SortMerge, std::move(left), std::move(right), join_conds);
            } else {
                // error
                throw RMDBError("No join executor selected!");
            }

            // table_join_executors = std::make_shared<JoinPlan>(T_NestLoop, std::move(left), std::move(right), join_conds);
            it = conds.erase(it);
            break;
        }
        // 根据连接条件，生成第2-n层join
        it = conds.begin();
        while (it != conds.end()) {
            std::shared_ptr<Plan> left_need_to_join_executors = nullptr;
            std::shared_ptr<Plan> right_need_to_join_executors = nullptr;
            bool isneedreverse = false;
            if (std::find(joined_tables.begin(), joined_tables.end(), it->lhs_col.tab_name) == joined_tables.end()) {
                left_need_to_join_executors = pop_scan(scantbl, it->lhs_col.tab_name, joined_tables, table_scan_executors);
            }
            if (std::find(joined_tables.begin(), joined_tables.end(), it->rhs_col.tab_name) == joined_tables.end()) {
                right_need_to_join_executors = pop_scan(scantbl, it->rhs_col.tab_name, joined_tables, table_scan_executors);
                isneedreverse = true;
            }

            if(left_need_to_join_executors != nullptr && right_need_to_join_executors != nullptr) {
                std::vector<Condition> join_conds{*it};
                std::shared_ptr<Plan> temp_join_executors = std::make_shared<JoinPlan>(T_NestLoop,
                                                                    std::move(left_need_to_join_executors),
                                                                    std::move(right_need_to_join_executors),
                                                                    join_conds);
                table_join_executors = std::make_shared<JoinPlan>(T_NestLoop, std::move(temp_join_executors),
                                                                    std::move(table_join_executors),
                                                                    std::vector<Condition>());
            } else if(left_need_to_join_executors != nullptr || right_need_to_join_executors != nullptr) {
                if(isneedreverse) {
                    std::map<CompOp, CompOp> swap_op = {
                        {OP_EQ, OP_EQ}, {OP_NE, OP_NE}, {OP_LT, OP_GT}, {OP_GT, OP_LT}, {OP_LE, OP_GE}, {OP_GE, OP_LE},
                    };
                    std::swap(it->lhs_col, it->rhs_col);
                    it->op = swap_op.at(it->op);
                    left_need_to_join_executors = std::move(right_need_to_join_executors);
                }
                std::vector<Condition> join_conds{*it};
                table_join_executors = std::make_shared<JoinPlan>(T_NestLoop, std::move(left_need_to_join_executors),
                                                                    std::move(table_join_executors), join_conds);
            } else {
                push_conds(std::move(&(*it)), table_join_executors);
            }
            it = conds.erase(it);
        }
    } else {
        table_join_executors = table_scan_executors[0];
        scantbl[0] = 1;
    }

    //连接剩余表
    for (size_t i = 0; i < tables.size(); i++) {
        if(scantbl[i] == -1) {
            table_join_executors = std::make_shared<JoinPlan>(T_NestLoop, std::move(table_scan_executors[i]),
                                                    std::move(table_join_executors), std::vector<Condition>());
        }
    }

    return table_join_executors;

}

std::shared_ptr<Plan> Planner::make_one_rel_with_jointree(std::shared_ptr<Query> query)
{
    auto x = std::dynamic_pointer_cast<ast::SelectStmt>(query->parse);
    std::vector<std::string> tables = query->tables;

    std::vector<Condition> join_conditions;
    std::map<std::string, std::vector<Condition>> table_conditions;
    
    for (auto &cond: query->conds) {
        bool is_single_table = true;
        std::string table_name = "";
        
        if (!cond.lhs_col.tab_name.empty()) {
            table_name = cond.lhs_col.tab_name;
        }
        
        if (!cond.is_rhs_val && !cond.rhs_col.tab_name.empty()) {
            if (table_name.empty()) {
                table_name = cond.rhs_col.tab_name;
            } else if (table_name != cond.rhs_col.tab_name) {
                is_single_table = false;
            }
        }
        
        if (is_single_table && !table_name.empty()) {
            table_conditions[table_name].push_back(cond);
        } else {
            join_conditions.push_back(cond);
        }
    }

    std::map<std::string, std::shared_ptr<Plan>> table_plans;
    for (const auto& table_name : tables) {
        std::vector<Condition> curr_conds = table_conditions[table_name];
        std::vector<std::string> index_col_names;
        std::vector<Condition> ret_conds;
        bool index_exist = get_index_cols(table_name, curr_conds, ret_conds, index_col_names);
        if (!index_exist) {
            index_col_names.clear();
            table_plans[table_name] = std::make_shared<ScanPlan>(T_SeqScan, sm_manager_, table_name, curr_conds, index_col_names);
        } else {
            table_plans[table_name] = std::make_shared<ScanPlan>(T_IndexScan, sm_manager_, table_name, ret_conds, index_col_names);
        }
    }

    std::shared_ptr<Plan> result_plan = nullptr;
    std::set<std::string> joined_tables;
    
    for (const auto& join_expr : x->jointree) {
        std::shared_ptr<Plan> left_plan = nullptr;
        std::shared_ptr<Plan> right_plan = nullptr;

        if (joined_tables.find(join_expr->left) == joined_tables.end()) {
            left_plan = table_plans[join_expr->left];
            joined_tables.insert(join_expr->left);
        } else {
            left_plan = result_plan;
        }

        right_plan = table_plans[join_expr->right];
        joined_tables.insert(join_expr->right);

        std::vector<Condition> join_conds;
        for (const auto& binary_expr : join_expr->conds) {
            Condition cond;

            if (auto lhs_col = std::dynamic_pointer_cast<ast::Col>(binary_expr->lhs)) {
                cond.lhs_col.tab_name = lhs_col->tab_name;
                cond.lhs_col.col_name = lhs_col->col_name;
            }

            switch (binary_expr->op) {
                case ast::SV_OP_EQ: cond.op = OP_EQ; break;
                case ast::SV_OP_NE: cond.op = OP_NE; break;
                case ast::SV_OP_LT: cond.op = OP_LT; break;
                case ast::SV_OP_GT: cond.op = OP_GT; break;
                case ast::SV_OP_LE: cond.op = OP_LE; break;
                case ast::SV_OP_GE: cond.op = OP_GE; break;
                default:
                    throw InternalError("Unsupported operator in join condition");
            }

            if (auto rhs_col = std::dynamic_pointer_cast<ast::Col>(binary_expr->rhs)) {
                cond.rhs_col.tab_name = rhs_col->tab_name;
                cond.rhs_col.col_name = rhs_col->col_name;
                cond.is_rhs_val = false;
            }
            
            join_conds.push_back(cond);
        }

        if (join_expr->type == SEMI_JOIN) {
            result_plan = std::make_shared<JoinPlan>(T_SemiJoin, std::move(left_plan), std::move(right_plan), join_conds);
        } else {
            result_plan = std::make_shared<JoinPlan>(T_NestLoop, std::move(left_plan), std::move(right_plan), join_conds);
        }
    }

    if (result_plan == nullptr && !tables.empty()) {
        result_plan = table_plans[tables[0]];
    }
    
    return result_plan;
}




// 逻辑优化函数实现

/**
 * @brief 谓词下推优化
 * 将WHERE条件尽可能下推到最接近数据源的位置
 */
std::shared_ptr<Query> Planner::optimize_predicate_pushdown(std::shared_ptr<Query> query, Context *context) {
    if (!query || !query->parse) {
        return query;
    }

    auto select_stmt = std::dynamic_pointer_cast<ast::SelectStmt>(query->parse);
    if (!select_stmt) {
        return query;
    }

    // TODO: 实现谓词下推逻辑
    // 1. 分析WHERE条件中的谓词
    // 2. 识别可以下推的单表谓词
    // 3. 将单表谓词下推到对应的表扫描操作
    // 4. 保留连接谓词在连接操作中
    if (query->tables.size() <= 1) {
        return query;
    }

    for (auto& cond : query->conds) {
        std::string table_name = "";
        bool single_table = true;

        if (!cond.lhs_col.tab_name.empty()) {
            table_name = cond.lhs_col.tab_name;
        }

        if (cond.is_rhs_val == false) {
            if (!cond.rhs_col.tab_name.empty()) {
                if (table_name.empty()) {
                    table_name = cond.rhs_col.tab_name;
                } else if (table_name != cond.rhs_col.tab_name) {
                    single_table = false;
                }
            }
        }

        if (single_table && !table_name.empty()) {}
    }
    
    return query;
}

/**
 * @brief 常量折叠优化
 * 在编译时计算常量表达式的值
 */
std::shared_ptr<Query> Planner::optimize_constant_folding(std::shared_ptr<Query> query, Context *context) {
    if (!query || !query->parse) {
        return query;
    }

    auto select_stmt = std::dynamic_pointer_cast<ast::SelectStmt>(query->parse);
    if (!select_stmt) {
        return query;
    }

    // TODO: 实现常量折叠逻辑
    // 1. 遍历WHERE条件中的表达式
    // 2. 识别常量表达式（如 1+1, 'abc' + 'def'等）
    // 3. 在编译时计算这些表达式的值
    // 4. 用计算结果替换原表达式

    return query;
}

/**
 * @brief 冗余条件消除优化
 * 移除重复或冗余的WHERE条件
 */
std::shared_ptr<Query> Planner::optimize_redundant_conditions(std::shared_ptr<Query> query, Context *context) {
    if (!query || !query->parse) {
        return query;
    }

    auto select_stmt = std::dynamic_pointer_cast<ast::SelectStmt>(query->parse);
    if (!select_stmt) {
        return query;
    }

    // TODO: 实现冗余条件消除逻辑
    // 1. 分析WHERE条件列表
    // 2. 识别重复的条件（如 a=1 AND a=1）
    // 3. 识别矛盾的条件（如 a=1 AND a=2）
    // 4. 识别包含关系的条件（如 a>5 AND a>3，可简化为a>5）
    // 5. 移除冗余条件，对矛盾条件给出警告

    return query;
}

/**
 * @brief 投影下推优化
 * 尽早移除不需要的列，减少数据传输
 */
std::shared_ptr<Query> Planner::optimize_projection_pushdown(std::shared_ptr<Query> query, Context *context) {
    if (!query || !query->parse) {
        return query;
    }

    auto select_stmt = std::dynamic_pointer_cast<ast::SelectStmt>(query->parse);
    if (!select_stmt) {
        return query;
    }

    // TODO: 实现投影下推逻辑
    // 1. 分析SELECT列表中需要的列
    // 2. 分析WHERE、GROUP BY、ORDER BY中引用的列
    // 3. 确定每个表实际需要的列集合
    // 4. 在表扫描阶段就只读取需要的列
    // 5. 在连接操作中尽早移除不再需要的列
    if (query->cols.empty() || (query->cols.size() == 1 && query->cols[0].col_name == "*")) {
        return query;
    }

    std::set<std::string> needed_cols;

    for (const auto& col : query->cols) {
        std::string full_name = col.tab_name.empty() ? col.col_name : col.tab_name + "." + col.col_name;
        needed_cols.insert(full_name);
    }

    for (const auto& cond : query->conds) {
        std::string lhs_name = cond.lhs_col.tab_name.empty() ? cond.lhs_col.col_name : cond.lhs_col.tab_name + "." + cond.lhs_col.col_name;
        needed_cols.insert(lhs_name);
        
        if (!cond.is_rhs_val) {
            std::string rhs_name = cond.rhs_col.tab_name.empty() ? cond.rhs_col.col_name : cond.rhs_col.tab_name + "." + cond.rhs_col.col_name;
            needed_cols.insert(rhs_name);
        }
    }
    return query;
}

/**
 * @brief 连接重排序优化
 * 根据表大小和选择性重新排列连接顺序
 */
std::shared_ptr<Query> Planner::optimize_join_reordering(std::shared_ptr<Query> query, Context *context) {
    if (!query || !query->parse) {
        return query;
    }

    auto select_stmt = std::dynamic_pointer_cast<ast::SelectStmt>(query->parse);
    if (!select_stmt) {
        return query;
    }

    // TODO: 实现连接重排序逻辑
    // 1. 分析表的大小（行数统计）
    // 2. 分析连接条件的选择性
    // 3. 分析WHERE条件对各表的过滤效果
    // 4. 使用动态规划或贪心算法找到最优连接顺序
    // 5. 重新排列query->tables和相关的连接条件
    if (query->tables.size() <= 1) {
        return query;
    }

    std::vector<std::pair<std::string, int>> table_cardinalities;
    for (const auto& table_name : query->tables) {
        try {
            auto fh = sm_manager_->fhs_.find(table_name);
            int record_count = 0;
            if (fh != sm_manager_->fhs_.end()) {
                auto file_hdr = fh->second->get_file_hdr();
                record_count = file_hdr.num_pages * 100; // 简单估算，假设每页平均100条记录
            }
            table_cardinalities.push_back({table_name, record_count});
        } catch (...) {
            table_cardinalities.push_back({table_name, 1000});
        }
    }

    std::sort(table_cardinalities.begin(), table_cardinalities.end(), 
              [](const auto& a, const auto& b) { return a.second < b.second; });

    std::vector<std::string> reordered_tables;
    for (const auto& pair : table_cardinalities) {
        reordered_tables.push_back(pair.first);
    }
    
    query->tables = reordered_tables;
    
    return query;
}


/**
 * @brief select plan 生成
 *
 * @param query 查询对象，包含解析后的AST和相关信息
 * @param context 查询上下文
 * @return std::shared_ptr<Plan> 生成的查询执行计划
 */
std::shared_ptr<Plan> Planner::generate_select_plan(std::shared_ptr<Query> query, Context *context) {
    // 参数有效性检查
    if (!query || !query->parse) {
        throw RMDBError("Invalid query object");
    }

    auto select_stmt = std::dynamic_pointer_cast<ast::SelectStmt>(query->parse);
    if (!select_stmt) {
        throw RMDBError("Query is not a SELECT statement");
    }

    // 逻辑优化阶段
    query = logical_optimization(std::move(query), context);

    // 物理优化阶段 - 生成基础的扫描和连接计划
    auto sel_cols = query->cols;
    std::shared_ptr<Plan> plannerRoot = physical_optimization(query, context);

    // 聚合处理阶段
    bool has_aggregation = false;
    std::vector<std::shared_ptr<ast::Col>> agg_exprs;
    std::vector<std::shared_ptr<ast::Col>> non_agg_exprs;

    // 检查SELECT列表中的聚合表达式
    for (const auto& expr : select_stmt->cols) {
        if (auto col_expr = std::dynamic_pointer_cast<ast::Col>(expr)) {

            
            if (col_expr->isAgg) {
                has_aggregation = true;
                
                // 对聚合表达式中的列进行表名推断（跳过COUNT(*)）
                if (col_expr->aggType != ast::AggFuncType::COUNT_ALL && 
                    col_expr->tab_name.empty() && col_expr->col_name != "*" && !col_expr->col_name.empty()) {
                    std::string inferred_tab_name;
                    for (const auto& table_name : query->tables) {
                        auto table_cols = sm_manager_->db_.get_table(table_name).cols;
                        for (const auto& col_meta : table_cols) {
                            if (col_meta.name == col_expr->col_name) {
                                if (!inferred_tab_name.empty()) {
                                    throw RMDBError("Ambiguous column name: " + col_expr->col_name);
                                }
                                inferred_tab_name = col_meta.tab_name;
                            }
                        }
                    }
                    if (inferred_tab_name.empty()) {
                        throw RMDBError("[AGG] Column not found: " + col_expr->tab_name + "." + col_expr->col_name);
                    }
                    col_expr->tab_name = inferred_tab_name;
                }
                
                agg_exprs.push_back(col_expr);
            } else {
                non_agg_exprs.push_back(col_expr);
            }
        }
    }

    // 验证GROUP BY语义正确性
    if (select_stmt->has_group_by && select_stmt->group_by_clause) {
        // 如果有GROUP BY，非聚合列必须出现在GROUP BY子句中
        for (const auto& non_agg_col : non_agg_exprs) {
            bool found_in_group_by = false;
            for (const auto& group_col : select_stmt->group_by_clause->group_by_cols) {
                if (non_agg_col->col_name == group_col->col_name &&
                    non_agg_col->tab_name == group_col->tab_name) {
                    found_in_group_by = true;
                    break;
                }
            }
            if (!found_in_group_by) {
                throw RMDBError("Column '" + non_agg_col->col_name +
                               "' must appear in GROUP BY clause or be used in an aggregate function");
            }
        }
    }

    // 如果需要聚合操作，添加聚合计划节点
    if (has_aggregation || select_stmt->has_group_by) {
        // 获取GROUP BY列和HAVING条件
        std::vector<std::shared_ptr<ast::Col>> group_cols;
        std::vector<std::shared_ptr<ast::BinaryExpr>> having_conds;

        if (select_stmt->has_group_by && select_stmt->group_by_clause) {
            group_cols = select_stmt->group_by_clause->group_by_cols;
            
            // 对GROUP BY列进行表名推断
            for (auto& group_col : group_cols) {
                if (group_col->tab_name.empty()) {
                    // 从可用的表中推断表名
                    std::string inferred_tab_name;
                    for (const auto& table_name : query->tables) {
                        auto table_cols = sm_manager_->db_.get_table(table_name).cols;
                        for (const auto& col_meta : table_cols) {
                            if (col_meta.name == group_col->col_name) {
                                if (!inferred_tab_name.empty()) {
                                    throw RMDBError("Ambiguous column name: " + group_col->col_name);
                                }
                                inferred_tab_name = col_meta.tab_name;
                            }
                        }
                    }
                    if (inferred_tab_name.empty()) {
                        throw RMDBError("[GROUP] Column not found: " + group_col->col_name);
                    }
                    group_col->tab_name = inferred_tab_name;
                }
            }
        }
        
        if (select_stmt->has_having && select_stmt->having_clause) {
            having_conds = select_stmt->having_clause->conds;
            
            // 处理HAVING条件中的别名解析
            for (size_t i = 0; i < having_conds.size(); i++) {
                auto& having_cond = having_conds[i];
                auto lhs_col = std::dynamic_pointer_cast<ast::Col>(having_cond->lhs);

                if (!lhs_col) {
                    continue;
                }
                
                // 如果已经是聚合函数，跳过处理
                if (lhs_col->isAgg) {
                    continue;
                }
                
                // 特殊处理COUNT(*)的情况：col_name为空但应该是聚合函数
                if (lhs_col->col_name.empty() && lhs_col->tab_name.empty()) {
                    // 这可能是COUNT(*)，检查是否在聚合表达式中
                    bool found_count_all = false;
                    for (const auto& agg_expr : agg_exprs) {
                        if (agg_expr->aggType == ast::AggFuncType::COUNT_ALL) {
                            lhs_col->isAgg = true;
                            lhs_col->aggType = ast::AggFuncType::COUNT_ALL;
                            lhs_col->col_name = "*";
                            found_count_all = true;
                            break;
                        }
                    }
                    if (found_count_all) {
                        continue;
                    }
                }
                
                // 检查左侧列是否是别名
                if (lhs_col->tab_name.empty()) {
                    // 首先检查是否是聚合函数的别名
                    bool found_alias = false;
                    for (const auto& agg_expr : agg_exprs) {
                        if (!agg_expr->alias.empty() && agg_expr->alias == lhs_col->col_name) {
                            // 找到对应的聚合表达式，替换为聚合表达式的信息
                            lhs_col->tab_name = agg_expr->tab_name;
                            lhs_col->col_name = agg_expr->col_name;
                            lhs_col->isAgg = true;
                            lhs_col->aggType = agg_expr->aggType;
                            found_alias = true;
                            break;
                        }
                    }
                    
                    // 如果不是聚合函数别名，检查是否是普通列的别名
                    if (!found_alias) {
                        for (const auto& sel_col : select_stmt->cols) {
                            if (!sel_col->alias.empty() && sel_col->alias == lhs_col->col_name) {
                                lhs_col->tab_name = sel_col->tab_name;
                                lhs_col->col_name = sel_col->col_name;
                                found_alias = true;
                                break;
                            }
                        }
                    }
                    
                    // 如果仍然没有找到，进行表名推断
                    if (!found_alias) {
                        std::string inferred_tab_name;
                        for (const auto& table_name : query->tables) {
                            auto table_cols = sm_manager_->db_.get_table(table_name).cols;
                            for (const auto& col_meta : table_cols) {
                                if (col_meta.name == lhs_col->col_name) {
                                    if (!inferred_tab_name.empty()) {
                                        throw RMDBError("Ambiguous column name: " + lhs_col->col_name);
                                    }
                                    inferred_tab_name = col_meta.tab_name;
                                }
                            }
                        }
                        if (inferred_tab_name.empty()) {
                            throw RMDBError("[HAVING] Column not found: " + lhs_col->col_name);
                        }
                        lhs_col->tab_name = inferred_tab_name;
                    }
                }
            }
        }

        plannerRoot = std::make_shared<AggregatePlan>(
            std::move(plannerRoot),
            agg_exprs,
            group_cols,
            having_conds,
            select_stmt->cols,
            select_stmt->has_group_by,
            !having_conds.empty()
        );
    }

    // 投影阶段 - 选择需要的列
    std::vector<TabCol> projection_cols;
    if (has_aggregation || select_stmt->has_group_by) {
        // 对于聚合查询，按照SELECT语句中的原始顺序构建投影列
        for (const auto& expr : select_stmt->cols) {
            if (auto col_expr = std::dynamic_pointer_cast<ast::Col>(expr)) {
                if (col_expr->isAgg) {
                    // 聚合列
                    std::string agg_col_name;
                    
                    // 如果有别名，使用别名作为列名
                    if (!col_expr->alias.empty()) {
                        agg_col_name = col_expr->alias;
                    } else {
                        // 否则使用聚合表达式作为列名
                        if (col_expr->aggType == ast::AggFuncType::COUNT_ALL) {
                            agg_col_name = "COUNT(*)";
                        } else {
                            std::string agg_name;
                            switch (col_expr->aggType) {
                                case ast::AggFuncType::COUNT: agg_name = "COUNT"; break;
                                case ast::AggFuncType::COUNT_ALL: agg_name = "COUNT"; break;
                                case ast::AggFuncType::MAX: agg_name = "MAX"; break;
                                case ast::AggFuncType::MIN: agg_name = "MIN"; break;
                                case ast::AggFuncType::SUM: agg_name = "SUM"; break;
                                case ast::AggFuncType::AVG: agg_name = "AVG"; break;
                            }
                            agg_col_name = agg_name + "(" + col_expr->col_name + ")";
                        }
                    }
                    projection_cols.push_back({"", agg_col_name});
                } else {
                    // 非聚合列（GROUP BY列）
                    projection_cols.push_back({col_expr->tab_name, col_expr->col_name});
                }
            }
        }
    } else {
        projection_cols = std::move(sel_cols);
    }
    
    plannerRoot = std::make_shared<ProjectionPlan>(
        T_Projection,
        std::move(plannerRoot),
        std::move(projection_cols)
    );

    // 处理ORDER BY
    if (select_stmt->has_sort && select_stmt->order) {
        std::vector<TabCol> order_cols;
        std::vector<bool> is_desc_list;
        
        for (const auto& order_item : select_stmt->order->order_items) {
            auto col = order_item->col;
            
            // 表名推断
            if (col->tab_name.empty()) {
                std::string inferred_tab_name;
                for (const auto& table_name : query->tables) {
                    auto table_cols = sm_manager_->db_.get_table(table_name).cols;
                    for (const auto& col_meta : table_cols) {
                        if (col_meta.name == col->col_name) {
                            if (!inferred_tab_name.empty()) {
                                throw RMDBError("Ambiguous column name: " + col->col_name);
                            }
                            inferred_tab_name = col_meta.tab_name;
                        }
                    }
                }
                if (inferred_tab_name.empty()) {
                    throw RMDBError("[ORDER BY] Column not found: " + col->col_name);
                }
                col->tab_name = inferred_tab_name;
            }
            
            order_cols.push_back({col->tab_name, col->col_name});
            is_desc_list.push_back(order_item->orderby_dir == ast::OrderBy_DESC);
        }
        
        plannerRoot = std::make_shared<SortPlan>(std::move(plannerRoot), order_cols, is_desc_list);
    }

    // 处理LIMIT
    if (select_stmt->has_limit && select_stmt->limit_count > 0) {
        plannerRoot = std::make_shared<LimitPlan>(std::move(plannerRoot), select_stmt->limit_count);
    }

    return plannerRoot;
}

// 生成DDL语句和DML语句的查询执行计划
std::shared_ptr<Plan> Planner::do_planner(std::shared_ptr<Query> query, Context *context)
{
    std::shared_ptr<Plan> plannerRoot;
    if (auto x = std::dynamic_pointer_cast<ast::CreateTable>(query->parse)) {
        // create table;
        std::vector<ColDef> col_defs;
        for (auto &field : x->fields) {
            if (auto sv_col_def = std::dynamic_pointer_cast<ast::ColDef>(field)) {
                ColDef col_def = {.name = sv_col_def->col_name,
                                  .type = interp_sv_type(sv_col_def->type_len->type),
                                  .len = sv_col_def->type_len->len};
                col_defs.push_back(col_def);
            } else {
                throw InternalError("Unexpected field type");
            }
        }
        plannerRoot = std::make_shared<DDLPlan>(T_CreateTable, x->tab_name, std::vector<std::string>(), col_defs);
    } else if (auto x = std::dynamic_pointer_cast<ast::DropTable>(query->parse)) {
        // drop table;
        plannerRoot = std::make_shared<DDLPlan>(T_DropTable, x->tab_name, std::vector<std::string>(), std::vector<ColDef>());
    } else if (auto x = std::dynamic_pointer_cast<ast::CreateIndex>(query->parse)) {
        // create index;
        plannerRoot = std::make_shared<DDLPlan>(T_CreateIndex, x->tab_name, x->col_names, std::vector<ColDef>());
    } else if (auto x = std::dynamic_pointer_cast<ast::DropIndex>(query->parse)) {
        // drop index
        plannerRoot = std::make_shared<DDLPlan>(T_DropIndex, x->tab_name, x->col_names, std::vector<ColDef>());
    } else if (auto x = std::dynamic_pointer_cast<ast::InsertStmt>(query->parse)) {
        // insert;
        plannerRoot = std::make_shared<DMLPlan>(T_Insert,
                                                std::shared_ptr<Plan>(),
                                                x->tab_name,
                                                query->values,
                                                std::vector<Condition>(),
                                                std::vector<SetClause>());
    } else if (auto x = std::dynamic_pointer_cast<ast::DeleteStmt>(query->parse)) {
        // delete;
        // 生成表扫描方式
        std::shared_ptr<Plan> table_scan_executors;
        // 只有一张表，不需要进行物理优化了
        // int index_no = get_indexNo(x->tab_name, query->conds);
        std::vector<std::string> index_col_names;
        std::vector<Condition> ret_conds;
        bool index_exist = get_index_cols(x->tab_name, query->conds,ret_conds, index_col_names);
        
        if (index_exist == false) {  // 该表没有索引
            index_col_names.clear();
            table_scan_executors = 
                std::make_shared<ScanPlan>(T_SeqScan, sm_manager_, x->tab_name, query->conds, index_col_names);
        } else {  // 存在索引
            table_scan_executors =
                std::make_shared<ScanPlan>(T_IndexScan, sm_manager_, x->tab_name, ret_conds, index_col_names);
        }

        plannerRoot = std::make_shared<DMLPlan>(T_Delete, table_scan_executors, x->tab_name,  
                                                std::vector<Value>(), query->conds, std::vector<SetClause>());
    } else if (auto x = std::dynamic_pointer_cast<ast::UpdateStmt>(query->parse)) {
        // update;
        // 生成表扫描方式
        std::shared_ptr<Plan> table_scan_executors;
        // 只有一张表，不需要进行物理优化了
        // int index_no = get_indexNo(x->tab_name, query->conds);
        std::vector<std::string> index_col_names;
        std::vector<Condition> ret_conds;
        bool index_exist = get_index_cols(x->tab_name, query->conds,ret_conds, index_col_names);

        if (index_exist == false) {  // 该表没有索引
        index_col_names.clear();
            table_scan_executors = 
                std::make_shared<ScanPlan>(T_SeqScan, sm_manager_, x->tab_name, query->conds, index_col_names);
        } else {  // 存在索引
            table_scan_executors =
                std::make_shared<ScanPlan>(T_IndexScan, sm_manager_, x->tab_name, ret_conds, index_col_names);
        }
        plannerRoot = std::make_shared<DMLPlan>(T_Update, table_scan_executors, x->tab_name,
                                                     std::vector<Value>(), query->conds, 
                                                     query->set_clauses);
    } else if (auto x = std::dynamic_pointer_cast<ast::SelectStmt>(query->parse)) {

        std::shared_ptr<plannerInfo> root = std::make_shared<plannerInfo>(x);
        // 生成select语句的查询执行计划
        std::shared_ptr<Plan> projection = generate_select_plan(std::move(query), context);
        plannerRoot = std::make_shared<DMLPlan>(T_select, projection, std::string(), std::vector<Value>(),
                                                    std::vector<Condition>(), std::vector<SetClause>());
    } else if (auto x = std::dynamic_pointer_cast<ast::ExplainStmt>(query->parse)) {
        Analyze analyzer(sm_manager_);
        auto analyzed_query = analyzer.do_analyze(x->select_stmt);
        std::shared_ptr<Plan> optimized_plan = generate_select_plan(std::move(analyzed_query), context);
        plannerRoot = std::make_shared<ExplainPlan>(optimized_plan);
    } else if (auto x = std::dynamic_pointer_cast<ast::TxnBegin>(query->parse)) {
        // transaction begin
        plannerRoot = std::make_shared<OtherPlan>(T_Transaction_begin, "");
    } else if (auto x = std::dynamic_pointer_cast<ast::TxnCommit>(query->parse)) {
        // transaction commit
        plannerRoot = std::make_shared<OtherPlan>(T_Transaction_commit, "");
    } else if (auto x = std::dynamic_pointer_cast<ast::TxnAbort>(query->parse)) {
        // transaction abort
        plannerRoot = std::make_shared<OtherPlan>(T_Transaction_abort, "");
    } else if (auto x = std::dynamic_pointer_cast<ast::TxnRollback>(query->parse)) {
        // transaction rollback
        plannerRoot = std::make_shared<OtherPlan>(T_Transaction_rollback, "");
    } else if (auto x = std::dynamic_pointer_cast<ast::CreateCheckpoint>(query->parse)) {
        // create checkpoint
        plannerRoot = std::make_shared<OtherPlan>(T_CreateCheckpoint, "");
    } else {
        throw InternalError("Unexpected AST root");
    }
    return plannerRoot;
}