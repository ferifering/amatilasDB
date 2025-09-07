#include "executor_semi_join.h"

void SemiJoinExecutor::beginTuple() {
    left_->beginTuple();
    left_end_ = left_->is_end();

    while (!left_end_ && !findMatchForCurrentLeft()) {
        left_->nextTuple();
        left_end_ = left_->is_end();
    }
}

void SemiJoinExecutor::nextTuple() {
    left_->nextTuple();
    left_end_ = left_->is_end();

    while (!left_end_ && !findMatchForCurrentLeft()) {
        left_->nextTuple();
        left_end_ = left_->is_end();
    }
}

std::unique_ptr<RmRecord> SemiJoinExecutor::Next() {
    if (is_end()) {
        return nullptr;
    }

    return left_->Next();
}

bool SemiJoinExecutor::findMatchForCurrentLeft() {
    if (left_->is_end()) {
        return false;
    }

    auto left_rec = left_->Next();
    if (!left_rec) {
        return false;
    }

    right_->beginTuple();
    
    while (!right_->is_end()) {
        auto right_rec = right_->Next();
        if (right_rec && check_conds(left_rec.get(), right_rec.get())) {
            return true;
        }
        right_->nextTuple();
    }
    
    return false;
}

bool SemiJoinExecutor::check_conds(RmRecord *left_rec, RmRecord *right_rec) {
    for (const auto &cond : fed_conds_) {
        if (!check_cond(left_rec, right_rec, cond)) {
            return false;
        }
    }
    return true;
}

bool SemiJoinExecutor::check_cond(RmRecord *left_rec, RmRecord *right_rec, const Condition &cond) {
    auto left_cols = left_->cols();
    auto right_cols = right_->cols();
    
    auto left_col_it = left_->get_col(left_cols, cond.lhs_col);
    auto right_col_it = right_->get_col(right_cols, cond.rhs_col);
    
    char *left_data = left_rec->data + left_col_it->offset;
    char *right_data = right_rec->data + right_col_it->offset;
    
    return evaluate_compare(left_data, right_data, left_col_it->type, right_col_it->type, left_col_it->len, right_col_it->len, cond.op);
}

bool SemiJoinExecutor::evaluate_compare(char *left_data, char *right_data, ColType left_type, ColType right_type, int left_len, int right_len, CompOp op) {
    assert(left_type == right_type);
    
    int cmp_result;
    switch (left_type) {
        case TYPE_INT: {
            int left_val = *(int*)left_data;
            int right_val = *(int*)right_data;
            cmp_result = (left_val < right_val) ? -1 : (left_val > right_val) ? 1 : 0;
            break;
        }
        case TYPE_FLOAT: {
            float left_val = *(float*)left_data;
            float right_val = *(float*)right_data;
            cmp_result = (left_val < right_val) ? -1 : (left_val > right_val) ? 1 : 0;
            break;
        }
        case TYPE_STRING: {
            cmp_result = memcmp(left_data, right_data, std::min(left_len, right_len));
            if (cmp_result == 0 && left_len != right_len) {
                cmp_result = (left_len < right_len) ? -1 : 1;
            }
            break;
        }
        default:
            throw InternalError("Unsupported column type for comparison");
    }
    
    switch (op) {
        case OP_EQ: return cmp_result == 0;
        case OP_NE: return cmp_result != 0;
        case OP_LT: return cmp_result < 0;
        case OP_GT: return cmp_result > 0;
        case OP_LE: return cmp_result <= 0;
        case OP_GE: return cmp_result >= 0;
        default:
            throw InternalError("Unsupported comparison operator");
    }
}

ColMeta *SemiJoinExecutor::get_col(const std::vector<ColMeta> &rec_cols, const TabCol &target) {
    auto pos = std::find_if(rec_cols.begin(), rec_cols.end(), [&](const ColMeta &col) {
        return col.tab_name == target.tab_name && col.name == target.col_name;
    });
    if (pos == rec_cols.end()) {
        throw ColumnNotFoundError(target.tab_name + "." + target.col_name);
    }
    return const_cast<ColMeta*>(&(*pos));
}

Rid &SemiJoinExecutor::rid() {
    return left_->rid();
}

const std::vector<ColMeta> &SemiJoinExecutor::cols() const {
    return left_->cols();
}

bool SemiJoinExecutor::is_end() const {
    return left_end_ || left_->is_end();
}

size_t SemiJoinExecutor::tupleLen() const {
    return left_->tupleLen();
}

std::string SemiJoinExecutor::getType() {
    return "SemiJoinExecutor";
}