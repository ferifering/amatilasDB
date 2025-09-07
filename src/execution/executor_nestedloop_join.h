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

class NestedLoopJoinExecutor : public AbstractExecutor
{
private:
    std::unique_ptr<AbstractExecutor> left_;  // 左儿子节点（需要join的表）
    std::unique_ptr<AbstractExecutor> right_; // 右儿子节点（需要join的表）
    size_t len_;                              // join后获得的每条记录的长度
    std::vector<ColMeta> cols_;               // join后获得的记录的字段

    std::vector<Condition> fed_conds_; // join条件
    bool isend;



public:
    NestedLoopJoinExecutor(std::unique_ptr<AbstractExecutor> left, std::unique_ptr<AbstractExecutor> right,
                           std::vector<Condition> conds)
    {
        left_ = std::move(left);
        right_ = std::move(right);
        // 调试：检查左右表tupleLen是否为0
        if (left_->tupleLen() == 0 || right_->tupleLen() == 0)
        {
            throw InternalError("NestedLoopJoinExecutor::Constructor Error: left tupleLen=" + std::to_string(left_->tupleLen()) + ", right tupleLen=" + std::to_string(right_->tupleLen()));
        }
        len_ = left_->tupleLen() + right_->tupleLen();
        cols_ = left_->cols();
        auto right_cols = right_->cols();
        for (auto &col : right_cols)
        {
            col.offset += left_->tupleLen();
        }

        cols_.insert(cols_.end(), right_cols.begin(), right_cols.end());
        isend = false;
        fed_conds_ = std::move(conds);
    }


        //2t
    void beginTuple() override
    {
        left_->beginTuple();
        right_->beginTuple();
    }

    void nextTuple() override
    {
        // 需要一直往后找，直到符合条件的，或直到left和right都is_end
        bool found_pair = false;
        if (!right_->is_end())
            right_->nextTuple();
        while (!left_->is_end())
        {
            auto left_rec = left_->Next();
            while (!right_->is_end() && !found_pair)
            {
                auto right_rec = right_->Next();
                // check conds
                if (check_conds(left_rec.get(), right_rec.get()))
                {
                    found_pair = true;
                    return;
                }
                right_->nextTuple();
            }
            if (found_pair)
            {
                break;
            }
            right_->beginTuple();
            left_->nextTuple();
        }
        isend = true;
        return;
    }

    std::unique_ptr<RmRecord> Next() override
    {
        // 处理连接操作
        assert(!isend);

        // 获取左右表记录
        auto left_rec = left_->Next();
        auto right_rec = right_->Next();

        // 调试输出 - 检查记录长度
        if (static_cast<size_t>(left_rec->size) != static_cast<size_t>(left_->tupleLen()) || static_cast<size_t>(right_rec->size) != static_cast<size_t>(right_->tupleLen()))
        {
            throw InternalError("NestedLoopJoinExecutor::Next Error: left_rec size=" + std::to_string(left_rec->size) +
                                ", left tupleLen=" + std::to_string(left_->tupleLen()) + "; right_rec size=" +
                                std::to_string(right_rec->size) + ", right tupleLen=" +
                                std::to_string(right_->tupleLen()));
        }

        // 调试：检查cols_向量初始化（示例，根据实际代码调整）
        if (cols_.empty())
        {
            throw InternalError("NestedLoopJoinExecutor::Next Error: cols_ vector is empty");
        }
        for (size_t i = 0; i < cols_.size(); ++i)
        {
            // 由于 ColMeta 没有 GetOffset 和 GetLen 成员，推测使用 offset 和 len 成员替代
            if (cols_[i].offset < 0 || cols_[i].len <= 0)
            {
                throw InternalError("NestedLoopJoinExecutor::Next Error: cols_ index=" + std::to_string(i) + " offset=" + std::to_string(cols_[i].offset) + " len=" + std::to_string(cols_[i].len));
            }
        }
        // 创建新记录并合并数据
        auto new_rec = std::make_unique<RmRecord>(len_);
        memcpy(new_rec->data, left_rec->data, left_rec->size);
        memcpy(new_rec->data + left_rec->size, right_rec->data, right_rec->size);

        // 检查合并后的记录
        if (static_cast<size_t>(new_rec->size) != static_cast<size_t>(len_))
        {
            throw InternalError(
                "NestedLoopJoinExecutor::Next Error: merged record size=" + std::to_string(new_rec->size) +
                ", expected=" + std::to_string(len_));
        }

        return new_rec;
    }

    bool check_conds(RmRecord *left_rec, RmRecord *right_rec)
    {
        int len = fed_conds_.size();
        if (!len)
        {
            return true;
        }
        bool ret = check_cond(left_rec, right_rec, fed_conds_[0]);
        for (int i = 1; i < len; i++)
        {
            ret &= check_cond(left_rec, right_rec, fed_conds_[i]);
            if (!ret)
            {
                return false;
            }
        }
        return ret;
    }

    bool check_cond(RmRecord *left_rec, RmRecord *right_rec, Condition cond_) {
        auto left_col_it = left_->get_col(left_->cols(), cond_.lhs_col);
        auto right_col_it = right_->get_col(right_->cols(), cond_.rhs_col);
        char *left_val = left_rec->data + left_col_it->offset;
        char *right_val = right_rec->data + right_col_it->offset;
        assert(left_col_it->type == right_col_it->type);
        int cmp = ix_compare(left_val, right_val, right_col_it->type, right_col_it->len);
        
        switch (cond_.op) {
            case OP_EQ: return cmp == 0;
            case OP_NE: return cmp != 0;
            case OP_LT: return cmp == -1;
            case OP_GT: return cmp == 1;
            case OP_LE: return cmp != 1;
            case OP_GE: return cmp != -1;
            default: throw InternalError("NestedLoopJoinExecutor::check_cond Error: unsupported operator");
        }
    }




    size_t tupleLen() const { return len_; };
    const std::vector<ColMeta> &cols() const
    {
        return cols_;
    };
    std::string getType() { return "NestedLoopJoinExecutor"; };
    bool is_end() const { return isend; };



    Rid &rid() override { return _abstract_rid; }


};