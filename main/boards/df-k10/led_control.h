#ifndef LED_CONTROL_H
#define LED_CONTROL_H

#include "led/circular_strip.h"

class LedStripControl {
private:
    CircularStrip* led_strip_;
    int brightness_level_;  // Mức độ sáng (0-8)

    int LevelToBrightness(int level) const;  // Chuyển đổi mức thành giá trị độ sáng thực tế
    StripColor RGBToColor(int red, int green, int blue);

public:
    explicit LedStripControl(CircularStrip* led_strip);
}; 

#endif // LED_STRIP_CONTROL_H
