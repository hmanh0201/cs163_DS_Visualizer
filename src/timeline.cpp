#include "Timeline.h"

Timeline::Timeline()
{
    handleRadius = 8.0f; 
    isDragging = false;
    stepValue = 0; 
}

Timeline::~Timeline()
{
}

void Timeline::Draw(Vector2 beginPos, Vector2 endPos, int totalSteps, int currentStep, const Palette& palette)
{
    // Bảo vệ logic: Tránh chia cho 0
    if (totalSteps <= 0) totalSteps = 1; 
    
    // Ép currentStep nằm trong giới hạn
    if (currentStep < 0) currentStep = 0;
    if (currentStep > totalSteps) currentStep = totalSteps;

    float sliderLength = endPos.x - beginPos.x;
    Vector2 mousePos = GetMousePosition();
    
    float handleX = beginPos.x + ((float)currentStep / totalSteps) * sliderLength;
    Vector2 handleCenter = { handleX, beginPos.y }; 
    Rectangle trackHitbox = { beginPos.x, beginPos.y - 10, sliderLength, 20.0f };

    // Xử lý kéo thả chuột
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) 
    {
        if (CheckCollisionPointCircle(mousePos, handleCenter, handleRadius * 1.5f) || 
            CheckCollisionPointRec(mousePos, trackHitbox)) 
        {
            isDragging = true;
        }
    }
    
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) 
    {
        isDragging = false;
    }

    // Nếu đang kéo chuột, tính toán lại bước hiện tại
    if (isDragging) 
    {
        float newX = mousePos.x;
        if (newX < beginPos.x) newX = beginPos.x;
        if (newX > endPos.x) newX = endPos.x;
        
        float percentage = (newX - beginPos.x) / sliderLength;
        
        // Làm tròn tới bước gần nhất
        currentStep = (int)(percentage * totalSteps + 0.5f);
        
        handleCenter.x = beginPos.x + ((float)currentStep / totalSteps) * sliderLength;
    }

    // CẬP NHẬT BIẾN LƯU TRỮ
    stepValue = currentStep;

    // Vẽ giao diện
    int lineThickness = 4; 
    DrawLineEx(beginPos, endPos, lineThickness, palette.border);
    DrawLineEx(beginPos, handleCenter, lineThickness, palette.primaryAcc);
    
    if (CheckCollisionPointCircle(mousePos, handleCenter, handleRadius) || isDragging) 
    {
        DrawCircleV(handleCenter, handleRadius + 4, Fade(palette.primaryAcc, 0.3f));
    }
    
    DrawCircleV(handleCenter, handleRadius, palette.primaryAcc);
}

// Hàm trả về bước hiện tại
int Timeline::getStep()
{
    return stepValue;
}