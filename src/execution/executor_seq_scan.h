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
#include "common/common.h"

class SeqScanExecutor : public AbstractExecutor
{
private:
    std::string tab_name_;             // 表的名称
    std::vector<Condition> conds_;     // scan的条件
    RmFileHandle *fh_;                 // 表的数据文件句柄
    std::vector<ColMeta> cols_;        // scan后生成的记录的字段
    size_t len_;                       // scan后生成的每条记录的长度
    std::vector<Condition> fed_conds_; // 同conds_，两个字段相同

    Rid rid_;

    std::unique_ptr<RecScan> scan_; // table_iterator

    SmManager *sm_manager_;

public:
    SeqScanExecutor(SmManager *sm_manager, std::string tab_name, std::vector<Condition> conds, Context *context)
    {
        sm_manager_ = sm_manager;
        tab_name_ = std::move(tab_name);
        conds_ = std::move(conds);
        TabMeta &tab = sm_manager_->db_.get_table(tab_name_);
        fh_ = sm_manager_->fhs_.at(tab_name_).get();
        cols_ = tab.cols;
        len_ = cols_.back().offset + cols_.back().len;

        context_ = context;

        fed_conds_ = conds_;
    }


    //2t
    // 检查所有条件是否满足
    bool check_cons(const std::vector<Condition> &conds, const RmRecord *record, const std::vector<ColMeta> &cols_meta)
    {
        for (const auto &cond : conds)
        {
            if (!check_con(cond, record, cols_meta))
                return false;
        }
        return true;
    }

    // 检查单个条件是否满足
    bool check_con(const Condition &cond, const RmRecord *record, const std::vector<ColMeta> &cols_meta)
    {
        auto left_col = get_col(cols_meta, cond.lhs_col);
        char *left_data = record->data + left_col->offset;
        char *right_data;
        ColType data_type;

        if (cond.is_rhs_val)
        {
            right_data = cond.rhs_val.raw->data;
            data_type = cond.rhs_val.type;
        }
        else
        {
            auto right_col = get_col(cols_meta, cond.rhs_col);
            right_data = record->data + right_col->offset;
            data_type = right_col->type;
        }

        char left_copy[left_col->len];
        char right_copy[left_col->len];
        memcpy(left_copy, left_data, left_col->len);
        memcpy(right_copy, right_data, left_col->len);

        if (left_col->type != data_type)
        {
            throw IncompatibleTypeError(coltype2str(left_col->type), coltype2str(data_type));
        }

        int cmp = ix_compare(left_copy, right_copy, data_type, left_col->len);
        switch (cond.op)
        {
        case OP_EQ:
            return cmp == 0;
        case OP_NE:
            return cmp != 0;
        case OP_LT:
            return cmp < 0;
        case OP_GT:
            return cmp > 0;
        case OP_LE:
            return cmp <= 0;
        case OP_GE:
            return cmp >= 0;
        default:
            throw InternalError("未知操作符");
        }
    }

    void beginTuple() override
    {
        scan_ = std::make_unique<RmScan>(fh_);
        while (!scan_->is_end())
        {
            try {
                auto rec = fh_->get_record(scan_->rid(), context_);
                if (check_cons(fed_conds_, rec.get(), cols_))
                {
                    rid_ = scan_->rid();
                    break;
                }
            } catch (const RecordNotFoundError&) {
                // MVCC: 记录在当前事务快照中不可见，跳过
            }
            scan_->next();
        }
    }

    void nextTuple() override
    {
        scan_->next();
        while (!scan_->is_end())
        {
            try {
                auto rec = fh_->get_record(scan_->rid(), context_);
                if (check_cons(fed_conds_, rec.get(), cols_))
                {
                    rid_ = scan_->rid();
                    break;
                }
            } catch (const RecordNotFoundError&) {
                // MVCC: 记录在当前事务快照中不可见，跳过
            }
            scan_->next();
        }
    }

    std::unique_ptr<RmRecord> Next() override
    {
        return fh_->get_record(rid_, context_);
    }

    size_t tupleLen() const override
    {
        return len_;
    }

    const std::vector<ColMeta> &cols() const override { return cols_; }

    std::string getType() override
    {
        return "SeqScanExecutor";
    }

    bool is_end() const override
    {
        return scan_->is_end();
    }

    ColMeta get_col_offset(const TabCol &target) override
    {
        auto pos = get_col(cols(), target);
        return *pos;
    }

    Rid &rid() override { return rid_; }
};