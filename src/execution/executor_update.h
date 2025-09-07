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

#include "execution_defs.h"
#include "execution_manager.h"
#include "executor_abstract.h"
#include "index/ix.h"
#include "system/sm.h"
#include "recovery/log_manager.h"
#include "common/common.h"

class UpdateExecutor : public AbstractExecutor {
private:
    TabMeta tab_;
    std::vector<Condition> conds_;
    RmFileHandle *fh_;
    std::vector<Rid> rids_;
    std::string tab_name_;
    std::vector<SetClause> set_clauses_;
    SmManager *sm_manager_;

public:
    UpdateExecutor(SmManager *sm_manager, const std::string &tab_name, std::vector<SetClause> set_clauses,
                   std::vector<Condition> conds, std::vector<Rid> rids, Context *context) {
        sm_manager_ = sm_manager;
        tab_name_ = tab_name;
        set_clauses_ = set_clauses;
        tab_ = sm_manager_->db_.get_table(tab_name);
        fh_ = sm_manager_->fhs_.at(tab_name).get();
        conds_ = conds;
        rids_ = rids;
        context_ = context;
    }

    std::unique_ptr<RmRecord> Next() override {
        // 类型检查与转换（支持表达式）
        for (auto &set_clause: set_clauses_) {
            auto lhs_col = tab_.get_col(set_clause.lhs.col_name);
            if (!set_clause.is_expr_) {
                // 简单赋值的类型检查
                if (lhs_col->type == TYPE_FLOAT && set_clause.rhs.type == TYPE_INT) {
                    set_clause.rhs.type = TYPE_FLOAT;
                    set_clause.rhs.float_val = (float) set_clause.rhs.int_val;
                } else if (lhs_col->type != set_clause.rhs.type) {
                    throw IncompatibleTypeError(coltype2str(lhs_col->type), coltype2str(set_clause.rhs.type));
                }
                if (!set_clause.is_add) {
                    set_clause.rhs.init_raw(lhs_col->len);
                }
            }
        }
        RmRecord old_rec;
        RmRecord new_rec;
        for (auto &rid: rids_) {
            // 先获取原始记录用于事务回滚
            auto original_rec = fh_->get_record(rid, context_);

            auto rec = fh_->get_record(rid, context_);
            old_rec = *rec;
            // 生成新记录（支持表达式求值）
            for (auto &set: set_clauses_) {
                auto col = tab_.get_col(set.lhs.col_name);
                if (set.is_expr_) {
                    // 表达式赋值：支持如 score = score + 5.5
                    Value expr_result = evaluate_expr(set.rhs_expr_.get(), rec.get(), tab_.cols);
                    expr_result.init_raw(col->len);
                    memcpy(rec->data + col->offset, expr_result.raw->data, col->len);
                } else if (!set.is_add) {
                    // 简单赋值
                    memcpy(rec->data + col->offset, set.rhs.raw->data, col->len);
                } else {
                    // 加法赋值（保留兼容性）
                    std::shared_ptr<RmRecord> raw = std::make_shared<RmRecord>(col->len);
                    if (set.rhs.type == TYPE_INT) {
                        *(int *) (raw->data) = *reinterpret_cast<int *>(rec->data + col->offset) + set.rhs.int_val;
                    } else if (set.rhs.type == TYPE_FLOAT) {
                        *(float *) (raw->data) =
                                *reinterpret_cast<float *>(rec->data + col->offset) + set.rhs.float_val;
                    }
                    memcpy(rec->data + col->offset, raw->data, col->len);
                }
            }
            new_rec = *rec;
            // 如果有事务，将原始记录保存到write_set中并记录日志
            if (context_->txn_ != nullptr) {
                WriteRecord* write_record = new WriteRecord(WType::UPDATE_TUPLE, tab_name_, rid, *original_rec);
                context_->txn_->append_write_record(write_record);

                // 记录UPDATE日志
                if (context_->log_mgr_ != nullptr) {
                    printf("[DEBUG] Creating UPDATE log for table: %s\n", tab_name_.c_str());
                    RmRecord new_record(rec->size, rec->data);
                    UpdateLogRecord update_log(context_->txn_->get_transaction_id(), *original_rec, new_record, rid, tab_name_, context_->txn_->get_prev_lsn());
                    printf("[DEBUG] Created UpdateLogRecord\n");
                    lsn_t lsn = context_->log_mgr_->add_log_to_buffer(&update_log);
                    printf("[DEBUG] Added UPDATE log to buffer, LSN: %d\n", lsn);
                    context_->txn_->set_prev_lsn(lsn);
                    printf("[DEBUG] Set prev_lsn for transaction\n");
                }
            }

            // 唯一索引预检查
            for(auto & index : tab_.indexes) {
                auto ih = sm_manager_->ihs_.at(sm_manager_->get_ix_manager()->get_index_name(tab_name_, index.cols)).get();
                char* old_key = new char[index.col_tot_len];
                char* new_key = new char[index.col_tot_len];
                int offset = 0;
                for(size_t j = 0; j < index.col_num; ++j) {
                    memcpy(old_key + offset, old_rec.data + index.cols[j].offset, index.cols[j].len);
                    memcpy(new_key + offset, new_rec.data + index.cols[j].offset, index.cols[j].len);
                    offset += index.cols[j].len;
                }
                if(memcmp(old_key, new_key, index.col_tot_len) != 0) {
                    auto leaf_node = ih->find_leaf_page(new_key,Operation::FIND,context_->txn_).first;
                    int idx = leaf_node->lower_bound(new_key);
                    if(!(idx == leaf_node->get_size() || ix_compare(leaf_node->get_key(idx),new_key,leaf_node->file_hdr->col_types_,leaf_node->file_hdr->col_lens_)!=0))
                        throw RMDBError("insert key not unique! --UpdateExecutor::Next()");
                }
                delete[] old_key;
                delete[] new_key;
            }
            for(auto &index: tab_.indexes) {
                auto ih = sm_manager_->ihs_.at(sm_manager_->get_ix_manager()->get_index_name(tab_name_, index.cols)).get();
                char old_key[index.col_tot_len];
                char new_key[index.col_tot_len];
                int offset = 0;
                for(size_t j = 0; j < index.col_num; ++j) {
                    memcpy(old_key + offset, old_rec.data + index.cols[j].offset, index.cols[j].len);
                    memcpy(new_key + offset, new_rec.data + index.cols[j].offset, index.cols[j].len);
                    offset += index.cols[j].len;
                }
                if(memcmp(old_key, new_key, index.col_tot_len) != 0) {
                    ih->delete_entry(old_key, context_->txn_);
                    ih->insert_entry(new_key, rid, context_->txn_);
                }
            }
            // 更新记录（仅保留基础数据写入）
            fh_->update_record(rid, rec->data, context_);
        }
        return nullptr;
    }

    ColMeta get_col_offset(const TabCol &target) override {
        auto pos = get_col(cols(), target);
        return *pos;
    }

    const std::vector<ColMeta> &cols() const override { return tab_.cols; }

    Rid &rid() override { return _abstract_rid; }
    std::string getType() override { return "UpdateExecutor"; }
};