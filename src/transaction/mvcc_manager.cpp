#include "common/common.h"
#include "system/sm_meta.h"
#include "transaction/transaction.h"
#include <algorithm>
#include <stdexcept>
#include <unordered_map>




//获取记录的undo日志列表
std::vector<UndoLog> MVCCManager::get_undo_logs(const Rid &rid, int fd,
                                                timestamp_t read_ts,
                                                txn_id_t reader_txn_id) {
  std::lock_guard<std::mutex> lock(manager_latch_);
  int64_t version_key = rid_to_key(rid, fd);

  // 检查是否存在该记录的版本链
  if (version_chains_.find(version_key) == version_chains_.end()) {
    return {};
  }

  // 从版本链中获取指定时间戳之前的undo日志
  return version_chains_[version_key]->get_undo_logs_until_timestamp(read_ts,
                                                             reader_txn_id);
}



 //为记录添加新的版本信息

void MVCCManager::add_version(const Rid &rid, int fd,
                              std::shared_ptr<UndoLog> undo_log) {
  std::lock_guard<std::mutex> lock(manager_latch_);
  int64_t version_key = rid_to_key(rid, fd);

  // 如果该记录的版本链不存在，则创建新的版本链
  if (version_chains_.find(version_key) == version_chains_.end()) {
    version_chains_[version_key] = std::make_shared<VersionChain>();
  }

  version_chains_[version_key]->insert_version(undo_log);
}

timestamp_t MVCCManager::get_next_timestamp() { return next_ts_.fetch_add(1); }

void MVCCManager::update_last_committed_ts(timestamp_t ts) {
  last_committed_ts_.store(ts);
}


void MVCCManager::assign_commit_timestamp(txn_id_t txn_id,
                                          timestamp_t commit_ts) {
  std::lock_guard<std::mutex> lock(manager_latch_);

  // 更新所有属于该事务的版本的时间戳，
  for (auto &[version_key, chain] : version_chains_) {
    std::lock_guard<std::mutex> chain_lock(chain->chain_latch_);
    auto version_node = chain->head_;
    while (version_node != nullptr) {
      if (version_node->txn_id_ == txn_id && version_node->ts_ == 0) {
        version_node->ts_ = commit_ts;
      }
      version_node = version_node->prev_version_;
    }
  }
}

std::shared_ptr<VersionChain> MVCCManager::get_version_chain(const Rid &rid,
                                                             int fd) {
  std::lock_guard<std::mutex> lock(manager_latch_);
  int64_t version_key = rid_to_key(rid, fd);

  if (version_chains_.find(version_key) == version_chains_.end()) {
    version_chains_[version_key] = std::make_shared<VersionChain>();
  }

  return version_chains_[version_key];
}

timestamp_t MVCCManager::get_last_committed_ts() {
  return last_committed_ts_.load();
}

bool MVCCManager::is_txn_active(txn_id_t txn_id) {
  std::lock_guard<std::mutex> lock(manager_latch_);
  return active_txns_.count(txn_id) > 0;
}


void MVCCManager::add_active_txn(txn_id_t txn_id) {
  std::lock_guard<std::mutex> lock(manager_latch_);
  active_txns_.insert(txn_id);
}

void MVCCManager::remove_active_txn(txn_id_t txn_id) {
  std::lock_guard<std::mutex> lock(manager_latch_);
  active_txns_.erase(txn_id);
}



std::unordered_set<txn_id_t> MVCCManager::get_active_txns() {
  std::lock_guard<std::mutex> lock(manager_latch_);
  return active_txns_;
}



std::vector<std::pair<Rid, int>>

MVCCManager::get_inserted_rids(txn_id_t txn_id) {
  std::lock_guard<std::mutex> lock(manager_latch_);
  std::vector<std::pair<Rid, int>> inserted_rids;

  for (auto &[version_key, chain] : version_chains_) {
    std::lock_guard<std::mutex> chain_lock(chain->chain_latch_);
    auto version_node = chain->head_;
    while (version_node != nullptr) {
      if (version_node->txn_id_ == txn_id && version_node->type_ == WType::INSERT_TUPLE) {
        // 从version_key中解析出file_descriptor和rid   
        int file_descriptor = static_cast<int>(version_key >> 48);
        int page_number = static_cast<int>((version_key >> 16) & 0xFFFFFFFF);
        int slot_number = static_cast<int>(version_key & 0xFFFF);
        Rid record_id{page_number, slot_number};
        inserted_rids.emplace_back(record_id, file_descriptor);
        break; // 每个version_key只需要添加一次
      }
      version_node = version_node->prev_version_;
    }
  }

  return inserted_rids;
}

void MVCCManager::rollback_transaction(txn_id_t txn_id) {
  std::lock_guard<std::mutex> lock(manager_latch_);

  // 移除所有属于该事务的版本
  for (auto &[version_key, chain] : version_chains_) {
    std::lock_guard<std::mutex> chain_lock(chain->chain_latch_);

    // 处理头节点
    while (chain->head_ != nullptr && chain->head_->txn_id_ == txn_id) {
      chain->head_ = chain->head_->prev_version_;
    }

    // 处理中间节点
    if (chain->head_ != nullptr) {
      auto version_node = chain->head_;
      while (version_node->prev_version_ != nullptr) {
        if (version_node->prev_version_->txn_id_ == txn_id) {
          version_node->prev_version_ = version_node->prev_version_->prev_version_;
        } else {
          version_node = version_node->prev_version_;
        }
      }
    }
  }
}

void MVCCManager::cleanup_version_chains(timestamp_t current_ts) {
  std::lock_guard<std::mutex> lock(manager_latch_);

  // 清理超过一定时间的旧版本
  timestamp_t cleanup_threshold = current_ts > 1000 ? current_ts - 1000 : 0;

  // 遍历所有版本链进行清理
  for (auto &[version_key, chain] : version_chains_) {
    std::lock_guard<std::mutex> chain_lock(chain->chain_latch_);

    
    if (chain->head_ == nullptr)
    // 跳过空版本链
      continue;

    // 保留头节点和一些最近的版本，清理旧版本
    auto version_node = chain->head_;
    int kept_versions_count = 0;
    // 最多保留10个版本
    const int max_keep_versions = 10; 

    while (version_node != nullptr && kept_versions_count < max_keep_versions) {
      // 检查版本是否在时间阈值内
      if (version_node->ts_ > cleanup_threshold) {
        version_node = version_node->prev_version_;
        kept_versions_count++;
      } else {
        break;
      }
    }

    // 通过设置prev_version_为nullptr来断开链接，让旧版本被垃圾回收
    if (version_node != nullptr && version_node->prev_version_ != nullptr) {
      version_node->prev_version_ = nullptr;
    }
  }
}

//---------------------------MVCC一些相关函数实现--------------------------------

auto IsWriteWriteConflict(timestamp_t tuple_ts, Transaction *txn) -> bool {
  
  //
  if (txn == nullptr) {
    return false;
  }

  //记录的时间戳大于事务的读时间戳，说明有写写冲突
  return tuple_ts > txn->get_start_ts();
}


auto ReconstructTuple(const TabMeta *schema, const RmRecord &base_tuple,
                      const TupleMeta &base_meta,
                      const std::vector<UndoLog> &undo_logs)
    -> std::optional<RmRecord> {
  // 如果没有undo日志，直接返回基础记录
  if (undo_logs.empty()) {
    return base_tuple;
  }

  // 从最新的undo日志开始重构
  RmRecord reconstructed_tuple = base_tuple;

  for (const auto &undo_log_entry : undo_logs) {
    if (undo_log_entry.type_ == WType::DELETE_TUPLE) {
      // 如果是删除操作，记录不可见
      return std::nullopt;
    } else if (undo_log_entry.type_ == WType::UPDATE_TUPLE) {
      // 如果是更新操作，使用undo日志中的旧值
      reconstructed_tuple = undo_log_entry.get_value();
      break; // 找到第一个可见版本即可
    } else if (undo_log_entry.type_ == WType::INSERT_TUPLE) {
      // 如果是插入操作，使用插入的值
      reconstructed_tuple = undo_log_entry.get_value();
      break;
    }
  }

  return reconstructed_tuple;
}



// 表达式计算实现-
Value evaluate_expr(const ExprNode *expr, const RmRecord *record,
                    const std::vector<ColMeta> &cols) {
  // 检查表达式是否为空
  if (expr == nullptr) {
    throw std::runtime_error("Null expression node encountered");
  }

  switch (expr->op_) {
  case EXPR_CONST:
    return expr->val_;

  case EXPR_COL: {
    // 列表达式从记录中提取指定列的值
    for (size_t col_index = 0; col_index < cols.size(); ++col_index) {
      if (cols[col_index].name == expr->col_.col_name) {
        // 从记录中提取值
        char *field_data = record->data + cols[col_index].offset;
        Value column_value;

        // 根据列的数据类型进行相应的值提取
        switch (cols[col_index].type) {
        case TYPE_INT:
          column_value.set_int(*(int *)field_data);
          break;
        case TYPE_FLOAT:
          column_value.set_float(*(float *)field_data);
          break;
        case TYPE_STRING:
          column_value.set_str(std::string(field_data, cols[col_index].len));
          // 移除字符串尾部的空字符，确保字符串格式正确
          column_value.str_val.erase(column_value.str_val.find('\0'));
          break;
        default:
          throw std::runtime_error("Unsupported data type for column");
        }
        return column_value;
      }
    }
    throw std::runtime_error(expr->col_.col_name + "' does not exist");
  }

  case EXPR_ADD:
  case EXPR_SUB:
  case EXPR_MUL:
  case EXPR_DIV: {
    Value left_operand = evaluate_expr(expr->left_.get(), record, cols);
    Value right_operand = evaluate_expr(expr->right_.get(), record, cols);

    // 类型转换和计算
    if (left_operand.type == TYPE_INT && right_operand.type == TYPE_INT) {
      // 两个操作数都是整数类型
      int calculation_result;
      switch (expr->op_) {
      case EXPR_ADD:
        calculation_result = left_operand.int_val + right_operand.int_val;
        break;
      case EXPR_SUB:
        calculation_result = left_operand.int_val - right_operand.int_val;
        break;
      case EXPR_MUL:
        calculation_result = left_operand.int_val * right_operand.int_val;
        break;
      case EXPR_DIV:
        if (right_operand.int_val == 0)
          throw std::runtime_error("Arithmetic : division by zero");
        calculation_result = left_operand.int_val / right_operand.int_val;
        break;
      default:
        throw std::runtime_error("Unsupported arithmetic operator");
      }
      Value result_value;
      result_value.set_int(calculation_result);
      return result_value;
    } else {
      // 转换为浮点数计算
      float left_float = (left_operand.type == TYPE_INT) ? static_cast<float>(left_operand.int_val)
                                             : left_operand.float_val;
      float right_float = (right_operand.type == TYPE_INT)
                          ? static_cast<float>(right_operand.int_val)
                          : right_operand.float_val;

      float float_calculation_result;
      switch (expr->op_) {
      case EXPR_ADD:
        float_calculation_result = left_float + right_float;
        break;
      case EXPR_SUB:
        float_calculation_result = left_float - right_float;
        break;
      case EXPR_MUL:
        float_calculation_result = left_float * right_float;
        break;
      case EXPR_DIV:
        // 检查错误
        if (right_float == 0.0f)
          throw std::runtime_error("Arithmetic: division by zero");
        float_calculation_result = left_float / right_float;
        break;
      default:
        throw std::runtime_error("Unsupported arithmetic operator");
      }
      Value result_value;
      result_value.set_float(float_calculation_result);
      return result_value;
    }
  }

  default:
    throw std::runtime_error("Unsupported expression type");
  }
}