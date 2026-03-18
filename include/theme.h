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
    ThemeMode currentMode = ThemeMode::LIGHT; // Mặc định mở lên là Light mode
    Palette lightPalette;
    Palette darkPalette;
};