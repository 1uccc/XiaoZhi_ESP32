/* * @Mô tả: hi8561_driver
 * @Tác giả: LILYGO_L
 * @Ngày: 2025-06-13 11:02:44
 * @LastEditTime: 2025-10-09 10:36:27
 * @Giấy phép: GPL 3.0 */
#pragma once

#include <stdint.h>
#include "soc/soc_caps.h"

#if SOC_MIPI_DSI_SUPPORTED
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_mipi_dsi.h"

/* *
 * @brief Lệnh khởi tạo màn hình LCD.
 * */
typedef struct
{
    int cmd;               /* <! Lệnh LCD cụ thể */
    const void *data;      /* <! Bộ đệm chứa dữ liệu cụ thể của lệnh */
    size_t data_bytes;     /* <! Kích thước của `dữ liệu` trong bộ nhớ, tính bằng byte */
    unsigned int delay_ms; /* <! Độ trễ tính bằng mili giây sau lệnh này */
} hi8561_lcd_init_cmd_t;

/* *
 * @brief Cấu hình nhà cung cấp bảng điều khiển LCD.
 *
 * @note Cấu trúc này cần được chuyển tới trường `vendor_config` trong `esp_lcd_panel_dev_config_t`.
 * */
typedef struct
{
    const hi8561_lcd_init_cmd_t *init_cmds; /* !< Con trỏ tới mảng lệnh khởi tạo. Đặt thành NULL nếu sử dụng các lệnh mặc định.
                                             * Mảng phải được khai báo là `static const` và được đặt bên ngoài hàm.
                                             * Vui lòng tham khảo `vendor_spec_init_default` trong tệp nguồn. */
    uint16_t init_cmds_size;                /* <! Số lệnh trong mảng trên */
    struct
    {
        esp_lcd_dsi_bus_handle_t dsi_bus;             /* !< Cấu hình bus MIPI-DSI */
        const esp_lcd_dpi_panel_config_t *dpi_config; /* !< Cấu hình bảng điều khiển MIPI-DPI */
        uint8_t lane_num;                             /* !< Số làn MIPI-DSI, mặc định là 2 nếu được đặt thành 0 */
    } mipi_config;
} hi8561_vendor_config_t;

/* *
 * @brief Tạo bảng LCD cho model HI8561
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
esp_err_t esp_lcd_new_panel_hi8561(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config,
                                   esp_lcd_panel_handle_t *ret_panel);

#endif
