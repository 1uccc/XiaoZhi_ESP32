/* * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Thượng Hải) CO LTD
 *
 * SPDX-Mã định danh giấy phép: Apache-2.0 */
/* *
 * @file
 * @brief LCD ESP: jd9853 */

#pragma once

#include "hal/spi_ll.h"
#include "esp_lcd_panel_vendor.h"

#ifdef __cplusplus
extern "C" {
#endif

/* *
 * @brief Lệnh khởi tạo màn hình LCD.
 * */
typedef struct {
    int cmd;                /* <! Lệnh LCD cụ thể */
    const void *data;       /* <! Bộ đệm chứa dữ liệu cụ thể của lệnh */
    size_t data_bytes;      /* <! Kích thước của `dữ liệu` trong bộ nhớ, tính bằng byte */
    unsigned int delay_ms;  /* <! Độ trễ tính bằng mili giây sau lệnh này */
} jd9853_lcd_init_cmd_t;

/* *
 * @brief Cấu hình nhà cung cấp bảng điều khiển LCD.
 *
 * @note Cấu trúc này cần được chuyển tới trường `vendor_config` trong `esp_lcd_panel_dev_config_t`.
 * */
typedef struct {
    const jd9853_lcd_init_cmd_t *init_cmds;     /* !< Con trỏ tới mảng lệnh khởi tạo. Đặt thành NULL nếu sử dụng các lệnh mặc định.
                                                 * Mảng phải được khai báo là `static const` và được đặt bên ngoài hàm.
                                                 * Vui lòng tham khảo `vendor_spec_init_default` trong tệp nguồn. */
    uint16_t init_cmds_size;                    /* <! Số lệnh trong mảng trên */
} jd9853_vendor_config_t;

/* *
 * @brief Tạo bảng điều khiển LCD cho model jd9853
 *
 * @note Việc khởi tạo cụ thể của nhà cung cấp có thể khác nhau giữa các nhà sản xuất, nên tham khảo ý kiến ​​của nhà cung cấp LCD để biết mã trình tự khởi tạo.
 *
 * @param[in] io Tay cầm IO của bảng điều khiển LCD
 * @param[in] panel_dev_config cấu hình thiết bị bảng điều khiển chung
 * @param[out] ret_panel Tay cầm bảng điều khiển LCD được trả lại
 * @return
 * - ESP_ERR_INVALID_ARG nếu tham số không hợp lệ
 * - ESP_ERR_NO_MEM nếu hết bộ nhớ
 * - ESP_OK khi thành công */
esp_err_t esp_lcd_new_panel_jd9853(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config, esp_lcd_panel_handle_t *ret_panel);

/* *
 * @brief Cấu trúc cấu hình bus bảng điều khiển LCD
 *
 * @param[in] sclk số pin đồng hồ SPI
 * @param[in] số pin mosi SPI MOSI
 * @param[in] max_trans_sz Kích thước truyền tối đa tính bằng byte
 * */
#define JD9853_PANEL_BUS_SPI_CONFIG(sclk, mosi, max_trans_sz)  \
    {                                                           \
        .sclk_io_num = sclk,                                    \
        .mosi_io_num = mosi,                                    \
        .miso_io_num = -1,                                      \
        .quadhd_io_num = -1,                                    \
        .quadwp_io_num = -1,                                    \
        .max_transfer_sz = max_trans_sz,                        \
    }

/* *
 * @brief Cấu trúc cấu hình IO của bảng LCD
 *
 * @param[in] cs SPI chip chọn số pin
 * @param[in] dữ liệu SPI dc/số pin lệnh
 * @param[in] cb Chức năng gọi lại khi quá trình truyền SPI hoàn tất
 * @param[in] cb_ctx Bối cảnh hàm gọi lại
 * */
#define JD9853_PANEL_IO_SPI_CONFIG(cs, dc, callback, callback_ctx) \
    {                                                               \
        .cs_gpio_num = cs,                                          \
        .dc_gpio_num = dc,                                          \
        .spi_mode = 0,                                              \
        .pclk_hz = 40 * 1000 * 1000,                                \
        .trans_queue_depth = 10,                                    \
        .on_color_trans_done = callback,                            \
        .user_ctx = callback_ctx,                                   \
        .lcd_cmd_bits = 8,                                          \
        .lcd_param_bits = 8,                                        \
    }

#ifdef __cplusplus
}
#endif
