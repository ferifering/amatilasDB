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

class DeleteExecutor : public AbstractExecutor
{
private:
    TabMeta tab_;                  // 表的元数据
    std::vector<Condition> conds_; // delete的条件
    RmFileHandle *fh_;             // 表的数据文件句柄
    std::vector<Rid> rids_;        // 需要删除的记录的位置
    std::string tab_name_;         // 表名称
    SmManager *sm_manager_;

public:
    DeleteExecutor(SmManager *sm_manager, const std::string &tab_name, std::vector<Condition> conds,
                   std::vector<Rid> rids, Context *context)
    {
        sm_manager_ = sm_manager;
        tab_name_ = tab_name;
        tab_ = sm_manager_->db_.get_table(tab_name);
        fh_ = sm_manager_->fhs_.at(tab_name).get();
        conds_ = conds;
        rids_ = rids;
        context_ = context;
    }


    //2t
    std::unique_ptr<RmRecord> Next() override
    {
        for (auto &rid : rids_)
        {
            // 在删除之前获取原始记录以支持回滚
            if (context_->txn_ != nullptr) {
                auto original_record = fh_->get_record(rid, context_);
                auto write_record = new WriteRecord(WType::DELETE_TUPLE, tab_name_, rid, *original_record);
                context_->txn_->append_write_record(write_record);
                
                // 记录DELETE日志
                if (context_->log_mgr_ != nullptr) {
                    DeleteLogRecord delete_log(context_->txn_->get_transaction_id(), *original_record, rid, tab_name_, context_->txn_->get_prev_lsn());
                    lsn_t lsn = context_->log_mgr_->add_log_to_buffer(&delete_log);
                    context_->txn_->set_prev_lsn(lsn);
                }
            }

            //删除对应索引
            for (auto index : tab_.indexes){
                auto ix_handler = sm_manager_->ihs_.at(sm_manager_->get_ix_manager()->get_index_name(tab_name_, index.cols)).get();
                char key[index.col_tot_len];
                int offset = 0;
                for (int i =0;i < index.col_num;i++ ){
                    memcpy(key + offset,fh_->get_record(rid,context_)->data + index.cols[i].offset, index.cols[i].len);
                    offset = offset + index.cols[i].len;
                }
                ix_handler->delete_entry(key,context_->txn_);
            }

            // 删除表中记录
            fh_->delete_record(rid, context_);
        }
        return nullptr;
    }

    Rid &rid() override { return _abstract_rid; }
};