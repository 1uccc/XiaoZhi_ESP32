#pragma once
#include <vector>
#include <functional>

#include <esp_timer.h>
#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>


class PowerManager {
private:
    esp_timer_handle_t timer_handle_;
    std::function<void(bool)> on_charging_status_changed_;
    std::function<void(bool)> on_low_battery_status_changed_;

    gpio_num_t charging_pin_ = GPIO_NUM_NC;
    std::vector<uint16_t> adc_values_;
    uint32_t battery_level_ = 0;
    bool is_charging_ = false;
    bool is_low_battery_ = false;
    int ticks_ = 0;
    const int kBatteryAdcInterval = 60;
    const int kBatteryAdcDataCount = 3;
    const int kLowBatteryLevel = 20;

    adc_oneshot_unit_handle_t adc_handle_;
    bool adc_handle_owned_ = false;  // Đánh dấu xem tay cầm ADC có được tạo bởi lớp này hay không
    adc_cali_handle_t adc_cali_handle_ = nullptr;  // Tay cầm hiệu chuẩn ADC

    void CheckBatteryStatus() {
        // Nhận trạng thái sạc
        bool new_charging_status = gpio_get_level(charging_pin_) == 1;
        if (new_charging_status != is_charging_) {
            is_charging_ = new_charging_status;
            if (on_charging_status_changed_) {
                on_charging_status_changed_(is_charging_);
            }
            ReadBatteryAdcData();
            return;
        }

        // Nếu dữ liệu nguồn pin không đủ, hãy đọc dữ liệu nguồn pin
        if (adc_values_.size() < kBatteryAdcDataCount) {
            ReadBatteryAdcData();
            return;
        }

        // Nếu dữ liệu nguồn pin đủ, dữ liệu nguồn pin sẽ được đọc sau mỗi tích tắc kBatteryAdcInterval
        ticks_++;
        if (ticks_ % kBatteryAdcInterval == 0) {
            ReadBatteryAdcData();
        }
    }

    void ReadBatteryAdcData() {
        int adc_raw = 0;
        int voltage_mv = 0;  // Điện áp hiệu chỉnh ADC (mV)
        
        // Lấy trung bình nhiều mẫu để cải thiện độ ổn định
        uint32_t adc_sum = 0;
        const int sample_count = 10;
        for (int i = 0; i < sample_count; i++) {
            int temp_raw = 0;
            ESP_ERROR_CHECK(adc_oneshot_read(adc_handle_, ADC_CHANNEL_5, &temp_raw));
            adc_sum += temp_raw;
            vTaskDelay(pdMS_TO_TICKS(10));  // Mỗi khoảng thời gian lấy mẫu là 10ms
        }
        adc_raw = adc_sum / sample_count;
        
        // Nhận điện áp chính xác bằng cách sử dụng hiệu chuẩn ADC
        if (adc_cali_handle_) {
            ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_handle_, adc_raw, &voltage_mv));
        } else {
            // Nếu không được hiệu chỉnh, hãy sử dụng tính toán tuyến tính
            voltage_mv = (int)(adc_raw * 3300.0f / 4095.0f);
        }
        
        // Tính điện áp thực tế của ắc quy dựa vào tỷ lệ chia điện áp
        // Tỉ số chia điện áp mạch: R21/(R20+R21) = 100K/300K = 1/3
        // Điện áp pin thực tế = Điện áp đo được ADC × 3
        int battery_voltage_mv = voltage_mv * 3;
        
        // Thêm giá trị điện áp vào hàng đợi để làm mịn
        adc_values_.push_back(battery_voltage_mv);
        if (adc_values_.size() > kBatteryAdcDataCount) {
            adc_values_.erase(adc_values_.begin());
        }
        
        uint32_t average_voltage = 0;
        for (auto value : adc_values_) {
            average_voltage += value;
        }
        average_voltage /= adc_values_.size();

        // Xác định phạm vi năng lượng của pin (dựa trên điện áp pin thực tế, đơn vị mV)
        const struct {
            uint16_t voltage_mv;  // Điện áp pin (mV)
            uint8_t level;        // Phần trăm pin
        } levels[] = {
            {3500, 0},    // 3.5V
            {3640, 20},   // 3.64V
            {3760, 40},   // 3.76V
            {3880, 60},   // 3.88V
            {4000, 80},   // 4.0V
            {4200, 100}   // 4.2V
        };

        // dưới giá trị tối thiểu
        if (average_voltage < levels[0].voltage_mv) {
            battery_level_ = 0;
        }
        // khi cao hơn giá trị tối đa
        else if (average_voltage >= levels[5].voltage_mv) {
            battery_level_ = 100;
        } else {
            // Nội suy tuyến tính để tính giá trị trung gian
            for (int i = 0; i < 5; i++) {
                if (average_voltage >= levels[i].voltage_mv && average_voltage < levels[i+1].voltage_mv) {
                    float ratio = static_cast<float>(average_voltage - levels[i].voltage_mv) / 
                                  (levels[i+1].voltage_mv - levels[i].voltage_mv);
                    battery_level_ = levels[i].level + ratio * (levels[i+1].level - levels[i].level);
                    break;
                }
            }
        }

        // Kiểm tra tình trạng pin yếu
        if (adc_values_.size() >= kBatteryAdcDataCount) {
            bool new_low_battery_status = battery_level_ <= kLowBatteryLevel;
            if (new_low_battery_status != is_low_battery_) {
                is_low_battery_ = new_low_battery_status;
                if (on_low_battery_status_changed_) {
                    on_low_battery_status_changed_(is_low_battery_);
                }
            }
        }

        ESP_LOGI("PowerManager", "ADC raw: %d, ADC voltage: %dmV, Battery: %ldmV (%.2fV), level: %ld%%", 
                 adc_raw, voltage_mv, average_voltage, average_voltage/1000.0f, battery_level_);
    }

public:
    // Trình xây dựng: Sử dụng bộ điều khiển ADC bên ngoài (để sử dụng lại ADC hiện có)
    PowerManager(gpio_num_t pin, adc_oneshot_unit_handle_t* external_adc_handle = nullptr) 
        : charging_pin_(pin), adc_handle_owned_(false) {
        if(charging_pin_ != GPIO_NUM_NC){
            // Khởi tạo pin sạc
            gpio_config_t io_conf = {};
            io_conf.intr_type = GPIO_INTR_DISABLE;
            io_conf.mode = GPIO_MODE_INPUT;
            io_conf.pin_bit_mask = (1ULL << charging_pin_);
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; 
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;     
            gpio_config(&io_conf);
        }
        
        // Tạo bộ hẹn giờ kiểm tra pin
        esp_timer_create_args_t timer_args = {
            .callback = [](void* arg) {
                PowerManager* self = static_cast<PowerManager*>(arg);
                self->CheckBatteryStatus();
            },
            .arg = this,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "battery_check_timer",
            .skip_unhandled_events = true,
        };
        ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer_handle_));
        ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handle_, 1000000));

        // Khởi tạo hoặc tái sử dụng ADC
        if (external_adc_handle != nullptr && *external_adc_handle != nullptr) {
            // Tái sử dụng tay cầm ADC bên ngoài
            adc_handle_ = *external_adc_handle;
            adc_handle_owned_ = false;
        } else {
            // Tạo tay cầm ADC mới
            adc_oneshot_unit_init_cfg_t init_config = {
                .unit_id = ADC_UNIT_1,  // GPIO6 tương ứng với ADC1
                .ulp_mode = ADC_ULP_MODE_DISABLE,
            };
            ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle_));
            adc_handle_owned_ = true;
        }
        
        // Định cấu hình kênh ADC
        adc_oneshot_chan_cfg_t chan_config = {
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_12,
        };
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle_, ADC_CHANNEL_5, &chan_config));  // GPIO6 = ADC1_CHANNEL_5
        
        // Khởi tạo hiệu chuẩn ADC
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = ADC_UNIT_1,
            .chan = ADC_CHANNEL_5,
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_12,
        };
        esp_err_t ret = adc_cali_create_scheme_curve_fitting(&cali_config, &adc_cali_handle_);
        if (ret == ESP_OK) {
            ESP_LOGI("PowerManager", "ADC calibration initialized successfully");
        } else {
            ESP_LOGW("PowerManager", "ADC calibration failed, using linear calculation");
            adc_cali_handle_ = nullptr;
        }
    }

    ~PowerManager() {
        if (timer_handle_) {
            esp_timer_stop(timer_handle_);
            esp_timer_delete(timer_handle_);
        }
        // Xóa tay cầm hiệu chuẩn ADC
        if (adc_cali_handle_) {
            adc_cali_delete_scheme_curve_fitting(adc_cali_handle_);
        }
        // Chỉ xóa tay cầm ADC khi nó được tạo bởi lớp này
        if (adc_handle_ && adc_handle_owned_) {
            adc_oneshot_del_unit(adc_handle_);
        }
    }

    bool IsCharging() {
        // Nếu pin đầy, sạc sẽ không còn hiển thị.
        if (battery_level_ == 100) {
            return false;
        }
        return is_charging_;
    }

    bool IsDischarging() {
        // Không có sự phân biệt giữa sạc và xả nên nó trực tiếp trở về trạng thái ngược lại.
        return !is_charging_;
    }

    uint8_t GetBatteryLevel() {
        return battery_level_;
    }

    void OnLowBatteryStatusChanged(std::function<void(bool)> callback) {
        on_low_battery_status_changed_ = callback;
    }

    void OnChargingStatusChanged(std::function<void(bool)> callback) {
        on_charging_status_changed_ = callback;
    }
};
