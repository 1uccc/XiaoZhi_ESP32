/* * SPDX-FileCopyrightText: 2022 Espressif Systems (Thượng Hải) CO LTD
 *
 * SPDX-Mã định danh giấy phép: Apache-2.0 */

/* *
 * @file
 * @brief Bộ mở rộng IO ESP: TCA9554 */

#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c_master.h"
#include "esp_io_expander.h"

#ifdef __cplusplus
extern "C" {
#endif

/* *
 * @brief Tạo đối tượng mở rộng IO TCA9554(A)
 *
 * @param[in] i2c_bus Bộ điều khiển bus I2C. Lấy được từ `i2c_new_master_bus()`
 * @param[in] dev_addr Địa chỉ thiết bị I2C của chip. Có thể là `ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_XXX` hoặc `ESP_IO_EXPANDER_I2C_TCA9554A_ADDRESS_XXX`.
 * @param[out] hand_ret Xử lý đối tượng mở rộng IO đã tạo
 *
 * @return
 * - ESP_OK: Thành công, ngược lại trả về ESP_ERR_xxx */
esp_err_t esp_io_expander_new_i2c_tca9554(i2c_master_bus_handle_t i2c_bus, uint32_t dev_addr, esp_io_expander_handle_t *handle_ret);

/* *
 * @tóm tắt địa chỉ I2C của TCA9554
 *
 * Định dạng địa chỉ 8 bit như sau:
 *
 * (Địa chỉ nô lệ)
 * ┌─────────────────┷─────────────────┐
 * ┌─────┐─────┐─────┐───── ┐─────┐─────┐─────┐─────┐
 * |  0 |  1 |  0 |  0 | A2 | A1 | A0 | R/W |
 * └─────┘─────┘─────┘───── ┘─────┘─────┘─────┘─────┘
 * └────────┯────────┘ └─────┯──────┘
 * (Đã sửa) (Có thể chọn Hareware)
 *
 * Và địa chỉ nô lệ 7 bit là dữ liệu quan trọng nhất đối với người dùng.
 * Ví dụ: nếu A0, A1, A2 của chip được kết nối với GND thì địa chỉ phụ 7 bit của chip đó là 0100000b(0x20).
 * Sau đó, người dùng có thể sử dụng `ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_000` để khởi tạo nó. */
#define ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_000    (0x20)
#define ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_001    (0x21)
#define ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_010    (0x22)
#define ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_011    (0x23)
#define ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_100    (0x24)
#define ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_101    (0x25)
#define ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_110    (0x26)
#define ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_111    (0x27)


/* *
 * @tóm tắt địa chỉ I2C của TCA9554A
 *
 * Định dạng địa chỉ 8 bit như sau:
 *
 * (Địa chỉ nô lệ)
 * ┌─────────────────┷─────────────────┐
 * ┌─────┐─────┐─────┐───── ┐─────┐─────┐─────┐─────┐
 * |  0 |  1 |  1 |  1 | A2 | A1 | A0 | R/W |
 * └─────┘─────┘─────┘───── ┘─────┘─────┘─────┘─────┘
 * └────────┯────────┘ └─────┯──────┘
 * (Đã sửa) (Có thể chọn Hareware)
 *
 * Và địa chỉ nô lệ 7 bit là dữ liệu quan trọng nhất đối với người dùng.
 * Ví dụ: nếu A0, A1, A2 của chip được kết nối với GND thì địa chỉ phụ 7 bit của chip đó là 0111000b(0x38).
 * Sau đó, người dùng có thể sử dụng `ESP_IO_EXPANDER_I2C_TCA9554A_ADDRESS_000` để khởi tạo nó. */
#define ESP_IO_EXPANDER_I2C_TCA9554A_ADDRESS_000    (0x38)
#define ESP_IO_EXPANDER_I2C_TCA9554A_ADDRESS_001    (0x39)
#define ESP_IO_EXPANDER_I2C_TCA9554A_ADDRESS_010    (0x3A)
#define ESP_IO_EXPANDER_I2C_TCA9554A_ADDRESS_011    (0x3B)
#define ESP_IO_EXPANDER_I2C_TCA9554A_ADDRESS_100    (0x3C)
#define ESP_IO_EXPANDER_I2C_TCA9554A_ADDRESS_101    (0x3D)
#define ESP_IO_EXPANDER_I2C_TCA9554A_ADDRESS_110    (0x3E)
#define ESP_IO_EXPANDER_I2C_TCA9554A_ADDRESS_111    (0x3F)

#ifdef __cplusplus
}
#endif
