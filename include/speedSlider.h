#pragma once
#include <raylib.h>
#include "theme.h"

class speedSlider 
{
    public:
        speedSlider(Vector2 beginPosition, Vector2 endPosition, float minValue, float maxValue);
        ~speedSlider();
        void Draw(const Palette& palette);
        float getSpeed();
        
    private:
        Vector2 beginPos;   
        Vector2 endPos;
        float minVal;
        float maxVal;
        float currentVal;
        
        float handleRadius; 
        bool isDragging;   
};