/* * SPDX-FileCopyrightText: 2021-2023 Espressif Systems (Thượng Hải) CO LTD
 *
 * SPDX-Mã định danh giấy phép: Apache-2.0 */
#pragma once

#include <stdbool.h>
#include "esp_err.h"
#include "esp_lcd_panel_dev.h"

#ifdef __cplusplus
extern "C" {
#endif

/* *
 * @brief Tạo bảng điều khiển LCD cho model ST7789
 *
 * @param[in] io Tay cầm IO của bảng điều khiển LCD
 * @param[in] panel_dev_config cấu hình thiết bị bảng điều khiển chung
 * @param[out] ret_panel Tay cầm bảng điều khiển LCD được trả lại
 * @return
 * - ESP_ERR_INVALID_ARG nếu tham số không hợp lệ
 * - ESP_ERR_NO_MEM nếu hết bộ nhớ
 * - ESP_OK khi thành công */
esp_err_t esp_lcd_new_panel_gc9309na(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config, esp_lcd_panel_handle_t *ret_panel);

#ifdef __cplusplus
}
#endif
