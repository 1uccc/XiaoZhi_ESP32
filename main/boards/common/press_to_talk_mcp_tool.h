#ifndef PRESS_TO_TALK_MCP_TOOL_H
#define PRESS_TO_TALK_MCP_TOOL_H

#include "mcp_server.h"
#include "settings.h"

// Lớp công cụ MCP chế độ nút để nói có thể tái sử dụng
class PressToTalkMcpTool {
private:
    bool press_to_talk_enabled_;

public:
    PressToTalkMcpTool();
    
    // Khởi tạo công cụ và đăng ký máy chủ MCP
    void Initialize();
    
    // Nhận trạng thái chế độ nút để nói hiện tại
    bool IsPressToTalkEnabled() const;

private:
    // Chức năng gọi lại của công cụ MCP
    ReturnValue HandleSetPressToTalk(const PropertyList& properties);
    
    // Phương pháp nội bộ: Đặt trạng thái nhấn để nói và lưu vào cài đặt
    void SetPressToTalkEnabled(bool enabled);
};

#endif // PRESS_TO_TALK_MCP_TOOL_H