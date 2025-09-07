/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "execution_sort.h"
#include <algorithm>
#include <iostream>

void SortExecutor::beginTuple() {
    prev_->beginTuple();
    tuples_.clear();
    
    while (!prev_->is_end()) {
        auto tuple = prev_->Next();
        if (tuple != nullptr) {
            tuples_.push_back(std::move(tuple));
        }
        prev_->nextTuple();
    }
    
    
    // 排序
    std::sort(tuples_.begin(), tuples_.end(), [this](const std::unique_ptr<RmRecord>& a, const std::unique_ptr<RmRecord>& b) {
        return compare_tuples(a, b);
    });
    
    position = 0;
}

void SortExecutor::nextTuple() {
    position++;
}

std::unique_ptr<RmRecord> SortExecutor::Next() {
    if (is_end()) {
        return nullptr;
    }
    
    // 创建当前元组的副本
    auto& current = tuples_[position];
    auto result = std::make_unique<RmRecord>(current->size);
    memcpy(result->data, current->data, current->size);
    return result;
}

bool SortExecutor::is_end() const {
    return position >= tuples_.size();
}

bool SortExecutor::compare_tuples(const std::unique_ptr<RmRecord>& a, const std::unique_ptr<RmRecord>& b) {
    for (size_t i = 0; i < cols_.size(); ++i) {
        const ColMeta& col = cols_[i];
        bool is_desc = is_desc_[i];
        
        char* a_data = a.get()->data + col.offset;
        char* b_data = b.get()->data + col.offset;
        
        int cmp = 0;
        
        switch (col.type) {
            case TYPE_INT: {
                int a_val = *(int*)a_data;
                int b_val = *(int*)b_data;
                if (a_val < b_val) cmp = -1;
                else if (a_val > b_val) cmp = 1;
                break;
            }
            case TYPE_FLOAT: {
                float a_val = *(float*)a_data;
                float b_val = *(float*)b_data;
                if (a_val < b_val) cmp = -1;
                else if (a_val > b_val) cmp = 1;
                break;
            }
            case TYPE_STRING: {
                std::string a_str(a_data, col.len);
                std::string b_str(b_data, col.len);
                // 去除尾部的空字符
                a_str = a_str.substr(0, a_str.find('\0'));
                b_str = b_str.substr(0, b_str.find('\0'));
                cmp = a_str.compare(b_str);
                break;
            }
        }
        
        if (cmp != 0) {
            return is_desc ? (cmp > 0) : (cmp < 0);
        }
    }
    
    return false; // 相等
}