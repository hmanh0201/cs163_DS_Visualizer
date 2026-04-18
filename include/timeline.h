#pragma once
#include <raylib.h>
#include "theme.h" 

class Timeline 
{
    public:
        Timeline();
        ~Timeline();
        void Draw(Vector2 beginPos, Vector2 endPos, int totalSteps, int currentStep, const Palette& palette);
        int getStep();
        
    private:
        float handleRadius; 
        bool isDragging;
        int stepValue; 
};