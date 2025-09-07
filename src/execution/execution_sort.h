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
#include <iostream>

class SortExecutor : public AbstractExecutor {
   private:
    std::unique_ptr<AbstractExecutor> prev_;
    std::vector<ColMeta> cols_;                 // 支持多个键排序
    std::vector<bool> is_desc_;                 // 每列的排序方向
    size_t tuple_num;
    std::vector<size_t> used_tuple;
    std::vector<std::unique_ptr<RmRecord>> tuples_;
    size_t position;
    std::unique_ptr<RmRecord> current_tuple;

   public:
    SortExecutor(std::unique_ptr<AbstractExecutor> prev, std::vector<TabCol> sel_cols, std::vector<bool> is_desc_list) 
        : prev_(std::move(prev)), is_desc_(is_desc_list), tuple_num(0), position(0) {
        

        for (size_t i = 0; i < sel_cols.size(); ++i) {
            ColMeta col_meta = prev_->get_col_offset(sel_cols[i]);
            cols_.push_back(col_meta);
        }
        

        used_tuple.clear();
        tuples_.clear();
    }

    void beginTuple() override;
    void nextTuple() override;
    std::unique_ptr<RmRecord> Next() override;
    bool is_end() const override;
    Rid &rid() override { return _abstract_rid; }
    
    const std::vector<ColMeta> &cols() const override {
        return prev_->cols();
    }
    
    size_t tupleLen() const override {
        return prev_->tupleLen();
    }
    
    ColMeta get_col_offset(const TabCol &target) override {
        return prev_->get_col_offset(target);
    }

private:
    bool compare_tuples(const std::unique_ptr<RmRecord>& a, const std::unique_ptr<RmRecord>& b);
};