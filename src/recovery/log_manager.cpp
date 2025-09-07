/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include <cstring>
#include "log_manager.h"

/**
 * @description: 添加日志记录到日志缓冲区中，并返回日志记录号
 * @param {LogRecord*} log_record 要写入缓冲区的日志记录
 * @return {lsn_t} 返回该日志的日志记录号
 */
lsn_t LogManager::add_log_to_buffer(LogRecord* log_record) {
    std::lock_guard<std::mutex> lock(latch_);
    
    // 分配LSN
    lsn_t lsn = global_lsn_.fetch_add(1);
    log_record->lsn_ = lsn;
    
    // 检查缓冲区是否有足够空间
    if (log_buffer_.is_full(log_record->log_tot_len_)) {
        // 缓冲区满，先刷盘
        flush_log_to_disk();
    }
    
    // 将日志记录序列化到缓冲区
    log_record->serialize(log_buffer_.buffer_ + log_buffer_.offset_);
    log_buffer_.offset_ += log_record->log_tot_len_;
    
    return lsn;
}

/**
 * @description: 把日志缓冲区的内容刷到磁盘中，由于目前只设置了一个缓冲区，因此需要阻塞其他日志操作
 */
void LogManager::flush_log_to_disk() {
    if (log_buffer_.offset_ == 0) {
        return; // 缓冲区为空，无需刷盘
    }
    
    // 记录刷盘前的最后一个LSN
    lsn_t last_lsn_in_buffer = global_lsn_.load() - 1;
    
    // 写入磁盘
    disk_manager_->write_log(log_buffer_.buffer_, log_buffer_.offset_);
    
    // 更新持久化LSN为实际刷盘的最后一条日志的LSN
    persist_lsn_ = last_lsn_in_buffer;
    
    // 清空缓冲区
    log_buffer_.offset_ = 0;
    memset(log_buffer_.buffer_, 0, sizeof(log_buffer_.buffer_));
}

/**
 * @description: 创建静态检查点，返回检查点的LSN
 * @return {lsn_t} 检查点的LSN
 */
lsn_t LogManager::create_checkpoint() {
    std::lock_guard<std::mutex> lock(latch_);
    
    // 先刷新所有日志到磁盘
    flush_log_to_disk();
    
    // 创建静态检查点日志记录
    StaticCheckpointLogRecord checkpoint_record;
    
    // 分配LSN并添加到缓冲区（避免重复加锁）
    lsn_t lsn = global_lsn_.fetch_add(1);
    checkpoint_record.lsn_ = lsn;
    
    // 检查缓冲区是否有足够空间
    if (log_buffer_.is_full(checkpoint_record.log_tot_len_)) {
        flush_log_to_disk();
    }
    
    // 将检查点记录序列化到缓冲区
    checkpoint_record.serialize(log_buffer_.buffer_ + log_buffer_.offset_);
    log_buffer_.offset_ += checkpoint_record.log_tot_len_;
    
    // 再次刷新到磁盘，确保检查点记录持久化
    flush_log_to_disk();
    
    // 更新检查点LSN
    checkpoint_lsn_ = lsn;
    
    return lsn;
}
