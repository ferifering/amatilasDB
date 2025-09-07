/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL
v2. You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "sm_manager.h"

#include <sys/stat.h>
#include <unistd.h>

#include <fstream>

#include "index/ix.h"
#include "record/rm.h"
#include "record_printer.h"

/**
 * @description: 判断是否为一个文件夹
 * @return {bool} 返回是否为一个文件夹
 * @param {string&} db_name 数据库文件名称，与文件夹同名
 */
bool SmManager::is_dir(const std::string &db_name) {
  struct stat st;
  return stat(db_name.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

/**
 * @description: 创建数据库，所有的数据库相关文件都放在数据库同名文件夹下
 * @param {string&} db_name 数据库名称
 */
void SmManager::create_db(const std::string &db_name) {
  if (is_dir(db_name)) {
    throw DatabaseExistsError(db_name);
  }
  // 为数据库创建一个子目录
  std::string cmd = "mkdir " + db_name;
  if (system(cmd.c_str()) < 0) { // 创建一个名为db_name的目录
    throw UnixError();
  }
  if (chdir(db_name.c_str()) < 0) { // 进入名为db_name的目录
    throw UnixError();
  }
  // 创建系统目录
  DbMeta *new_db = new DbMeta();
  new_db->name_ = db_name;

  // 注意，此处ofstream会在当前目录创建(如果没有此文件先创建)和打开一个名为DB_META_NAME的文件
  std::ofstream ofs(DB_META_NAME);

  // 将new_db中的信息，按照定义好的operator<<操作符，写入到ofs打开的DB_META_NAME文件中
  ofs << *new_db; // 注意：此处重载了操作符<<

  delete new_db;

  // 创建日志文件
  disk_manager_->create_file(LOG_FILE_NAME);

  // 回到根目录
  if (chdir("..") < 0) {
    throw UnixError();
  }
}

/**
 * @description: 删除数据库，同时需要清空相关文件以及数据库同名文件夹
 * @param {string&} db_name 数据库名称，与文件夹同名
 */
void SmManager::drop_db(const std::string &db_name) {
  if (!is_dir(db_name)) {
    throw DatabaseNotFoundError(db_name);
  }
  std::string cmd = "rm -r " + db_name;
  if (system(cmd.c_str()) < 0) {
    throw UnixError();
  }
}

/**
 * @description:
 * 打开数据库，找到数据库对应的文件夹，并加载数据库元数据和相关文件
 * @param {string&} db_name 数据库名称，与文件夹同名
 */
void SmManager::open_db(const std::string &db_name) {
  // 数据库文件夹是否存在
  if (!is_dir(db_name)) {
    throw DatabaseNotFoundError(db_name);
  }

  if (chdir(db_name.c_str()) < 0) { // 进入名为db_name的目录
    throw UnixError();
  }

  // 读取数据库元数据
  std::ifstream ifs(DB_META_NAME);
  ifs >> db_;
  ifs.close();

  // 打开所有file_meta

  for (auto &[tab_name, tab_meta] : db_.tabs_) {
    fhs_.emplace(tab_name, rm_manager_->open_file(tab_name));

    for (auto index : tab_meta.indexes) {
      // TODO:
      auto index_name = ix_manager_->get_index_name(tab_name, index.cols);
      ihs_.emplace(index_name, ix_manager_->open_index(tab_name, index.cols));
      drop_index(tab_name, index.cols, nullptr); // 删除索引文件
      std::vector<std::string> cols;
      for (auto &col : index.cols) {
        cols.emplace_back(col.name);
      }
      create_index(tab_name, cols, nullptr);
    }
  }
}

/**
 * @description: 把数据库相关的元数据刷入磁盘中
 */
void SmManager::flush_meta() {
  // 默认清空文件
  std::ofstream ofs(DB_META_NAME);
  ofs << db_;
}

/**
 * @description: 关闭数据库并把数据落盘
 */
void SmManager::close_db() {

  // 元数据信息落盘
  std::ofstream ofs(DB_META_NAME);
  ofs << db_;
  // 关闭所有文件，close_file里实现落盘
  for (auto it = fhs_.begin(); it != fhs_.end(); it++) {
    rm_manager_->close_file(it->second.get());
  }
  for (auto it = ihs_.begin(); it != ihs_.end(); it++) {
    ix_manager_->close_index(it->second.get());
  }

  ihs_.clear();
  fhs_.clear();
  // 清空元数据
  db_.name_ = "";
  db_.tabs_.clear();

  if (chdir("..") < 0) {
    throw UnixError();
  }
}

/**
 * @description:
 * 显示所有的表,通过测试需要将其结果写入到output.txt,详情看题目文档
 * @param {Context*} context
 */
void SmManager::show_tables(Context *context) {
  std::fstream outfile;
  outfile.open("output.txt", std::ios::out | std::ios::app);
  outfile << "| Tables |\n";
  RecordPrinter printer(1);
  printer.print_separator(context);
  printer.print_record({"Tables"}, context);
  printer.print_separator(context);
  for (auto &entry : db_.tabs_) {
    auto &tab = entry.second;
    printer.print_record({tab.name}, context);
    outfile << "| " << tab.name << " |\n";
  }
  printer.print_separator(context);
  outfile.close();
}

/**
 * @description: 显示表的元数据
 * @param {string&} tab_name 表名称
 * @param {Context*} context
 */
void SmManager::desc_table(const std::string &tab_name, Context *context) {
  TabMeta &tab = db_.get_table(tab_name);

  std::vector<std::string> captions = {"Field", "Type", "Index"};
  RecordPrinter printer(captions.size());
  // Print header
  printer.print_separator(context);
  printer.print_record(captions, context);
  printer.print_separator(context);
  // Print fields
  for (auto &col : tab.cols) {
    std::vector<std::string> field_info = {col.name, coltype2str(col.type),
                                           col.index ? "YES" : "NO"};
    printer.print_record(field_info, context);
  }
  // Print footer
  printer.print_separator(context);
}

/**
 * @description: 创建表
 * @param {string&} tab_name 表的名称
 * @param {vector<ColDef>&} col_defs 表的字段
 * @param {Context*} context
 */
void SmManager::create_table(const std::string &tab_name,
                             const std::vector<ColDef> &col_defs,
                             Context *context) {
  if (db_.is_table(tab_name)) {
    throw TableExistsError(tab_name);
  }
  // Create table meta
  int curr_offset = 0;
  TabMeta tab;
  tab.name = tab_name;
  for (auto &col_def : col_defs) {
    ColMeta col = {.tab_name = tab_name,
                   .name = col_def.name,
                   .type = col_def.type,
                   .len = col_def.len,
                   .offset = curr_offset,
                   .index = false};
    curr_offset += col_def.len;
    tab.cols.push_back(col);
  }
  // Create & open record file
  int record_size =
      curr_offset; // record_size就是col
                   // meta所占的大小（表的元数据也是以记录的形式进行存储的）
  rm_manager_->create_file(tab_name, record_size);
  db_.tabs_[tab_name] = tab;
  // fhs_[tab_name] = rm_manager_->open_file(tab_name);
  fhs_.emplace(tab_name, rm_manager_->open_file(tab_name));

  flush_meta();
}

/**
 * @description: 删除表
 * @param {string&} tab_name 表的名称
 * @param {Context*} context
 */
void SmManager::drop_table(const std::string &tab_name, Context *context) {

  //  检查表是否存在
  if (!db_.is_table(tab_name)) {
    throw TableNotFoundError(tab_name);
  }

  context->lock_mgr_->lock_exclusive_on_table(context->txn_,
                                              fhs_[tab_name]->GetFd());
  // 删除相关索引文件
  TabMeta &obj_table = db_.get_table(tab_name);
  auto table_hdr_ptr = fhs_[tab_name].get();
  for (auto index : obj_table.indexes) {
    drop_index(tab_name, index.cols, context);
  }
  // 删除表文件
  rm_manager_->close_file(table_hdr_ptr);
  rm_manager_->destroy_file(tab_name);

  db_.tabs_.erase(tab_name);
  fhs_.erase(tab_name);
}

// show index
void SmManager::show_indexes(const std::string &tab_name, Context *context) {
  auto &tab = db_.get_table(tab_name);

  std::fstream outfile;
  outfile.open("output.txt", std::ios::out | std::ios::app);
  RecordPrinter printer(1);
  printer.print_separator(context);
  printer.print_record({"index"}, context);
  printer.print_separator(context);
  for (auto &index : tab.indexes) {
    // for output.txt content
    std::string tmp = "| " + tab_name + " | unique | (" + index.cols[0].name;
    for (size_t i = 1; i < index.cols.size(); i++) {
      tmp += "," + index.cols[i].name;
    }
    tmp += ") |\n";
    outfile << tmp;
    printer.print_record({ix_manager_->get_index_name(tab_name, index.cols)},
                         context);
  }
  printer.print_separator(context);
  outfile.close();
}

/**
 * @description: 创建索引
 * @param {string&} tab_name 表的名称
 * @param {vector<string>&} col_names 索引包含的字段名称
 * @param {Context*} context
 */
void SmManager::create_index(const std::string &tab_name,
                             const std::vector<std::string> &col_names,
                             Context *context) {
  TabMeta &tab_meta = db_.get_table(tab_name);
  if (context != nullptr)
    context->lock_mgr_->lock_shared_on_table(context->txn_,
                                             fhs_[tab_name]->GetFd());
  if (tab_meta.is_index(col_names)) {
    throw IndexExistsError(tab_name, col_names);
  }
  
  // 如果在事务中，记录CREATE_INDEX日志
  if (context != nullptr && context->txn_ != nullptr) {
    CreateIndexLogRecord create_index_log(context->txn_->get_transaction_id(), 
                                          tab_name, col_names, 
                                          context->txn_->get_prev_lsn());
    lsn_t lsn = context->log_mgr_->add_log_to_buffer(&create_index_log);
    context->txn_->set_prev_lsn(lsn);
  }
  
  std::vector<ColMeta> index_cols;
  for (const std::string &col_name : col_names)
    index_cols.emplace_back(*tab_meta.get_col(col_name));
  ix_manager_->create_index(tab_name, index_cols);
  std::unique_ptr<IxIndexHandle> index =
      ix_manager_->open_index(tab_name, index_cols);
  int col_tot_len = 0;
  for (const ColMeta &index_col : index_cols) {
    col_tot_len += index_col.len;
  }
  RmFileHandle *table = fhs_.at(tab_name).get();
  RmScan rows(table);
  for (; !rows.is_end(); rows.next()) {
    std::unique_ptr<RmRecord> row = table->get_record(rows.rid(), context);
    if (rows.rid().slot_no < 0 && rows.rid().page_no == 0)
      break;
    int offset = 0;
    char key[col_tot_len];
    for (auto &index_col : index_cols) {
      memcpy(key + offset, row->data + index_col.offset, index_col.len);
      offset += index_col.len;
    }
    try {
      index->insert_entry(key, rows.rid(),
                          context == nullptr ? nullptr : context->txn_);
    } catch (RMDBError &e) {
      ix_manager_->close_index(index.get());
      ix_manager_->destroy_index(tab_name, col_names);
      throw RMDBError("index unique check error -- SmManager::create_index");
    }
  }
  tab_meta.indexes.emplace_back(IndexMeta{
      tab_name, col_tot_len, static_cast<int>(index_cols.size()), index_cols});
  ihs_.emplace(ix_manager_->get_index_name(tab_name, col_names),
               std::move(index));
  flush_meta();
}

/**
 * @description: 删除索引
 * @param {string&} tab_name 表名称
 * @param {vector<string>&} col_names 索引包含的字段名称
 * @param {Context*} context
 */
void SmManager::drop_index(const std::string &tab_name,
                           const std::vector<std::string> &col_names,
                           Context *context) {
  if (!ix_manager_->exists(tab_name, col_names)) {
    throw IndexNotFoundError(tab_name, col_names);
  }
  
  // 如果在事务中，记录DROP_INDEX日志
  if (context != nullptr && context->txn_ != nullptr) {
    DropIndexLogRecord drop_index_log(context->txn_->get_transaction_id(), 
                                      tab_name, col_names, 
                                      context->txn_->get_prev_lsn());
    lsn_t lsn = context->log_mgr_->add_log_to_buffer(&drop_index_log);
    context->txn_->set_prev_lsn(lsn);
  }
  
  std::string index_name = ix_manager_->get_index_name(tab_name, col_names);
  IxIndexHandle *index = ihs_.at(index_name).get();
  // 清除缓存
  for (int i = 0; i < index->get_pages_num(); i++) {
    PageId page_id = {index->get_fd(), i};
    while (buffer_pool_manager_->unpin_page(page_id, true))
      ;
    buffer_pool_manager_->delete_page(page_id);
  }
  ix_manager_->close_index(index);
  ix_manager_->destroy_index(tab_name, col_names);
  ihs_.erase(index_name);
  TabMeta &tab = db_.get_table(tab_name);
  tab.indexes.erase(tab.get_index_meta(col_names));
  flush_meta();
}

/**
 * @description: 删除索引
 * @param {string&} tab_name 表名称
 * @param {vector<ColMeta>&} 索引包含的字段元数据
 * @param {Context*} context
 */
void SmManager::drop_index(const std::string &tab_name,
                           const std::vector<ColMeta> &cols, Context *context) {
  std::vector<std::string> col_names;
  for (const ColMeta &col : cols) {
    col_names.push_back(col.name);
  }
  drop_index(tab_name, col_names, context);
}

/**
 * @description: 获取索引文件名
 * @param {string&} tab_name 表名称
 * @param {vector<ColMeta>&} cols 索引包含的字段元数据
 * @return {string} 索引文件名
 */
std::string SmManager::get_ix_file_name(const std::string &tab_name,
                                        const std::vector<ColMeta> &cols) {
  return ix_manager_->get_index_name(tab_name, cols);
}

/**
 * @description: 用于故障恢复时重建索引
 * @param {string&} tab_name 表名称
 * @param {TabMeta&} table_meta 表元数据
 * @param {vector<string>&} col_names 索引包含的字段名称
 * @param {string&} index_name 索引名称
 * @param {Context*} context 上下文
 */
void SmManager::redo_index(const std::string &tab_name,
                           const TabMeta &table_meta,
                           const std::vector<std::string> &col_names,
                           const std::string &index_name, Context *context) {
  try {
    // 获取索引对应的列元数据
    std::vector<ColMeta> index_cols;
    for (const std::string &col_name : col_names) {
      auto col_iter = const_cast<TabMeta&>(table_meta).get_col(col_name);
      index_cols.emplace_back(*col_iter);
    }

    // 删除旧的索引文件（如果存在）
    if (ix_manager_->exists(tab_name, col_names)) {
      // 检查索引是否在ihs_中打开，如果是则先关闭
      auto ihs_iter = ihs_.find(index_name);
      if (ihs_iter != ihs_.end()) {
        IxIndexHandle *index = ihs_iter->second.get();
        // 清除缓存
        for (int i = 0; i < index->get_pages_num(); i++) {
          PageId page_id = {index->get_fd(), i};
          while (buffer_pool_manager_->unpin_page(page_id, true))
            ;
          buffer_pool_manager_->delete_page(page_id);
        }
        ix_manager_->close_index(index);
        ihs_.erase(ihs_iter);
      }
      ix_manager_->destroy_index(tab_name, col_names);
    }

    // 创建新的索引文件
    ix_manager_->create_index(tab_name, index_cols);

    // 打开索引文件
    std::unique_ptr<IxIndexHandle> index =
        ix_manager_->open_index(tab_name, index_cols);

    // 计算索引键的总长度
    int col_tot_len = 0;
    for (const ColMeta &index_col : index_cols) {
      col_tot_len += index_col.len;
    }

    // 扫描表中的所有记录，重建索引
    RmFileHandle *table = fhs_.at(tab_name).get();
    RmScan rows(table);
    for (; !rows.is_end(); rows.next()) {
      std::unique_ptr<RmRecord> row = table->get_record(rows.rid(), context);
      if (rows.rid().slot_no < 0 && rows.rid().page_no == 0)
        break;

      // 构造索引键
      int offset = 0;
      char key[col_tot_len];
      for (const auto &index_col : index_cols) {
        memcpy(key + offset, row->data + index_col.offset, index_col.len);
        offset += index_col.len;
      }

      // 插入索引项
      index->insert_entry(key, rows.rid(),
                          context == nullptr ? nullptr : context->txn_);
    }

    // 将索引句柄加入到索引句柄映射中
    ihs_.emplace(index_name, std::move(index));

    printf("    Successfully rebuilt index: %s\n", index_name.c_str());
  } catch (const std::exception &e) {
    printf("    Failed to rebuild index %s: %s\n", index_name.c_str(),
           e.what());
  }
}