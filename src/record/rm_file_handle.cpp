/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL
v2. You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "rm_file_handle.h"
#include "common/common.h"
#include "transaction/transaction_manager.h"

void update_file_hdr(DiskManager *disk_manager, int fd,
                     const RmFileHdr &file_hdr) {
  disk_manager->write_page(fd, RM_FILE_HDR_PAGE, (char *)&file_hdr,
                           sizeof(RmFileHdr));
}
/**
 * @description: 获取当前表中记录号为rid的记录（支持MVCC）
 * @param {Rid&} rid 记录号，指定记录的位置
 * @param {Context*} context
 * @return {unique_ptr<RmRecord>} rid对应的记录对象指针
 */
std::unique_ptr<RmRecord> RmFileHandle::get_record(const Rid &rid,
                                                   Context *context) const {
  RmPageHandle page_handle = fetch_page_handle(rid.page_no);

  if (!Bitmap::is_set(page_handle.bitmap, rid.slot_no)) {
    buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);
    throw RecordNotFoundError(rid.page_no, rid.slot_no);
  }

  char *slot_data = page_handle.get_slot(rid.slot_no);
  auto base_record =
      std::make_unique<RmRecord>(file_hdr_.record_size, slot_data);

  // 如果没有事务上下文，直接返回基础记录
  if (context == nullptr || context->txn_ == nullptr) {
    buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);
    return base_record;
  }

  // MVCC版本可见性检查
  auto &mvcc_manager = MVCCManager::get_instance();
  timestamp_t read_ts = context->txn_->get_start_ts();
  txn_id_t reader_txn_id = context->txn_->get_transaction_id();

  // 获取活跃事务集合用于可见性判断
  auto active_txns = mvcc_manager.get_active_txns();
  auto all_version_logs =
      mvcc_manager.get_undo_logs(rid, fd_, read_ts, reader_txn_id);

  // MVCC调试信息已移除

  buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);

  // 如果没有版本链，返回基础记录
  if (all_version_logs.empty()) {
    return base_record;
  }

  // 沿着版本链找第一个可见的版本
  auto visible_record = std::make_unique<RmRecord>(file_hdr_.record_size);
  bool found_visible = false;
  bool record_exists = true;
  bool has_insert_version = false;

  // 检查是否有INSERT版本
  for (const auto &log : all_version_logs) {
    if (log.type_ == WType::INSERT_TUPLE) {
      has_insert_version = true;
      break;
    }
  }

  // 按版本链顺序检查可见性
  for (const auto &log : all_version_logs) {
    bool is_visible = false;

    // MVCC可见性判断
    if (log.txn_id_ == reader_txn_id) {
      // 自己的修改总是可见
      is_visible = true;
    } else if (log.ts_ > 0 && log.ts_ <= read_ts &&
               !active_txns.count(log.txn_id_)) {
      // 已提交（ts_ > 0）且提交时间戳 <= 读时间戳，且事务不再活跃的版本可见
      is_visible = true;
    }
    // 其他事务未提交的修改（ts_ == 0）或仍在活跃事务集合中的修改不可见

    if (is_visible) {
      if (log.type_ == WType::INSERT_TUPLE) {
        // INSERT版本：使用插入的值
        memcpy(visible_record->data, log.value_->data, file_hdr_.record_size);
        found_visible = true;
        record_exists = true;
        break;
      } else if (log.type_ == WType::UPDATE_TUPLE) {
        // UPDATE版本：使用更新后的新值
        memcpy(visible_record->data, log.value_->data, file_hdr_.record_size);
        found_visible = true;
        record_exists = true;
        break;
      } else if (log.type_ == WType::DELETE_TUPLE) {
        // DELETE版本：记录被删除，但只有对当前事务可见的删除才生效
        record_exists = false;
        break;
      }
    }
  }

  // 如果没有找到可见版本，检查基础记录
  if (!found_visible && record_exists) {
    if (has_insert_version) {
      // 有INSERT版本但不可见，记录不存在
      record_exists = false;
    } else {
      // 没有INSERT版本，使用基础记录
      return base_record;
    }
  }

  // 如果记录不存在
  if (!record_exists) {
    throw RecordNotFoundError(rid.page_no, rid.slot_no);
  }

  return visible_record;
}

/**
 * @description: 在当前表中插入一条记录，不指定插入位置
 * @param {char*} buf 要插入的记录的数据
 * @param {Context*} context
 * @return {Rid} 插入的记录的记录号（位置）
 */
Rid RmFileHandle::insert_record(char *buf, Context *context) {
  // Todo:
  // 1. 获取当前未满的page handle
  // 2. 在page handle中找到空闲slot位置
  // 3. 将buf复制到空闲slot位置
  // 4. 更新page_handle.page_hdr中的数据结构
  // 注意考虑插入一条记录后页面已满的情况，需要更新file_hdr_.first_free_page_no

  // 1. 获取当前未满的page handle
  RmPageHandle page_handle = create_page_handle();

  // 2. 查找空闲槽位
  int slot_no = Bitmap::first_bit(false, page_handle.bitmap,
                                  file_hdr_.num_records_per_page);
  if (slot_no == -1) {
    buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);
    throw std::logic_error("No free slot found in page");
  }

  // 创建Rid用于加锁
  Rid rid{page_handle.page->get_page_id().page_no, slot_no};

  // 加锁
  if (context != nullptr) {
    context->lock_mgr_->lock_IX_on_table(context->txn_, fd_);
    context->lock_mgr_->lock_exclusive_on_record(context->txn_, rid, fd_);
  }

  // 3. 复制数据到槽位
  char *slot = page_handle.get_slot(slot_no);
  memcpy(slot, buf, file_hdr_.record_size);

  // 4. 更新位图和记录数
  Bitmap::set(page_handle.bitmap, slot_no);
  page_handle.page_hdr->num_records++;

  // 5. 检查页面是否变满
  if (page_handle.page_hdr->num_records == file_hdr_.num_records_per_page) {
    file_hdr_.first_free_page_no = page_handle.page_hdr->next_free_page_no;
    update_file_hdr(disk_manager_, fd_, file_hdr_);
  }

  // MVCC: 为插入操作创建版本记录
  if (context != nullptr && context->txn_ != nullptr) {
    auto &mvcc_manager = MVCCManager::get_instance();
    auto undo_log = std::make_shared<UndoLog>(
        WType::INSERT_TUPLE, 0, context->txn_->get_transaction_id(), rid,
        RmRecord(file_hdr_.record_size, buf));
    mvcc_manager.add_version(rid, fd_, undo_log);
  }

  // 6. 标记页面为脏并释放
  buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), true);

  return rid;
}

/**
 * @description: 在当前表中的指定位置插入一条记录
 * @param {Rid&} rid 要插入记录的位置
 * @param {char*} buf 要插入记录的数据
 */
void RmFileHandle::insert_record(const Rid &rid, char *buf) {
  if (rid.page_no == file_hdr_.num_pages) {
    create_new_page_handle();
  }

  // 对于日志恢复的时候如果当前页面不存在，需要创建新的页面
  RmPageHandle page_handle = fetch_page_handle(rid.page_no);

  // 检查槽位是否已被占用（用于重复插入检查）
  bool exist = Bitmap::is_set(page_handle.bitmap, rid.slot_no);

  // 复制数据到槽位
  char *slot = page_handle.get_slot(rid.slot_no);
  memcpy(slot, buf, file_hdr_.record_size);

  // 更新位图，如果记录不存在才增加记录数
  Bitmap::set(page_handle.bitmap, rid.slot_no);
  if (!exist) {
    page_handle.page_hdr->num_records++;
  }

  // 检查页面是否变满
  if (!exist &&
      page_handle.page_hdr->num_records == file_hdr_.num_records_per_page) {
    file_hdr_.first_free_page_no = page_handle.page_hdr->next_free_page_no;
  }

  // 标记页面为脏并释放
  buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), true);
}

/**
 * @description: 删除记录文件中记录号为rid的记录（支持MVCC）
 * @param {Rid&} rid 要删除的记录的记录号（位置）
 * @param {Context*} context
 */
void RmFileHandle::delete_record(const Rid &rid, Context *context) {
  RmPageHandle page_handle = fetch_page_handle(rid.page_no);

  // 检查记录是否存在
  if (!Bitmap::is_set(page_handle.bitmap, rid.slot_no)) {
    buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);
    throw RecordNotFoundError(rid.page_no, rid.slot_no);
  }

  // MVCC: 检查写写冲突
  if (context != nullptr && context->txn_ != nullptr) {
    auto &mvcc_manager = MVCCManager::get_instance();
    auto undo_logs = mvcc_manager.get_undo_logs(
        rid, fd_, UINT64_MAX, context->txn_->get_transaction_id());

    // 检查是否有其他事务的未提交版本
    for (const auto &log : undo_logs) {
      if (log.txn_id_ != context->txn_->get_transaction_id() && log.ts_ == 0) {
        buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(),
                                         false);
        throw TransactionAbortException(
            context->txn_->get_transaction_id(),
            AbortReason::
                ATTEMPTED_INTENTION_LOCK_ON_RECORD_LOCKED_BY_OTHER_TXN);
      }
      if (log.txn_id_ != context->txn_->get_transaction_id() &&
          log.ts_ > context->txn_->get_start_ts()) {
        buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(),
                                         false);
        throw TransactionAbortException(
            context->txn_->get_transaction_id(),
            AbortReason::
                ATTEMPTED_INTENTION_LOCK_ON_RECORD_LOCKED_BY_OTHER_TXN);
      }
    }

    // 保存删除前的记录用于undo
    char *slot_data = page_handle.get_slot(rid.slot_no);
    auto undo_log = std::make_shared<UndoLog>(
        WType::DELETE_TUPLE, 0, context->txn_->get_transaction_id(), rid,
        RmRecord(file_hdr_.record_size, slot_data));
    mvcc_manager.add_version(rid, fd_, undo_log);

    // MVCC模式下不直接删除物理记录，只创建删除版本
    buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);
    return;
  }

  // 非事务模式下才直接删除物理记录
  // 使用删除前的状态判断页面是否满
  bool was_full =
      (page_handle.page_hdr->num_records == file_hdr_.num_records_per_page);

  Bitmap::reset(page_handle.bitmap, rid.slot_no);
  page_handle.page_hdr->num_records--;

  // 页面从满变为不满时加入空闲链表
  if (was_full) {
    page_handle.page_hdr->next_free_page_no = file_hdr_.first_free_page_no;
    file_hdr_.first_free_page_no = page_handle.page->get_page_id().page_no;
    update_file_hdr(disk_manager_, fd_, file_hdr_);
  }

  buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), true);
}

/**
 * @description: 更新记录文件中记录号为rid的记录（支持MVCC）
 * @param {Rid&} rid 要更新的记录的记录号（位置）
 * @param {char*} buf 新记录的数据
 * @param {Context*} context
 */
void RmFileHandle::update_record(const Rid &rid, char *buf, Context *context) {
  RmPageHandle page_handle = fetch_page_handle(rid.page_no);

  if (!Bitmap::is_set(page_handle.bitmap, rid.slot_no)) {
    buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), false);
    throw RecordNotFoundError(rid.page_no, rid.slot_no);
  }

  // 保存原始数据
  char *slot_data = page_handle.get_slot(rid.slot_no);
  char *original_data = new char[file_hdr_.record_size];
  memcpy(original_data, slot_data, file_hdr_.record_size);

  // MVCC版本管理：不直接修改基础记录
  if (context != nullptr && context->txn_ != nullptr) {
    auto &mvcc_manager = MVCCManager::get_instance();
    auto undo_logs = mvcc_manager.get_undo_logs(
        rid, fd_, UINT64_MAX, context->txn_->get_transaction_id());

    // 检查是否有其他事务的未提交版本
    for (const auto &log : undo_logs) {
      if (log.txn_id_ != context->txn_->get_transaction_id() && log.ts_ == 0) {
        delete[] original_data;
        buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(),
                                         false);
        throw TransactionAbortException(
            context->txn_->get_transaction_id(),
            AbortReason::
                ATTEMPTED_INTENTION_LOCK_ON_RECORD_LOCKED_BY_OTHER_TXN);
      }
      if (log.txn_id_ != context->txn_->get_transaction_id() &&
          log.ts_ > context->txn_->get_start_ts()) {
        delete[] original_data;
        buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(),
                                         false);
        throw TransactionAbortException(
            context->txn_->get_transaction_id(),
            AbortReason::
                ATTEMPTED_INTENTION_LOCK_ON_RECORD_LOCKED_BY_OTHER_TXN);
      }
    }

    // 创建新版本，存储新值（不直接修改物理记录）
    RmRecord new_record(file_hdr_.record_size, buf);
    auto version_log =
        std::make_shared<UndoLog>(WType::UPDATE_TUPLE,
                                  0, // 临时时间戳，提交时再分配
                                  context->txn_->get_transaction_id(), rid,
                                  new_record // 存储新值作为这个事务的版本
        );

    // 添加到版本链
    mvcc_manager.add_version(rid, fd_, version_log);
  } else {
    // 非事务上下文，直接更新记录
    memcpy(slot_data, buf, file_hdr_.record_size);
  }

  delete[] original_data;
  buffer_pool_manager_->unpin_page(page_handle.page->get_page_id(), true);
}

/**
 * 以下函数为辅助函数，仅提供参考，可以选择完成如下函数，也可以删除如下函数，在单元测试中不涉及如下函数接口的直接调用
 */
/**
 * @description: 获取指定页面的页面句柄
 * @param {int} page_no 页面号
 * @return {RmPageHandle} 指定页面的句柄
 */
RmPageHandle RmFileHandle::fetch_page_handle(int page_no) const {
  // 检查页面号是否有效
  if (page_no < 0 || page_no > file_hdr_.num_pages) {
    auto file_name = disk_manager_->get_file_name(fd_);
    throw PageNotExistError(file_name, page_no);
  }

  // 通过缓冲池获取页面
  Page *page = buffer_pool_manager_->fetch_page({fd_, page_no});
  if (page == nullptr) {
    throw std::logic_error("All pages have been pinned");
  }

  return RmPageHandle(&file_hdr_, page);
}

/**
 * @description: 创建一个新的page handle
 * @return {RmPageHandle} 新的PageHandle
 */
RmPageHandle RmFileHandle::create_new_page_handle() {
  // 1. 使用缓冲池来创建一个新page
  PageId new_page_id = {fd_, INVALID_PAGE_ID};
  Page *new_page = buffer_pool_manager_->new_page(&new_page_id);
  if (new_page == nullptr) {
    throw std::logic_error("All pages have been pinned");
  }

  // 2. 初始化页面句柄
  RmPageHandle new_page_handle(&file_hdr_, new_page);

  // 3. 初始化页面头
  new_page_handle.page_hdr->num_records = 0;
  // 调用该函数的时候，应该是当前文件的所有空页都满了
  assert(file_hdr_.first_free_page_no == RM_NO_PAGE);
  new_page_handle.page_hdr->next_free_page_no = RM_NO_PAGE;

  // 4. 初始化位图（全部设为0）
  Bitmap::init(new_page_handle.bitmap, file_hdr_.bitmap_size);
  memset(new_page_handle.bitmap, 0, file_hdr_.bitmap_size);

  // 5. 更新文件头
  file_hdr_.first_free_page_no = new_page_id.page_no;
  file_hdr_.num_pages++;
  update_file_hdr(disk_manager_, fd_, file_hdr_);
  return new_page_handle;
}

/**
 * @brief 创建或获取一个空闲的page handle
 *
 * @return RmPageHandle 返回生成的空闲page handle
 * @note pin the page, remember to unpin it outside!
 */
RmPageHandle RmFileHandle::create_page_handle() {
  auto free_page_no = file_hdr_.first_free_page_no;
  return free_page_no == RM_NO_PAGE ? create_new_page_handle()
                                    : fetch_page_handle(free_page_no);
}

/**
 * @description:
 * 当一个页面从没有空闲空间的状态变为有空闲空间状态时，更新文件头和页头中空闲页面相关的元数据
 */
void RmFileHandle::release_page_handle(RmPageHandle &page_handle) {
  page_handle.page_hdr->next_free_page_no = file_hdr_.first_free_page_no;
  file_hdr_.first_free_page_no = page_handle.page->get_page_id().page_no;
  update_file_hdr(disk_manager_, fd_, file_hdr_);
}