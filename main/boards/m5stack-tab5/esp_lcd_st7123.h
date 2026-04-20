/* * SPDX-FileCopyrightText: 2025 Espressif Systems (Thượng Hải) CO LTD
 *
 * SPDX-Mã định danh giấy phép: Apache-2.0 */
/* *
 * @file
 * @brief LCD ESP: ST7123 */

 #pragma once

 #include <stdint.h>
 #include "soc/soc_caps.h"
 
 #if SOC_MIPI_DSI_SUPPORTED
 #include "esp_lcd_panel_vendor.h"
 #include "esp_lcd_mipi_dsi.h"
 
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
 } st7123_lcd_init_cmd_t;
 
 /* *
  * @brief Cấu hình nhà cung cấp bảng điều khiển LCD.
  *
  * @note Cấu trúc này cần được chuyển tới `esp_lcd_panel_dev_config_t::vendor_config`.
  * */
 typedef struct {
     const st7123_lcd_init_cmd_t *init_cmds;       /* !< Con trỏ tới mảng lệnh khởi tạo. Đặt thành NULL nếu sử dụng các lệnh mặc định.
                                                      * Mảng phải được khai báo là `static const` và được đặt bên ngoài hàm.
                                                      * Vui lòng tham khảo `vendor_spec_init_default` trong tệp nguồn. */
     uint16_t init_cmds_size;                        /* <! Số lệnh trong mảng trên */
     struct {
         esp_lcd_dsi_bus_handle_t dsi_bus;               /* !< Cấu hình bus MIPI-DSI */
         const esp_lcd_dpi_panel_config_t *dpi_config;   /* !< Cấu hình bảng điều khiển MIPI-DPI */
         uint8_t  lane_num;                              /* !< Số làn đường MIPI-DSI */
     } mipi_config;
 } st7123_vendor_config_t;
 
 /* *
  * @brief Tạo bảng điều khiển LCD cho model ST7123
  *
  * @note Việc khởi tạo cụ thể của nhà cung cấp có thể khác nhau giữa các nhà sản xuất, nên tham khảo ý kiến ​​của nhà cung cấp LCD để biết mã trình tự khởi tạo.
  *
  * @param[in] io Tay cầm IO của bảng điều khiển LCD
  * @param[in] panel_dev_config Cấu hình thiết bị bảng điều khiển chung
  * @param[out] ret_panel Tay cầm bảng điều khiển LCD được trả lại
  * @return
  * - ESP_ERR_INVALID_ARG nếu tham số không hợp lệ
  * - ESP_OK khi thành công
  * - Nếu không thì thất bại */
 esp_err_t esp_lcd_new_panel_st7123(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config,
                                      esp_lcd_panel_handle_t *ret_panel);
 
 /* *
  * @brief Cấu trúc cấu hình bus MIPI-DSI
  * */
 #define ST7123_PANEL_BUS_DSI_2CH_CONFIG()              \
     {                                                    \
         .bus_id = 0,                                     \
         .num_data_lanes = 2,                             \
         .lane_bit_rate_mbps = 1000,                      \
     }
 
 /* *
  * @brief Cấu trúc cấu hình IO của bảng MIPI-DBI
  * */
 #define ST7123_PANEL_IO_DBI_CONFIG()  \
     {                                   \
         .virtual_channel = 0,           \
         .lcd_cmd_bits = 8,              \
         .lcd_param_bits = 8,            \
     }
 
 /* *
  * @brief Cấu trúc cấu hình MIPI dpi
  *
  * @note Refresh_rate = (dpi_clock_freq_mhz * 1000000) / (h_res + hsync_pulse_width + hsync_back_porch + hsync_front_porch)
  */ (v_res + vsync_pulse_width + vsync_back_porch + vsync_front_porch)
  *
  * @param[in] px_format Định dạng pixel của bảng điều khiển
  * */
 #define ST7123_800_1280_PANEL_60HZ_DPI_CONFIG(px_format) \
     {                                                      \
         .dpi_clk_src = MIPI_DSI_DPI_CLK_SRC_DEFAULT,       \
         .dpi_clock_freq_mhz = 80,                          \
         .virtual_channel = 0,                              \
         .pixel_format = px_format,                         \
         .num_fbs = 1,                                      \
         .video_timing = {                                  \
             .h_size = 800,                                 \
             .v_size = 1280,                                \
             .hsync_back_porch = 140,                       \
             .hsync_pulse_width = 40,                       \
             .hsync_front_porch = 40,                       \
             .vsync_back_porch = 16,                        \
             .vsync_pulse_width = 4,                        \
             .vsync_front_porch = 16,                       \
         },                                                 \
         .flags.use_dma2d = true,                           \
     }
 #endif
 
 #ifdef __cplusplus
 }
 #endif
 