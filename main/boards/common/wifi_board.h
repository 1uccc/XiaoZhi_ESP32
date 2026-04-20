#ifndef WIFI_BOARD_H
#define WIFI_BOARD_H

#include "board.h"
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <esp_timer.h>

class WifiBoard : public Board {
protected:
    esp_timer_handle_t connect_timer_ = nullptr;
    bool in_config_mode_ = false;
    NetworkEventCallback network_event_callback_ = nullptr;

    virtual std::string GetBoardJson() override;

    /* *
     * Xử lý sự kiện mạng (được gọi từ cuộc gọi lại của trình quản lý WiFi)
     * Sự kiện @param Loại sự kiện mạng
     * @param data Dữ liệu bổ sung (ví dụ: SSID cho các sự kiện Kết nối/Đã kết nối) */
    void OnNetworkEvent(NetworkEvent event, const std::string& data = "");

    /* *
     * Bắt đầu thử kết nối WiFi */
    void TryWifiConnect();

    /* *
     * Vào chế độ cấu hình WiFi */
    void StartWifiConfigMode();

    /* *
     * Gọi lại khi hết thời gian kết nối WiFi */
    static void OnWifiConnectTimeout(void* arg);

public:
    WifiBoard();
    virtual ~WifiBoard();
    
    virtual std::string GetBoardType() override;
    
    /* *
     * Bắt đầu kết nối mạng không đồng bộ
     * Hàm này trả về ngay lập tức. Các sự kiện mạng được thông báo thông qua lệnh gọi lại do SetNetworkEventCallback() đặt. */
    virtual void StartNetwork() override;
    
    virtual NetworkInterface* GetNetwork() override;
    virtual void SetNetworkEventCallback(NetworkEventCallback callback) override;
    virtual const char* GetNetworkStateIcon() override;
    virtual void SetPowerSaveLevel(PowerSaveLevel level) override;
    virtual AudioCodec* GetAudioCodec() override { return nullptr; }
    virtual std::string GetDeviceStatusJson() override;
    
    /* *
     * Vào chế độ cấu hình WiFi (an toàn luồng, có thể được gọi từ bất kỳ tác vụ nào) */
    void EnterWifiConfigMode();
    
    /* *
     * Kiểm tra xem ở chế độ cấu hình WiFi */
    bool IsInWifiConfigMode() const;
};

#endif // WIFI_BOARD_H
