#include "speedSlider.h"

speedSlider::speedSlider(Vector2 beginPosition, Vector2 endPosition, float minValue, float maxValue)
{
    beginPos = beginPosition;
    endPos = endPosition;
    minVal = minValue;
    maxVal = maxValue;

    currentVal = (minValue + maxValue) / 2.0f; 
    
    handleRadius = 8.0f; 
    isDragging = false;
}

speedSlider::~speedSlider()
{
}

void speedSlider::Draw(const Palette& palette)
{
    float sliderLength = endPos.x - beginPos.x;
    Vector2 mousePos = GetMousePosition();
    float handleX = beginPos.x + ((currentVal - minVal) / (maxVal - minVal)) * sliderLength;
    Vector2 handleCenter = { handleX, beginPos.y }; 
    Rectangle trackHitbox = { beginPos.x, beginPos.y - 10, sliderLength, 20.0f };
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
    if (isDragging) 
    {
        float newX = mousePos.x;
        if (newX < beginPos.x) newX = beginPos.x;
        if (newX > endPos.x) newX = endPos.x;
        currentVal = minVal + ((newX - beginPos.x) / sliderLength) * (maxVal - minVal);
        handleCenter.x = newX; 
    }
    int lineThickness = 4; 
    DrawLineEx(beginPos, endPos, lineThickness, palette.border);
    DrawLineEx(beginPos, handleCenter, lineThickness, palette.primaryAcc);
    if (CheckCollisionPointCircle(mousePos, handleCenter, handleRadius) || isDragging) 
    {
        DrawCircleV(handleCenter, handleRadius + 4, Fade(palette.primaryAcc, 0.3f));
    }
    DrawCircleV(handleCenter, handleRadius, palette.primaryAcc);
}

float speedSlider::getSpeed()
{
    return currentVal;
}