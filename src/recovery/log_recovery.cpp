/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL
v2. You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERLY FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "log_recovery.h"

// 日志读取器类 - 统一管理日志文件读取和解析
class LogReader {
public:
  explicit LogReader(DiskManager *disk_manager) : disk_manager_(disk_manager) {
    load_log_data();
  }

  // 遍历所有日志记录
  template <typename Func> void for_each_log(Func &&func) {
    uint32_t offset = 0;
    while (offset < log_data_size_) {
      if (!validate_log_at_offset(offset)) {
        // 如果遇到无效的日志记录，停止遍历以避免数据损坏
        break;
      }

      uint32_t log_len = get_log_length(offset);
      LogType log_type = get_log_type(offset);

      func(log_data_.data() + offset, log_type, offset);
      // 防止无限循环：确保offset有进展
      if (log_len == 0) {
        break;
      }
      offset += log_len;
    }
  }

  // 从指定LSN开始遍历
  template <typename Func>
  void for_each_log_from_lsn(lsn_t start_lsn, Func &&func) {
    uint32_t offset = 0;
    bool found_start = false;

    while (offset < log_data_size_) {
      if (!validate_log_at_offset(offset)) {
        // 如果遇到无效的日志记录，停止遍历以避免数据损坏
        break;
      }

      lsn_t current_lsn = get_lsn(offset);
      if (!found_start && current_lsn >= start_lsn) {
        found_start = true;
      }

      if (found_start) {
        LogType log_type = get_log_type(offset);
        func(log_data_.data() + offset, log_type, offset);
      }

      uint32_t log_len = get_log_length(offset);
      // 防止无限循环：确保offset有进展
      if (log_len == 0) {
        break;
      }
      offset += log_len;
    }
  }

private:
  DiskManager *disk_manager_;
  std::vector<char> log_data_;
  uint32_t log_data_size_ = 0;

  void load_log_data() {
    int log_file_size = disk_manager_->get_file_size(LOG_FILE_NAME);
    if (log_file_size <= 0)
      return;

    const int CHUNK_SIZE = 64 * 1024;
    std::unique_ptr<char[]> chunk_buffer(new char[CHUNK_SIZE]);

    uint32_t bytes_read = 0;
    while (bytes_read < static_cast<uint32_t>(log_file_size)) {
      uint32_t chunk_size =
          std::min(static_cast<uint32_t>(CHUNK_SIZE),
                   static_cast<uint32_t>(log_file_size) - bytes_read);
      disk_manager_->read_log(chunk_buffer.get(), chunk_size, bytes_read);
      log_data_.insert(log_data_.end(), chunk_buffer.get(),
                       chunk_buffer.get() + chunk_size);
      bytes_read += chunk_size;
    }
    log_data_size_ = static_cast<uint32_t>(log_data_.size());
  }

  bool validate_log_at_offset(uint32_t offset) {
    // 检查是否有足够的空间读取日志头
    if (offset + LOG_HEADER_SIZE > log_data_size_)
      return false;

    uint32_t log_len = get_log_length(offset);

    // 检查日志长度的合理性
    if (log_len < LOG_HEADER_SIZE || log_len > log_data_size_)
      return false;

    // 检查完整的日志记录是否在数据范围内
    if (offset + log_len > log_data_size_)
      return false;

    // 检查日志类型是否有效
    LogType log_type = get_log_type(offset);
    if (log_type < LogType::UPDATE || log_type > LogType::DROP_INDEX)
      return false;

    return true;
  }

  uint32_t get_log_length(uint32_t offset) {
    return *reinterpret_cast<uint32_t *>(log_data_.data() + offset +
                                         OFFSET_LOG_TOT_LEN);
  }

  LogType get_log_type(uint32_t offset) {
    return *reinterpret_cast<LogType *>(log_data_.data() + offset +
                                        OFFSET_LOG_TYPE);
  }

  lsn_t get_lsn(uint32_t offset) {
    return *reinterpret_cast<lsn_t *>(log_data_.data() + offset + OFFSET_LSN);
  }

  txn_id_t get_txn_id(uint32_t offset) {
    return *reinterpret_cast<txn_id_t *>(log_data_.data() + offset +
                                         OFFSET_LOG_TID);
  }
};

// 表操作辅助类 - 统一管理表和文件句柄验证
class TableOperationHelper {
public:
  explicit TableOperationHelper(SmManager *sm_manager)
      : sm_manager_(sm_manager) {}

  RmFileHandle *get_file_handle(const std::string &table_name) {
    if (!sm_manager_->db_.is_table(table_name))
      return nullptr;

    auto fh_iter = sm_manager_->fhs_.find(table_name);
    if (fh_iter == sm_manager_->fhs_.end() || fh_iter->second == nullptr) {
      return nullptr;
    }
    return fh_iter->second.get();
  }

private:
  SmManager *sm_manager_;
};

/**
 * @description: Analyze log, initialize active transaction table and dirty page
 * table
 */
void RecoveryManager::analyze() {
  // 清空之前的状态
  lsn_mapping_.clear();
  active_txn_.clear();
  committed_txns_.clear();

  LogReader reader(disk_manager_);
  reader.for_each_log(
      [this](char *log_data, LogType log_type, uint32_t offset) {
        this->process_log_for_analyze(log_data, log_type);
      });
}

// 处理单个日志记录用于分析阶段
void RecoveryManager::process_log_for_analyze(char *log_data,
                                              LogType log_type) {
  if (log_data == nullptr) {
    printf("Error: log_data is null in process_log_for_analyze\n");
    return;
  }

  try {
    switch (log_type) {
    case LogType::BEGIN: {
      BeginLogRecord record(INVALID_TXN_ID);
      record.deserialize(log_data);
      active_txn_.insert(record.log_tid_);
      lsn_mapping_[record.lsn_] = record.log_tid_;
      break;
    }
    case LogType::COMMIT: {
      CommitLogRecord record(INVALID_TXN_ID);
      record.deserialize(log_data);
      active_txn_.erase(record.log_tid_);
      committed_txns_.insert(record.log_tid_);
      lsn_mapping_[record.lsn_] = record.log_tid_;
      break;
    }
    case LogType::ABORT: {
      AbortLogRecord record(INVALID_TXN_ID);
      record.deserialize(log_data);
      active_txn_.erase(record.log_tid_);
      lsn_mapping_[record.lsn_] = record.log_tid_;
      break;
    }
    case LogType::STATIC_CHECKPOINT: {
      StaticCheckpointLogRecord record;
      record.deserialize(log_data);
      // 静态检查点：清空所有状态，保留检查点映射
      // 静态检查点表示之前的所有事务都已经持久化到磁盘
      active_txn_.clear();
      committed_txns_.clear();
      dirty_page_table_.clear();
      lsn_mapping_.clear();
      lsn_mapping_[record.lsn_] = record.log_tid_;
      // 记录检查点LSN用于后续处理
      checkpoint_start_lsn_ = record.lsn_;
      break;
    }
    case LogType::INSERT:
    case LogType::DELETE:
    case LogType::UPDATE:
    case LogType::CREATE_INDEX:
    case LogType::DROP_INDEX: {
      // 对于数据操作，只需要记录LSN映射
      lsn_t lsn = *reinterpret_cast<lsn_t *>(log_data + OFFSET_LSN);
      txn_id_t txn_id =
          *reinterpret_cast<txn_id_t *>(log_data + OFFSET_LOG_TID);
      lsn_mapping_[lsn] = txn_id;
      break;
    }
    }
  } catch (const std::exception &e) {
    printf("Error in process_log_for_analyze: %s\n", e.what());
  }
}

/**
 * @description: Redo insert operation
 */
void RecoveryManager::redo_insert(InsertLogRecord *log_record) {
  TableOperationHelper helper(sm_manager_);
  RmFileHandle *file_handle = helper.get_file_handle(log_record->table_name_);
  if (!file_handle)
    return;

  try {
    if (!file_handle->is_record(log_record->rid_)) {
      file_handle->insert_record(log_record->rid_,
                                 log_record->insert_value_.data);
      is_need_redo_indexes_ = true;
    }
  } catch (const std::exception &e) {
    printf("Error in redo_insert: %s\n", e.what());
  }
}

/**
 * @description: Redo delete operation
 */
void RecoveryManager::redo_delete(DeleteLogRecord *log_record) {
  TableOperationHelper helper(sm_manager_);
  RmFileHandle *file_handle = helper.get_file_handle(log_record->table_name_);
  if (!file_handle)
    return;

  try {
    if (file_handle->is_record(log_record->rid_)) {
      file_handle->delete_record(log_record->rid_, nullptr);
      is_need_redo_indexes_ = true;
    }
  } catch (const std::exception &e) {
    printf("Error in redo_delete: %s\n", e.what());
  }
}

/**
 * @description: Redo update operation
 */
void RecoveryManager::redo_update(UpdateLogRecord *log_record) {
  TableOperationHelper helper(sm_manager_);
  RmFileHandle *file_handle = helper.get_file_handle(log_record->table_name_);
  if (!file_handle)
    return;

  try {
    if (file_handle->is_record(log_record->rid_)) {
      file_handle->update_record(log_record->rid_, log_record->new_value_.data,
                                 nullptr);
      is_need_redo_indexes_ = true;
    }
  } catch (const std::exception &e) {
    printf("Error in redo_update: %s\n", e.what());
  }
}

/**
 * @description: Undo insert operation (delete record)
 */
void RecoveryManager::undo_insert(InsertLogRecord *log_record) {
  TableOperationHelper helper(sm_manager_);
  RmFileHandle *file_handle = helper.get_file_handle(log_record->table_name_);
  if (!file_handle)
    return;

  try {
    if (file_handle->is_record(log_record->rid_)) {
      file_handle->delete_record(log_record->rid_, nullptr);
      is_need_redo_indexes_ = true;
    }
  } catch (const std::exception &e) {
    printf("Error in undo_insert: %s\n", e.what());
  }
}

/**
 * @description: Undo delete operation (reinsert record)
 */
void RecoveryManager::undo_delete(DeleteLogRecord *log_record) {
  TableOperationHelper helper(sm_manager_);
  RmFileHandle *file_handle = helper.get_file_handle(log_record->table_name_);
  if (!file_handle)
    return;

  try {
    if (!file_handle->is_record(log_record->rid_)) {
      file_handle->insert_record(log_record->rid_,
                                 log_record->delete_value_.data);
      is_need_redo_indexes_ = true;
    }
  } catch (const std::exception &e) {
    printf("Error in undo_delete: %s\n", e.what());
  }
}

/**
 * @description: Undo update operation (restore to old value)
 */
void RecoveryManager::undo_update(UpdateLogRecord *log_record) {
  TableOperationHelper helper(sm_manager_);
  RmFileHandle *file_handle = helper.get_file_handle(log_record->table_name_);
  if (!file_handle)
    return;

  try {
    if (file_handle->is_record(log_record->rid_)) {
      file_handle->update_record(log_record->rid_, log_record->old_value_.data,
                                 nullptr);
      is_need_redo_indexes_ = true;
    }
  } catch (const std::exception &e) {
    printf("Error in undo_update: %s\n", e.what());
  }
}

/**
 * @description: Redo all operations not persisted to disk - Memory optimized
 * version
 */
void RecoveryManager::redo() {
  LogReader reader(disk_manager_);

  reader.for_each_log([&](char *log_data, LogType log_type, uint32_t offset) {
    txn_id_t txn_id = *reinterpret_cast<txn_id_t *>(log_data + OFFSET_LOG_TID);

    if (committed_txns_.count(txn_id) > 0) {
      process_log_for_redo(log_data, log_type);
    }
  });
}

// 处理单个日志记录用于重做阶段
void RecoveryManager::process_log_for_redo(char *log_data, LogType log_type) {
  if (log_data == nullptr) {
    printf("Error: log_data is null in process_log_for_redo\n");
    return;
  }

  try {
    switch (log_type) {
    case LogType::INSERT: {
      std::unique_ptr<InsertLogRecord> insert_record(new InsertLogRecord());
      insert_record->deserialize(log_data);
      redo_insert(insert_record.get());
      break;
    }
    case LogType::DELETE: {
      std::unique_ptr<DeleteLogRecord> delete_record(new DeleteLogRecord());
      delete_record->deserialize(log_data);
      redo_delete(delete_record.get());
      break;
    }
    case LogType::UPDATE: {
      std::unique_ptr<UpdateLogRecord> update_record(new UpdateLogRecord());
      update_record->deserialize(log_data);
      redo_update(update_record.get());
      break;
    }
    case LogType::CREATE_INDEX: {
      std::unique_ptr<CreateIndexLogRecord> create_index_record(
          new CreateIndexLogRecord());
      create_index_record->deserialize(log_data);
      
      // 在redo阶段重新执行创建索引操作
      try {
        // 检查表是否存在
        if (sm_manager_->db_.is_table(create_index_record->table_name_)) {
          // 检查索引是否已经存在，如果不存在则创建
          if (!sm_manager_->get_ix_manager()->exists(create_index_record->table_name_,
                                                     create_index_record->col_names_)) {
            auto &table_meta = sm_manager_->db_.get_table(create_index_record->table_name_);
            std::vector<ColMeta> index_cols;
            
            // 获取列元数据
            for (const std::string &col_name : create_index_record->col_names_) {
              auto col_iter = const_cast<TabMeta &>(table_meta).get_col(col_name);
              index_cols.emplace_back(*col_iter);
            }
            
            // 创建索引文件
            sm_manager_->get_ix_manager()->create_index(create_index_record->table_name_, index_cols);
            
            // 将索引信息添加到表元数据中
            IndexMeta index_meta;
            index_meta.tab_name = create_index_record->table_name_;
            index_meta.cols = index_cols;
            index_meta.col_num = static_cast<int>(index_cols.size());
            
            // 计算索引键的总长度
            int col_tot_len = 0;
            for (const ColMeta &col : index_cols) {
              col_tot_len += col.len;
            }
            index_meta.col_tot_len = col_tot_len;
            
            const_cast<TabMeta &>(table_meta).indexes.emplace_back(index_meta);
            
            // 标记需要重建索引数据
            is_need_redo_indexes_ = true;
          }
        }
      } catch (const std::exception &e) {
        printf("Error redoing CREATE_INDEX for table %s: %s\n",
               create_index_record->table_name_.c_str(), e.what());
      }
      break;
    }
    case LogType::DROP_INDEX: {
      std::unique_ptr<DropIndexLogRecord> drop_index_record(
          new DropIndexLogRecord());
      drop_index_record->deserialize(log_data);
      // 在redo阶段重新执行删除索引操作
      try {
        if (sm_manager_->get_ix_manager()->exists(drop_index_record->table_name_,
                                                  drop_index_record->col_names_)) {
          std::string index_name = sm_manager_->get_ix_manager()->get_index_name(
              drop_index_record->table_name_, drop_index_record->col_names_);
          
          // 检查索引是否在ihs_中打开，如果是则先关闭
          auto ihs_iter = sm_manager_->ihs_.find(index_name);
          if (ihs_iter != sm_manager_->ihs_.end()) {
            IxIndexHandle *index = ihs_iter->second.get();
            // 清除缓存
            for (int i = 0; i < index->get_pages_num(); i++) {
              PageId page_id = {index->get_fd(), i};
              while (buffer_pool_manager_->unpin_page(page_id, true))
                ;
              buffer_pool_manager_->delete_page(page_id);
            }
            sm_manager_->get_ix_manager()->close_index(index);
            sm_manager_->ihs_.erase(ihs_iter);
          }
          
          // 删除索引文件
          sm_manager_->get_ix_manager()->destroy_index(drop_index_record->table_name_,
                                                       drop_index_record->col_names_);
          
          // 从表元数据中移除索引信息
          if (sm_manager_->db_.is_table(drop_index_record->table_name_)) {
            TabMeta &tab = sm_manager_->db_.get_table(drop_index_record->table_name_);
            auto index_iter = std::find_if(
                tab.indexes.begin(), tab.indexes.end(), [&](const IndexMeta &idx) {
                  if (idx.cols.size() != drop_index_record->col_names_.size())
                    return false;
                  for (size_t i = 0; i < idx.cols.size(); i++) {
                    if (idx.cols[i].name != drop_index_record->col_names_[i])
                      return false;
                  }
                  return true;
                });
            if (index_iter != tab.indexes.end()) {
              tab.indexes.erase(index_iter);
            }
          }
        }
      } catch (const std::exception &e) {
        printf("Error redoing DROP_INDEX for table %s: %s\n",
               drop_index_record->table_name_.c_str(), e.what());
      }
      break;
    }
    case LogType::BEGIN:
    case LogType::COMMIT:
    case LogType::ABORT:
    case LogType::STATIC_CHECKPOINT:
      // 这些日志类型在redo阶段不需要处理
      break;
    default:
      printf("Warning: Unknown log type %d in process_log_for_redo\n",
             static_cast<int>(log_type));
      break;
    }
  } catch (const std::exception &e) {
    printf("Error in process_log_for_redo: %s\n", e.what());
  }
}

/**
 * @description: Rollback incomplete transactions - Memory optimized version
 */
void RecoveryManager::undo() {
  if (active_txn_.empty()) {
    return;
  }

  LogReader reader(disk_manager_);

  // 收集未完成事务的所有操作记录
  std::vector<std::unique_ptr<LogRecord>> undo_records;

  reader.for_each_log([&](char *log_data, LogType log_type, uint32_t offset) {
    txn_id_t txn_id = *reinterpret_cast<txn_id_t *>(log_data + OFFSET_LOG_TID);

    if (active_txn_.count(txn_id) > 0) {
      switch (log_type) {
      case LogType::INSERT: {
        std::unique_ptr<InsertLogRecord> insert_record(new InsertLogRecord());
        insert_record->deserialize(log_data);
        undo_records.push_back(std::move(insert_record));
        break;
      }
      case LogType::DELETE: {
        std::unique_ptr<DeleteLogRecord> delete_record(new DeleteLogRecord());
        delete_record->deserialize(log_data);
        undo_records.push_back(std::move(delete_record));
        break;
      }
      case LogType::UPDATE: {
        std::unique_ptr<UpdateLogRecord> update_record(new UpdateLogRecord());
        update_record->deserialize(log_data);
        undo_records.push_back(std::move(update_record));
        break;
      }
      case LogType::CREATE_INDEX: {
        std::unique_ptr<CreateIndexLogRecord> create_index_record(
            new CreateIndexLogRecord());
        create_index_record->deserialize(log_data);
        undo_records.push_back(std::move(create_index_record));
        break;
      }
      case LogType::DROP_INDEX: {
        std::unique_ptr<DropIndexLogRecord> drop_index_record(
            new DropIndexLogRecord());
        drop_index_record->deserialize(log_data);
        undo_records.push_back(std::move(drop_index_record));
        break;
      }
      case LogType::BEGIN:
      case LogType::COMMIT:
      case LogType::ABORT:
      case LogType::STATIC_CHECKPOINT:
        // 这些日志类型不需要收集到撤销记录中
        break;
      }
    }
  });

  // 按LSN逆序回滚
  std::sort(
      undo_records.begin(), undo_records.end(),
      [](const std::unique_ptr<LogRecord> &a,
         const std::unique_ptr<LogRecord> &b) { return a->lsn_ > b->lsn_; });

  for (auto &record : undo_records) {
    switch (record->log_type_) {
    case LogType::INSERT: {
      InsertLogRecord *insert_record =
          static_cast<InsertLogRecord *>(record.get());
      undo_insert(insert_record);
      break;
    }
    case LogType::DELETE: {
      DeleteLogRecord *delete_record =
          static_cast<DeleteLogRecord *>(record.get());
      undo_delete(delete_record);
      break;
    }
    case LogType::UPDATE: {
      UpdateLogRecord *update_record =
          static_cast<UpdateLogRecord *>(record.get());
      undo_update(update_record);
      break;
    }
    case LogType::CREATE_INDEX: {
      CreateIndexLogRecord *create_index_record =
          static_cast<CreateIndexLogRecord *>(record.get());
      undo_create_index(create_index_record);
      break;
    }
    case LogType::DROP_INDEX: {
      DropIndexLogRecord *drop_index_record =
          static_cast<DropIndexLogRecord *>(record.get());
      undo_drop_index(drop_index_record);
      break;
    }
    case LogType::BEGIN:
    case LogType::COMMIT:
    case LogType::ABORT:
    case LogType::STATIC_CHECKPOINT:
      // 这些日志类型不需要撤销操作
      break;
    }
  }
}

/**
 * @description: 基于静态检查点的故障恢复
 */
void RecoveryManager::recover_from_checkpoint() {

  // 获取最后一个检查点的LSN
  lsn_t checkpoint_lsn = get_last_checkpoint_lsn();

  if (checkpoint_lsn == INVALID_LSN) {
    // 没有检查点，执行完整的恢复流程
    analyze();
    redo();
    undo();
  } else {
    // 从检查点开始恢复
    analyze_from_lsn(checkpoint_lsn);
    redo_from_lsn(checkpoint_lsn);
    undo();
  }

  // 只在需要时恢复索引
  if (is_need_redo_indexes_) {
    redo_indexes();
  }
}

/**
 * @description: 获取最后一个检查点的LSN
 * @return {lsn_t} 最后一个检查点的LSN，如果没有找到则返回INVALID_LSN
 */
lsn_t RecoveryManager::get_last_checkpoint_lsn() {
  LogReader reader(disk_manager_);
  lsn_t last_checkpoint_lsn = INVALID_LSN;

  reader.for_each_log([&](char *log_data, LogType log_type, uint32_t offset) {
    if (log_type == LogType::STATIC_CHECKPOINT) {
      StaticCheckpointLogRecord checkpoint_record;
      checkpoint_record.deserialize(log_data);
      last_checkpoint_lsn = checkpoint_record.lsn_;
    }
  });

  return last_checkpoint_lsn;
}

/**
 * @description: 从指定LSN开始分析日志
 * @param {lsn_t} start_lsn 开始分析的LSN
 */
void RecoveryManager::analyze_from_lsn(lsn_t start_lsn) {
  // 对于静态检查点恢复，需要先完整分析所有日志，然后从检查点开始处理
  // 这样可以正确识别所有已提交和活跃的事务

  // 首先完整分析所有日志以建立事务状态
  lsn_mapping_.clear();
  active_txn_.clear();
  committed_txns_.clear();

  LogReader reader(disk_manager_);
  reader.for_each_log(
      [this](char *log_data, LogType log_type, uint32_t offset) {
        this->process_log_for_analyze(log_data, log_type);
      });

  // 注意：静态检查点后的事务状态已经在process_log_for_analyze中正确处理
  // 静态检查点会清空所有状态，只保留检查点后的事务信息
}

/**
 * @description: 从指定LSN开始重做操作
 * @param {lsn_t} start_lsn 开始重做的LSN
 */
void RecoveryManager::redo_from_lsn(lsn_t start_lsn) {
  LogReader reader(disk_manager_);

  // 从指定LSN开始重做，只处理检查点之后的已提交事务操作
  reader.for_each_log_from_lsn(start_lsn, [&](char *log_data, LogType log_type,
                                              uint32_t offset) {
    // 获取当前日志的LSN和事务ID
    lsn_t current_lsn = *reinterpret_cast<lsn_t *>(log_data + OFFSET_LSN);
    txn_id_t txn_id = *reinterpret_cast<txn_id_t *>(log_data + OFFSET_LOG_TID);

    // 只处理检查点之后的日志记录
    if (current_lsn > start_lsn && committed_txns_.count(txn_id) > 0) {
      process_log_for_redo(log_data, log_type);
    }
  });
}

/**
 * @description: 重建所有表的索引
 */
void RecoveryManager::redo_indexes() {
  // 使用智能指针管理Context对象，避免内存泄漏
  std::unique_ptr<Context> context(
      new Context(nullptr, nullptr, &transaction_));

  try {
    for (auto &[table_name, _] : sm_manager_->fhs_) {
      auto &table_meta = sm_manager_->db_.get_table(table_name);
      for (auto &index_meta : table_meta.indexes) {
        // 为每个索引单独创建列名向量，避免状态污染
        std::vector<std::string> col_names;
        for (auto &col : index_meta.cols) {
          col_names.emplace_back(col.name);
        }
        
        // 生成正确的索引名称：表名 + "_" + 列名1 + "_" + 列名2 + ... + ".idx"
        std::string index_name = sm_manager_->get_ix_manager()->get_index_name(
            table_name, col_names);
            
        printf("Rebuilding index: %s for table: %s\n", index_name.c_str(), table_name.c_str());
        
        try {
          sm_manager_->redo_index(table_name, table_meta, col_names, index_name,
                                  context.get());
        } catch (const std::exception &e) {
          printf("Failed to rebuild index %s: %s\n", index_name.c_str(), e.what());
          // 继续处理其他索引
        }
      }
    }
  } catch (const std::exception &e) {
    printf("Error in redo_indexes: %s\n", e.what());
    // 智能指针会自动清理资源
  }
}

/**
 * @description: 撤销创建索引操作
 * @param {CreateIndexLogRecord*} log_record 创建索引日志记录
 */
void RecoveryManager::undo_create_index(CreateIndexLogRecord *log_record) {
  try {
    // 撤销创建索引操作：删除索引
    if (sm_manager_->get_ix_manager()->exists(log_record->table_name_,
                                              log_record->col_names_)) {
      std::string index_name = sm_manager_->get_ix_manager()->get_index_name(
          log_record->table_name_, log_record->col_names_);

      // 检查索引是否在ihs_中打开，如果是则先关闭
      auto ihs_iter = sm_manager_->ihs_.find(index_name);
      if (ihs_iter != sm_manager_->ihs_.end()) {
        IxIndexHandle *index = ihs_iter->second.get();
        // 清除缓存
        for (int i = 0; i < index->get_pages_num(); i++) {
          PageId page_id = {index->get_fd(), i};
          while (buffer_pool_manager_->unpin_page(page_id, true))
            ;
          buffer_pool_manager_->delete_page(page_id);
        }
        sm_manager_->get_ix_manager()->close_index(index);
        sm_manager_->ihs_.erase(ihs_iter);
      }

      // 删除索引文件
      sm_manager_->get_ix_manager()->destroy_index(log_record->table_name_,
                                                   log_record->col_names_);

      // 从表元数据中移除索引信息
      if (sm_manager_->db_.is_table(log_record->table_name_)) {
        TabMeta &tab = sm_manager_->db_.get_table(log_record->table_name_);
        auto index_iter = std::find_if(
            tab.indexes.begin(), tab.indexes.end(), [&](const IndexMeta &idx) {
              if (idx.cols.size() != log_record->col_names_.size())
                return false;
              for (size_t i = 0; i < idx.cols.size(); i++) {
                if (idx.cols[i].name != log_record->col_names_[i])
                  return false;
              }
              return true;
            });
        if (index_iter != tab.indexes.end()) {
          tab.indexes.erase(index_iter);
        }
      }
    }
  } catch (const std::exception &e) {
    printf("Error undoing CREATE_INDEX for table %s: %s\n",
           log_record->table_name_.c_str(), e.what());
  }
}

/**
 * @description: 撤销删除索引操作
 * @param {DropIndexLogRecord*} log_record 删除索引日志记录
 */
void RecoveryManager::undo_drop_index(DropIndexLogRecord *log_record) {

  try {
    // 撤销删除索引操作：重新创建索引
    // 检查表是否存在
    if (!sm_manager_->db_.is_table(log_record->table_name_)) {
      return;
    }

    // 检查索引是否已经存在
    if (sm_manager_->get_ix_manager()->exists(log_record->table_name_,
                                              log_record->col_names_)) {
      return;
    }

    // 重新创建索引
    auto &table_meta = sm_manager_->db_.get_table(log_record->table_name_);
    std::vector<ColMeta> index_cols;
    for (const std::string &col_name : log_record->col_names_) {
      auto col_iter = const_cast<TabMeta &>(table_meta).get_col(col_name);
      index_cols.emplace_back(*col_iter);
    }

    // 将索引信息添加回表的元数据中
    IndexMeta index_meta;
    index_meta.tab_name = log_record->table_name_;
    index_meta.cols = index_cols;
    index_meta.col_num = static_cast<int>(index_cols.size());
    
    // 计算索引键的总长度
    int col_tot_len = 0;
    for (const ColMeta &col : index_cols) {
      col_tot_len += col.len;
    }
    index_meta.col_tot_len = col_tot_len;
    
    const_cast<TabMeta &>(table_meta).indexes.emplace_back(index_meta);

    std::unique_ptr<Context> context(
        new Context(nullptr, nullptr, &transaction_));
    std::string index_name = sm_manager_->get_ix_manager()->get_index_name(
        log_record->table_name_, log_record->col_names_);
    sm_manager_->redo_index(log_record->table_name_, table_meta,
                            log_record->col_names_, index_name, context.get());
    // 智能指针会自动清理资源
  } catch (const std::exception &e) {
  }
}