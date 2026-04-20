#pragma once

#include "display/lcd_display.h"

/* *
 * Lớp hiển thị biểu thức GIF @brief Otto robot
 * Kế thừa SpiLcdDisplay và thêm hỗ trợ biểu thức GIF thông qua EmojiCollection */
class OttoEmojiDisplay : public SpiLcdDisplay {
   public:
    /* *
     * @brief constructor, các tham số giống với SpiLcdDisplay */
    OttoEmojiDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel, int width, int height, int offset_x, int offset_y, bool mirror_x, bool mirror_y, bool swap_xy);

    virtual ~OttoEmojiDisplay() = default;
    virtual void SetStatus(const char* status) override;
    virtual void SetPreviewImage(std::unique_ptr<LvglImage> image) override;

   private:
    void InitializeOttoEmojis();
    void SetupPreviewImage();
};