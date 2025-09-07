/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "executor_aggregate.h"

AggregateExecutor::AggregateExecutor(std::unique_ptr<AbstractExecutor> prev,
                                   std::vector<std::shared_ptr<ast::Col>> agg_exprs,
                                   std::vector<std::shared_ptr<ast::Col>> group_by_cols,
                                   std::vector<std::shared_ptr<ast::BinaryExpr>> having_conds,
                                   std::vector<std::shared_ptr<ast::Col>> select_cols,
                                   bool has_group_by,
                                   bool has_having)
    : prev_(std::move(prev)), context(nullptr), sm_manager_(nullptr) {
    
    dataMap.clear();
    
    int offset = 0;
    
    // 处理group by字段
    for (auto& gCol : group_by_cols) {
        AggColMeta aggMeta;
        ColMeta colMeta;
        
        // 从上游执行器获取列信息
        TabCol tabCol = {gCol->tab_name, gCol->col_name};
        colMeta = prev_->get_col_offset(tabCol);
        aggMeta.col = colMeta;
        
        // 设置输出列信息
        colMeta.offset = offset;
        colMeta.tab_name = gCol->tab_name;
        colMeta.name = gCol->col_name;
        
        // 处理别名
        if (!gCol->alias.empty()) {
            aggMeta.alias = gCol->alias;
        } else {
            aggMeta.alias = gCol->col_name;
        }
        
        colMetas.push_back(colMeta);
        groupByCols.push_back(aggMeta);
        offset += colMeta.len;
    }
    
    // 处理聚合函数
    for (auto& aCol : agg_exprs) {
        AggColMeta aggMeta;
        ColMeta colMeta;
        
        aggMeta.ag_type = aCol->aggType;
        colMeta.offset = offset;
        
        if (aCol->aggType == ast::AggFuncType::COUNT_ALL || aCol->col_name == "*") {
            // 对于COUNT(*)，设置虚拟列信息，不调用get_col_offset
            aggMeta.ag_type = ast::AggFuncType::COUNT_ALL;
            aggMeta.col.name = "*";
            aggMeta.col.tab_name = "*";
            aggMeta.col.type = TYPE_INT;
            aggMeta.col.len = sizeof(int);
            aggMeta.col.offset = 0;
            
            colMeta.tab_name = "*";
            colMeta.name = "*";
        } else {
            // 提取实际列名（去除聚合函数包装）
            std::string actual_col_name = aCol->col_name;
            if (actual_col_name.find('(') != std::string::npos && actual_col_name.find(')') != std::string::npos) {
                size_t start = actual_col_name.find('(') + 1;
                size_t end = actual_col_name.find(')');
                actual_col_name = actual_col_name.substr(start, end - start);
            }
            
            TabCol tabCol = {aCol->tab_name, actual_col_name};
            aggMeta.col = prev_->get_col_offset(tabCol);
            
            // 检查字符串类型聚合函数兼容性
            if (aggMeta.col.type == TYPE_STRING && aggMeta.ag_type == ast::AggFuncType::SUM) {
                throw IncompatibleTypeError(coltype2str(aggMeta.col.type), "SUM function");
            }
            
            colMeta.tab_name = aCol->tab_name;
            colMeta.name = actual_col_name;
        }
        
        // 处理别名
        if (!aCol->alias.empty()) {
            aggMeta.alias = aCol->alias;
            colMeta.name = aCol->alias;  // 使用别名作为列名
        } else {
            // 生成默认的聚合函数名
            std::string agg_name;
            switch (aggMeta.ag_type) {
                case ast::AggFuncType::COUNT: agg_name = "COUNT"; break;
                case ast::AggFuncType::COUNT_ALL: agg_name = "COUNT(*)"; break;
                case ast::AggFuncType::MAX: agg_name = "MAX"; break;
                case ast::AggFuncType::MIN: agg_name = "MIN"; break;
                case ast::AggFuncType::SUM: agg_name = "SUM"; break;
                case ast::AggFuncType::AVG: agg_name = "AVG"; break;
            }
            if (aggMeta.ag_type == ast::AggFuncType::COUNT_ALL) {
                aggMeta.alias = agg_name;
                colMeta.name = agg_name;
            } else {
                aggMeta.alias = agg_name + "(" + colMeta.name + ")";
                colMeta.name = agg_name + "(" + colMeta.name + ")";
            }
        }
        
        GetCol(aggMeta, colMeta);
        colMetas.push_back(colMeta);
        offset += colMeta.len;
        aggMetas.push_back(aggMeta);
    }
    
    // 处理having条件
    for (auto& cond : having_conds) {
        HavingCondition havingCondition;
        AggColMeta aggMeta;
        
        if (cond->lhs && std::dynamic_pointer_cast<ast::Col>(cond->lhs)) {
            auto lhs_col = std::dynamic_pointer_cast<ast::Col>(cond->lhs);
            if (lhs_col->isAgg) {
                havingCondition.is_agg = true;
                aggMeta.ag_type = lhs_col->aggType;
                
                if (lhs_col->aggType == ast::AggFuncType::COUNT_ALL || lhs_col->col_name == "*") {
                    // 对于COUNT(*)，设置虚拟列信息，不调用get_col_offset
                    aggMeta.ag_type = ast::AggFuncType::COUNT_ALL;
                    aggMeta.col.name = "*";
                    aggMeta.col.tab_name = "*";
                    aggMeta.col.type = TYPE_INT;
                    aggMeta.col.len = sizeof(int);
                    aggMeta.col.offset = 0;
                } else {
                    std::string actual_col_name = lhs_col->col_name;
                    if (actual_col_name.find('(') != std::string::npos && actual_col_name.find(')') != std::string::npos) {
                        size_t start = actual_col_name.find('(') + 1;
                        size_t end = actual_col_name.find(')');
                        actual_col_name = actual_col_name.substr(start, end - start);
                    }
                    
                    TabCol tabCol = {lhs_col->tab_name, actual_col_name};
                    aggMeta.col = prev_->get_col_offset(tabCol);
                    
                    // 检查字符串类型聚合函数兼容性
                    if (aggMeta.col.type == TYPE_STRING && 
                        aggMeta.ag_type != ast::AggFuncType::COUNT && 
                        aggMeta.ag_type != ast::AggFuncType::COUNT_ALL) {
                        throw IncompatibleTypeError(coltype2str(aggMeta.col.type), "aggregate function");
                    }
                }
            }
        }
        
        // 设置操作符
        switch (cond->op) {
            case ast::SV_OP_EQ: havingCondition.op = OP_EQ; break;
            case ast::SV_OP_NE: havingCondition.op = OP_NE; break;
            case ast::SV_OP_LT: havingCondition.op = OP_LT; break;
            case ast::SV_OP_GT: havingCondition.op = OP_GT; break;
            case ast::SV_OP_LE: havingCondition.op = OP_LE; break;
            case ast::SV_OP_GE: havingCondition.op = OP_GE; break;
        }
        
        // 设置右值
        auto rhs_int = std::dynamic_pointer_cast<ast::IntLit>(cond->rhs);
        auto rhs_float = std::dynamic_pointer_cast<ast::FloatLit>(cond->rhs);
        auto rhs_string = std::dynamic_pointer_cast<ast::StringLit>(cond->rhs);
        
        if (rhs_int) {
            havingCondition.rhs_val.set_int(rhs_int->val);
        } else if (rhs_float) {
            havingCondition.rhs_val.set_float(rhs_float->val);
        } else if (rhs_string) {
            havingCondition.rhs_val.set_str(rhs_string->val);
        }
        
        havingCondition.agg_col = aggMeta;
        havingConds.push_back(havingCondition);
    }
    
    tupleLength = GetTupleLen(aggMetas, groupByCols);
    resultIndex = 0;
}

void AggregateExecutor::GetCol(AggColMeta &aggMeta, ColMeta &colMeta) {
    switch (aggMeta.ag_type) {
        case ast::AggFuncType::SUM:
            colMeta.type = aggMeta.col.type;
            colMeta.len = aggMeta.col.len;
            break;
        case ast::AggFuncType::COUNT:
        case ast::AggFuncType::COUNT_ALL:
            colMeta.type = TYPE_INT;
            colMeta.len = sizeof(int);
            break;
        case ast::AggFuncType::AVG:
            colMeta.type = TYPE_FLOAT;
            colMeta.len = sizeof(float);
            break;
        case ast::AggFuncType::MIN:
        case ast::AggFuncType::MAX:
            colMeta.type = aggMeta.col.type;
            colMeta.len = aggMeta.col.len;
            break;
        default:
            throw std::runtime_error("Unknown AggFuncType");
    }
}

size_t AggregateExecutor::GetTupleLen(const std::vector<AggColMeta> &aggMetas, const std::vector<AggColMeta> &groupByCols) {
    size_t len = 0;
    for (const auto& col : groupByCols) {
        len += col.col.len;
    }
    for (const auto& aggMeta : aggMetas) {
        switch (aggMeta.ag_type) {
            case ast::AggFuncType::SUM:
                len += aggMeta.col.len;
                break;
            case ast::AggFuncType::COUNT:
            case ast::AggFuncType::COUNT_ALL:
                len += sizeof(int);
                break;
            case ast::AggFuncType::AVG:
                len += sizeof(float);
                break;
            case ast::AggFuncType::MIN:
            case ast::AggFuncType::MAX:
                len += aggMeta.col.len;
                break;
            default:
                throw std::runtime_error("Unknown AggFuncType");
        }
    }
    return len;
}

void AggregateExecutor::beginTuple() {
    groupByResults.clear();
    dataMap.clear();
    resultKeys.clear();
    havingRes.clear();
    count0 = false;
    resultIndex = 0;
    prev_->beginTuple();
    
    // 如果没有group by，创建一个空键
    if (groupByCols.empty()) {
        std::vector<Value> key;
        groupByResults[key] = std::vector<AggValue>(aggMetas.size());
        resultKeys.push_back(key);
    }
    
    // 处理空结果集的情况
    if (prev_->is_end()) {
        // 检查是否所有聚合函数都是COUNT
        bool all_count = true;
        for (const auto& agg : aggMetas) {
            if (agg.ag_type != ast::AggFuncType::COUNT && agg.ag_type != ast::AggFuncType::COUNT_ALL) {
                all_count = false;
                break;
            }
        }
        if (all_count) {
            count0 = true;
        } else {
            resultIndex++; // 跳过空结果
        }
        return;
    }
    
    // 处理所有元组
    while (!prev_->is_end()) {
        auto tuple = prev_->Next();
        ProcessTuple(tuple);
        dataMap.clear();
        prev_->nextTuple();
    }
    
    // 找到第一个满足having条件的结果
    while (resultIndex < resultKeys.size() && !havingConds.empty() && !Check_Having_Conditions(resultKeys[resultIndex])) {
        ++resultIndex;
    }
}

void AggregateExecutor::ProcessTuple(std::unique_ptr<RmRecord> &tuple) {
    // 读取数据到dataMap
    for (const auto& col : prev_->cols()) {
        dataMap[col.name] = tuple->data + col.offset;
    }
    
    // 构建分组键
    std::vector<Value> key;
    for (const auto& gCol : groupByCols) {
        char* data = dataMap[gCol.col.name];
        Value val(gCol.col.type, data, gCol.col.len);
        key.push_back(val);
    }
    
    // 如果是新的分组，初始化聚合结果
    if (groupByResults.find(key) == groupByResults.end()) {
        groupByResults[key] = std::vector<AggValue>(aggMetas.size());
        resultKeys.push_back(key);
    }
    
    // 如果有having条件，初始化having结果
    if (!havingConds.empty() && havingRes.find(key) == havingRes.end()) {
        havingRes[key] = std::vector<AggValue>(havingConds.size());
    }
    
    // 计算聚合值
    std::vector<AggValue>& aggResult = groupByResults[key];
    for (size_t i = 0; i < aggMetas.size(); i++) {
        CalculateAggValue(aggMetas[i], aggResult[i]);
    }
    
    // 计算having条件的聚合值
    if (!havingConds.empty()) {
        std::vector<AggValue>& havingResult = havingRes[key];
        for (size_t i = 0; i < havingConds.size(); i++) {
            const auto& cond = havingConds[i];
            if (cond.is_agg) {
                CalculateAggValue(cond.agg_col, havingResult[i]);
            }
        }
    }
}

void AggregateExecutor::CalculateAggValue(const AggColMeta &aggMeta, AggValue& res) {
    char* data = nullptr;
    
    // 对于COUNT(*)，不需要访问具体列数据
    if (aggMeta.ag_type != ast::AggFuncType::COUNT_ALL && aggMeta.col.name != "*" && !aggMeta.col.name.empty()) {
        auto it = dataMap.find(aggMeta.col.name);
        if (it != dataMap.end()) {
            data = it->second;
        }
    }
    
    switch (aggMeta.ag_type) {
        case ast::AggFuncType::COUNT:
        case ast::AggFuncType::COUNT_ALL:
            res.type = TYPE_INT;
            res.int_val++;
            break;
            
        case ast::AggFuncType::SUM:
            if (aggMeta.col.type == TYPE_INT) {
                res.type = TYPE_INT;
                res.int_val += *(int*)data;
            } else if (aggMeta.col.type == TYPE_FLOAT) {
                res.type = TYPE_FLOAT;
                res.float_val += *(float*)data;
            }
            break;
            
        case ast::AggFuncType::AVG:
            res.count++;
            if (aggMeta.col.type == TYPE_INT) {
                res.type = TYPE_FLOAT;
                res.float_val += *(int*)data;
            } else if (aggMeta.col.type == TYPE_FLOAT) {
                res.type = TYPE_FLOAT;
                res.float_val += *(float*)data;
            }
            break;
            
        case ast::AggFuncType::MAX:
            if (aggMeta.col.type == TYPE_INT) {
                if (!res.maxFlag) {
                    res.int_val = *(int*)data;
                    res.maxFlag = true;
                }
                res.type = TYPE_INT;
                res.int_val = std::max(res.int_val, *(int*)data);
            } else if (aggMeta.col.type == TYPE_FLOAT) {
                if (!res.maxFlag) {
                    res.float_val = *(float*)data;
                    res.maxFlag = true;
                }
                res.type = TYPE_FLOAT;
                res.float_val = std::max(res.float_val, *(float*)data);
            } else if (aggMeta.col.type == TYPE_STRING) {
                if (!res.maxFlag) {
                    res.str_val = std::string(data, aggMeta.col.len);
                    res.maxFlag = true;
                }
                res.type = TYPE_STRING;
                res.str_val = std::max(res.str_val, std::string(data, aggMeta.col.len));
            }
            break;
            
        case ast::AggFuncType::MIN:
            if (aggMeta.col.type == TYPE_INT) {
                if (!res.minFlag) {
                    res.int_val = *(int*)data;
                    res.minFlag = true;
                }
                res.type = TYPE_INT;
                res.int_val = std::min(res.int_val, *(int*)data);
            } else if (aggMeta.col.type == TYPE_FLOAT) {
                if (!res.minFlag) {
                    res.float_val = *(float*)data;
                    res.minFlag = true;
                }
                res.type = TYPE_FLOAT;
                res.float_val = std::min(res.float_val, *(float*)data);
            } else if (aggMeta.col.type == TYPE_STRING) {
                if (!res.minFlag) {
                    res.str_val = std::string(data, aggMeta.col.len);
                    res.minFlag = true;
                }
                res.type = TYPE_STRING;
                res.str_val = std::min(res.str_val, std::string(data, aggMeta.col.len));
            }
            break;
            
        default:
            throw std::runtime_error("Unknown AggFuncType");
    }
}

void AggregateExecutor::MergeKeyAndResult(const std::vector<Value>& key, const std::vector<AggValue>& result, RmRecord& record) {
    char* data = record.data;
    int offset = 0;
    
    // 合并分组键
    for (size_t i = 0; i < key.size(); ++i) {
        const Value& val = key[i];
        const ColMeta& col = groupByCols[i].col;
        
        switch (col.type) {
            case TYPE_INT:
                *(int*)(data + offset) = val.int_val;
                break;
            case TYPE_FLOAT:
                *(float*)(data + offset) = val.float_val;
                break;
            case TYPE_STRING:
                memcpy(data + offset, val.str_val.c_str(), col.len);
                break;
        }
        offset += col.len;
    }
    
    // 合并聚合结果
    for (size_t i = 0; i < result.size(); ++i) {
        const AggValue& aggVal = result[i];
        const AggColMeta& aggMeta = aggMetas[i];
        
        // 根据聚合函数类型处理结果
        switch (aggMeta.ag_type) {
            case ast::AggFuncType::COUNT:
            case ast::AggFuncType::COUNT_ALL:
                *(int*)(data + offset) = aggVal.int_val;
                break;
            case ast::AggFuncType::AVG:
                if (aggMeta.col.type == TYPE_INT) {
                    float avg_val = static_cast<float>(aggVal.int_val) / aggVal.count;
                    *(float*)(data + offset) = avg_val;
                } else {
                    float avg_val = aggVal.float_val / aggVal.count;
                    *(float*)(data + offset) = avg_val;
                }
                break;
            case ast::AggFuncType::SUM:
            case ast::AggFuncType::MIN:
            case ast::AggFuncType::MAX:
                switch (aggMeta.col.type) {
                    case TYPE_INT:
                        *(int*)(data + offset) = aggVal.int_val;
                        break;
                    case TYPE_FLOAT:
                        *(float*)(data + offset) = aggVal.float_val;
                        break;
                    case TYPE_STRING:
                        memcpy(data + offset, aggVal.str_val.c_str(), aggMeta.col.len);
                        break;
                }
                break;
        }
        // 根据聚合类型计算结果长度
         switch (aggMeta.ag_type) {
             case ast::AggFuncType::COUNT:
             case ast::AggFuncType::COUNT_ALL:
                 offset += sizeof(int);
                 break;
             case ast::AggFuncType::AVG:
                 offset += sizeof(float);
                 break;
             case ast::AggFuncType::SUM:
             case ast::AggFuncType::MIN:
             case ast::AggFuncType::MAX:
                 offset += aggMeta.col.len;
                 break;
             default:
                 offset += aggMeta.col.len;
                 break;
         }
    }
}

bool AggregateExecutor::Check_Having_Conditions(const std::vector<Value>& key) {
    if (havingConds.empty()) {
        return true;
    }
    
    auto hRes = havingRes[key];
    
    for (size_t i = 0; i < havingConds.size(); i++) {
        const auto& cond = havingConds[i];
        if (!cond.is_agg) continue;
        
        ColType left_t = hRes[i].type;
        char* left = nullptr;
        ColType right_t = cond.rhs_val.type;
        char* right = nullptr;
        
        // 获取左值数据
        if (left_t == TYPE_INT) {
            left = (char*)&hRes[i].int_val;
        } else if (left_t == TYPE_FLOAT) {
            left = (char*)&hRes[i].float_val;
        } else if (left_t == TYPE_STRING) {
            left = (char*)hRes[i].str_val.c_str();
        }
        
        // 获取右值数据
        if (right_t == TYPE_INT) {
            right = (char*)&cond.rhs_val.int_val;
        } else if (right_t == TYPE_FLOAT) {
            right = (char*)&cond.rhs_val.float_val;
        } else if (right_t == TYPE_STRING) {
            right = (char*)cond.rhs_val.str_val.c_str();
        }
        
        int cmp = 0;
        if (left_t == TYPE_INT && right_t == TYPE_FLOAT) {
            float left_float = *(int*)left;
            cmp = ix_compare((char*)&left_float, right, TYPE_FLOAT, sizeof(float));
        } else if (left_t == TYPE_FLOAT && right_t == TYPE_INT) {
            float right_float = *(int*)right;
            cmp = ix_compare(left, (char*)&right_float, TYPE_FLOAT, sizeof(float));
        } else {
            cmp = ix_compare(left, right, right_t, hRes[i].get_len());
        }
        
        switch (cond.op) {
            case OP_EQ: if (cmp != 0) return false; break;
            case OP_NE: if (cmp == 0) return false; break;
            case OP_LT: if (cmp >= 0) return false; break;
            case OP_GT: if (cmp <= 0) return false; break;
            case OP_LE: if (cmp > 0) return false; break;
            case OP_GE: if (cmp < 0) return false; break;
            default: throw std::runtime_error("Unexpected op type");
        }
    }
    
    return true;
}

void AggregateExecutor::nextTuple() {
    do {
        resultIndex++;
    } while (resultIndex < resultKeys.size() && !havingConds.empty() && !Check_Having_Conditions(resultKeys[resultIndex]));
}

std::unique_ptr<RmRecord> AggregateExecutor::Next() {
    if (is_end()) {
        return nullptr;
    }
    
    if (count0) {
        auto res = std::make_unique<RmRecord>(sizeof(int));
        *(int*)res->data = 0;
        return res;
    }
    
    auto &key = resultKeys[resultIndex];
    auto &result = groupByResults[key];
    auto res = std::make_unique<RmRecord>(tupleLength);
    
    MergeKeyAndResult(key, result, *res);
    return res;
}

bool AggregateExecutor::is_end() const {
    return resultIndex >= resultKeys.size();
}

Rid &AggregateExecutor::rid() {
    return _abstract_rid;
}

const std::vector<ColMeta> &AggregateExecutor::cols() const {
    return this->colMetas;
}

size_t AggregateExecutor::tupleLen() const {
    return tupleLength;
}