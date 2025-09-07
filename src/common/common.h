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

#include <cassert>
#include <cstring>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <atomic>
#include <optional>
#include "defs.h"
#include "record/rm_defs.h"
#include "transaction/txn_defs.h"
#include "system/sm_meta.h"

// 前向声明
class Transaction;


struct TabCol {
    std::string tab_name;
    std::string col_name;

    friend bool operator<(const TabCol &x, const TabCol &y) {
        return std::make_pair(x.tab_name, x.col_name) < std::make_pair(y.tab_name, y.col_name);
    }
};

struct Value {
    ColType type;  // type of value
    union {
        int int_val;      // int value
        float float_val;  // float value
    };
    std::string str_val;  // string value

    std::shared_ptr<RmRecord> raw;  // raw record buffer

    Value() {
        type = TYPE_INT;
        int_val = 0;
    }

    Value(ColType type_, char *val, int len) {
        type = type_;
        switch (type) {
            case TYPE_INT:
                assert(len == sizeof(int));
                int_val = *(int *) (val);
                break;
            case TYPE_FLOAT:
                assert(len == sizeof(float));
                float_val = *(float *) (val);
                break;
            case TYPE_STRING:
                str_val = std::string(val, len);
                break;
            default:
                break;
        }
    }

    void set_int(int int_val_) {
        type = TYPE_INT;
        int_val = int_val_;
    }

    void set_float(float float_val_) {
        type = TYPE_FLOAT;
        float_val = float_val_;
    }

    void set_str(std::string str_val_) {
        type = TYPE_STRING;
        str_val = std::move(str_val_);
    }

    float get_float() {
        if (type == TYPE_INT) {
            return (float) int_val;
        }
        return float_val;
    }

    int get_int() {
        if (type == TYPE_FLOAT) {
            return (int) float_val;
        }
        return int_val;
    }

    size_t get_len() {
        if (type == TYPE_INT) {
            return sizeof(int);
        } else if (type == TYPE_FLOAT) {
            return sizeof(float);
        } else if (type == TYPE_STRING) {
            return str_val.size();
        }
        return 0;
    }

    char* get_data() {
        if (type == TYPE_INT) {
            return (char*)&int_val;
        } else if (type == TYPE_FLOAT) {
            return (char*)&float_val;
        } else if (type == TYPE_STRING) {
            return (char*)str_val.c_str();
        }
        return nullptr;
    }

    std::size_t hash() const {
        std::size_t seed = 0;
        switch (type) {
            case TYPE_INT:
                seed ^= std::hash<int>{}(int_val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                break;
            case TYPE_FLOAT:
                seed ^= std::hash<float>{}(float_val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                break;
            case TYPE_STRING:
                seed ^= std::hash<std::string>{}(str_val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
                break;
            default:
                break;
        }
        return seed;
    }

    void init_raw(int len) {
        assert(raw == nullptr);
        raw = std::make_shared<RmRecord>(len);
        if (type == TYPE_INT) {
            assert(len == sizeof(int));
            *(int *)(raw->data) = int_val;
        } else if (type == TYPE_FLOAT) {
            assert(len == sizeof(float));
            *(float *)(raw->data) = float_val;
        } else if (type == TYPE_STRING) {
            if (len < (int)str_val.size()) {
                throw StringOverflowError();
            }
            memset(raw->data, 0, len);
            memcpy(raw->data, str_val.c_str(), str_val.size());
        }
    }

    bool operator==(const Value& other) const {
        if (type != other.type) {
            return false;
        }
        switch (type) {
            case TYPE_INT:
                return int_val == other.int_val;
            case TYPE_FLOAT:
                return float_val == other.float_val;
            case TYPE_STRING:
                return str_val == other.str_val;
            default:
                return false;
        }
    }

    bool operator<(const Value& other) const {
        if (type != other.type) {
            return type < other.type;
        }
        switch (type) {
            case TYPE_INT:
                return int_val < other.int_val;
            case TYPE_FLOAT:
                return float_val < other.float_val;
            case TYPE_STRING:
                return str_val < other.str_val;
            default:
                return false;
        }
    }
};

enum CompOp { OP_EQ, OP_NE, OP_LT, OP_GT, OP_LE, OP_GE };

struct Condition {
    TabCol lhs_col;   // left-hand side column
    CompOp op;        // comparison operator
    bool is_rhs_val;  // true if right-hand side is a value (not a column)
    TabCol rhs_col;   // right-hand side column
    Value rhs_val;    // right-hand side value
};

// 表达式操作类型
enum ExprOp
{
    EXPR_CONST,
    EXPR_COL,
    EXPR_ADD,
    EXPR_SUB,
    EXPR_MUL,
    EXPR_DIV
};

// 表达式节点
struct ExprNode
{
    ExprOp op_;
    Value val_;                       // 常量值
    TabCol col_;                      // 列引用
    std::shared_ptr<ExprNode> left_;  // 左子表达式
    std::shared_ptr<ExprNode> right_; // 右子表达式

    ExprNode(ExprOp op) : op_(op), left_(nullptr), right_(nullptr) {}

    // 创建常量表达式
    static std::shared_ptr<ExprNode> make_const(const Value &val)
    {
        auto node = std::make_shared<ExprNode>(EXPR_CONST);
        node->val_ = val;
        return node;
    }

    // 创建列引用表达式
    static std::shared_ptr<ExprNode> make_col(const TabCol &col)
    {
        auto node = std::make_shared<ExprNode>(EXPR_COL);
        node->col_ = col;
        return node;
    }

    // 创建二元运算表达式
    static std::shared_ptr<ExprNode> make_binary(ExprOp op,
                                                 std::shared_ptr<ExprNode> left,
                                                 std::shared_ptr<ExprNode> right)
    {
        auto node = std::make_shared<ExprNode>(op);
        node->left_ = left;
        node->right_ = right;
        return node;
    }
};

struct SetClause {
    TabCol lhs;
    Value rhs;                           // 简单赋值
    std::shared_ptr<ExprNode> rhs_expr_; // 表达式赋值
    bool is_expr_;                       // 是否为表达式
    bool is_add;                         // 兼容原有代码

    SetClause() : is_expr_(false), is_add(false) {}

    // 简单赋值构造函数
    SetClause(const TabCol &lhs_col, const Value &rhs_val)
        : lhs(lhs_col), rhs(rhs_val), is_expr_(false), is_add(false) {}

    // 表达式赋值构造函数
    SetClause(const TabCol &lhs_col, std::shared_ptr<ExprNode> expr)
        : lhs(lhs_col), rhs_expr_(expr), is_expr_(true), is_add(false) {}

    // 简单赋值构造函数（带is_add参数）
    SetClause(const TabCol &lhs_col, const Value &rhs_val, bool is_add_flag)
        : lhs(lhs_col), rhs(rhs_val), is_expr_(false), is_add(is_add_flag) {}
};

//-------------------------- MVCC-------------------------------------
//版本链拉出来方便管理
struct UndoLog
{
    // 操作类型：INSERT/DELETE/UPDATE，标识创建此版本的操作
    WType type_;                                     
    // 创建此版本的事务时间戳，用于版本可见性判断
    timestamp_t ts_;                  
    // 创建此版本的事务ID，用于事务隔离和冲突检测               
    txn_id_t txn_id_;                               
    // 记录位置标识符，指向磁盘上的物理位置
    Rid rid_;                                    
    // 版本值（使用智能指针管理内存），存储记录的实际数据
    std::shared_ptr<RmRecord> value_;             
    // 指向前一个版本的UndoLog，形成版本链
    std::shared_ptr<UndoLog> prev_version_;        

    UndoLog(WType type, timestamp_t ts, txn_id_t txn_id, const Rid &rid, const RmRecord &val)
        : type_(type), ts_(ts), txn_id_(txn_id), rid_(rid), prev_version_(nullptr) 
    {
        // 使用智能指针管理RmRecord的内存，确保自动释放和共享所有权
        value_ = std::make_shared<RmRecord>(val);
    }
    UndoLog() = default;

 

   //获取记录值的常量引用
    const RmRecord& get_value() const { return *value_; }
    RmRecord& get_value() { return *value_; }
};



class VersionChain
{
public:

   //头指针，指向最新的版本
    std::shared_ptr<UndoLog> head_; 
    std::mutex chain_latch_;        

    VersionChain() : head_(nullptr) {}

 
     //向版本链头部插入新版本
    void insert_version(std::shared_ptr<UndoLog> new_version)
    {
        std::lock_guard<std::mutex> lock(chain_latch_);
        new_version->prev_version_ = head_;
        head_ = new_version;
    }

   
   //获取指定时间戳之前的所有版本日志
    std::vector<UndoLog> get_undo_logs_until_timestamp(timestamp_t read_ts, txn_id_t reader_txn_id)
    {
        std::lock_guard<std::mutex> lock(chain_latch_);
        std::vector<UndoLog> version_list;
        auto current_version = head_;

        bool is_conflict_detection_mode = (read_ts == UINT64_MAX);
        while (current_version != nullptr) {
            if (is_conflict_detection_mode) {
               
                version_list.push_back(*current_version);
            } else {
                if (current_version->txn_id_ == reader_txn_id) {
                    version_list.push_back(*current_version);
                } else {
                    version_list.push_back(*current_version);
                }
            }
            current_version = current_version->prev_version_;
        }

        return version_list;
    }

   
   //获取对指定事务可见的版本
    std::optional<UndoLog> get_visible_version(timestamp_t read_ts, txn_id_t reader_txn_id, const std::unordered_set<txn_id_t> &active_txns)
    {
        std::lock_guard<std::mutex> lock(chain_latch_);
        auto current_version = head_;
        while (current_version != nullptr)
        {
            // MVCC可见性规则：
            // 版本由已提交事务产生，且时间戳在快照范围内
            // 读取自己的修改
            if ((current_version->ts_ <= read_ts && !active_txns.count(current_version->txn_id_)) ||
                (current_version->txn_id_ == reader_txn_id))
            {
                return *current_version;
            }
            current_version = current_version->prev_version_;
        }
        // 没有找到可见版本
        return std::nullopt;
    }
};

//MVCC管理器
class MVCCManager
{
private:
    std::unordered_map<int64_t, std::shared_ptr<VersionChain>> version_chains_; 
    std::mutex manager_latch_;
    std::atomic<timestamp_t> last_committed_ts_{0}; 
    std::atomic<timestamp_t> next_ts_{1};          
    std::unordered_set<txn_id_t> active_txns_;      

public:
    //拿manager的实例
    static MVCCManager &get_instance()
    {
        static MVCCManager instance;
        return instance;
    }

  
    // 向指定记录的版本链中添加新版本
    void add_version(const Rid &rid, int fd, std::shared_ptr<UndoLog> undo_log);
    
  
   // 获取指定记录在给定时间戳之前的所有版本日志
    std::vector<UndoLog> get_undo_logs(const Rid &rid, int fd, timestamp_t read_ts, txn_id_t reader_txn_id);
    

    //更新最后提交的事务时间戳

    void update_last_committed_ts(timestamp_t commit_timestamp);
    
 
    // 获取最后提交的事务时间戳
    timestamp_t get_last_committed_ts();
    
 
    // 获取下一个可用的全局时间戳
    timestamp_t get_next_timestamp();
    
    //为事务分配提交时间戳
    void assign_commit_timestamp(txn_id_t txn_id, timestamp_t commit_timestamp);
    

    // 将事务add到活跃事务集合
    void add_active_txn(txn_id_t txn_id);
    
    void rollback_transaction(txn_id_t txn_id);                          
    std::vector<std::pair<Rid, int>> get_inserted_rids(txn_id_t txn_id); 
    void cleanup_version_chains(timestamp_t current_ts); 
   //活跃事务集合中remove事务
    void remove_active_txn(txn_id_t txn_id);
    bool is_txn_active(txn_id_t txn_id);


    std::unordered_set<txn_id_t> get_active_txns(); 
    std::shared_ptr<VersionChain> get_version_chain(const Rid &rid, int fd); 
               

private:

   //将记录标识符和文件描述符转换为唯一的64位键值
    int64_t rid_to_key(const Rid &rid, int fd)
    {
        return (static_cast<int64_t>(fd) << 48) |
               (static_cast<int64_t>(rid.page_no) << 16) |
               static_cast<int64_t>(rid.slot_no);
    }
};


// 根据基础元组和撤销日志重构指定版本的元组
auto ReconstructTuple(const TabMeta *schema, const RmRecord &base_tuple, const TupleMeta &base_meta,
                      const std::vector<UndoLog> &undo_logs) -> std::optional<RmRecord>;

//计算值
Value evaluate_expr(const ExprNode *expr, const RmRecord *record, const std::vector<ColMeta> &cols);
//检测是写写冲突
auto IsWriteWriteConflict(timestamp_t tuple_ts, Transaction *txn) -> bool;


