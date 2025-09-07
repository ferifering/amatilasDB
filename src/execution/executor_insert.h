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
#include <iostream>

class InsertExecutor : public AbstractExecutor {
   private:
    TabMeta tab_;                   // 表的元数据
    std::vector<Value> values_;     // 需要插入的数据
    RmFileHandle *fh_;              // 表的数据文件句柄
    std::string tab_name_;          // 表名称
    Rid rid_;                       // 插入的位置，由于系统默认插入时不指定位置，因此当前rid_在插入后才赋值
    SmManager *sm_manager_;

   public:
    InsertExecutor(SmManager *sm_manager, const std::string &tab_name, std::vector<Value> values, Context *context) {
        sm_manager_ = sm_manager;
        tab_ = sm_manager_->db_.get_table(tab_name);
        values_ = values;
        tab_name_ = tab_name;
        if (values.size() != tab_.cols.size()) {
            throw InvalidValueCountError();
        }
        fh_ = sm_manager_->fhs_.at(tab_name).get();
        context_ = context;
    };

    std::unique_ptr<RmRecord> Next() override {
        // Make record buffer
        RmRecord rec(fh_->get_file_hdr().record_size);
        for (size_t i = 0; i < values_.size(); i++) {
            auto &col = tab_.cols[i];
            auto &val = values_[i];
            if (col.type != val.type) {
                // if (col.type == TYPE_FLOAT && val.type == TYPE_INT) {
                //     val.f_ = (float)val.i_;
                //     val.type = TYPE_FLOAT;
                // } else {
                //     throw IncompatibleTypeError(coltype2str(col.type), coltype2str(val.type));
                // }
            }
            val.init_raw(col.len);
            memcpy(rec.data + col.offset, val.raw->data, col.len);
        }

        // 唯一索引预检查
        for(auto & index : tab_.indexes) {
            auto ih = sm_manager_->ihs_.at(sm_manager_->get_ix_manager()->get_index_name(tab_name_, index.cols)).get();

            char* key = new char[index.col_tot_len];
            int offset = 0;
            for(size_t j = 0; j < index.col_num; ++j) {
                memcpy(key + offset, rec.data + index.cols[j].offset, index.cols[j].len);
                offset += index.cols[j].len;
            }

            auto leaf_node = ih->find_leaf_page(key,Operation::FIND,context_->txn_).first;
            int idx = leaf_node->lower_bound(key);
            if(!(idx == leaf_node->get_size() || ix_compare(leaf_node->get_key(idx),key,leaf_node->file_hdr->col_types_,leaf_node->file_hdr->col_lens_)!=0))
                throw RMDBError("insert key not unique! --InsertExecutor::Next()");
        }

        // Insert into record file
        rid_ = fh_->insert_record(rec.data, context_);

        // 记录INSERT操作到事务的write_set中以支持回滚
        if (context_->txn_ != nullptr) {
            auto write_record = new WriteRecord(WType::INSERT_TUPLE, tab_name_, rid_);
            context_->txn_->append_write_record(write_record);
            
            // 记录INSERT日志
            if (context_->log_mgr_ != nullptr) {
                std::cout << "[DEBUG] Creating INSERT log for table: " << tab_name_ << std::endl;
                RmRecord log_record(rec.size, rec.data);
                std::cout << "[DEBUG] Created RmRecord for log, size: " << rec.size << std::endl;
                InsertLogRecord insert_log(context_->txn_->get_transaction_id(), log_record, rid_, tab_name_, context_->txn_->get_prev_lsn());
                std::cout << "[DEBUG] Created InsertLogRecord" << std::endl;
                lsn_t lsn = context_->log_mgr_->add_log_to_buffer(&insert_log);
                std::cout << "[DEBUG] Added log to buffer, LSN: " << lsn << std::endl;
                context_->txn_->set_prev_lsn(lsn);
                std::cout << "[DEBUG] Set prev_lsn for transaction" << std::endl;
            }
        }

        // Insert into index
        for(size_t i = 0; i < tab_.indexes.size(); ++i) {
            auto& index = tab_.indexes[i];
            auto ih = sm_manager_->ihs_.at(sm_manager_->get_ix_manager()->get_index_name(tab_name_, index.cols)).get();
            char* key = new char[index.col_tot_len];
            int offset = 0;
            for(size_t j = 0; j < index.col_num; ++j) {
                memcpy(key + offset, rec.data + index.cols[j].offset, index.cols[j].len);
                offset += index.cols[j].len;
            }
            ih->insert_entry(key, rid_, context_->txn_);
        }
        return nullptr;
    }
    Rid &rid() override { return rid_; }
};