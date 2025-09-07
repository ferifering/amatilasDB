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

#include <utility>
#include <unordered_map>
#include <map>
#include "execution_defs.h"
#include "execution_common.h"
#include "executor_abstract.h"
#include "index/ix.h"
#include "system/sm.h"
#include "parser/ast.h"

// 聚合值结构体，继承自Value
struct AggValue : public Value {
    bool minFlag = false;
    bool maxFlag = false;
    int count = 0; // 用于AVG和COUNT计算
    std::string colName;

    AggValue() : Value() {
        type = TYPE_INT;
        int_val = 0;
        minFlag = false;
        maxFlag = false;
        count = 0;
    }

    AggValue(const ColType& t, const char* val, int len) : Value(t, (char*)val, len) {
        minFlag = false;
        maxFlag = false;
        count = 0;
    }
};

// 聚合列元数据
struct AggColMeta {
    ast::AggFuncType ag_type;
    ColMeta col;
    std::string alias;  // 别名
};

// Having条件
struct HavingCondition {
    bool is_agg = false;
    ast::AggFuncType ag_type;
    TabCol lhs_col;
    CompOp op;
    Value rhs_val;
    AggColMeta agg_col;
};

// 聚合列定义
struct AggCol {
    ast::AggFuncType AggType;
    TabCol col;
    std::string alias;
};

// Vector Value哈希器
struct VectorValueHasher {
    std::size_t operator()(const std::vector<Value>& vec) const {
        std::size_t seed = vec.size();
        for(const auto& val : vec) {
            switch(val.type) {
            case TYPE_INT:
                seed ^= std::hash<int>{}(val.int_val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                break;
            case TYPE_FLOAT:
                seed ^= std::hash<float>{}(val.float_val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                break;
            case TYPE_STRING:
                seed ^= std::hash<std::string>{}(val.str_val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                break;
            default:
                break;
            }
        }
        return seed;
    }
};

class AggregateExecutor : public AbstractExecutor {
private:
    std::unique_ptr<AbstractExecutor> prev_;  // 上游执行器
    std::vector<AggColMeta> aggMetas;         // 聚合函数列表
    std::vector<AggColMeta> groupByCols;      // 分组字段列表
    std::vector<HavingCondition> havingConds; // having条件列表
    std::unordered_map<std::vector<Value>, std::vector<AggValue>, VectorValueHasher> groupByResults; // 分组聚合结果
    std::unordered_map<std::vector<Value>, std::vector<AggValue>, VectorValueHasher> havingRes; // having结果
    std::vector<ColMeta> colMetas;            // 输出列元数据
    std::map<std::string, char*> dataMap;     // 字段名和数据的映射
    size_t tupleLength;                       // 每个元组的长度
    std::vector<std::vector<Value>> resultKeys; // 聚合结果的键列表
    size_t resultIndex;                       // 当前结果索引
    Context* context;                         // 上下文信息
    bool count0 = false;                      // 是否为空结果集
    SmManager* sm_manager_;
    Rid _abstract_rid;
    
    // 辅助函数
    void GetCol(AggColMeta &aggMeta, ColMeta &colMeta);
    size_t GetTupleLen(const std::vector<AggColMeta> &aggMetas, const std::vector<AggColMeta> &groupByCols);
    void ProcessTuple(std::unique_ptr<RmRecord> &tuple);
    void CalculateAggValue(const AggColMeta &aggMeta, AggValue& res);
    void MergeKeyAndResult(const std::vector<Value>& key, const std::vector<AggValue>& result, RmRecord& record);
    bool Check_Having_Conditions(const std::vector<Value>& key);
    
public:
    AggregateExecutor(std::unique_ptr<AbstractExecutor> prev,
                     std::vector<std::shared_ptr<ast::Col>> agg_exprs,
                     std::vector<std::shared_ptr<ast::Col>> group_by_cols,
                     std::vector<std::shared_ptr<ast::BinaryExpr>> having_conds,
                     std::vector<std::shared_ptr<ast::Col>> select_cols,
                     bool has_group_by,
                     bool has_having);

    void beginTuple() override;
    void nextTuple() override;
    std::unique_ptr<RmRecord> Next() override;
    bool is_end() const override;
    Rid &rid() override;
    const std::vector<ColMeta> &cols() const override;
    size_t tupleLen() const override;
    std::string getType() override { return "AggregateExecutor"; }
};