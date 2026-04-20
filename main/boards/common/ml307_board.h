#ifndef ML307_BOARD_H
#define ML307_BOARD_H

#include <memory>
#include <at_modem.h>
#include "board.h"


class Ml307Board : public Board {
protected:
    std::unique_ptr<AtModem> modem_;
    gpio_num_t tx_pin_;
    gpio_num_t rx_pin_;
    gpio_num_t dtr_pin_;
    NetworkEventCallback network_event_callback_;

    virtual std::string GetBoardJson() override;

    // Trình trợ giúp nội bộ để kích hoạt gọi lại sự kiện mạng
    void OnNetworkEvent(NetworkEvent event, const std::string& data = "");
    
    // Tác vụ khởi tạo mạng (chạy trong tác vụ FreeRTOS)
    static void NetworkTaskEntry(void* arg);
    void NetworkTask();

public:
    Ml307Board(gpio_num_t tx_pin, gpio_num_t rx_pin, gpio_num_t dtr_pin = GPIO_NUM_NC);
    virtual std::string GetBoardType() override;
    virtual void StartNetwork() override;
    virtual void SetNetworkEventCallback(NetworkEventCallback callback) override;
    virtual NetworkInterface* GetNetwork() override;
    virtual const char* GetNetworkStateIcon() override;
    virtual void SetPowerSaveLevel(PowerSaveLevel level) override;
    virtual AudioCodec* GetAudioCodec() override { return nullptr; }
    virtual std::string GetDeviceStatusJson() override;
};

#endif // ML307_BOARD_H
