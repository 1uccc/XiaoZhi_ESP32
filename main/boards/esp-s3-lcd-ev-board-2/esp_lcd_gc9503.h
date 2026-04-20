/* * SPDX-FileCopyrightText: 2023 Espressif Systems (Thượng Hải) CO LTD
 *
 * SPDX-Mã định danh giấy phép: Apache-2.0 */
/* *
 * @file
 * @brief LCD ESP: GC9503 */

#pragma once

#include <stdint.h>

#include <esp_lcd_panel_vendor.h>
#include <esp_lcd_panel_rgb.h>

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
} gc9503_lcd_init_cmd_t;

/* *
 * @brief Cấu hình nhà cung cấp bảng điều khiển LCD.
 *
 * @note Cấu trúc này cần được chuyển tới trường `vendor_config` trong `esp_lcd_panel_dev_config_t`.
 * */
typedef struct {
    const esp_lcd_rgb_panel_config_t *rgb_config;   /* !< Cấu hình bảng điều khiển RGB */
    const gc9503_lcd_init_cmd_t *init_cmds;         /* !< Con trỏ tới mảng lệnh khởi tạo. Đặt thành NULL nếu sử dụng các lệnh mặc định.
                                                     * Mảng phải được khai báo là `static const` và được đặt bên ngoài hàm.
                                                     * Vui lòng tham khảo `vendor_spec_init_default` trong tệp nguồn. */
    uint16_t init_cmds_size;                        /* <! Số lệnh trong mảng trên */
    struct {
        unsigned int mirror_by_cmd: 1;              /* <! Chức năng `mirror()` sẽ được triển khai bằng lệnh LCD nếu được đặt thành 1.
                                                     * Nếu không, chức năng sẽ được thực hiện bằng phần mềm. */
        unsigned int auto_del_panel_io: 1;          /* <! Tự động xóa phiên bản IO của bảng điều khiển nếu được đặt thành 1. Tất cả các cờ `*_by_cmd` sẽ không hợp lệ.
                                                     * Nếu các chân IO của bảng điều khiển đang chia sẻ các chân khác của giao diện RGB để lưu GPIO,
                                                     * Vui lòng đặt nó thành 1 để giải phóng bảng IO và các chân của nó (ngoại trừ tín hiệu CS). */
    } flags;
} gc9503_vendor_config_t;

/* *
 * @brief Tạo bảng điều khiển LCD cho model GC9503
 *
 * @note Khi `auto_del_panel_io` được đặt thành 1, chức năng này trước tiên sẽ khởi tạo GC9503 với cách khởi tạo cụ thể của nhà cung cấp và sau đó gọi `esp_lcd_new_rgb_panel()` để tạo bảng điều khiển LCD RGB. Và hàm `esp_lcd_panel_init()` sẽ chỉ khởi tạo RGB.
 * @note Khi `auto_del_panel_io` được đặt thành 0, chức năng này sẽ chỉ gọi `esp_lcd_new_rgb_panel()` để tạo bảng điều khiển LCD RGB. Và hàm `esp_lcd_panel_init()` sẽ khởi tạo cả GC9503 và RGB.
 * @note Việc khởi tạo cụ thể của nhà cung cấp có thể khác nhau giữa các nhà sản xuất, nên tham khảo ý kiến ​​​​của nhà cung cấp LCD để biết mã trình tự khởi tạo.
 *
 * @param[in] io Tay cầm IO của bảng điều khiển LCD
 * @param[in] panel_dev_config Cấu hình thiết bị bảng điều khiển chung (cần có `vendor_config` và `rgb_config`)
 * @param[out] ret_panel Tay cầm bảng điều khiển LCD được trả lại
 * @return
 * - ESP_ERR_INVALID_ARG nếu tham số không hợp lệ
 * - ESP_OK khi thành công
 * - Nếu không thì thất bại */
esp_err_t esp_lcd_new_panel_gc9503(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config,
                                   esp_lcd_panel_handle_t *ret_panel);

/* *
 * @brief Cấu trúc cấu hình IO bảng SPI 3 dây
 *
 * @param[in] line_cfg Cấu hình dòng SPI
 * @param[in] scl_active_edge Tín hiệu SCL cạnh hoạt động, 0: cạnh tăng, 1: cạnh giảm
 * */
#define GC9503_PANEL_IO_3WIRE_SPI_CONFIG(line_cfg, scl_active_edge) \
    {                                                               \
        .line_config = line_cfg,                                    \
        .expect_clk_speed = PANEL_IO_3WIRE_SPI_CLK_MAX,             \
        .spi_mode = scl_active_edge ? 1 : 0,                        \
        .lcd_cmd_bytes = 1,                                         \
        .lcd_param_bytes = 1,                                       \
        .flags = {                                                  \
            .use_dc_bit = 1,                                        \
            .dc_zero_on_data = 0,                                   \
            .lsb_first = 0,                                         \
            .cs_high_active = 0,                                    \
            .del_keep_cs_inactive = 1,                              \
        },                                                          \
    }

/* *
 * @brief Cấu trúc thời gian RGB
 *
 * @note Refresh_rate = (pclk_hz * data_width) / (h_res + hsync_pulse_width + hsync_back_porch + hsync_front_porch)
 */ (v_res + vsync_pulse_width + vsync_back_porch + vsync_front_porch)
 */bit_per_pixel
 * .pclk_hz = 16*1000*1000,\
        .h_res = 384, \
        .v_res = 960,\
        .hsync_pulse_width = 24, \
        .hsync_back_porch = 20,\
        .hsync_front_porch = 40,\
        .vsync_pulse_width = 30,\
        .vsync_back_porch = 18,\
        .vsync_front_porch = 20,\
        .flags.pclk_active_neg = 0, \

                .hsync_pulse_width = 24,\
        .hsync_back_porch = 20,\
        .hsync_front_porch = 40,\
        .vsync_pulse_width = 30,\
        .vsync_back_porch = 18,\
        .vsync_front_porch = 20,\ */

 #define GC9503_800_480_PANEL_60HZ_RGB_TIMING()      \
 {                                               \
     .pclk_hz = 16 * 1000 * 1000,                \
     .h_res = 800,                               \
     .v_res = 480,                               \
     .hsync_pulse_width = 10,                    \
     .hsync_back_porch = 10,                     \
     .hsync_front_porch = 20,                    \
     .vsync_pulse_width = 10,                    \
     .vsync_back_porch = 10,                     \
     .vsync_front_porch = 10,                    \
     .flags = {                                  \
        .hsync_idle_low = 0,                    \
        .vsync_idle_low = 0,                    \
        .de_idle_high = 0,                      \
        .pclk_active_neg = 0,                   \
        .pclk_idle_high = 0,                    \
    },                                          \
 }

#ifdef __cplusplus
}
#endif
