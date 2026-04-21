#pragma once
#include "raylib.h"

// Enum xác định chế độ hiện tại
enum class ThemeMode {
    LIGHT,
    DARK
};

// Struct chứa toàn bộ các màu cần thiết cho giao diện
struct Palette {
    Color background;       // Màu nền chính
    Color surface;          // Màu nền phụ (Sidebar, Cards)
    Color textPrimary;      // Tiêu đề, text chính
    Color textSecondary;    // Text phụ, icon chưa active
    Color border;           // Đường viền (Cards, Divider)
    Color primaryAcc;       // Màu chủ đạo (Icon xanh dương)
    
    // Màu cho nút bấm (dạng gradient)
    Color btnGradientStart; // Tím nhạt
    Color btnGradientEnd;   // Xanh dương
    Color btnText;          // Chữ trên nút
    
    // Log text
    Color logSuccess;       // Chữ màu xanh lá trong Execute Log

    // Màu cho Visualization (Singly Linked List, Trees, Graphs...)
    Color nodeDefault;      // Màu node ở trạng thái bình thường
    Color nodeHighlight;    // Màu node khi đang quét/tìm kiếm (Search, Update)
    Color nodeSuccess;      // Màu node khi hoàn tất thao tác thành công
    Color nodeText;         // Màu chữ (value) bên trong node
    Color edgeDefault;      // Màu cạnh nối bình thường
    Color edgeHighlight;    // Màu cạnh nối khi dòng điện (logic) chạy qua

    // Màu cho các trạng thái thuật toán Đồ thị (MST, CC)
    Color success;          // Xanh lá (Chấp nhận cạnh)
    Color error;            // Đỏ (Phát hiện chu trình / Từ chối cạnh)
};

class ThemeManager {
public:
    static ThemeManager& GetInstance(); // Lấy instance duy nhất của ThemeManager
    void SetTheme(ThemeMode mode); // Đổi theme
    void ToggleTheme(); // Chuyển đổi qua lại giữa Sáng/Tối
    const Palette& GetColors(); // Lấy bảng màu hiện tại để vẽ
    ThemeMode GetCurrentMode();

private:
    ThemeManager();
    ThemeMode currentMode;
    Palette lightPalette;
    Palette darkPalette;
};