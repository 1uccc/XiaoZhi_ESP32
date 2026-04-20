#include "wifi_board.h"
#include "audio/codecs/es8311_audio_codec.h"
// Display
#include "display/display.h"
#include "display/lcd_display.h"
#include "lvgl_theme.h"
// Backlight
// PwmBacklight được khai báo trong các tiêu đề đèn nền được kéo bởi display/lcd_display bao gồm thông qua ngăn xếp lvgl

#include "application.h"
#include "button.h"
#include "config.h"
#include "esp_video.h"

#include <esp_log.h>
#include <inttypes.h>
#include <driver/i2c_master.h>
#include <esp_lvgl_port.h>
#include <soc/clk_tree_defs.h>
// thẻ SD
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include <driver/sdmmc_host.h>
#include <driver/sdspi_host.h>
// Điều khiển nguồn SD (LDO trên chip)
#include "sd_pwr_ctrl_by_on_chip_ldo.h"

// Nhà cung cấp MIPI-DSI/LCD bao gồm (thư viện có thể thay thế một số)
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_mipi_dsi.h"
#include "esp_ldo_regulator.h"
#include "esp_lcd_ek79007.h"
#include "esp_lcd_touch_gt911.h"

// Thư viện bao gồm
#include "bsp/esp32_p4_function_ev_board.h"
#include "bsp/touch.h"

#define TAG "ESP32P4FuncEV"

class ESP32P4FunctionEvBoard : public WifiBoard
{
private:
    i2c_master_bus_handle_t codec_i2c_bus_ = nullptr;
    Button boot_button_;
    LcdDisplay *display_ = nullptr;
    esp_lcd_touch_handle_t tp_ = nullptr;
    EspVideo* camera_ = nullptr;

    void InitializeI2cBuses()
    {
        ESP_ERROR_CHECK(bsp_i2c_init());
        codec_i2c_bus_ = bsp_i2c_get_handle();
    }

    // Không cần khởi tạo bus I2C bằng cảm ứng cho bảng này (được xử lý ở nơi khác)
    void InitializeTouchI2cBus()
    {
        // Không cần thực hiện
    }

    void InitializeLCD()
    {
        bsp_display_config_t config = {
            .hdmi_resolution = BSP_HDMI_RES_NONE,
            .dsi_bus = {
                .phy_clk_src = (mipi_dsi_phy_clock_source_t)SOC_MOD_CLK_PLL_F20M,
                .lane_bit_rate_mbps = 1000,
            },
        };

        bsp_lcd_handles_t handles;
        ESP_ERROR_CHECK(bsp_display_new_with_handles(&config, &handles));

        display_ = new MipiLcdDisplay(handles.io, handles.panel, 1024, 600, 0, 0, true, true, false);
    }

    void InitializeButtons()
    {
        boot_button_.OnClick([this]()
                             {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting) {
                EnterWifiConfigMode();
                return;
            }
            app.ToggleChatState();
        });
    }

    void InitializeTouch()
    {
        ESP_ERROR_CHECK(bsp_touch_new(NULL, &tp_));
    }

    void InitializeSdCard()
    {
        ESP_LOGI(TAG, "Initializing SD card");
        esp_err_t ret = bsp_sdcard_mount();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to mount SD card: %s", esp_err_to_name(ret));
        } else {
            ESP_LOGI(TAG, "SD card mounted successfully");
        }
    }

    void InitializeCamera()
    {
        ESP_LOGI(TAG, "Initializing camera");

        // Sử dụng khởi tạo camera BSP cho ESP-P4
        bsp_camera_cfg_t camera_cfg = {0};
        esp_err_t ret = bsp_camera_start(&camera_cfg);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to initialize BSP camera: %s", esp_err_to_name(ret));
            ESP_LOGI(TAG, "Attempting alternative camera initialization");

            // Giải pháp thay thế: Khởi tạo EspVideo trực tiếp nếu BSP không thành công
            // Điều này cung cấp nhiều quyền kiểm soát hơn đối với cấu hình máy ảnh
            static esp_cam_ctlr_dvp_pin_config_t dvp_pin_config = {
                .data_width = CAM_CTLR_DATA_WIDTH_8,
                .data_io = {
                    [0] = BSP_I2C_SDA,  // Tái sử dụng chân I2C nếu chân camera không được xác định
                    [1] = BSP_I2C_SCL,
                    [2] = GPIO_NUM_NC,
                    [3] = GPIO_NUM_NC,
                    [4] = GPIO_NUM_NC,
                    [5] = GPIO_NUM_NC,
                    [6] = GPIO_NUM_NC,
                    [7] = GPIO_NUM_NC,
                },
                .vsync_io = GPIO_NUM_NC,
                .de_io = GPIO_NUM_NC,
                .pclk_io = GPIO_NUM_NC,
                .xclk_io = GPIO_NUM_NC,
            };

            esp_video_init_sccb_config_t sccb_config = {
                .init_sccb = false,  // Sử dụng bus I2C hiện có
                .i2c_handle = codec_i2c_bus_,  // Tái sử dụng bus I2C hiện có
                .freq = 100000,
            };

            esp_video_init_dvp_config_t dvp_config = {
                .sccb_config = sccb_config,
                .reset_pin = GPIO_NUM_NC,
                .pwdn_pin = GPIO_NUM_NC,
                .dvp_pin = dvp_pin_config,
                .xclk_freq = 20000000,  // 20 MHz điển hình cho máy ảnh
            };

            esp_video_init_config_t video_config = {
                .dvp = &dvp_config,
            };

            // Cố gắng tạo camera với cấu hình trực tiếp
            camera_ = new EspVideo(video_config);
            ESP_LOGI(TAG, "Camera initialized with direct configuration");
        } else {
            ESP_LOGI(TAG, "Camera initialized successfully via BSP");
        }
    }

    void InitializeFonts()
    {
        ESP_LOGI(TAG, "Initializing font support");
        // Việc khởi tạo phông chữ được xử lý bởi hệ thống Tài sản
        // Bảng hỗ trợ tải phông chữ từ phân vùng nội dung
        // Xác minh rằng phông chữ được tải đúng cách bằng cách kiểm tra chủ đề
        auto& theme_manager = LvglThemeManager::GetInstance();
        auto current_theme = theme_manager.GetTheme("light");
        if (current_theme != nullptr) {
            auto text_font = current_theme->text_font();
            if (text_font != nullptr && text_font->font() != nullptr) {
                ESP_LOGI(TAG, "Custom font loaded successfully: line_height=%d", text_font->font()->line_height);
            } else {
                ESP_LOGW(TAG, "Custom font not loaded, using built-in font");
            }
        }
    }

public:

    ESP32P4FunctionEvBoard() : boot_button_(0)
    {
        InitializeI2cBuses();
        // Âm thanh được khởi tạo bởi Es8311AudioCodec
        InitializeLCD();
        InitializeButtons();
        InitializeTouch();
        InitializeSdCard();
        InitializeCamera();
        InitializeFonts();
        GetBacklight()->RestoreBrightness();
    }

    ~ESP32P4FunctionEvBoard()
    {
        // Làm sạch con trỏ hiển thị
        delete display_;
        display_ = nullptr;
        // Tháo thẻ SD
        esp_err_t ret = bsp_sdcard_unmount();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to unmount SD card: %s", esp_err_to_name(ret));
        }
        // Nếu các tài nguyên khác cần dọn dẹp, hãy thêm vào đây
    }

    virtual AudioCodec *GetAudioCodec() override
    {
        static Es8311AudioCodec audio_codec(
            codec_i2c_bus_, (i2c_port_t)BSP_I2C_NUM, AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            BSP_I2S_MCLK, BSP_I2S_SCLK, BSP_I2S_LCLK, BSP_I2S_DOUT, BSP_I2S_DSIN,
            BSP_POWER_AMP_IO, ES8311_CODEC_DEFAULT_ADDR, true, false);
        return &audio_codec;
    }

    virtual Display *GetDisplay() override { return display_; }

    virtual Backlight *GetBacklight() override
    {
        static PwmBacklight backlight(BSP_LCD_BACKLIGHT, DISPLAY_BACKLIGHT_OUTPUT_INVERT);
        return &backlight;
    }

    virtual Camera *GetCamera() override
    {
        return camera_;
    }
};

DECLARE_BOARD(ESP32P4FunctionEvBoard);
