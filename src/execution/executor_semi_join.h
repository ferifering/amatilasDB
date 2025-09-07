#pragma once

#include "execution_defs.h"
#include "execution_manager.h"
#include "executor_abstract.h"
#include "../index/ix.h"
#include "../system/sm.h"
#include <unordered_set>

class SemiJoinExecutor : public AbstractExecutor
{
private:
    std::unique_ptr<AbstractExecutor> left_;
    std::unique_ptr<AbstractExecutor> right_;
    size_t len_;
    std::vector<ColMeta> cols_;

    std::vector<Condition> fed_conds_;
    bool left_end_;
    
public:
    SemiJoinExecutor(std::unique_ptr<AbstractExecutor> left, std::unique_ptr<AbstractExecutor> right,
                     std::vector<Condition> conds)
    {
        left_ = std::move(left);
        right_ = std::move(right);

        if (left_->tupleLen() == 0 || right_->tupleLen() == 0)
        {
            throw InternalError("SemiJoinExecutor::Constructor Error: left tupleLen=" + std::to_string(left_->tupleLen()) + ", right tupleLen=" + std::to_string(right_->tupleLen()));
        }

        len_ = left_->tupleLen();
        cols_ = left_->cols();
        
        left_end_ = false;
        fed_conds_ = std::move(conds);
    }

    void beginTuple() override;
    void nextTuple() override;
    std::unique_ptr<RmRecord> Next() override;
    
    bool findMatchForCurrentLeft();
    bool check_conds(RmRecord *left_rec, RmRecord *right_rec);
    bool check_cond(RmRecord *left_rec, RmRecord *right_rec, const Condition &cond);
    bool evaluate_compare(char *left_data, char *right_data, ColType left_type, ColType right_type, int left_len, int right_len, CompOp op);
    ColMeta *get_col(const std::vector<ColMeta> &rec_cols, const TabCol &target);
    
    size_t tupleLen() const override;
    const std::vector<ColMeta> &cols() const override;
    std::string getType() override;
    bool is_end() const override;
    Rid &rid() override;
};