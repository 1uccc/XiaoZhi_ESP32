#ifndef BOARD_H
#define BOARD_H

#include <http.h>
#include <web_socket.h>
#include <mqtt.h>
#include <udp.h>
#include <string>
#include <functional>
#include <network_interface.h>

#include "led/led.h"
#include "backlight.h"
#include "camera.h"
#include "assets.h"

/* *
 * Sự kiện mạng để gọi lại thống nhất */
enum class NetworkEvent {
    Scanning,              // Mạng đang quét (quét WiFi, v.v.)
    Connecting,            // Mạng đang kết nối (dữ liệu: SSID/tên mạng)
    Connected,             // Đã kết nối mạng thành công (dữ liệu: SSID/tên mạng)
    Disconnected,          // Mạng bị ngắt kết nối
    WifiConfigModeEnter,   // Đã vào chế độ cấu hình WiFi
    WifiConfigModeExit,    // Đã thoát chế độ cấu hình WiFi
    // Sự kiện cụ thể của modem di động
    ModemDetecting,        // Phát hiện modem (tốc độ truyền, loại mô-đun)
    ModemErrorNoSim,       // Không phát hiện thấy thẻ SIM
    ModemErrorRegDenied,   // Đăng ký mạng bị từ chối
    ModemErrorInitFailed,  // Khởi tạo modem không thành công
    ModemErrorTimeout      // Hết thời gian hoạt động
};

// Bảng liệt kê mức tiết kiệm năng lượng
enum class PowerSaveLevel {
    LOW_POWER,    // Tiết kiệm điện năng tối đa (tiêu thụ điện năng thấp nhất)
    BALANCED,     // Tiết kiệm điện năng trung bình (cân bằng)
    PERFORMANCE,  // Không tiết kiệm điện năng (tiêu thụ điện năng tối đa/hiệu suất tối đa)
};

// Loại gọi lại sự kiện mạng (sự kiện, dữ liệu)
// dữ liệu chứa thông tin bổ sung như SSID cho các sự kiện Kết nối/Đã kết nối
using NetworkEventCallback = std::function<void(NetworkEvent event, const std::string& data)>;

void* create_board();
class AudioCodec;
class Display;
class Board {
private:
    Board(const Board&) = delete; // Vô hiệu hóa hàm tạo bản sao
    Board& operator=(const Board&) = delete; // Vô hiệu hóa các hoạt động gán

protected:
    Board();
    std::string GenerateUuid();

    // Mã định danh duy nhất của thiết bị do phần mềm tạo ra
    std::string uuid_;

public:
    static Board& GetInstance() {
        static Board* instance = static_cast<Board*>(create_board());
        return *instance;
    }

    virtual ~Board() = default;
    virtual std::string GetBoardType() = 0;
    virtual std::string GetUuid() { return uuid_; }
    virtual Backlight* GetBacklight() { return nullptr; }
    virtual Led* GetLed();
    virtual AudioCodec* GetAudioCodec() = 0;
    virtual bool GetTemperature(float& esp32temp);
    virtual Display* GetDisplay();
    virtual Camera* GetCamera();
    virtual NetworkInterface* GetNetwork() = 0;
    virtual void StartNetwork() = 0;
    virtual void SetNetworkEventCallback(NetworkEventCallback callback) { (void)callback; }
    virtual const char* GetNetworkStateIcon() = 0;
    virtual bool GetBatteryLevel(int &level, bool& charging, bool& discharging);
    virtual std::string GetSystemInfoJson();
    virtual void SetPowerSaveLevel(PowerSaveLevel level) = 0;
    virtual std::string GetBoardJson() = 0;
    virtual std::string GetDeviceStatusJson() = 0;
};

#define DECLARE_BOARD(BOARD_CLASS_NAME) \
void* create_board() { \
    return new BOARD_CLASS_NAME(); \
}

#endif // BOARD_H
