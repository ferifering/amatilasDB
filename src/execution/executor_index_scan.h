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

class IndexScanExecutor : public AbstractExecutor {
   private:
    std::string tab_name_;                      // 表名称
    TabMeta tab_;                               // 表的元数据
    std::vector<Condition> conds_;              // 扫描条件
    RmFileHandle *fh_;                          // 表的数据文件句柄
    std::vector<ColMeta> cols_;                 // 需要读取的字段
    size_t len_;                                // 选取出来的一条记录的长度
    std::vector<Condition> fed_conds_;          // 扫描条件，和conds_字段相同
    std::vector<Condition> index_conds_;
    bool is_con_closed_;
    Condition con_closed_;
    std::vector<std::string> index_col_names_;  // index scan涉及到的索引包含的字段
    IndexMeta index_meta_;                      // index scan涉及到的索引元数据

    Rid rid_;
    std::unique_ptr<RecScan> scan_;

    SmManager *sm_manager_;

   public:
    IndexScanExecutor(SmManager *sm_manager, std::string tab_name, std::vector<Condition> conds, std::vector<std::string> index_col_names,
                      Context *context) {
        sm_manager_ = sm_manager;
        context_ = context;
        tab_name_ = std::move(tab_name);
        tab_ = sm_manager_->db_.get_table(tab_name_);
        conds_ = std::move(conds);
        index_col_names_ = index_col_names;
        index_meta_ = *(tab_.get_index_meta(index_col_names_));
        fh_ = sm_manager_->fhs_.at(tab_name_).get();
        cols_ = tab_.cols;
        len_ = cols_.back().offset + cols_.back().len;
        is_con_closed_ = false;
        std::map<CompOp, CompOp> swap_op = {
                {OP_EQ, OP_EQ}, {OP_NE, OP_NE}, {OP_LT, OP_GT}, {OP_GT, OP_LT}, {OP_LE, OP_GE}, {OP_GE, OP_LE},
        };

        // 确保所有条件的左侧是表的列
        for (auto &cond : conds_) {
            if (cond.lhs_col.tab_name != tab_name_) {
                assert(!cond.is_rhs_val && cond.rhs_col.tab_name == tab_name_);
                std::swap(cond.lhs_col, cond.rhs_col);
                cond.op = swap_op.at(cond.op);
            }
        }
        fed_conds_ = conds_;

        // 按照索引列的顺序处理条件
        std::vector<Condition> eq_conds;
        std::vector<Condition> other_conds; // 不包含!=

        // 创建一个映射，存储每个列名对应的条件
        std::map<std::string, std::vector<Condition>> col_conds_map;

        for (auto &cond : fed_conds_) {
            if(cond.op != OP_NE && cond.is_rhs_val) {
                col_conds_map[cond.lhs_col.col_name].push_back(cond);
            }
        }

        // 按照索引列的顺序处理条件
        for(auto &index_col : index_meta_.cols) {
            auto it = col_conds_map.find(index_col.name);
            if(it != col_conds_map.end()) {
                // 找到了这个列的条件
                for(auto &cond : it->second) {
                    if(cond.op == OP_EQ) {
                        eq_conds.push_back(cond);
                    } else {
                        other_conds.push_back(cond);
                    }
                }
            }
        }

        // 处理索引条件
        for(auto &index_col : index_meta_.cols) {
            bool found = false;
            for(auto &eq_cond : eq_conds) {
                if(eq_cond.lhs_col.col_name == index_col.name) {
                    found = true;
                    index_conds_.push_back(eq_cond);
                    break;
                }
            }
            if(found) continue;
            for(auto &other_cond : other_conds) {
                if(other_cond.lhs_col.col_name == index_col.name) {
                    found = true;
                    index_conds_.push_back(other_cond);
                    break;
                }
            }
            if(found){
                get_condition(index_conds_.back(), other_conds);
            }
            break;
        }
    }



    void beginTuple() override {
        context_->lock_mgr_->lock_IS_on_table(context_->txn_,fh_->GetFd());
        IxIndexHandle* ix_handle = sm_manager_->ihs_.at(sm_manager_->get_ix_manager()->get_index_name(tab_name_,index_col_names_)).get();
        char lower_bound[index_meta_.col_tot_len],upper_bound[index_meta_.col_tot_len];
        set_bound(lower_bound,upper_bound);
        int offset = 0,res = 0;
        for (auto & col : index_meta_.cols){
            res = ix_compare(lower_bound + offset, upper_bound + offset, col.type, col.len);
            if (res != 0)
                break;
            offset += col.len;
        }
        if(res > 0){
            auto lower = ix_handle->leaf_end();
            auto upper = ix_handle->leaf_end();
            scan_ = std::make_unique<IxScan>(ix_handle, lower, upper, sm_manager_->get_bpm());
        }
        else{
            auto lower = ix_handle->lower_bound(lower_bound);
            auto upper = ix_handle->upper_bound(upper_bound);
            scan_ = std::make_unique<IxScan>(ix_handle, lower, upper, sm_manager_->get_bpm());
        }
        while(!scan_->is_end()){
            rid_ = scan_->rid();
            try {
                auto record = fh_->get_record(rid_,context_);
                if(check_cons(conds_,record.get(),cols_)){
                    context_->lock_mgr_->lock_shared_on_record(context_->txn_, rid_, fh_->GetFd());
                    break;
                }
            } catch (RecordNotFoundError &e) {
                // 记录在当前事务快照中不可见，跳过
            }
            scan_->next();
        }
    }

    void nextTuple() override {
        scan_->next();
        while(!scan_->is_end()){
            rid_ = scan_->rid();
            try {
                auto record = fh_->get_record(rid_,context_);
                if(check_cons(conds_,record.get(),cols_)){
                    context_->lock_mgr_->lock_shared_on_record(context_->txn_, rid_, fh_->GetFd());
                    break;
                }
            } catch (RecordNotFoundError &e) {
                // 记录在当前事务快照中不可见，跳过
            }
            scan_->next();
        }
    }

    std::unique_ptr<RmRecord> Next() override {
        return scan_->is_end()? nullptr:fh_->get_record(rid(),context_);
    }

    Rid &rid() override { return rid_; }

    ~IndexScanExecutor() override = default;

    size_t tupleLen() const override {
        return len_;
    }

    const std::vector<ColMeta> &cols() const override {
        return cols_;
    }

    std::string getType() override {
        return "IndexScanExecutor";
    }

    bool is_end() const override {
        return scan_->is_end();
    }

    ColMeta get_col_offset(const TabCol &target) override {
        return AbstractExecutor::get_col_offset(target);
    }

    void set_limit(const std::string& col_name,char* bound,int offset,int col_len,bool max_or_min = false){
        int int_max = max_or_min? -2147483648 : 2147483647;
        float float_max = max_or_min? 1.175494E-38:3.402823E38;
        char* datetime_max = const_cast<char *>(max_or_min ? "0000-01-01 00:00:00" : "9999-12-31 23:59:59");
        if (tab_.get_col(col_name)->type == TYPE_INT)
            memcpy(bound + offset,&int_max,col_len);
        else if (tab_.get_col(col_name)->type == TYPE_FLOAT)
            memcpy(bound + offset,&float_max,col_len);
        else if (tab_.get_col(col_name)->type == TYPE_STRING){
            for (int i = 0;i < col_len;i++){
                bound[offset + i] = max_or_min? 0x00:0xff;
            }
        } else if (tab_.get_col(col_name)->type == TYPE_DATETIME)
            memcpy(bound + offset,datetime_max,col_len);
    }

    void set_bound(char* lower_bound,char* upper_bound){
        int i = 0,offset = 0;
        for (;i < index_conds_.size();i++){
            int col_len = index_meta_.cols[i].len;
            if (index_conds_[i].op == OP_EQ){
                memcpy(lower_bound + offset,index_conds_[i].rhs_val.raw->data,col_len);
                memcpy(upper_bound + offset,index_conds_[i].rhs_val.raw->data,col_len);
                offset += col_len;
            }else{
                if(index_conds_[i].op == OP_LE || index_conds_[i].op == OP_LT){
                    memcpy(upper_bound+offset,index_conds_[i].rhs_val.raw->data,col_len);
                    if(is_con_closed_){
                        memcpy(lower_bound+offset,con_closed_.rhs_val.raw->data,col_len);
                    }else{
                        set_limit(index_conds_[i].lhs_col.col_name,lower_bound,offset,col_len, true);
                    }
                    offset += col_len;
                }
                if(index_conds_[i].op == OP_GE || index_conds_[i].op == OP_GT){
                    memcpy(lower_bound+offset,index_conds_[i].rhs_val.raw->data,col_len);
                    if(is_con_closed_){
                        memcpy(upper_bound+offset,con_closed_.rhs_val.raw->data,col_len);
                    }else{
                        set_limit(index_conds_[i].lhs_col.col_name,upper_bound,offset,col_len);
                    }
                    offset += col_len;
                }
            }
        }
        for(;i<index_meta_.cols.size();++i){
            int len = index_meta_.cols[i].len;
            set_limit(index_meta_.cols[i].name,lower_bound,offset,len, true);
            set_limit(index_meta_.cols[i].name,upper_bound,offset,len);
            offset += len;
        }
    }

    bool get_condition(Condition last_neq_cond, std::vector<Condition> &other_conditions){
        auto op = last_neq_cond.op;
        if (op == OP_LE || op == OP_LT){ // <= || <
            for (const auto& cond:other_conditions) {
                if ((cond.op == OP_GE || cond.op == OP_GT) && cond.lhs_col.col_name == last_neq_cond.lhs_col.col_name){ // > || >=
                    con_closed_ = cond;
                    return true;
                }
            }
        }else if (op == OP_GT || op == OP_GE){ // > || >=
            for (const auto& cond:other_conditions) {
                if ((cond.op == OP_LE || cond.op == OP_LT) && cond.lhs_col.col_name == last_neq_cond.lhs_col.col_name){ // <= || <
                    con_closed_ = cond;
                    return true;
                }
            }
        }
        return false;
    }

    bool check_con (const Condition &cond, const RmRecord *record ,const std::vector<ColMeta> &cols_meta){
        auto lhs_col = get_col(cols_meta,cond.lhs_col);
        char *lhs_data = record->data + lhs_col->offset;
        char *rhs_data;
        ColType rhs_type;
        if(cond.is_rhs_val){
            rhs_type = cond.rhs_val.type;
            rhs_data = cond.rhs_val.raw->data;
        }else{
            auto rhs_col = get_col(cols_meta, cond.rhs_col);
            rhs_type = rhs_col->type;
            rhs_data = record->data + rhs_col->offset;
        }
        char lhs_copy[lhs_col->len];
        char rhs_copy[lhs_col->len];
        memcpy(lhs_copy,lhs_data,lhs_col->len);
        memcpy(rhs_copy,rhs_data,lhs_col->len);
        int cmp = ix_compare(lhs_copy,rhs_copy,rhs_type,lhs_col->len);
        if (cond.op == OP_EQ) return cmp == 0;
        else if (cond.op == OP_NE) return cmp != 0;
        else if (cond.op == OP_LT) return cmp < 0;
        else if (cond.op == OP_LE) return cmp <= 0;
        else if (cond.op == OP_GT) return cmp > 0;
        else if (cond.op == OP_GE) return cmp >= 0;
        else throw RMDBError("unexpected operation!");
    }

    bool check_cons(const std::vector<Condition> &conds, const RmRecord *record,const std::vector<ColMeta> &cols_meta){
        for(auto &cond : conds){
            if(!check_con(cond,record,cols_meta)){
                return false;
            }
        }
        return true;
    }
};