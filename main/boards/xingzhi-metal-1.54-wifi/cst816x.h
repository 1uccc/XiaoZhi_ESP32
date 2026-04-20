#ifndef _CST816X_H_
#define _CST816X_H_

#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c.h"
#include "i2c_device.h"
#include <driver/i2c_master.h>
#include <sys/time.h>
#include <array>

#define ES8311_VOL_MIN 0
#define ES8311_VOL_MAX 100

enum class TouchEventType {
    SINGLE_CLICK,    // sự kiện nhấp chuột
    DOUBLE_CLICK,    // Sự kiện nhấp đúp
    LONG_PRESS_START,// Nhấn và giữ để bắt đầu sự kiện
    LONG_PRESS_END   // Nhấn và giữ để kết thúc sự kiện
};

struct TouchEvent {
    TouchEventType type;  
    int x;                
    int y;               
};

class Cst816x : public I2cDevice {
private:
    struct TouchPoint_t {
        int num = 0; 
        int x = -1;   
        int y = -1;   
    };

    struct TouchThresholdConfig {
        int x;                          // Tọa độ mục tiêu X
        int y;                          // Tọa độ mục tiêu Y
        int64_t single_click_thresh_us; // Thời lượng nhấp chuột tối đa (chúng tôi)
        int64_t double_click_window_us; // Cửa sổ nhấp đúp chuột (chúng tôi)
        int64_t long_press_thresh_us;   // Ngưỡng nhấn giữ (chúng tôi)
    };

    const TouchThresholdConfig DEFAULT_THRESHOLD = {
        .x = -1, .y = -1,                  
        .single_click_thresh_us = 120000,  // 150ms
        .double_click_window_us = 240000,  // 150ms
        .long_press_thresh_us = 4000000    // 4000ms
    };

    const std::array<TouchThresholdConfig, 3> TOUCH_THRESHOLD_TABLE = {
        {
            {20, 600, 200000, 240000, 2000000}, // Âm lượng+
            {40, 600, 200000, 240000, 4000000}, // nút khởi động
            {60, 600, 200000, 240000, 2000000}  // âm lượng-
        }
    };

    const TouchThresholdConfig& getThresholdConfig(int x, int y);

    uint8_t* read_buffer_ = nullptr;  
    TouchPoint_t tp_;                 

    bool is_touching_ = false;              
    int64_t touch_start_time_ = 0;          // Chạm vào thời gian bắt đầu (chúng tôi)
    int64_t last_release_time_ = 0;         // Thời gian phát hành lần cuối (chúng tôi)
    int click_count_ = 0;                   // Số lần nhấp chuột (để phát hiện nhấp đúp)
    bool long_press_started_ = false;       // Liệu báo chí dài đã được kích hoạt hay chưa

    bool is_volume_long_pressing_ = false;   // Âm lượng có được điều chỉnh bằng cách nhấn và giữ hay không
    int volume_long_press_dir_ = 0;          // Hướng điều chỉnh: 1=tăng, -1=giảm
    int64_t last_volume_adjust_time_ = 0;    // Thời điểm điều chỉnh âm lượng lần cuối (chúng tôi)
    const int64_t VOL_ADJ_INTERVAL_US = 200000; // Khoảng thời gian điều chỉnh âm lượng (100ms)
    const int VOL_ADJ_STEP = 5;                // Điều chỉnh kích thước bước mỗi lần

    int64_t getCurrentTimeUs();

public:
    Cst816x(i2c_master_bus_handle_t i2c_bus, uint8_t addr);
    ~Cst816x();

    void InitCst816d();
    void UpdateTouchPoint();
    void resetTouchCounters();
    static void touchpad_daemon(void* param);
    
    const TouchPoint_t& GetTouchPoint() { return tp_; }
};

#endif