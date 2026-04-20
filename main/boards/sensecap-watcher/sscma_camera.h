#ifndef SSCMA_CAMERA_H
#define SSCMA_CAMERA_H

#include <cstdint>
#include <lvgl.h>
#include <thread>
#include <memory>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <esp_io_expander_tca95xx_16bit.h>
#include <esp_jpeg_dec.h>
#include <mbedtls/base64.h>

#include "sscma_client.h"
#include "camera.h"

struct SscmaData {
    uint8_t* img;
    size_t len;
};
struct JpegData {
    uint8_t* buf;
    size_t len;
};

class SscmaCamera : public Camera {
private:
    lv_img_dsc_t preview_image_;
    std::string explain_url_;
    std::string explain_token_;
    sscma_client_io_handle_t sscma_client_io_handle_;
    sscma_client_handle_t sscma_client_handle_;
    QueueHandle_t sscma_data_queue_;
    JpegData jpeg_data_;
    jpeg_dec_handle_t jpeg_dec_;
    jpeg_dec_io_t *jpeg_io_;
    jpeg_dec_header_info_t *jpeg_out_;
    // Phát hiện máy trạng thái
    enum DetectionState {
        IDLE,           // trạng thái nhàn rỗi
        VALIDATING,     // Đang xác minh (phát hiện liên tục trong 3 giây)
        COOLDOWN        // Thời gian hạ nhiệt (chờ phát hiện lại)
    };
    
    DetectionState detection_state = IDLE;
    int64_t state_start_time = 0;
    bool need_start_cooldown = false; // Có cần thiết phải bắt đầu một giai đoạn nghỉ ngơi không?
    int64_t last_detected_time = 0; // Thời gian đối tượng được phát hiện lần cuối trong quá trình xác minh
    
    int detect_target = 0;
    int detect_threshold = 75;
    int detect_duration_sec = 2; // Thời lượng phát hiện là 2 giây, xác nhận sự hiện diện liên tục của người đó
    int detect_invoke_interval_sec = 8; // Thời gian hồi chiêu mặc định là 15 giây để tránh bắt đầu phiên thường xuyên
    int detect_debounce_sec = 1; // Thời gian gỡ lỗi cho nhân viên rời đi trong quá trình xác minh là 1 giây
    int inference_en = 0; // Công tắc kích hoạt suy luận (0: tắt, 1: bật)
    bool sscma_restarted_ = false;
    
    sscma_client_model_t *model;
    int model_class_cnt = 0;
public:
    SscmaCamera(esp_io_expander_handle_t io_exp_handle);
    ~SscmaCamera();
    void InitializeMcpTools();

    virtual void SetExplainUrl(const std::string& url, const std::string& token);
    virtual bool Capture();
    // chức năng điều khiển lật
    virtual bool SetHMirror(bool enabled) override;
    virtual bool SetVFlip(bool enabled) override;
    virtual std::string Explain(const std::string& question);

};

#endif // ESP32_CAMERA_H
