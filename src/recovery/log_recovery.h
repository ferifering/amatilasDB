/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL
v2. You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#pragma once

#include "log_manager.h"
#include "storage/disk_manager.h"
#include "system/sm_manager.h"
#include "record/rm_scan.h"
#include "transaction/transaction_manager.h"
#include <set>
#include <unordered_map>
#include <map>

class RedoLogsInPage {
public:
  RedoLogsInPage() { table_file_ = nullptr; }
  RmFileHandle *table_file_;
  std::vector<lsn_t> redo_logs_; // 在该page上需要redo的操作的lsn
};

class RecoveryManager {
public:
  RecoveryManager(DiskManager *disk_manager,
                  BufferPoolManager *buffer_pool_manager, SmManager *sm_manager,
                  LogManager *log_manager, TransactionManager *transaction_manager) 
    : disk_manager_(disk_manager), buffer_pool_manager_(buffer_pool_manager), 
      sm_manager_(sm_manager), log_manager_(log_manager), 
      transaction_manager_(transaction_manager), transaction_(666) {
  }

  void analyze();
  void redo();
  void undo();

  // 基于静态检查点的恢复
  void recover_from_checkpoint();

  // 获取最后一个检查点的LSN
  lsn_t get_last_checkpoint_lsn();

private:
  // Redo operations
  void redo_insert(InsertLogRecord *log_record);
  void redo_delete(DeleteLogRecord *log_record);
  void redo_update(UpdateLogRecord *log_record);

  // Undo operations
  void undo_insert(InsertLogRecord *log_record);
  void undo_delete(DeleteLogRecord *log_record);
  void undo_update(UpdateLogRecord *log_record);
  void undo_create_index(CreateIndexLogRecord *log_record);
  void undo_drop_index(DropIndexLogRecord *log_record);

private:
  LogBuffer buffer_;                       // 读入日志
  DiskManager *disk_manager_;              // 用来读写文件
  BufferPoolManager *buffer_pool_manager_; // 对页面进行读写
  SmManager *sm_manager_;                  // 访问数据库元数据
  LogManager *log_manager_;                // 日志管理器
  TransactionManager *transaction_manager_; // 事务管理器

  std::unordered_map<txn_id_t, lsn_t> lsn_mapping_; // 事务ID到最后LSN的映射
  std::set<txn_id_t> active_txn_;                   // 活跃事务集合
  std::set<txn_id_t> committed_txns_;               // 已提交事务集合
  std::map<PageId, lsn_t> dirty_page_table_;        // 脏页表

  // 基于静态检查点的恢复相关
  lsn_t checkpoint_start_lsn_ = INVALID_LSN; // 检查点开始的LSN
  bool is_need_redo_indexes_ = false;        // 是否需要重建索引的标志
  Transaction transaction_;                  // 用于索引重建的事务对象

  // 从指定LSN开始分析日志
  void analyze_from_lsn(lsn_t start_lsn);

  // 从指定LSN开始重做操作
  void redo_from_lsn(lsn_t start_lsn);
  
  // 重建所有表的索引
  void redo_indexes();
  
  // 处理单个日志记录用于分析阶段
  void process_log_for_analyze(char* log_data, LogType log_type);
  
  // 处理单个日志记录用于重做阶段
  void process_log_for_redo(char* log_data, LogType log_type);
};