#ifndef DUAL_NETWORK_BOARD_H
#define DUAL_NETWORK_BOARD_H

#include "board.h"
#include "wifi_board.h"
#include "ml307_board.h"
#include <memory>

// enum Loại mạng
enum class NetworkType {
    WIFI,
    ML307
};

// Loại bảng mạng kép, có thể chuyển đổi giữa WiFi và ML307
class DualNetworkBoard : public Board {
private:
    // Sử dụng con trỏ lớp cơ sở để lưu trữ bảng hiện đang hoạt động
    std::unique_ptr<Board> current_board_;
    NetworkType network_type_ = NetworkType::ML307;  // Mặc định là ML307

    // Cấu hình chân ML307
    gpio_num_t ml307_tx_pin_;
    gpio_num_t ml307_rx_pin_;
    gpio_num_t ml307_dtr_pin_;
    
    // Tải loại mạng từ Cài đặt
    NetworkType LoadNetworkTypeFromSettings(int32_t default_net_type);
    
    // Lưu loại mạng vào Cài đặt
    void SaveNetworkTypeToSettings(NetworkType type);

    // Khởi tạo board tương ứng với loại mạng hiện tại
    void InitializeCurrentBoard();
 
public:
    DualNetworkBoard(gpio_num_t ml307_tx_pin, gpio_num_t ml307_rx_pin, gpio_num_t ml307_dtr_pin = GPIO_NUM_NC, int32_t default_net_type = 1);
    virtual ~DualNetworkBoard() = default;
 
    // Chuyển đổi loại mạng
    void SwitchNetworkType();
    
    // Nhận loại mạng hiện tại
    NetworkType GetNetworkType() const { return network_type_; }
    
    // Lấy tham chiếu bảng hiện đang hoạt động
    Board& GetCurrentBoard() const { return *current_board_; }
    
    // Viết lại giao diện Board
    virtual std::string GetBoardType() override;
    virtual void StartNetwork() override;
    virtual void SetNetworkEventCallback(NetworkEventCallback callback) override;
    virtual NetworkInterface* GetNetwork() override;
    virtual const char* GetNetworkStateIcon() override;
    virtual void SetPowerSaveLevel(PowerSaveLevel level) override;
    virtual std::string GetBoardJson() override;
    virtual std::string GetDeviceStatusJson() override;
};

#endif // DUAL_NETWORK_BOARD_H