#include "theme.h"

// Trả về instance duy nhất của ThemeManager (Singleton Pattern)
ThemeManager& ThemeManager::GetInstance() {
    static ThemeManager instance;
    return instance;
}

// Constructor: Nơi định nghĩa mã màu thực tế cho cả 2 chế độ Sáng/Tối
ThemeManager::ThemeManager() {
    // Khởi tạo mã màu cho Light Theme (Giao diện Sáng)
    lightPalette = {
        GetColor(0xF8FAFCFF), // background: Xám rất nhạt
        GetColor(0xFFFFFFFF), // surface: Trắng tinh cho Sidebar và Cards
        GetColor(0x0F172AFF), // textPrimary: Đen nhạt / Xanh than
        GetColor(0x64748BFF), // textSecondary: Xám cho text phụ
        GetColor(0xE2E8F0FF), // border: Xám nhạt cho đường viền
        GetColor(0x3B82F6FF), // primaryAcc: Xanh blue cho icon
        GetColor(0x8B5CF6FF), // btnGradientStart: Tím cho nút bấm
        GetColor(0x3B82F6FF), // btnGradientEnd: Xanh dương cho nút bấm
        GetColor(0xFFFFFFFF), // btnText: Chữ màu trắng trên nút
        GetColor(0x10B981FF), // logSuccess: Xanh lá cây cho Execute Log

        // Màu cho Visualization (Light Mode)
        GetColor(0x3B82F6FF), // nodeDefault: Xanh dương (tương đương primaryAcc)
        GetColor(0xF59E0BFF), // nodeHighlight: Cam nổi bật (dễ nhìn trên nền sáng)
        GetColor(0x10B981FF), // nodeSuccess: Xanh lá (tương đương logSuccess)
        GetColor(0xFFFFFFFF), // nodeText: Chữ màu trắng bên trong Node
        GetColor(0x94A3B8FF), // edgeDefault: Xám bạc (nhạt hơn border để không rối mắt)
        GetColor(0xF59E0BFF)  // edgeHighlight: Cam (giống nodeHighlight)
    };

    // Khởi tạo mã màu cho Dark Theme (Giao diện Tối)
    darkPalette = {
        GetColor(0x0F172AFF), // background: Xanh đen tối làm nền chính
        GetColor(0x162032FF), // surface: Màu xanh xám đậm của sidebar/card
        GetColor(0xF8FAFCFF), // textPrimary: Trắng nhạt cho chữ
        GetColor(0x94A3B8FF), // textSecondary: Xám nhạt cho text phụ
        GetColor(0x334155FF), // border: Xám viền tối
        GetColor(0x3B82F6FF), // primaryAcc: Xanh blue (giữ nguyên để nổi bật)
        GetColor(0x8B5CF6FF), // btnGradientStart: Tím (giữ nguyên)
        GetColor(0x3B82F6FF), // btnGradientEnd: Xanh (giữ nguyên)
        GetColor(0xFFFFFFFF), // btnText: Trắng
        GetColor(0x34D399FF), // logSuccess: Xanh lá sáng hơn một chút để dễ nhìn trên nền tối

        // Màu cho Visualization (Dark Mode)
        GetColor(0x3B82F6FF), // nodeDefault: Xanh dương (nổi bật hoàn hảo trên nền tối)
        GetColor(0xFBBF24FF), // nodeHighlight: Vàng sáng (Bắt sáng rất mạnh trên nền đen)
        GetColor(0x34D399FF), // nodeSuccess: Xanh lá neon (tương đương logSuccess)
        GetColor(0xFFFFFFFF), // nodeText: Chữ màu trắng bên trong Node
        GetColor(0x64748BFF), // edgeDefault: Xám vừa (đủ để thấy, không bị lấn át Node)
        GetColor(0xFBBF24FF)  // edgeHighlight: Vàng sáng (giống nodeHighlight)
    };
    
    // Đảm bảo khởi tạo đúng với chế độ mặc định
    currentMode = ThemeMode::LIGHT; 
}

void ThemeManager::SetTheme(ThemeMode mode) {
    currentMode = mode;
}

void ThemeManager::ToggleTheme() {
    currentMode = (currentMode == ThemeMode::LIGHT) ? ThemeMode::DARK : ThemeMode::LIGHT;
}

const Palette& ThemeManager::GetColors() {
    return (currentMode == ThemeMode::LIGHT) ? lightPalette : darkPalette;
}

ThemeMode ThemeManager::GetCurrentMode() {
    return currentMode;
}