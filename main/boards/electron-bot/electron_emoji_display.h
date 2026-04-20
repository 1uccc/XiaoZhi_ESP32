#pragma once

#include "display/lcd_display.h"

/* *
 * @brief Lớp hiển thị biểu thức GIF của robot điện tử
 * Kế thừa SpiLcdDisplay và thêm hỗ trợ biểu thức GIF thông qua EmojiCollection */
class ElectronEmojiDisplay : public SpiLcdDisplay {
   public:
    /* *
     * @brief constructor, các tham số giống với SpiLcdDisplay */
    ElectronEmojiDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel, int width, int height, int offset_x, int offset_y, bool mirror_x, bool mirror_y, bool swap_xy);

    virtual ~ElectronEmojiDisplay() = default;
    virtual void SetStatus(const char* status) override;

   private:
    void InitializeElectronEmojis();
    void SetupChatLabel();
};