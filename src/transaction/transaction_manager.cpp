/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL
v2. You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "transaction_manager.h"
#include "record/rm_file_handle.h"
#include "system/sm_manager.h"
#include "recovery/log_manager.h"

std::unordered_map<txn_id_t, Transaction *> TransactionManager::txn_map = {};

// 7t
/**
 * @description: 事务的开始方法
 * @return {Transaction*} 开始事务的指针
 * @param {Transaction*} txn
 * 事务指针，空指针代表需要创建新事务，否则开始已有事务
 * @param {LogManager*} log_manager 日志管理器指针
 */
Transaction *TransactionManager::begin(Transaction *txn,
                                       LogManager *log_manager) {
    // Todo:
    // 1. 判断传入事务参数是否为空指针
    // 2. 如果为空指针，创建新事务
    // 3. 把开始事务加入到全局事务表中
    // 4. 返回当前事务指针
    // 如果需要支持MVCC请在上述过程中添加代码

    // 1
    if (txn == nullptr) {
        // 2
        txn = new Transaction(next_txn_id_);
        next_txn_id_++;
    } else {
        // 如果重用现有事务，需要清空write_set
        txn->get_write_set()->clear();
        txn->get_lock_set()->clear();
    }
    // 3
    // MVCC: 设置事务的读时间戳为新分配的时间戳
    auto& mvcc_manager = MVCCManager::get_instance();
    timestamp_t read_ts = mvcc_manager.get_next_timestamp();
    txn->set_start_ts(read_ts);
    
    // 设置事务状态为GROWING
    txn->set_state(TransactionState::GROWING);
    
    // MVCC: 将事务添加到活跃事务集合
    mvcc_manager.add_active_txn(txn->get_transaction_id());
    
    // 将事务添加到水印管理中
    watermark_.AddTxn(txn->get_start_ts());
    
    // 将事务加入到全局事务表中
    std::unique_lock<std::mutex> lock(latch_);
    txn_map[txn->get_transaction_id()] = txn;
    running_txns_.insert(txn->get_transaction_id());
    lock.unlock();

    // 记录BEGIN日志
    if (log_manager != nullptr) {
        BeginLogRecord begin_log(txn->get_transaction_id());
        begin_log.prev_lsn_ = txn->get_prev_lsn();
        lsn_t lsn = log_manager->add_log_to_buffer(&begin_log);
        txn->set_prev_lsn(lsn);
    }

    // 4
    return txn;

}

/**
 * @description: 事务的提交方法
 * @param {Transaction*} txn 需要提交的事务
 * @param {LogManager*} log_manager 日志管理器指针
 */
void TransactionManager::commit(Transaction *txn, LogManager *log_manager) {
    // 检查事务状态
    if (txn->get_state() != TransactionState::GROWING) {
        return;
    }
    
    // Todo:
    // 1. 如果存在未提交的写操作，提交所有的写操作
    // 2. 释放所有锁
    // 3. 释放事务相关资源，eg.锁集
    // 4. 把事务日志刷入磁盘中
    // 5. 更新事务状态
    // 如果需要支持MVCC请在上述过程中添加代码

    // 记录COMMIT日志
    if (log_manager != nullptr) {
        CommitLogRecord commit_log(txn->get_transaction_id());
        lsn_t lsn = log_manager->add_log_to_buffer(&commit_log);
        txn->set_prev_lsn(lsn);
    }

    // MVCC: 分配提交时间戳并更新版本链
    auto& mvcc_manager = MVCCManager::get_instance();
    timestamp_t commit_ts = mvcc_manager.get_next_timestamp();
    txn->set_commit_ts(commit_ts);
    
    // 更新所有该事务的版本的时间戳
    mvcc_manager.assign_commit_timestamp(txn->get_transaction_id(), commit_ts);
    
    // MVCC: 从活跃事务集合中移除（必须在assign_commit_timestamp之后立即执行）
    mvcc_manager.remove_active_txn(txn->get_transaction_id());
    
    // 更新最后提交时间戳
    mvcc_manager.update_last_committed_ts(commit_ts);

    // 2 释放锁
    auto lock_set = txn->get_lock_set();
    for (auto &lock_data_id: *lock_set) {
        lock_manager_->unlock(txn, lock_data_id);
    }

    // 3 释放资源
    lock_set->clear();

    // 4 日志刷入磁盘
    if (log_manager != nullptr) {
        log_manager->flush_log_to_disk();
    }

    // 5 更新事务状态
    txn->set_state(TransactionState::COMMITTED);
    
    // 从水印管理中移除事务并更新提交时间戳
    watermark_.RemoveTxn(txn->get_start_ts());
    watermark_.UpdateCommitTs(txn->get_commit_ts());
    
    // 从运行事务集合中移除
    std::unique_lock<std::mutex> lock(latch_);
    running_txns_.erase(txn->get_transaction_id());
    lock.unlock();
    
    // 定期进行垃圾回收
    if (commit_ts % 100 == 0) {
        mvcc_manager.cleanup_version_chains(commit_ts);
        GarbageCollection();
    }
}

/**
 * @description: 事务的终止（回滚）方法
 * @param {Transaction *} txn 需要回滚的事务
 * @param {LogManager} *log_manager 日志管理器指针
 */
void TransactionManager::abort(Transaction *txn, LogManager *log_manager) {
    // Todo:
    // 1. 回滚所有写操作
    // 2. 释放所有锁
    // 3. 清空事务相关资源，eg.锁集
    // 4. 把事务日志刷入磁盘中
    // 5. 更新事务状态
    // 如果需要支持MVCC请在上述过程中添加代码

    // 1 回滚写操作

    auto write_set = txn->get_write_set();
    std::cout << "[DEBUG] Transaction " << txn->get_transaction_id() << " abort: write_set size = " << write_set->size() << std::endl;
    
    // 逆序回滚写操作
    for (auto i = write_set->rbegin(); i != write_set->rend(); i++) {
        WriteRecord *write_record = *i;
        auto &tab_name = write_record->GetTableName();
        auto &rid = write_record->GetRid();
        auto type = write_record->GetWriteType();
        
        std::cout << "[DEBUG] Rolling back operation: type=" << static_cast<int>(type) 
                  << ", table=" << tab_name << ", rid=(" << rid.page_no << "," << rid.slot_no << ")" << std::endl;

        // 获取表的文件句柄
        auto &fh = sm_manager_->fhs_[tab_name];
        auto &ih = sm_manager_->ihs_[tab_name];
        (void)ih; // 避免未使用变量警告

        // 获取表的元数据
        auto &tab_meta = sm_manager_->db_.get_table(tab_name);
        
        switch (type) {
            case WType::INSERT_TUPLE: {
                // 回滚插入：先删除索引项，再删除记录
                std::cout << "[DEBUG] INSERT_TUPLE rollback: deleting record at (" << rid.page_no << "," << rid.slot_no << ")" << std::endl;
                // 首先获取当前记录用于删除索引项
                auto insert_record = fh->get_record(rid, nullptr);
                std::cout << "[DEBUG] Got record for index deletion" << std::endl;
                
                // 删除索引项
                for(size_t idx = 0; idx < tab_meta.indexes.size(); ++idx) {
                    auto& index = tab_meta.indexes[idx];
                    auto index_handle = sm_manager_->ihs_.at(sm_manager_->get_ix_manager()->get_index_name(tab_name, index.cols)).get();
                    char* key = new char[index.col_tot_len];
                    int offset = 0;
                    for(int j = 0; j < index.col_num; ++j) {
                        memcpy(key + offset, insert_record->data + index.cols[j].offset, index.cols[j].len);
                        offset += index.cols[j].len;
                    }
                    index_handle->delete_entry(key, txn);
                    delete[] key;
                }
                // 删除记录
                fh->delete_record(rid, nullptr);
                std::cout << "[DEBUG] INSERT_TUPLE rollback completed" << std::endl;
                break;
            }
            case WType::DELETE_TUPLE: {
                // 回滚删除：重新插入删除的记录和对应的索引项
                std::cout << "[DEBUG] DELETE_TUPLE rollback: reinserting record at (" << rid.page_no << "," << rid.slot_no << ")" << std::endl;
                fh->insert_record(rid, write_record->GetRecord().data);
                std::cout << "[DEBUG] Record reinserted" << std::endl;
                // 重新插入索引项
                for(size_t idx = 0; idx < tab_meta.indexes.size(); ++idx) {
                    auto& index = tab_meta.indexes[idx];
                    auto index_handle = sm_manager_->ihs_.at(sm_manager_->get_ix_manager()->get_index_name(tab_name, index.cols)).get();
                    char* key = new char[index.col_tot_len];
                    int offset = 0;
                    for(int j = 0; j < index.col_num; ++j) {
                        memcpy(key + offset, write_record->GetRecord().data + index.cols[j].offset, index.cols[j].len);
                        offset += index.cols[j].len;
                    }
                    index_handle->insert_entry(key, rid, txn);
                    delete[] key;
                }
                std::cout << "[DEBUG] DELETE_TUPLE rollback completed" << std::endl;
                break;
            }
            case WType::UPDATE_TUPLE: {
                // 回滚更新：恢复原来的记录
                std::cout << "[DEBUG] UPDATE_TUPLE rollback: restoring record at (" << rid.page_no << "," << rid.slot_no << ")" << std::endl;
                // 首先获取当前记录用于删除索引
                auto update_current_record = fh->get_record(rid, nullptr);
                
                // 更新索引：删除当前记录的索引项，插入原始记录的索引项
                for(size_t idx = 0; idx < tab_meta.indexes.size(); ++idx) {
                    auto& index = tab_meta.indexes[idx];
                    auto index_handle = sm_manager_->ihs_.at(sm_manager_->get_ix_manager()->get_index_name(tab_name, index.cols)).get();
                    
                    // 删除当前记录的索引项
                    char* current_key = new char[index.col_tot_len];
                    int offset = 0;
                    for(int j = 0; j < index.col_num; ++j) {
                        memcpy(current_key + offset, update_current_record->data + index.cols[j].offset, index.cols[j].len);
                        offset += index.cols[j].len;
                    }
                    index_handle->delete_entry(current_key, txn);
                    delete[] current_key;
                    
                    // 插入原始记录的索引项
                    char* original_key = new char[index.col_tot_len];
                    offset = 0;
                    for(int j = 0; j < index.col_num; ++j) {
                        memcpy(original_key + offset, write_record->GetRecord().data + index.cols[j].offset, index.cols[j].len);
                        offset += index.cols[j].len;
                    }
                    index_handle->insert_entry(original_key, rid, txn);
                    delete[] original_key;
                }
                
                // 恢复记录到原始状态
                fh->update_record(rid, write_record->GetRecord().data, nullptr);
                std::cout << "[DEBUG] UPDATE_TUPLE rollback completed" << std::endl;
                break;
            }
        }
    }


    // 2 释放锁
    auto lock_set = txn->get_lock_set();
    for (auto &lock_data_id: *lock_set) {
        lock_manager_->unlock(txn, lock_data_id);
    }

    // 3 清空资源
    lock_set->clear();
    write_set->clear();

    // 4. 刷盘
    if (log_manager != nullptr) {

        log_manager->flush_log_to_disk();
    }

    // 5 更新事务状态
    txn->set_state(TransactionState::ABORTED);
    
    // MVCC: 回滚事务的所有版本
    auto& mvcc_manager = MVCCManager::get_instance();
    mvcc_manager.rollback_transaction(txn->get_transaction_id());
    
    // 从活跃事务集合中移除
    mvcc_manager.remove_active_txn(txn->get_transaction_id());
    
    // 从水印管理中移除事务
    watermark_.RemoveTxn(txn->get_start_ts());
    
    // 从运行事务集合中移除
    std::unique_lock<std::mutex> lock(latch_);
    running_txns_.erase(txn->get_transaction_id());
    lock.unlock();
    
    // 定期进行垃圾回收
    timestamp_t current_ts = mvcc_manager.get_next_timestamp();
    if (current_ts % 100 == 0) {
        mvcc_manager.cleanup_version_chains(current_ts);
        GarbageCollection();
    }
}

// MVCC相关方法实现

/**
 * @description: 更新元组的undo链接
 * @param {RID} rid 记录ID
 * @param {UndoLink} undo_link undo链接
 * @param {std::string} table_name 表名
 */
void TransactionManager::UpdateUndoLink(Rid rid, std::optional<UndoLink> undo_link, const std::string& table_name) {
    std::unique_lock<std::shared_mutex> lock(page_version_info_mutex_);
    auto key = std::make_pair(table_name, rid);
    if (undo_link.has_value()) {
        page_version_info_[key].undo_link_ = undo_link.value();
    } else {
        page_version_info_.erase(key);
    }
}

/**
 * @description: 更新元组的版本链接
 * @param {Rid} rid 记录ID
 * @param {VersionUndoLink} version_link 版本链接
 * @param {std::string} table_name 表名
 */
void TransactionManager::UpdateVersionLink(Rid rid, std::optional<VersionUndoLink> version_link, const std::string& table_name) {
    std::unique_lock<std::shared_mutex> lock(page_version_info_mutex_);
    auto key = std::make_pair(table_name, rid);
    if (version_link.has_value()) {
        page_version_info_[key].version_link_ = version_link.value();
    } else {
        page_version_info_[key].version_link_ = std::nullopt;
    }
}

/**
 * @description: 获取元组的undo链接
 * @param {Rid} rid 记录ID
 * @param {std::string} table_name 表名
 * @return {std::optional<UndoLink>} undo链接
 */
std::optional<UndoLink> TransactionManager::GetUndoLink(Rid rid, const std::string& table_name) {
    std::shared_lock<std::shared_mutex> lock(page_version_info_mutex_);
    auto key = std::make_pair(table_name, rid);
    auto it = page_version_info_.find(key);
    if (it != page_version_info_.end()) {
        return it->second.undo_link_;
    }
    return std::nullopt;
}

/**
 * @description: 获取元组的版本链接
 * @param {Rid} rid 记录ID
 * @param {std::string} table_name 表名
 * @return {std::optional<VersionUndoLink>} 版本链接
 */
std::optional<VersionUndoLink> TransactionManager::GetVersionLink(Rid rid, const std::string& table_name) {
    std::shared_lock<std::shared_mutex> lock(page_version_info_mutex_);
    auto key = std::make_pair(table_name, rid);
    auto it = page_version_info_.find(key);
    if (it != page_version_info_.end()) {
        return it->second.version_link_;
    }
    return std::nullopt;
}

/**
 * @description: 获取undo日志（可选）
 * @param {UndoLink} undo_link undo链接
 * @return {std::optional<UndoLog>} undo日志
 */
std::optional<UndoLog> TransactionManager::GetUndoLogOptional(UndoLink undo_link) {
    if (!undo_link.IsValid()) {
        return std::nullopt;
    }
    
    auto txn = get_transaction(undo_link.prev_txn_);
    if (txn == nullptr) {
        return std::nullopt;
    }
    
    return txn->GetUndoLog(undo_link.prev_log_idx_);
}

/**
 * @description: 获取undo日志
 * @param {UndoLink} undo_link undo链接
 * @return {UndoLog} undo日志
 */
UndoLog TransactionManager::GetUndoLog(UndoLink undo_link) {
    auto opt_log = GetUndoLogOptional(undo_link);
    if (!opt_log.has_value()) {
        throw std::runtime_error("Invalid undo link");
    }
    return opt_log.value();
}

/**
 * @description: 获取水印时间戳
 * @return {timestamp_t} 水印时间戳
 */
timestamp_t TransactionManager::GetWatermark() {
    return watermark_.GetWatermark();
}

/**
 * @description: 垃圾回收
 */
void TransactionManager::GarbageCollection() {
    auto watermark = GetWatermark();
    
    // 清理已提交且时间戳小于水印的事务
    std::unique_lock<std::mutex> lock(latch_);
    auto it = txn_map.begin();
    while (it != txn_map.end()) {
        auto* txn = it->second;
        if ((txn->get_state() == TransactionState::COMMITTED || 
             txn->get_state() == TransactionState::ABORTED) &&
            txn->get_commit_ts() < watermark) {
            delete txn;
            it = txn_map.erase(it);
        } else {
            ++it;
        }
    }
}