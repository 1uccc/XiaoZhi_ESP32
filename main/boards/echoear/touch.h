/* * SPDX-FileCopyrightText: 2023 Espressif Systems (Thượng Hải) CO LTD
 *
 * SPDX-Mã định danh giấy phép: Apache-2.0 */

/* *
 * @file
 * @brief Màn hình cảm ứng BSP
 *
 * Tệp này cung cấp API để khởi tạo màn hình cảm ứng cơ bản.
 * Nó rất hữu ích cho những người dùng muốn sử dụng màn hình cảm ứng mà không có Thư viện đồ họa LVGL mặc định.
 *
 * Để khởi tạo LCD tiêu chuẩn với thư viện đồ họa LVGL, bạn có thể gọi hàm tất cả trong một bsp_display_start(). */

#pragma once
#include "esp_lcd_touch.h"

#ifdef __cplusplus
extern "C" {
#endif

/* *
 * @brief Cấu trúc cấu hình cảm ứng BSP
 * */
typedef struct {
    void *dummy;    /* !< Chuẩn bị cho việc sử dụng trong tương lai. */
} bsp_touch_config_t;

/* *
 * @brief Tạo màn hình cảm ứng mới
 *
 * Nếu bạn muốn giải phóng tài nguyên do chức năng này phân bổ, bạn có thể sử dụng API Esp_lcd_touch, tức là:
 *
 * \code{.c}
 * đặc biệt_lcd_touch_del(tp);
 * \endcode
 *
 * @param[in] config cấu hình cảm ứng
 * @param[out] ret_touch Esp_lcd_touch tay cầm màn hình cảm ứng
 * @return
 * - ESP_OK Khi thành công
 * - Khác Esp_lcd_touch bị lỗi */
esp_err_t bsp_touch_new(const bsp_touch_config_t *config, esp_lcd_touch_handle_t *ret_touch);

#ifdef __cplusplus
}
#endif
