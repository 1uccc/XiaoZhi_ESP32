#ifndef __MOVEMENTS_H__
#define __MOVEMENTS_H__

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "oscillator.h"

// -- Hằng số
#define FORWARD 1
#define BACKWARD -1
#define LEFT 1
#define RIGHT -1
#define BOTH 0
#define SMALL 5
#define MEDIUM 15
#define BIG 30

// -- Mặc định giới hạn đồng bằng servo. độ / giây
#define SERVO_LIMIT_DEFAULT 240

// -- Chỉ số servo để dễ dàng truy cập
#define RIGHT_PITCH 0
#define RIGHT_ROLL 1
#define LEFT_PITCH 2
#define LEFT_ROLL 3
#define BODY 4
#define HEAD 5
#define SERVO_COUNT 6

class Otto {
public:
    Otto();
    ~Otto();

    // -- Khởi tạo Otto
    void Init(int right_pitch, int right_roll, int left_pitch, int left_roll, int body, int head);
    // - Chức năng gắn và tách
    void AttachServos();
    void DetachServos();

    // -- Bộ dao động
    void SetTrims(int right_pitch, int right_roll, int left_pitch, int left_roll, int body,
                  int head);

    // -- Chức năng chuyển động được xác định trước
    void MoveServos(int time, int servo_target[]);
    void MoveSingle(int position, int servo_number);
    void OscillateServos(int amplitude[SERVO_COUNT], int offset[SERVO_COUNT], int period,
                         double phase_diff[SERVO_COUNT], float cycle);

    // -- HOME = Otto ở vị trí nghỉ
    void Home(bool hands_down = true);
    bool GetRestState();
    void SetRestState(bool state);

    // - động tác tay
    void HandAction(int action, int times = 1, int amount = 30, int period = 1000);
    // động tác: 1=giơ tay trái, 2=giơ tay phải, 3=giơ cả hai tay, 4=đặt tay trái, 5=đặt tay phải, 6=đặt cả hai tay, 7=vẫy tay trái, 8=v vẫy tay phải,
    // 9=vẫy cả hai tay, 10=tát tay trái, 11=tát tay phải, 12=tát cả hai tay

    // - chuyển động cơ thể
    void BodyAction(int action, int times = 1, int amount = 30, int period = 1000);
    // hành động: 1=rẽ trái, 2=rẽ phải

    // -- Chuyển động của đầu
    void HeadAction(int action, int times = 1, int amount = 10, int period = 500);
    // hành động: 1=ngẩng đầu lên, 2=cúi đầu xuống, 3=gật đầu, 4=trở về giữa, 5=gật đầu liên tục

private:
    Oscillator servo_[SERVO_COUNT];

    int servo_pins_[SERVO_COUNT];
    int servo_trim_[SERVO_COUNT];
    int servo_initial_[SERVO_COUNT] = {180, 180, 0, 0, 90, 90};

    unsigned long final_time_;
    unsigned long partial_time_;
    float increment_[SERVO_COUNT];

    bool is_otto_resting_;

    void Execute(int amplitude[SERVO_COUNT], int offset[SERVO_COUNT], int period,
                 double phase_diff[SERVO_COUNT], float steps);
};

#endif  // __MOVEMENTS_H__