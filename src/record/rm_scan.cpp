/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#include "rm_scan.h"
#include "rm_file_handle.h"

/**
 * @brief 初始化file_handle和rid
 * @param file_handle
 */
RmScan::RmScan(const RmFileHandle *file_handle) : file_handle_(file_handle) {
    // 从第一个记录页面的第一个槽位开始扫描
    rid_.page_no = RM_FIRST_RECORD_PAGE;
    rid_.slot_no = 0;

    // 如果第一个槽位无效，立即找到第一个有效记录
    if (!file_handle_->is_record(rid_)) {
        next();
    }
}

/**
 * @brief 找到文件中下一个存放了记录的位置
 */
void RmScan::next() {
    if (is_end()) return;

    const auto& file_hdr = file_handle_->file_hdr_;
    int num_pages = file_hdr.num_pages;
    int records_per_page = file_hdr.num_records_per_page;

    // 从有效位置开始扫描
    if (rid_.slot_no == -1) {
        rid_.slot_no = 0;
    } else {
        rid_.slot_no++;
    }

    bool found = false;
    while (!found && rid_.page_no < num_pages) {
        RmPageHandle page_handle = file_handle_->fetch_page_handle(rid_.page_no);

        // 仅扫描当前页面内的槽位
        while (rid_.slot_no < records_per_page) {
            if (Bitmap::is_set(page_handle.bitmap, rid_.slot_no)) {
                found = true;
                break;
            }
            rid_.slot_no++;
        }

        file_handle_->buffer_pool_manager_->unpin_page(
                page_handle.page->get_page_id(), false);

        if (!found) {
            rid_.page_no++;
            rid_.slot_no = 0;
        }
    }

    if (!found) {
        rid_.page_no = num_pages;
        rid_.slot_no = 0;
    }
}

/**
 * @brief ​ 判断是否到达文件末尾
 */
bool RmScan::is_end() const {
    return rid_.page_no >= file_handle_->file_hdr_.num_pages;
}

/**
 * @brief RmScan内部存放的rid
 */
Rid RmScan::rid() const {
    return rid_;
}