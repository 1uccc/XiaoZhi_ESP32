#include "otto_movements.h"

#include <algorithm>

#include "freertos/idf_additions.h"
#include "oscillator.h"

static const char* TAG = "OttoMovements";

#define HAND_HOME_POSITION 45

Otto::Otto() {
    is_otto_resting_ = false;
    has_hands_ = false;
    // Khởi tạo tất cả các chân servo thành -1 (không được kết nối)
    for (int i = 0; i < SERVO_COUNT; i++) {
        servo_pins_[i] = -1;
        servo_trim_[i] = 0;
    }
}

Otto::~Otto() {
    DetachServos();
}

unsigned long IRAM_ATTR millis() {
    return (unsigned long)(esp_timer_get_time() / 1000ULL);
}

void Otto::Init(int left_leg, int right_leg, int left_foot, int right_foot, int left_hand,
                int right_hand) {
    servo_pins_[LEFT_LEG] = left_leg;
    servo_pins_[RIGHT_LEG] = right_leg;
    servo_pins_[LEFT_FOOT] = left_foot;
    servo_pins_[RIGHT_FOOT] = right_foot;
    servo_pins_[LEFT_HAND] = left_hand;
    servo_pins_[RIGHT_HAND] = right_hand;

    // Kiểm tra xem có servo tay không
    has_hands_ = (left_hand != -1 && right_hand != -1);

    AttachServos();
    is_otto_resting_ = false;
}

// ///////////////////////////////////////////////////////////////
// -- CHỨC NĂNG GẮN & THÁO ----------------------------------//
// ///////////////////////////////////////////////////////////////
void Otto::AttachServos() {
    for (int i = 0; i < SERVO_COUNT; i++) {
        if (servo_pins_[i] != -1) {
            servo_[i].Attach(servo_pins_[i]);
        }
    }
}

void Otto::DetachServos() {
    for (int i = 0; i < SERVO_COUNT; i++) {
        if (servo_pins_[i] != -1) {
            servo_[i].Detach();
        }
    }
}

// ///////////////////////////////////////////////////////////////
// -- Bộ dao động TRIMS ------------------------------------------//
// ///////////////////////////////////////////////////////////////
void Otto::SetTrims(int left_leg, int right_leg, int left_foot, int right_foot, int left_hand,
                    int right_hand) {
    servo_trim_[LEFT_LEG] = left_leg;
    servo_trim_[RIGHT_LEG] = right_leg;
    servo_trim_[LEFT_FOOT] = left_foot;
    servo_trim_[RIGHT_FOOT] = right_foot;

    if (has_hands_) {
        servo_trim_[LEFT_HAND] = left_hand;
        servo_trim_[RIGHT_HAND] = right_hand;
    }

    for (int i = 0; i < SERVO_COUNT; i++) {
        if (servo_pins_[i] != -1) {
            servo_[i].SetTrim(servo_trim_[i]);
        }
    }
}

// ///////////////////////////////////////////////////////////////
// -- CHỨC NĂNG CHUYỂN ĐỘNG CƠ BẢN -------------------------------------//
// ///////////////////////////////////////////////////////////////
void Otto::MoveServos(int time, int servo_target[]) {
    if (GetRestState() == true) {
        SetRestState(false);
    }

    final_time_ = millis() + time;
    if (time > 10) {
        for (int i = 0; i < SERVO_COUNT; i++) {
            if (servo_pins_[i] != -1) {
                increment_[i] = (servo_target[i] - servo_[i].GetPosition()) / (time / 10.0);
            }
        }

        for (int iteration = 1; millis() < final_time_; iteration++) {
            partial_time_ = millis() + 10;
            for (int i = 0; i < SERVO_COUNT; i++) {
                if (servo_pins_[i] != -1) {
                    servo_[i].SetPosition(servo_[i].GetPosition() + increment_[i]);
                }
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    } else {
        for (int i = 0; i < SERVO_COUNT; i++) {
            if (servo_pins_[i] != -1) {
                servo_[i].SetPosition(servo_target[i]);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(time));
    }

    // điều chỉnh cuối cùng cho mục tiêu.
    bool f = true;
    int adjustment_count = 0;
    while (f && adjustment_count < 10) {
        f = false;
        for (int i = 0; i < SERVO_COUNT; i++) {
            if (servo_pins_[i] != -1 && servo_target[i] != servo_[i].GetPosition()) {
                f = true;
                break;
            }
        }
        if (f) {
            for (int i = 0; i < SERVO_COUNT; i++) {
                if (servo_pins_[i] != -1) {
                    servo_[i].SetPosition(servo_target[i]);
                }
            }
            vTaskDelay(pdMS_TO_TICKS(10));
            adjustment_count++;
        }
    };
}

void Otto::MoveSingle(int position, int servo_number) {
    if (position > 180)
        position = 90;
    if (position < 0)
        position = 90;

    if (GetRestState() == true) {
        SetRestState(false);
    }

    if (servo_number >= 0 && servo_number < SERVO_COUNT && servo_pins_[servo_number] != -1) {
        servo_[servo_number].SetPosition(position);
    }
}

void Otto::OscillateServos(int amplitude[SERVO_COUNT], int offset[SERVO_COUNT], int period,
                           double phase_diff[SERVO_COUNT], float cycle = 1) {
    for (int i = 0; i < SERVO_COUNT; i++) {
        if (servo_pins_[i] != -1) {
            servo_[i].SetO(offset[i]);
            servo_[i].SetA(amplitude[i]);
            servo_[i].SetT(period);
            servo_[i].SetPh(phase_diff[i]);
        }
    }

    double ref = millis();
    double end_time = period * cycle + ref;

    while (millis() < end_time) {
        for (int i = 0; i < SERVO_COUNT; i++) {
            if (servo_pins_[i] != -1) {
                servo_[i].Refresh();
            }
        }
        vTaskDelay(5);
    }
    vTaskDelay(pdMS_TO_TICKS(10));
}

void Otto::Execute(int amplitude[SERVO_COUNT], int offset[SERVO_COUNT], int period,
                   double phase_diff[SERVO_COUNT], float steps = 1.0) {
    if (GetRestState() == true) {
        SetRestState(false);
    }

    int cycles = (int)steps;

    // -- Thực hiện các chu trình hoàn chỉnh
    if (cycles >= 1)
        for (int i = 0; i < cycles; i++)
            OscillateServos(amplitude, offset, period, phase_diff);

    // -- Thực hiện chu trình chưa hoàn thành cuối cùng
    OscillateServos(amplitude, offset, period, phase_diff, (float)steps - cycles);
    vTaskDelay(pdMS_TO_TICKS(10));
}

// ---------------------------------------------------------
// -- Thực thi2: Lấy góc tuyệt đối làm tâm dao động
// -- Thông số:
// -- Biên độ: mảng biên độ (biên độ dao động của từng servo)
// -- center_angle: mảng góc tuyệt đối (0-180 độ), dùng làm vị trí tâm dao động
// -- thời gian: khoảng thời gian (mili giây)
// -- Phase_diff: mảng lệch pha (radian)
// -- bước: số bước/số chu kỳ (có thể là số thập phân)
// ---------------------------------------------------------
void Otto::Execute2(int amplitude[SERVO_COUNT], int center_angle[SERVO_COUNT], int period,
                    double phase_diff[SERVO_COUNT], float steps = 1.0) {
    if (GetRestState() == true) {
        SetRestState(false);
    }

    // Chuyển đổi góc tuyệt đối thành offset (offset = center_angle - 90)
    int offset[SERVO_COUNT];
    for (int i = 0; i < SERVO_COUNT; i++) {
        offset[i] = center_angle[i] - 90;
    }

    int cycles = (int)steps;

    // -- Thực hiện các chu trình hoàn chỉnh
    if (cycles >= 1)
        for (int i = 0; i < cycles; i++)
            OscillateServos(amplitude, offset, period, phase_diff);

    // -- Thực hiện chu trình chưa hoàn thành cuối cùng
    OscillateServos(amplitude, offset, period, phase_diff, (float)steps - cycles);
    vTaskDelay(pdMS_TO_TICKS(10));
}

// ///////////////////////////////////////////////////////////////
// -- TRANG CHỦ = Otto ở vị trí nghỉ -------------------------------//
// ///////////////////////////////////////////////////////////////
void Otto::Home(bool hands_down) {
    if (is_otto_resting_ == false) {  // Chỉ về vị trí nghỉ ngơi nếu cần thiết
        // Chuẩn bị giá trị vị trí ban đầu cho tất cả các servo
        int homes[SERVO_COUNT];
        for (int i = 0; i < SERVO_COUNT; i++) {
            if (i == LEFT_HAND || i == RIGHT_HAND) {
                if (hands_down) {
                    // Nếu cần thiết lập lại bằng tay, hãy đặt thành mặc định
                    if (i == LEFT_HAND) {
                        homes[i] = HAND_HOME_POSITION;
                    } else {                                  // RIGHT_HAND
                        homes[i] = 180 - HAND_HOME_POSITION;  // Vị trí gương bên phải
                    }
                } else {
                    // Nếu không cần thiết lập lại tay thì giữ nguyên vị trí hiện tại
                    homes[i] = servo_[i].GetPosition();
                }
            } else {
                // Động cơ chân và chân luôn được đặt lại
                homes[i] = 90;
            }
        }

        MoveServos(700, homes);
        is_otto_resting_ = true;
    }

    vTaskDelay(pdMS_TO_TICKS(200));
}

bool Otto::GetRestState() {
    return is_otto_resting_;
}

void Otto::SetRestState(bool state) {
    is_otto_resting_ = state;
}

// ///////////////////////////////////////////////////////////////
// -- CHUỖI CHUYỂN ĐỘNG ĐƯỢC XÁC ĐỊNH TRƯỚC -----------------------------//
// ///////////////////////////////////////////////////////////////
// -- Phong trào Otto: Nhảy
// -- Thông số:
// -- bước: Số bước
// -- T: Dấu chấm
// ---------------------------------------------------------
void Otto::Jump(float steps, int period) {
    int up[SERVO_COUNT] = {90, 90, 150, 30, HAND_HOME_POSITION, 180 - HAND_HOME_POSITION};
    MoveServos(period, up);
    int down[SERVO_COUNT] = {90, 90, 90, 90, HAND_HOME_POSITION, 180 - HAND_HOME_POSITION};
    MoveServos(period, down);
}

// ---------------------------------------------------------
// -- Dáng đi Otto: Đi bộ (tiến hoặc lùi)
// -- Thông số:
// -- * bước: Số bước
// -- * T : Dấu chấm
// -- * Chỉ thị: Hướng: TIẾN/LÙI
// -- * số lượng: Phạm vi vung tay, 0 nghĩa là không vung
// ---------------------------------------------------------
void Otto::Walk(float steps, int period, int dir, int amount) {
    // - Thông số dao động cho việc đi bộ
    // -- Sevos hông đang trong giai đoạn
    // -- Chân servo đang trong pha
    // -- Hông và bàn chân lệch pha 90 độ
    // -- -90 : Tiến về phía trước
    // -- 90 : Đi lùi
    // -- Chân servo cũng có độ lệch tương tự (cho nhón chân một chút)
    int A[SERVO_COUNT] = {30, 30, 30, 30, 0, 0};
    int O[SERVO_COUNT] = {0, 0, 5, -5, HAND_HOME_POSITION - 90, HAND_HOME_POSITION};
    double phase_diff[SERVO_COUNT] = {0, 0, DEG2RAD(dir * -90), DEG2RAD(dir * -90), 0, 0};

    // Nếu số lượng>0 và có tay điều khiển, hãy đặt biên độ và pha của tay
    if (amount > 0 && has_hands_) {
        // Biên độ cánh tay sử dụng tham số lượng được truyền vào
        A[LEFT_HAND] = amount;
        A[RIGHT_HAND] = amount;

        // Tay trái và chân phải cùng pha, tay phải và chân trái cùng pha nên cánh tay sẽ lắc lư tự nhiên khi robot bước đi.
        phase_diff[LEFT_HAND] = phase_diff[RIGHT_LEG];  // Tay trái và chân phải cùng một pha
        phase_diff[RIGHT_HAND] = phase_diff[LEFT_LEG];  // Tay phải và chân trái cùng pha
    } else {
        A[LEFT_HAND] = 0;
        A[RIGHT_HAND] = 0;
    }

    // -- Hãy dao động các servo!
    Execute(A, O, period, phase_diff, steps);
}

// ---------------------------------------------------------
// -- Dáng đi Otto: Xoay (trái hoặc phải)
// -- Thông số:
// -- * Steps: Số bước
// -- * T: Dấu chấm
// -- * Dir: Hướng: TRÁI/PHẢI
// -- * số lượng: Phạm vi vung tay, 0 nghĩa là không vung
// ---------------------------------------------------------
void Otto::Turn(float steps, int period, int dir, int amount) {
    // -- Phối hợp tương tự như khi đi bộ (xem Otto::walk)
    // -- Biên độ dao động của hông không phải là số nguyên
    // -- Khi biên độ servo hông bên phải cao hơn, các bước thực hiện
    // - chân phải to hơn chân trái. Vì vậy, robot mô tả một
    // -- cung trái
    int A[SERVO_COUNT] = {30, 30, 30, 30, 0, 0};
    int O[SERVO_COUNT] = {0, 0, 5, -5, HAND_HOME_POSITION - 90, HAND_HOME_POSITION};
    double phase_diff[SERVO_COUNT] = {0, 0, DEG2RAD(-90), DEG2RAD(-90), 0, 0};

    if (dir == LEFT) {
        A[0] = 30;  // - servo hông trái
        A[1] = 0;   // -- Phải hông servo
    } else {
        A[0] = 0;
        A[1] = 30;
    }

    // Nếu số lượng>0 và có tay điều khiển, hãy đặt biên độ và pha của tay
    if (amount > 0 && has_hands_) {
        // Biên độ cánh tay sử dụng tham số lượng được truyền vào
        A[LEFT_HAND] = amount;
        A[RIGHT_HAND] = amount;

        // Pha vung tay khi rẽ: tay trái và chân trái cùng pha, tay phải và chân phải cùng pha, tăng cường hiệu quả đánh lái
        phase_diff[LEFT_HAND] = phase_diff[LEFT_LEG];    // Tay trái và chân trái cùng một pha
        phase_diff[RIGHT_HAND] = phase_diff[RIGHT_LEG];  // Tay phải và chân phải cùng một pha
    } else {
        A[LEFT_HAND] = 0;
        A[RIGHT_HAND] = 0;
    }

    // -- Hãy dao động các servo!
    Execute(A, O, period, phase_diff, steps);
}

// ---------------------------------------------------------
// -- Dáng đi Otto: Uốn cong sang một bên
// -- Thông số:
// -- bước: Số lần uốn
// -- T: Chu kỳ một lần uốn
// -- dir: RIGHT=Uốn cong phải LEFT=Uốn cong trái
// ---------------------------------------------------------
void Otto::Bend(int steps, int period, int dir) {
    // Các thông số của tất cả các phong trào. Mặc định: Uốn cong trái
    int bend1[SERVO_COUNT] = {90, 90, 62, 35, HAND_HOME_POSITION, 180 - HAND_HOME_POSITION};
    int bend2[SERVO_COUNT] = {90, 90, 62, 105, HAND_HOME_POSITION, 180 - HAND_HOME_POSITION};
    int homes[SERVO_COUNT] = {90, 90, 90, 90, HAND_HOME_POSITION, 180 - HAND_HOME_POSITION};

    // Thời gian của một lần uốn cong, được hạn chế để tránh chuyển động quá nhanh.
    // T=max(T, 600);
    // Thay đổi các tham số nếu chọn đúng hướng
    if (dir == -1) {
        bend1[2] = 180 - 35;
        bend1[3] = 180 - 60;  // Không phải 65. Otto bị mất cân bằng
        bend2[2] = 180 - 105;
        bend2[3] = 180 - 60;
    }

    // Thời gian chuyển động uốn cong. Đã sửa tham số để tránh rơi
    int T2 = 800;

    // Chuyển động uốn cong
    for (int i = 0; i < steps; i++) {
        MoveServos(T2 / 2, bend1);
        MoveServos(T2 / 2, bend2);
        vTaskDelay(pdMS_TO_TICKS(period * 0.8));
        MoveServos(500, homes);
    }
}

// ---------------------------------------------------------
// -- Dáng đi Otto: Lắc một chân
// -- Thông số:
// -- bước: Số lần lắc
// -- T: Chu kỳ 1 lần rung
// Error 500 (Server Error)!!1500.That’s an error.There was an error. Please try again later.That’s all we know.
// ---------------------------------------------------------
void Otto::ShakeLeg(int steps, int period, int dir) {
    // Biến này thay đổi số lần lắc
    int numberLegMoves = 2;

    // Các thông số của tất cả các phong trào. Mặc định: Chân phải
    int shake_leg1[SERVO_COUNT] = {90, 90, 58, 35, HAND_HOME_POSITION, 180 - HAND_HOME_POSITION};
    int shake_leg2[SERVO_COUNT] = {90, 90, 58, 120, HAND_HOME_POSITION, 180 - HAND_HOME_POSITION};
    int shake_leg3[SERVO_COUNT] = {90, 90, 58, 60, HAND_HOME_POSITION, 180 - HAND_HOME_POSITION};
    int homes[SERVO_COUNT] = {90, 90, 90, 90, HAND_HOME_POSITION, 180 - HAND_HOME_POSITION};

    // Thay đổi các thông số nếu chọn chân trái
    if (dir == LEFT) {
        shake_leg1[2] = 180 - 35;
        shake_leg1[3] = 180 - 58;
        shake_leg2[2] = 180 - 120;
        shake_leg2[3] = 180 - 58;
        shake_leg3[2] = 180 - 60;
        shake_leg3[3] = 180 - 58;
    }

    // Thời gian chuyển động uốn cong. Đã sửa tham số để tránh rơi
    int T2 = 1000;
    // Thời gian của một lần lắc, được hạn chế để tránh chuyển động quá nhanh.
    period = period - T2;
    period = std::max(period, 200 * numberLegMoves);

    for (int j = 0; j < steps; j++) {
        // Chuyển động uốn cong
        MoveServos(T2 / 2, shake_leg1);
        MoveServos(T2 / 2, shake_leg2);

        // Lắc chuyển động
        for (int i = 0; i < numberLegMoves; i++) {
            MoveServos(period / (2 * numberLegMoves), shake_leg3);
            MoveServos(period / (2 * numberLegMoves), shake_leg2);
        }
        MoveServos(500, homes);  // Trở về vị trí ban đầu
    }

    vTaskDelay(pdMS_TO_TICKS(period));
}

// ---------------------------------------------------------
// -- Động tác Otto: Ngồi (ngồi xuống)
// ---------------------------------------------------------
void Otto::Sit() {
    int target[SERVO_COUNT] = {120, 60, 0, 180, 45, 135};
    MoveServos(600, target);
}

// ---------------------------------------------------------
// -- Phong trào Otto: lên xuống
// -- Thông số:
// -- * bước: Số lần nhảy
// -- * T: Dấu chấm
// -- * h: Độ cao nhảy: NHỎ/TRUNG BÌNH/LỚN
// -- (hoặc một số theo độ 0 - 90)
// ---------------------------------------------------------
void Otto::UpDown(float steps, int period, int height) {
    // -- Cả hai chân đều lệch pha 180 độ
    // - Biên độ và độ lệch của chân giống nhau
    // -- Pha ban đầu của chân phải là -90 nên bắt đầu
    // -- ở một vị trí cực đoan (không phải ở giữa)
    int A[SERVO_COUNT] = {0, 0, height, height, 0, 0};
    int O[SERVO_COUNT] = {0, 0, height, -height, HAND_HOME_POSITION, 180 - HAND_HOME_POSITION};
    double phase_diff[SERVO_COUNT] = {0, 0, DEG2RAD(-90), DEG2RAD(90), 0, 0};

    // -- Hãy dao động các servo!
    Execute(A, O, period, phase_diff, steps);
}

// ---------------------------------------------------------
// -- Phong trào Otto: lắc lư sang bên
// -- Thông số:
// -- bước: Số bước
// -- T : Chu kì
// -- h : Số lần xoay (khoảng từ 0 đến 50)
// ---------------------------------------------------------
void Otto::Swing(float steps, int period, int height) {
    // -- Cả hai chân đều đồng pha. Độ lệch bằng một nửa biên độ
    // -- Nó làm cho robot lắc lư từ bên này sang bên kia
    int A[SERVO_COUNT] = {0, 0, height, height, 0, 0};
    int O[SERVO_COUNT] = {
        0, 0, height / 2, -height / 2, HAND_HOME_POSITION, 180 - HAND_HOME_POSITION};
    double phase_diff[SERVO_COUNT] = {0, 0, DEG2RAD(0), DEG2RAD(0), 0, 0};

    // -- Hãy dao động các servo!
    Execute(A, O, period, phase_diff, steps);
}

// ---------------------------------------------------------
// -- Phong trào Otto: đu sang bên mà gót chân không chạm sàn
// -- Thông số:
// -- bước: Số bước
// -- T : Chu kì
// -- h : Số lần xoay (khoảng từ 0 đến 50)
// ---------------------------------------------------------
void Otto::TiptoeSwing(float steps, int period, int height) {
    // -- Cả hai chân đều đồng pha. Độ lệch không bằng một nửa biên độ để nhón chân
    // -- Nó làm cho robot lắc lư từ bên này sang bên kia
    int A[SERVO_COUNT] = {0, 0, height, height, 0, 0};
    int O[SERVO_COUNT] = {0, 0, height, -height, HAND_HOME_POSITION, 180 - HAND_HOME_POSITION};
    double phase_diff[SERVO_COUNT] = {0, 0, 0, 0, 0, 0};

    // -- Hãy dao động các servo!
    Execute(A, O, period, phase_diff, steps);
}

// ---------------------------------------------------------
// -- dáng đi Otto: giật giật
// -- Thông số:
// -- bước: Số lần giật
// -- T: Chu kỳ của một jitter
// -- h: chiều cao (Giá trị từ 5 - 25)
// ---------------------------------------------------------
void Otto::Jitter(float steps, int period, int height) {
    // -- Cả hai chân đều lệch pha 180 độ
    // - Biên độ và độ lệch của chân giống nhau
    // -- Pha ban đầu của chân phải là -90 nên bắt đầu
    // -- ở một vị trí cực đoan (không phải ở giữa)
    // -- h bị hạn chế để tránh va chạm vào chân
    height = std::min(25, height);
    int A[SERVO_COUNT] = {height, height, 0, 0, 0, 0};
    int O[SERVO_COUNT] = {0, 0, 0, 0, HAND_HOME_POSITION, 180 - HAND_HOME_POSITION};
    double phase_diff[SERVO_COUNT] = {DEG2RAD(-90), DEG2RAD(90), 0, 0, 0, 0};

    // -- Hãy dao động các servo!
    Execute(A, O, period, phase_diff, steps);
}

// ---------------------------------------------------------
// -- Dáng đi Otto: Đi lên & rẽ (Jitter khi lên & xuống)
// -- Thông số:
// -- bước: Số lần uốn
// -- T: Chu kỳ một lần uốn
// -- h: chiều cao (Giá trị từ 5 - 15)
// ---------------------------------------------------------
void Otto::AscendingTurn(float steps, int period, int height) {
    // -- Cả bàn chân và cẳng chân đều lệch pha 180 độ
    // -- Pha ban đầu của chân phải là -90 nên bắt đầu
    // -- ở một vị trí cực đoan (không phải ở giữa)
    // -- h bị hạn chế để tránh va chạm vào chân
    height = std::min(13, height);
    int A[SERVO_COUNT] = {height, height, height, height, 0, 0};
    int O[SERVO_COUNT] = {
        0, 0, height + 4, -height + 4, HAND_HOME_POSITION, 180 - HAND_HOME_POSITION};
    double phase_diff[SERVO_COUNT] = {DEG2RAD(-90), DEG2RAD(90), DEG2RAD(-90), DEG2RAD(90), 0, 0};

    // -- Hãy dao động các servo!
    Execute(A, O, period, phase_diff, steps);
}

// ---------------------------------------------------------
// -- Kiểu dáng Otto: Người đi trên mặt trăng. Otto di chuyển giống Michael Jackson
// -- Thông số:
// -- Steps: Số bước
// -- T: Dấu chấm
// --h: Chiều cao. Giá trị tiêu biểu từ 15 đến 40
// -- dir: Hướng: TRÁI / PHẢI
// ---------------------------------------------------------
void Otto::Moonwalker(float steps, int period, int height, int dir) {
    // -- Chuyển động này tương tự như chuyển động của robot sâu bướm: Một chuyển động di chuyển
    // - Sóng truyền từ bên này sang bên kia
    // -- Hai chân Otto tương đương với cấu hình tối thiểu. Nó được biết
    // -- 2 động cơ servo đó có thể di chuyển như một con sâu nếu chúng lệch pha 120 độ
    // -- Trong ví dụ của Otto, hai bàn chân đối xứng nhau nên ta có:
    // -- 180 - 120 = 60 độ. Độ lệch pha thực tế được cung cấp cho các bộ dao động
    // -- là 60 độ.
    // - Biên độ của cả hai đều bằng nhau. Độ lệch bằng một nửa biên độ cộng với một chút
    // - lệch sao cho robot nhón chân nhẹ nhàng

    int A[SERVO_COUNT] = {0, 0, height, height, 0, 0};
    int O[SERVO_COUNT] = {
        0, 0, height / 2 + 2, -height / 2 - 2, HAND_HOME_POSITION, 180 - HAND_HOME_POSITION};
    int phi = -dir * 90;
    double phase_diff[SERVO_COUNT] = {0, 0, DEG2RAD(phi), DEG2RAD(-60 * dir + phi), 0, 0};

    // -- Hãy dao động các servo!
    Execute(A, O, period, phase_diff, steps);
}

// ----------------------------------------------------------
// -- Kiểu dáng Otto: Crusaito. Sự kết hợp giữa moonwalker và đi bộ
// -- Thông số:
// -- bước: Số bước
// -- T: Dấu chấm
// -- h: chiều cao (Giá trị trong khoảng 20 - 50)
// -- dir: Hướng: TRÁI / PHẢI
// -----------------------------------------------------------
void Otto::Crusaito(float steps, int period, int height, int dir) {
    int A[SERVO_COUNT] = {25, 25, height, height, 0, 0};
    int O[SERVO_COUNT] = {
        0, 0, height / 2 + 4, -height / 2 - 4, HAND_HOME_POSITION, 180 - HAND_HOME_POSITION};
    double phase_diff[SERVO_COUNT] = {90, 90, DEG2RAD(0), DEG2RAD(-60 * dir), 0, 0};

    // -- Hãy dao động các servo!
    Execute(A, O, period, phase_diff, steps);
}

// ---------------------------------------------------------
// -- Kiểu dáng Otto: Vỗ cánh
// -- Thông số:
// -- bước: Số bước
// -- T: Dấu chấm
// -- h: chiều cao (Giá trị trong khoảng 10 - 30)
// -- dir: hướng: PHÍA TRƯỚC, LÙI
// ---------------------------------------------------------
void Otto::Flapping(float steps, int period, int height, int dir) {
    int A[SERVO_COUNT] = {12, 12, height, height, 0, 0};
    int O[SERVO_COUNT] = {
        0, 0, height - 10, -height + 10, HAND_HOME_POSITION, 180 - HAND_HOME_POSITION};
    double phase_diff[SERVO_COUNT] = {
        DEG2RAD(0), DEG2RAD(180), DEG2RAD(-90 * dir), DEG2RAD(90 * dir), 0, 0};

    // -- Hãy dao động các servo!
    Execute(A, O, period, phase_diff, steps);
}

// ---------------------------------------------------------
// -- Dáng đi Otto: WhirlwindLeg (chân gió lốc)
// -- Thông số:
// -- bước: Số bước
// -- thời gian: Khoảng thời gian (khuyến nghị 100-800 mili giây)
// -- Biên độ: biên độ (Giá trị trong khoảng 20 - 40)
// ---------------------------------------------------------
void Otto::WhirlwindLeg(float steps, int period, int amplitude) {


    int target[SERVO_COUNT] = {90, 90, 180, 90, 45, 20};
    MoveServos(100, target);
    target[RIGHT_FOOT] = 160;
    MoveServos(500, target);
    vTaskDelay(pdMS_TO_TICKS(1000));

    int C[SERVO_COUNT] = {90, 90, 180, 160, 45, 20};
    int A[SERVO_COUNT] = {amplitude, 0, 0, 0, amplitude, 0};
    double phase_diff[SERVO_COUNT] = {DEG2RAD(20), 0, 0, 0, DEG2RAD(20), 0};
    Execute2(A, C, period, phase_diff, steps);

}

// ---------------------------------------------------------
// --Động tác tay: Giơ tay
// -- Thông số:
// -- thời kỳ: thời gian hành động
// -- dir: hướng 1=tay trái, -1=tay phải, 0=cả hai tay
// ---------------------------------------------------------
void Otto::HandsUp(int period, int dir) {
    if (!has_hands_) {
        return;
    }

    int target[SERVO_COUNT] = {90, 90, 90, 90, HAND_HOME_POSITION, 180 - HAND_HOME_POSITION};

    if (dir == 0) {
        target[LEFT_HAND] = 170;
        target[RIGHT_HAND] = 10;
    } else if (dir == LEFT) {
        target[LEFT_HAND] = 170;
        target[RIGHT_HAND] = servo_[RIGHT_HAND].GetPosition();
    } else if (dir == RIGHT) {
        target[RIGHT_HAND] = 10;
        target[LEFT_HAND] = servo_[LEFT_HAND].GetPosition();
    }

    MoveServos(period, target);
}

// ---------------------------------------------------------
// --Động tác tay: hạ tay xuống
// -- Thông số:
// -- thời kỳ: thời gian hành động
// -- dir: hướng 1=tay trái, -1=tay phải, 0=cả hai tay
// ---------------------------------------------------------
void Otto::HandsDown(int period, int dir) {
    if (!has_hands_) {
        return;
    }

    int target[SERVO_COUNT] = {90, 90, 90, 90, HAND_HOME_POSITION, 180 - HAND_HOME_POSITION};

    if (dir == LEFT) {
        target[RIGHT_HAND] = servo_[RIGHT_HAND].GetPosition();
    } else if (dir == RIGHT) {
        target[LEFT_HAND] = servo_[LEFT_HAND].GetPosition();
    }

    MoveServos(period, target);
}

// ---------------------------------------------------------
// -Động tác tay: vẫy tay
// -- Thông số:
// -- dir: hướng TRÁI/PHẢI/Cả HAI
// ---------------------------------------------------------
void Otto::HandWave(int dir) {
    if (!has_hands_) {
        return;
    }
    if (dir == LEFT) {
        int center_angle[SERVO_COUNT] = {90, 90, 90, 90, 160, 135};
        int A[SERVO_COUNT] = {0, 0, 0, 0, 20, 0};
        double phase_diff[SERVO_COUNT] = {0, 0, 0, 0, DEG2RAD(90), 0};
        Execute2(A, center_angle, 300, phase_diff, 5);
    }
    else if (dir == RIGHT) {
        int center_angle[SERVO_COUNT] = {90, 90, 90, 90, 45, 20};
        int A[SERVO_COUNT] = {0, 0, 0, 0, 0, 20};
        double phase_diff[SERVO_COUNT] = {0, 0, 0, 0, 0, DEG2RAD(90)};
        Execute2(A, center_angle, 300, phase_diff, 5);
    }
    else {
        int center_angle[SERVO_COUNT] = {90, 90, 90, 90, 160, 20};
        int A[SERVO_COUNT] = {0, 0, 0, 0, 20, 20};
        double phase_diff[SERVO_COUNT] = {0, 0, 0, 0, DEG2RAD(90), DEG2RAD(90)};
        Execute2(A, center_angle, 300, phase_diff, 5);
    }
}


// ---------------------------------------------------------
// -- Động tác tay: Cối xay gió lớn
// -- Thông số:
// -- bước: số hành động
// -- thời gian: thời gian hành động (mili giây)
// -- biên độ: biên độ dao động (độ)
// ---------------------------------------------------------
void Otto::Windmill(float steps, int period, int amplitude) {
    if (!has_hands_) {
        return;
    }

    int center_angle[SERVO_COUNT] = {90, 90, 90, 90, 90, 90};
    int A[SERVO_COUNT] = {0, 0, 0, 0, amplitude, amplitude};
    double phase_diff[SERVO_COUNT] = {0, 0, 0, 0, DEG2RAD(90), DEG2RAD(90)};
    Execute2(A, center_angle, period, phase_diff, steps);
}

// ---------------------------------------------------------
// --Chuyển động tay: cất cánh
// -- Thông số:
// -- bước: số hành động
// -- thời gian: thời gian hành động (mili giây), giá trị càng nhỏ thì tốc độ càng nhanh
// -- biên độ: biên độ dao động (độ)
// ---------------------------------------------------------
void Otto::Takeoff(float steps, int period, int amplitude) {
    if (!has_hands_) {
        return;
    }

    Home(true);

    int center_angle[SERVO_COUNT] = {90, 90, 90, 90, 90, 90};
    int A[SERVO_COUNT] = {0, 0, 0, 0, amplitude, amplitude};
    double phase_diff[SERVO_COUNT] = {0, 0, 0, 0, DEG2RAD(90), DEG2RAD(-90)};
    Execute2(A, center_angle, period, phase_diff, steps);
}

// ---------------------------------------------------------
// -- Động tác tay: Thể hình
// -- Thông số:
// -- bước: số hành động
// -- thời gian: thời gian hành động (mili giây)
// -- biên độ: biên độ dao động (độ)
// ---------------------------------------------------------
void Otto::Fitness(float steps, int period, int amplitude) {
    if (!has_hands_) {
        return;
    }
    int target[SERVO_COUNT] = {90, 90, 90, 0, 160, 135};
    MoveServos(100, target);
    target[LEFT_FOOT] = 20;
    MoveServos(400, target);
    vTaskDelay(pdMS_TO_TICKS(2000));

    int C[SERVO_COUNT] = {90, 90, 20, 90, 160, 135};
    int A[SERVO_COUNT] = {0, 0, 0, 0, 0, amplitude};
    double phase_diff[SERVO_COUNT] = {0, 0, 0, 0, 0, 0};
    Execute2(A, C, period, phase_diff, steps);

}

// ---------------------------------------------------------
// - Động tác tay: Chào hỏi
// -- Thông số:
// -- dir: hướng TRÁI=tay trái, PHẢI=tay phải
// -- bước: số hành động
// ---------------------------------------------------------
void Otto::Greeting(int dir, float steps) {
    if (!has_hands_) {
        return;
    }
    if (dir == LEFT) {
        int target[SERVO_COUNT] = {90, 90, 150, 150, 45, 135};
        MoveServos(400, target);
        int C[SERVO_COUNT] = {90, 90, 150, 150, 160, 135};
        int A[SERVO_COUNT] = {0, 0, 0, 0, 20, 0};
        double phase_diff[SERVO_COUNT] = {0, 0, 0, 0, 0, 0};
        Execute2(A, C, 300, phase_diff, steps);
    }
    else if (dir == RIGHT) {
        int target[SERVO_COUNT] = {90, 90, 30, 30, 45, 135};
        MoveServos(400, target);
        int C[SERVO_COUNT] = {90, 90, 30, 30, 45, 20};
        int A[SERVO_COUNT] = {0, 0, 0, 0, 0, 20};
        double phase_diff[SERVO_COUNT] = {0, 0, 0, 0, 0, 0};
        Execute2(A, C, 300, phase_diff, steps);
    }

}

// ---------------------------------------------------------
// -- Động tác tay: Nhút nhát
// -- Thông số:
// -- dir: hướng TRÁI=tay trái, PHẢI=tay phải
// -- bước: số hành động
// ---------------------------------------------------------
void Otto::Shy(int dir, float steps) {
    if (!has_hands_) {
        return;
    }

    if (dir == LEFT) {
        int target[SERVO_COUNT] = {90, 90, 150, 150, 45, 135};
        MoveServos(400, target);
        int C[SERVO_COUNT] = {90, 90, 150, 150, 45, 135};
        int A[SERVO_COUNT] = {0, 0, 0, 0, 20, 20};
        double phase_diff[SERVO_COUNT] = {0, 0, 0, 0, DEG2RAD(90), DEG2RAD(-90)};
        Execute2(A, C, 300, phase_diff, steps);
    }
    else if (dir == RIGHT) {
        int target[SERVO_COUNT] = {90, 90, 30, 30, 45, 135};
        MoveServos(400, target);
        int C[SERVO_COUNT] = {90, 90, 30, 30, 45, 135};
        int A[SERVO_COUNT] = {0, 0, 0, 0, 0, 20};
        double phase_diff[SERVO_COUNT] = {0, 0, 0, 0, DEG2RAD(90), DEG2RAD(-90)};
        Execute2(A, C, 300, phase_diff, steps);
    }
}

// ---------------------------------------------------------
// -- Động tác tay: Thể dục vô tuyến
// ---------------------------------------------------------
void Otto::RadioCalisthenics() {
    if (!has_hands_) {
        return;
    }

    const int period = 1000; 
    const float steps = 8.0; 

    int C1[SERVO_COUNT] = {90, 90, 90, 90, 145, 45};
    int A1[SERVO_COUNT] = {0, 0, 0, 0, 45, 45};
    double phase_diff1[SERVO_COUNT] = {0, 0, 0, 0, DEG2RAD(90), DEG2RAD(-90)};
    Execute2(A1, C1, period, phase_diff1, steps);

    int C2[SERVO_COUNT] = {90, 90, 115, 65, 90, 90};
    int A2[SERVO_COUNT] = {0, 0, 25, 25, 0, 0};
    double phase_diff2[SERVO_COUNT] = {0, 0, DEG2RAD(90), DEG2RAD(-90), 0, 0};
    Execute2(A2, C2, period, phase_diff2, steps);
    
    int C3[SERVO_COUNT] = {90, 90, 130, 130, 90, 90};
    int A3[SERVO_COUNT] = {0, 0, 0, 0, 20, 0};
    double phase_diff3[SERVO_COUNT] = {0, 0, 0, 0, 0, 0};
    Execute2(A3, C3, period, phase_diff3, steps);

    int C4[SERVO_COUNT] = {90, 90, 50, 50, 90, 90};
    int A4[SERVO_COUNT] = {0, 0, 0, 0, 0, 20};
    double phase_diff4[SERVO_COUNT] = {0, 0, 0, 0, 0, 0};
    Execute2(A4, C4, period, phase_diff4, steps);
}

// ---------------------------------------------------------
// --Động tác tay: Sự kỳ diệu của tình yêu xoay theo vòng tròn
// ---------------------------------------------------------
void Otto::MagicCircle() {
    if (!has_hands_) {
        return;
    }

    int A[SERVO_COUNT] = {30, 30, 30, 30, 50, 50};
    int O[SERVO_COUNT] = {0, 0, 5, -5, 0, 0};
    double phase_diff[SERVO_COUNT] = {0, 0, DEG2RAD(-90), DEG2RAD(-90), DEG2RAD(-90) , DEG2RAD(90)};

    Execute(A, O, 700, phase_diff, 40);
}

// ---------------------------------------------------------
// -- Display actions: hiển thị nhiều action theo chuỗi
// ---------------------------------------------------------
void Otto::Showcase() {
    if (GetRestState() == true) {
        SetRestState(false);
    }

    // 1. Tiến 3 bước về phía trước
    Walk(3, 1000, FORWARD, 50);
    vTaskDelay(pdMS_TO_TICKS(500));

    // 2. vẫy tay
    if (has_hands_) {
        HandWave(LEFT);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // 3. Khiêu vũ (dùng thể dục vô tuyến)
    if (has_hands_) {
        RadioCalisthenics();
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // 4. Đi bộ trên mặt trăng
    Moonwalker(3, 900, 25, LEFT);
    vTaskDelay(pdMS_TO_TICKS(500));

    // 5. Xoay
    Swing(3, 1000, 30);
    vTaskDelay(pdMS_TO_TICKS(500));

    // 6. Cất cánh
    if (has_hands_) {
        Takeoff(5, 300, 40);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // 7. Thể hình
    if (has_hands_) {
        Fitness(5, 1000, 25);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // 8. Lùi lại 3 bước
    Walk(3, 1000, BACKWARD, 50);
}

void Otto::EnableServoLimit(int diff_limit) {
    for (int i = 0; i < SERVO_COUNT; i++) {
        if (servo_pins_[i] != -1) {
            servo_[i].SetLimiter(diff_limit);
        }
    }
}

void Otto::DisableServoLimit() {
    for (int i = 0; i < SERVO_COUNT; i++) {
        if (servo_pins_[i] != -1) {
            servo_[i].DisableLimiter();
        }
    }
}
