#ifndef __OTTO_MOVEMENTS_H__
#define __OTTO_MOVEMENTS_H__

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
#define LEFT_LEG 0
#define RIGHT_LEG 1
#define LEFT_FOOT 2
#define RIGHT_FOOT 3
#define LEFT_HAND 4
#define RIGHT_HAND 5
#define SERVO_COUNT 6

class Otto {
public:
    Otto();
    ~Otto();

    // -- Khởi tạo Otto
    void Init(int left_leg, int right_leg, int left_foot, int right_foot, int left_hand = -1,
              int right_hand = -1);
    // - Chức năng gắn và tách
    void AttachServos();
    void DetachServos();

    // -- Bộ dao động
    void SetTrims(int left_leg, int right_leg, int left_foot, int right_foot, int left_hand = 0,
                  int right_hand = 0);

    // -- Chức năng chuyển động được xác định trước
    void MoveServos(int time, int servo_target[]);
    void MoveSingle(int position, int servo_number);
    void OscillateServos(int amplitude[SERVO_COUNT], int offset[SERVO_COUNT], int period,
                         double phase_diff[SERVO_COUNT], float cycle);
    void Execute2(int amplitude[SERVO_COUNT], int center_angle[SERVO_COUNT], int period,
                  double phase_diff[SERVO_COUNT], float steps);

    // -- HOME = Otto ở vị trí nghỉ
    void Home(bool hands_down = true);
    bool GetRestState();
    void SetRestState(bool state);

    // -- Chức năng chuyển động được xác định trước
    void Jump(float steps = 1, int period = 2000);

    void Walk(float steps = 4, int period = 1000, int dir = FORWARD, int amount = 0);
    void Turn(float steps = 4, int period = 2000, int dir = LEFT, int amount = 0);
    void Bend(int steps = 1, int period = 1400, int dir = LEFT);
    void ShakeLeg(int steps = 1, int period = 2000, int dir = RIGHT);
    void Sit();  // ngồi xuống

    void UpDown(float steps = 1, int period = 1000, int height = 20);
    void Swing(float steps = 1, int period = 1000, int height = 20);
    void TiptoeSwing(float steps = 1, int period = 900, int height = 20);
    void Jitter(float steps = 1, int period = 500, int height = 20);
    void AscendingTurn(float steps = 1, int period = 900, int height = 20);

    void Moonwalker(float steps = 1, int period = 900, int height = 20, int dir = LEFT);
    void Crusaito(float steps = 1, int period = 900, int height = 20, int dir = FORWARD);
    void Flapping(float steps = 1, int period = 1000, int height = 20, int dir = FORWARD);
    void WhirlwindLeg(float steps = 1, int period = 300, int amplitude = 30);

    // - động tác tay
    void HandsUp(int period = 1000, int dir = 0);      // giơ tay lên
    void HandsDown(int period = 1000, int dir = 0);    // bỏ tay xuống
    void HandWave(int dir = LEFT);  // sóng
    void Windmill(float steps = 10, int period = 500, int amplitude = 90);  // cối xay gió lớn
    void Takeoff(float steps = 5, int period = 300, int amplitude = 40);   // cởi
    void Fitness(float steps = 5, int period = 1000, int amplitude = 25);  // sự thích hợp
    void Greeting(int dir = LEFT, float steps = 5);  // chào
    void Shy(int dir = LEFT, float steps = 5);  // xấu hổ
    void RadioCalisthenics();  // thể dục đài phát thanh
    void MagicCircle();  // Sự kỳ diệu của tình yêu cứ quay vòng
    void Showcase();  // Hiển thị hành động (nối nhiều hành động)

    // -- Bộ hạn chế servo
    void EnableServoLimit(int speed_limit_degree_per_sec = SERVO_LIMIT_DEFAULT);
    void DisableServoLimit();

private:
    Oscillator servo_[SERVO_COUNT];

    int servo_pins_[SERVO_COUNT];
    int servo_trim_[SERVO_COUNT];

    unsigned long final_time_;
    unsigned long partial_time_;
    float increment_[SERVO_COUNT];

    bool is_otto_resting_;
    bool has_hands_;  // Có servo tay không?

    void Execute(int amplitude[SERVO_COUNT], int offset[SERVO_COUNT], int period,
                 double phase_diff[SERVO_COUNT], float steps);

};

#endif  // __OTTO_MOVEMENTS_H__