/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "execution_manager.h"

#include "executor_delete.h"
#include "executor_index_scan.h"
#include "executor_insert.h"
#include "executor_nestedloop_join.h"
#include "executor_projection.h"
#include "executor_seq_scan.h"
#include "executor_update.h"
#include "index/ix.h"
#include "record_printer.h"
#include "recovery/log_manager.h"

const char *help_info = "Supported SQL syntax:\n"
                   "  command ;\n"
                   "command:\n"
                   "  CREATE TABLE table_name (column_name type [, column_name type ...])\n"
                   "  DROP TABLE table_name\n"
                   "  CREATE INDEX table_name (column_name)\n"
                   "  DROP INDEX table_name (column_name)\n"
                   "  INSERT INTO table_name VALUES (value [, value ...])\n"
                   "  DELETE FROM table_name [WHERE where_clause]\n"
                   "  UPDATE table_name SET column_name = value [, column_name = value ...] [WHERE where_clause]\n"
                   "  SELECT selector FROM table_name [WHERE where_clause]\n"
                   "type:\n"
                   "  {INT | FLOAT | CHAR(n)}\n"
                   "where_clause:\n"
                   "  condition [AND condition ...]\n"
                   "condition:\n"
                   "  column op {column | value}\n"
                   "column:\n"
                   "  [table_name.]column_name\n"
                   "op:\n"
                   "  {= | <> | < | > | <= | >=}\n"
                   "selector:\n"
                   "  {* | column [, column ...]}\n";

// 主要负责执行DDL语句
void QlManager::run_mutli_query(std::shared_ptr<Plan> plan, Context *context){
    if (auto x = std::dynamic_pointer_cast<DDLPlan>(plan)) {
        switch(x->tag) {
            case T_CreateTable:
            {
                sm_manager_->create_table(x->tab_name_, x->cols_, context);
                break;
            }
            case T_DropTable:
            {
                sm_manager_->drop_table(x->tab_name_, context);
                break;
            }
            case T_CreateIndex:
            {
                sm_manager_->create_index(x->tab_name_, x->tab_col_names_, context);
                break;
            }
            case T_DropIndex:
            {
                sm_manager_->drop_index(x->tab_name_, x->tab_col_names_, context);
                break;
            }
            default:
                throw InternalError("Unexpected field type");
                break;  
        }
    }
}

// 执行help; show tables; desc table; begin; commit; abort;语句
void QlManager::run_cmd_utility(std::shared_ptr<Plan> plan, txn_id_t *txn_id, Context *context) {
    if (auto x = std::dynamic_pointer_cast<OtherPlan>(plan)) {
        switch(x->tag) {
            case T_Help:
            {
                memcpy(context->data_send_ + *(context->offset_), help_info, strlen(help_info));
                *(context->offset_) = strlen(help_info);
                break;
            }
            case T_ShowTable:
            {
                sm_manager_->show_tables(context);
                break;
            }
            case T_ShowIndex:
            {
                sm_manager_->show_indexes(x->tab_name_,context);
                break;
            }
            case T_DescTable:
            {
                sm_manager_->desc_table(x->tab_name_, context);
                break;
            }
            case T_Transaction_begin:
            {
                // 显示开启一个事务
                context->txn_->set_txn_mode(true);
                // 设置事务状态为GROWING
                context->txn_->set_state(TransactionState::GROWING);
                // 清空write_set，开始新的事务
                context->txn_->get_write_set()->clear();
                context->txn_->get_lock_set()->clear();
                
                // 记录BEGIN日志
                if (context->log_mgr_ != nullptr) {
                    BeginLogRecord begin_log(context->txn_->get_transaction_id());
                    lsn_t lsn = context->log_mgr_->add_log_to_buffer(&begin_log);
                    context->txn_->set_prev_lsn(lsn);
                }
                break;
            }  
            case T_Transaction_commit:
            {
                context->txn_ = txn_mgr_->get_transaction(*txn_id);
                txn_mgr_->commit(context->txn_, context->log_mgr_);
                break;
            }    
            case T_Transaction_rollback:
            {
                context->txn_ = txn_mgr_->get_transaction(*txn_id);
                txn_mgr_->abort(context->txn_, context->log_mgr_);
                break;
            }    
            case T_Transaction_abort:
            {
                context->txn_ = txn_mgr_->get_transaction(*txn_id);
                txn_mgr_->abort(context->txn_, context->log_mgr_);
                break;
            }
            case T_CreateCheckpoint:
            {
                // 创建静态检查点
                // 1. 停止接收新事务（通过提交当前事务实现）
                if (context->txn_->get_state() == TransactionState::GROWING) {
                    txn_mgr_->commit(context->txn_, context->log_mgr_);
                }
                
                // 2. 将日志缓冲区内容写到日志文件中
                context->log_mgr_->flush_log_to_disk();
                
                // 3. 创建检查点记录
                lsn_t checkpoint_lsn = context->log_mgr_->create_checkpoint();
                
                // 4. 将数据库缓冲区中的所有脏页刷新到磁盘
                if (buffer_pool_manager_) {
                    buffer_pool_manager_->flush_all_dirty_pages();
                }
                
                // 向客户端发送成功响应
                std::string success_msg = "Static checkpoint created successfully.\n";
                memcpy(context->data_send_ + *(context->offset_), success_msg.c_str(), success_msg.length());
                *(context->offset_) += success_msg.length();
                break;
            }
            default:
                throw InternalError("Unexpected field type");
                break;                        
        }

    } else if (auto x = std::dynamic_pointer_cast<ExplainPlan>(plan)) {
        std::string explain_output = format_explain_plan(x->subplan_, 0);

        memcpy(context->data_send_ + *(context->offset_), explain_output.c_str(), explain_output.length());
        *(context->offset_) += explain_output.length();

        std::fstream outfile;
        outfile.open("output.txt", std::ios::out | std::ios::app);
        outfile << explain_output;
        outfile.close();
        
    } else if(auto x = std::dynamic_pointer_cast<SetKnobPlan>(plan)) {
        switch (x->set_knob_type_)
        {
        case ast::SetKnobType::EnableNestLoop: {
            planner_->set_enable_nestedloop_join(x->bool_value_);
            break;
        }
        case ast::SetKnobType::EnableSortMerge: {
            planner_->set_enable_sortmerge_join(x->bool_value_);
            break;
        }
        default: {
            throw RMDBError("Not implemented!\n");
            break;
        }
        }
    }
}

// 执行select语句，select语句的输出除了需要返回客户端外，还需要写入output.txt文件中
void QlManager::select_from(std::unique_ptr<AbstractExecutor> executorTreeRoot, std::vector<TabCol> sel_cols, 
                            Context *context) {
    std::vector<std::string> captions;
    captions.reserve(sel_cols.size());
    for (auto &sel_col : sel_cols) {
        captions.push_back(sel_col.col_name);
    }

    // Print header into buffer
    RecordPrinter rec_printer(sel_cols.size());
    rec_printer.print_separator(context);
    rec_printer.print_record(captions, context);
    rec_printer.print_separator(context);
    // print header into file
    std::fstream outfile;
    outfile.open("output.txt", std::ios::out | std::ios::app);
    outfile << "|";
    for(size_t i = 0; i < captions.size(); ++i) {
        outfile << " " << captions[i] << " |";
    }
    outfile << "\n";

    // Print records
    size_t num_rec = 0;
    // 执行query_plan
    for (executorTreeRoot->beginTuple(); !executorTreeRoot->is_end(); executorTreeRoot->nextTuple()) {
        auto Tuple = executorTreeRoot->Next();
        std::vector<std::string> columns;
        for (auto &col : executorTreeRoot->cols()) {
            std::string col_str;
            char *rec_buf = Tuple->data + col.offset;
            if (col.type == TYPE_INT) {
                col_str = std::to_string(*(int *)rec_buf);
            } else if (col.type == TYPE_FLOAT) {
                col_str = std::to_string(*(float *)rec_buf);
            } else if (col.type == TYPE_STRING) {
                col_str = std::string((char *)rec_buf, col.len);
                col_str.resize(strlen(col_str.c_str()));
            }
            columns.push_back(col_str);
        }
        // print record into buffer
        rec_printer.print_record(columns, context);
        // print record into file
        outfile << "|";
        for(size_t i = 0; i < columns.size(); ++i) {
            outfile << " " << columns[i] << " |";
        }
        outfile << "\n";
        num_rec++;
    }
    outfile.close();
    // Print footer into buffer
    rec_printer.print_separator(context);
    // Print record count into buffer
    RecordPrinter::print_record_count(num_rec, context);
}

// 执行DML语句
void QlManager::run_dml(std::unique_ptr<AbstractExecutor> exec){
    exec->Next();
}

// 格式化EXPLAIN输出
std::string QlManager::format_explain_plan(std::shared_ptr<Plan> plan, int depth) {
    std::string indent(depth * 2, ' ');
    std::string result;
    
    if (!plan) {
        return indent + "NULL Plan\n";
    }
    
    switch (plan->tag) {
        case T_SeqScan: {
            auto scan_plan = std::dynamic_pointer_cast<ScanPlan>(plan);
            result += indent + "-> Seq Scan on " + scan_plan->tab_name_;
            if (!scan_plan->conds_.empty()) {
                result += " (Filter: ";
                for (size_t i = 0; i < scan_plan->conds_.size(); ++i) {
                    if (i > 0) result += " AND ";
                    result += scan_plan->conds_[i].lhs_col.col_name + " " + 
                             (scan_plan->conds_[i].op == OP_EQ ? "=" :
                              scan_plan->conds_[i].op == OP_NE ? "<>" :
                              scan_plan->conds_[i].op == OP_LT ? "<" :
                              scan_plan->conds_[i].op == OP_GT ? ">" :
                              scan_plan->conds_[i].op == OP_LE ? "<=" : ">=") + " ";
                    if (scan_plan->conds_[i].is_rhs_val) {
                         auto& val = scan_plan->conds_[i].rhs_val;
                         switch (val.type) {
                             case TYPE_INT:
                                 result += std::to_string(val.int_val);
                                 break;
                             case TYPE_FLOAT:
                                 result += std::to_string(val.float_val);
                                 break;
                             case TYPE_STRING:
                                 result += "'" + val.str_val + "'";
                                 break;
                             default:
                                 result += "value";
                         }
                     } else {
                         result += scan_plan->conds_[i].rhs_col.col_name;
                     }
                }
                result += ")";
            }
            result += "\n";
            break;
        }
        case T_IndexScan: {
            auto scan_plan = std::dynamic_pointer_cast<ScanPlan>(plan);
            result += indent + "-> Index Scan on " + scan_plan->tab_name_;
            if (!scan_plan->conds_.empty()) {
                result += " (Index Cond: ";
                for (size_t i = 0; i < scan_plan->conds_.size(); ++i) {
                    if (i > 0) result += " AND ";
                    result += scan_plan->conds_[i].lhs_col.col_name + " " + 
                             (scan_plan->conds_[i].op == OP_EQ ? "=" :
                              scan_plan->conds_[i].op == OP_NE ? "<>" :
                              scan_plan->conds_[i].op == OP_LT ? "<" :
                              scan_plan->conds_[i].op == OP_GT ? ">" :
                              scan_plan->conds_[i].op == OP_LE ? "<=" : ">=") + " ";
                    if (scan_plan->conds_[i].is_rhs_val) {
                         auto& val = scan_plan->conds_[i].rhs_val;
                         switch (val.type) {
                             case TYPE_INT:
                                 result += std::to_string(val.int_val);
                                 break;
                             case TYPE_FLOAT:
                                 result += std::to_string(val.float_val);
                                 break;
                             case TYPE_STRING:
                                 result += "'" + val.str_val + "'";
                                 break;
                             default:
                                 result += "value";
                         }
                     } else {
                         result += scan_plan->conds_[i].rhs_col.col_name;
                     }
                }
                result += ")";
            }
            result += "\n";
            break;
        }
        case T_NestLoop: {
            auto join_plan = std::dynamic_pointer_cast<JoinPlan>(plan);
            result += indent + "-> Nested Loop Join";
            if (!join_plan->conds_.empty()) {
                result += " (Join Cond: ";
                for (size_t i = 0; i < join_plan->conds_.size(); ++i) {
                    if (i > 0) result += " AND ";
                    result += join_plan->conds_[i].lhs_col.tab_name + "." + join_plan->conds_[i].lhs_col.col_name + " " + 
                             (join_plan->conds_[i].op == OP_EQ ? "=" :
                              join_plan->conds_[i].op == OP_NE ? "<>" :
                              join_plan->conds_[i].op == OP_LT ? "<" :
                              join_plan->conds_[i].op == OP_GT ? ">" :
                              join_plan->conds_[i].op == OP_LE ? "<=" : ">=") + " " +
                             join_plan->conds_[i].rhs_col.tab_name + "." + join_plan->conds_[i].rhs_col.col_name;
                }
                result += ")";
            }
            result += "\n";
            result += format_explain_plan(join_plan->left_, depth + 1);
            result += format_explain_plan(join_plan->right_, depth + 1);
            break;
        }
        case T_SortMerge: {
            auto join_plan = std::dynamic_pointer_cast<JoinPlan>(plan);
            result += indent + "-> Sort Merge Join";
            if (!join_plan->conds_.empty()) {
                result += " (Join Cond: ";
                for (size_t i = 0; i < join_plan->conds_.size(); ++i) {
                    if (i > 0) result += " AND ";
                    result += join_plan->conds_[i].lhs_col.tab_name + "." + join_plan->conds_[i].lhs_col.col_name + " " + 
                             (join_plan->conds_[i].op == OP_EQ ? "=" :
                              join_plan->conds_[i].op == OP_NE ? "<>" :
                              join_plan->conds_[i].op == OP_LT ? "<" :
                              join_plan->conds_[i].op == OP_GT ? ">" :
                              join_plan->conds_[i].op == OP_LE ? "<=" : ">=") + " " +
                             join_plan->conds_[i].rhs_col.tab_name + "." + join_plan->conds_[i].rhs_col.col_name;
                }
                result += ")";
            }
            result += "\n";
            result += format_explain_plan(join_plan->left_, depth + 1);
            result += format_explain_plan(join_plan->right_, depth + 1);
            break;
        }
        case T_Projection: {
            auto proj_plan = std::dynamic_pointer_cast<ProjectionPlan>(plan);
            result += indent + "-> Projection";
            if (!proj_plan->sel_cols_.empty()) {
                result += " (Output: ";
                for (size_t i = 0; i < proj_plan->sel_cols_.size(); ++i) {
                    if (i > 0) result += ", ";
                    if (!proj_plan->sel_cols_[i].tab_name.empty()) {
                        result += proj_plan->sel_cols_[i].tab_name + ".";
                    }
                    result += proj_plan->sel_cols_[i].col_name;
                }
                result += ")";
            }
            result += "\n";
            result += format_explain_plan(proj_plan->subplan_, depth + 1);
            break;
        }
        case T_Sort: {
            auto sort_plan = std::dynamic_pointer_cast<SortPlan>(plan);
            result += indent + "-> Sort";
            if (!sort_plan->sel_cols_.empty()) {
                result += " (Order: ";
                for (size_t i = 0; i < sort_plan->sel_cols_.size(); ++i) {
                    if (i > 0) result += ", ";
                    if (!sort_plan->sel_cols_[i].tab_name.empty()) {
                        result += sort_plan->sel_cols_[i].tab_name + ".";
                    }
                    result += sort_plan->sel_cols_[i].col_name;
                    if (i < sort_plan->is_desc_list_.size()) {
                        result += sort_plan->is_desc_list_[i] ? " DESC" : " ASC";
                    }
                }
                result += ")";
            }
            result += "\n";
            result += format_explain_plan(sort_plan->subplan_, depth + 1);
            break;
        }
        case T_Aggregate: {
            auto agg_plan = std::dynamic_pointer_cast<AggregatePlan>(plan);
            result += indent + "-> Aggregate";
            if (!agg_plan->group_by_cols_.empty()) {
                result += " (Group By: ";
                for (size_t i = 0; i < agg_plan->group_by_cols_.size(); ++i) {
                    if (i > 0) result += ", ";
                    result += agg_plan->group_by_cols_[i]->col_name;
                }
                result += ")";
            }
            result += "\n";
            result += format_explain_plan(agg_plan->prev_, depth + 1);
            break;
        }
        default:
            result += indent + "-> Unknown Plan Type\n";
            break;
    }
    
    return result;
}