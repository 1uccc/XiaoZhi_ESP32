/* * SPDX-FileCopyrightText: 2023-2024 Espressif Systems (Thượng Hải) CO LTD
 *
 * SPDX-Mã định danh giấy phép: Apache-2.0 */
#include "soc/soc_caps.h"

#if SOC_MIPI_DSI_SUPPORTED
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_check.h"
#include "esp_lcd_panel_commands.h"
#include "esp_lcd_panel_interface.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_log.h"
#include "rm69a10_driver.h"

#define RM69A10_PAD_CONTROL (0xB2)
#define RM69A10_DSI_2_LANE (0x10)
#define RM69A10_DSI_4_LANE (0x00)

#define RM69A10_CMD_SHLR_BIT (1ULL << 0)
#define RM69A10_CMD_UPDN_BIT (1ULL << 1)
#define RM69A10_MDCTL_VALUE_DEFAULT (0x01)

static const rm69a10_lcd_init_cmd_t vendor_specific_init_default[] = {
    // {cmd, { data }, data_size, delay_ms}
    /* *** CMD_Trang 3 *** */
    {0xFE, (uint8_t[]){0xFD}, 1, 0},
    {0x80, (uint8_t[]){0xFC}, 1, 0},
    {0xFE, (uint8_t[]){0x00}, 1, 0},
    {0x2A, (uint8_t[]){0x00, 0x00, 0x02, 0x37}, 4, 0},
    {0x2B, (uint8_t[]){0x00, 0x00, 0x04, 0xCF}, 4, 0},
    {0x31, (uint8_t[]){0x00, 0x03, 0x02, 0x34}, 4, 0},
    {0x30, (uint8_t[]){0x00, 0x00, 0x04, 0xCF}, 4, 0},
    {0x12, (uint8_t[]){0x00}, 1, 0},
    {0x35, (uint8_t[]){0x00}, 1, 0},
#if CONFIG_SCREEN_PIXEL_FORMAT_RGB565
    {0x3A, (uint8_t[]){0x75}, 1, 0}, // định dạng pixel giao diện 16bit/pixel
#elif CONFIG_SCREEN_PIXEL_FORMAT_RGB888
    {0x3A, (uint8_t[]){0x77}, 1, 0}, // định dạng pixel giao diện 24bit/pixel
#endif
    // {0x51, (uint8_t[]){0xFE}, 1, 0},
    {0x51, (uint8_t[]){0x00}, 1, 0}, // Đặt độ sáng màn hình về 0
    {0x11, (uint8_t[]){0x00}, 0, 120},
    {0x29, (uint8_t[]){0x00}, 0, 0},
    // ============ Gamma KẾT THÚC============
};

typedef struct
{
    esp_lcd_panel_io_handle_t io;
    int reset_gpio_num;
    uint8_t madctl_val; // lưu giá trị hiện tại của thanh ghi LCD_CMD_MADCTL
    const rm69a10_lcd_init_cmd_t *init_cmds;
    uint16_t init_cmds_size;
    uint8_t lane_num;
    struct
    {
        unsigned int reset_level : 1;
    } flags;
    // Để lưu lại các chức năng ban đầu của bảng điều khiển MIPI dpi
    esp_err_t (*del)(esp_lcd_panel_t *panel);
    esp_err_t (*init)(esp_lcd_panel_t *panel);
} rm69a10_panel_t;

static const char *TAG = "rm69a10";

static esp_err_t panel_rm69a10_send_init_cmds(rm69a10_panel_t *rm69a10);

static esp_err_t panel_rm69a10_del(esp_lcd_panel_t *panel);
static esp_err_t panel_rm69a10_init(esp_lcd_panel_t *panel);
static esp_err_t panel_rm69a10_reset(esp_lcd_panel_t *panel);
static esp_err_t panel_rm69a10_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y);
static esp_err_t panel_rm69a10_invert_color(esp_lcd_panel_t *panel, bool invert_color_data);
static esp_err_t panel_rm69a10_sleep(esp_lcd_panel_t *panel, bool sleep);
static esp_err_t panel_rm69a10_on_off(esp_lcd_panel_t *panel, bool on_off);

esp_err_t esp_lcd_new_panel_rm69a10(const esp_lcd_panel_io_handle_t io, const esp_lcd_panel_dev_config_t *panel_dev_config,
                                    esp_lcd_panel_handle_t *ret_panel)
{
    // ESP_LOGI(TAG, "phiên bản: %d.%d.%d", ESP_LCD_RM69A10_VER_MAJOR, ESP_LCD_RM69A10_VER_MINOR,
    // ESP_LCD_RM69A10_VER_PATCH);
    ESP_RETURN_ON_FALSE(io && panel_dev_config && ret_panel, ESP_ERR_INVALID_ARG, TAG, "invalid arguments");
    rm69a10_vendor_config_t *vendor_config = (rm69a10_vendor_config_t *)panel_dev_config->vendor_config;
    ESP_RETURN_ON_FALSE(vendor_config && vendor_config->mipi_config.dpi_config && vendor_config->mipi_config.dsi_bus, ESP_ERR_INVALID_ARG, TAG,
                        "invalid vendor config");

    esp_err_t ret = ESP_OK;
    rm69a10_panel_t *rm69a10 = (rm69a10_panel_t *)calloc(1, sizeof(rm69a10_panel_t));
    ESP_RETURN_ON_FALSE(rm69a10, ESP_ERR_NO_MEM, TAG, "no mem for rm69a10 panel");

    if (panel_dev_config->reset_gpio_num >= 0)
    {
        gpio_config_t io_conf = {
            .pin_bit_mask = 1ULL << panel_dev_config->reset_gpio_num,
            .mode = GPIO_MODE_OUTPUT,
        };
        ESP_GOTO_ON_ERROR(gpio_config(&io_conf), err, TAG, "configure GPIO for RST line failed");
    }

    rm69a10->io = io;
    rm69a10->init_cmds = vendor_config->init_cmds;
    rm69a10->init_cmds_size = vendor_config->init_cmds_size;
    rm69a10->lane_num = vendor_config->mipi_config.lane_num;
    rm69a10->reset_gpio_num = panel_dev_config->reset_gpio_num;
    rm69a10->flags.reset_level = panel_dev_config->flags.reset_active_high;
    rm69a10->madctl_val = RM69A10_MDCTL_VALUE_DEFAULT;

    // Tạo bảng điều khiển MIPI dpi
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_dpi(vendor_config->mipi_config.dsi_bus, vendor_config->mipi_config.dpi_config, ret_panel), err, TAG,
                      "create MIPI DPI panel failed");
    ESP_LOGD(TAG, "new MIPI DPI panel @%p", *ret_panel);

    // Lưu lại các chức năng ban đầu của bảng điều khiển MIPI dpi
    rm69a10->del = (*ret_panel)->del;
    rm69a10->init = (*ret_panel)->init;
    // Ghi đè các chức năng của bảng MIPIDPI
    (*ret_panel)->del = panel_rm69a10_del;
    (*ret_panel)->init = panel_rm69a10_init;
    (*ret_panel)->reset = panel_rm69a10_reset;
    (*ret_panel)->mirror = panel_rm69a10_mirror;
    (*ret_panel)->invert_color = panel_rm69a10_invert_color;
    (*ret_panel)->disp_sleep = panel_rm69a10_sleep;
    (*ret_panel)->disp_on_off = panel_rm69a10_on_off;
    (*ret_panel)->user_data = rm69a10;
    ESP_LOGD(TAG, "new rm69a10 panel @%p", rm69a10);

    return ESP_OK;

err:
    if (rm69a10)
    {
        if (panel_dev_config->reset_gpio_num >= 0)
        {
            gpio_reset_pin(static_cast<gpio_num_t>(panel_dev_config->reset_gpio_num));
        }
        free(rm69a10);
    }
    return ret;
}

static esp_err_t panel_rm69a10_send_init_cmds(rm69a10_panel_t *rm69a10)
{
    esp_lcd_panel_io_handle_t io = rm69a10->io;
    const rm69a10_lcd_init_cmd_t *init_cmds = NULL;
    uint16_t init_cmds_size = 0;
    // uint8_t làn_command = RM69A10_DSI_2_LANE;
    // bool is_cmd_overwriting = false;

    // chuyển đổi (rm69a10->lane_num)
    // {
    // trường hợp 0:
    // trường hợp 2:
    // làn_lệnh = RM69A10_DSI_2_LANE;
    // phá vỡ;
    // trường hợp 4:
    // làn_lệnh = RM69A10_DSI_4_LANE;
    // phá vỡ;
    // mặc định:
    // ESP_LOGE(TAG, "Số làn không hợp lệ %d", rm69a10->lane_num);
    // trả về ESP_ERR_INVALID_ARG;
    // }
    // ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, RM69A10_PAD_control, (uint8_t[]){
    // làn_lệnh,
    // },
    // 1),
    // TAG, "lệnh gửi không thành công");

    // khởi tạo cụ thể của nhà cung cấp, nó có thể khác nhau giữa các nhà sản xuất
    // nên tham khảo ý kiến ​​nhà cung cấp LCD để biết mã trình tự khởi tạo
    // nếu (rm69a10->init_cmds)
    // {
    // init_cmds = rm69a10->init_cmds;
    // init_cmds_size = rm69a10->init_cmds_size;
    // }
    // else
    // {
    // init_cmds = nhà cung cấp_spec_init_default;
    // init_cmds_size = sizeof(vendor_special_init_default) / sizeof(rm69a10_lcd_init_cmd_t);
    // }

    init_cmds = vendor_specific_init_default;
    init_cmds_size = sizeof(vendor_specific_init_default) / sizeof(rm69a10_lcd_init_cmd_t);

    for (int i = 0; i < init_cmds_size; i++)
    {
        // // Kiểm tra xem lệnh đã được sử dụng hay xung đột với nội bộ
        // if (init_cmds[i].data_bytes > 0)
        // {
        // chuyển đổi (init_cmds[i].cmd)
        // {
        // trường hợp LCD_CMD_MADCTL:
        // is_cmd_overwriting = đúng;
        // rm69a10->madctl_val = ((uint8_t *)init_cmds[i].data)[0];
        // phá vỡ;
        // mặc định:
        // is_cmd_overwriting = sai;
        // phá vỡ;
        // }

        // nếu (is_cmd_overwriting)
        // {
        // is_cmd_overwriting = sai;
        // ESP_LOGW(TAG, "Lệnh %02Xh đã được sử dụng và sẽ bị ghi đè bởi chuỗi khởi tạo bên ngoài",
        // init_cmds[i].cmd);
        // }
        // }

        // Gửi lệnh
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, init_cmds[i].cmd, init_cmds[i].data, init_cmds[i].data_bytes), TAG, "send command failed");
        vTaskDelay(pdMS_TO_TICKS(init_cmds[i].delay_ms));
        // printf("Ciallo\n");
    }

    ESP_LOGD(TAG, "send init commands success");

    return ESP_OK;
}

static esp_err_t panel_rm69a10_del(esp_lcd_panel_t *panel)
{
    rm69a10_panel_t *rm69a10 = (rm69a10_panel_t *)panel->user_data;

    if (rm69a10->reset_gpio_num >= 0)
    {
        gpio_reset_pin(static_cast<gpio_num_t>(rm69a10->reset_gpio_num));
    }
    // Xóa bảng điều khiển MIPI dpi
    rm69a10->del(panel);
    ESP_LOGD(TAG, "del rm69a10 panel @%p", rm69a10);
    free(rm69a10);

    return ESP_OK;
}

static esp_err_t panel_rm69a10_init(esp_lcd_panel_t *panel)
{
    rm69a10_panel_t *rm69a10 = (rm69a10_panel_t *)panel->user_data;

    ESP_RETURN_ON_ERROR(panel_rm69a10_send_init_cmds(rm69a10), TAG, "send init commands failed");
    ESP_RETURN_ON_ERROR(rm69a10->init(panel), TAG, "init MIPI DPI panel failed");

    return ESP_OK;
}

static esp_err_t panel_rm69a10_reset(esp_lcd_panel_t *panel)
{
    rm69a10_panel_t *rm69a10 = (rm69a10_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = rm69a10->io;

    // Thực hiện thiết lập lại phần cứng
    if (rm69a10->reset_gpio_num >= 0)
    {
        gpio_set_level(static_cast<gpio_num_t>(rm69a10->reset_gpio_num), rm69a10->flags.reset_level);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(static_cast<gpio_num_t>(rm69a10->reset_gpio_num), !rm69a10->flags.reset_level);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    else if (io)
    { // Thực hiện thiết lập lại phần mềm
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_SWRESET, NULL, 0), TAG, "send command failed");
        vTaskDelay(pdMS_TO_TICKS(20));
    }

    return ESP_OK;
}

static esp_err_t panel_rm69a10_sleep(esp_lcd_panel_t *panel, bool sleep)
{
    rm69a10_panel_t *rm69a10 = (rm69a10_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = rm69a10->io;

    if (sleep == true)
    {
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, 0x10, 0x00, 0), TAG, "esp_lcd_panel_io_tx_param fail");
        ESP_LOGI(TAG, "panel_rm69a10 sleep on");
    }
    else
    {
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, 0x11, 0x00, 0), TAG, "esp_lcd_panel_io_tx_param fail");
        ESP_LOGI(TAG, "panel_rm69a10 sleep off");
    }

    vTaskDelay(pdMS_TO_TICKS(120));

    return ESP_OK;
}

static esp_err_t panel_rm69a10_on_off(esp_lcd_panel_t *panel, bool on_off)
{
    rm69a10_panel_t *rm69a10 = (rm69a10_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = rm69a10->io;

    if (on_off == true)
    {
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, 0x29, 0x00, 0), TAG, "esp_lcd_panel_io_tx_param fail");
        ESP_LOGI(TAG, "panel_rm69a10 display on");
    }
    else
    {
        ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, 0x28, 0x00, 0), TAG, "esp_lcd_panel_io_tx_param fail");
        ESP_LOGI(TAG, "panel_rm69a10 display off");
    }

    vTaskDelay(pdMS_TO_TICKS(120));

    return ESP_OK;
}

static esp_err_t panel_rm69a10_mirror(esp_lcd_panel_t *panel, bool mirror_x, bool mirror_y)
{
    rm69a10_panel_t *rm69a10 = (rm69a10_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = rm69a10->io;
    uint8_t madctl_val = rm69a10->madctl_val;

    ESP_RETURN_ON_FALSE(io, ESP_ERR_INVALID_STATE, TAG, "invalid panel IO");

    // Điều khiển gương thông qua lệnh LCD
    if (mirror_x)
    {
        madctl_val |= RM69A10_CMD_SHLR_BIT;
    }
    else
    {
        madctl_val &= ~RM69A10_CMD_SHLR_BIT;
    }
    if (mirror_y)
    {
        madctl_val |= RM69A10_CMD_UPDN_BIT;
    }
    else
    {
        madctl_val &= ~RM69A10_CMD_UPDN_BIT;
    }

    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, LCD_CMD_MADCTL, (uint8_t[]){madctl_val}, 1), TAG, "send command failed");
    rm69a10->madctl_val = madctl_val;

    return ESP_OK;
}

static esp_err_t panel_rm69a10_invert_color(esp_lcd_panel_t *panel, bool invert_color_data)
{
    rm69a10_panel_t *rm69a10 = (rm69a10_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = rm69a10->io;
    uint8_t command = 0;

    ESP_RETURN_ON_FALSE(io, ESP_ERR_INVALID_STATE, TAG, "invalid panel IO");

    if (invert_color_data)
    {
        command = LCD_CMD_INVON;
    }
    else
    {
        command = LCD_CMD_INVOFF;
    }
    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, command, NULL, 0), TAG, "send command failed");

    return ESP_OK;
}

esp_err_t set_rm69a10_brightness(esp_lcd_panel_t *panel, uint8_t brightness)
{
    rm69a10_panel_t *rm69a10 = (rm69a10_panel_t *)panel->user_data;
    esp_lcd_panel_io_handle_t io = rm69a10->io;

    ESP_RETURN_ON_ERROR(esp_lcd_panel_io_tx_param(io, 0x51, &brightness, 1), TAG, "esp_lcd_panel_io_tx_param fail");

    return ESP_OK;
}

#endif
