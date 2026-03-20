#pragma once
#include "SLL.h"
#include <iostream>

SLL::SLL(int width, int height) 
{
    StartX = 0;
    StartY = height * 65 / 1080;
    EndX = width;
    EndY = height;
    curIndex = 0;

    Image image = LoadImage("assets/setting.png");
    double scale = height * 0.035f;
    int w = image.width * scale / image.height;
    int h = scale;
    ImageResize(&image, w, h);
    setting = LoadTextureFromImage(image);
    UnloadImage(image);

    image = LoadImage("assets/pause.png");
    scale = 48;
    w = image.width * scale / image.height;
    h = scale;
    ImageResize(&image, w, h);
    pause = LoadTextureFromImage(image);
    UnloadImage(image);

    image = LoadImage("assets/play.png");
    scale = 48;
    w = image.width * scale / image.height;
    h = scale;
    ImageResize(&image, w, h);
    play = LoadTextureFromImage(image);
    UnloadImage(image);

    font = LoadFontEx("assets/fonts/Roboto/Roboto-Bold.ttf", 24, 0, 0);
    titleFont = LoadFontEx("assets/fonts/Roboto/Roboto-Bold.ttf", 36, 0, 0);
    miniTitleFont = LoadFontEx("assets/fonts/Roboto/Roboto-Medium.ttf", 20, 0, 0);
    SetTextureFilter(titleFont.texture, TEXTURE_FILTER_BILINEAR);   
    SetTextureFilter(miniTitleFont.texture, TEXTURE_FILTER_BILINEAR);   
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
}

SLL::~SLL()
{
    UnloadTexture(setting);
    UnloadTexture(pause);
    UnloadTexture(play);

    UnloadFont(font);
    UnloadFont(titleFont);
    UnloadFont(miniTitleFont);
}

void SLL::Solve(const Palette &colors)
{
    Draw(colors);
}

void SLL::Draw(const Palette &colors)
{  
    ClearBackground(colors.background);
    
    //Draw Controls
    DrawLineEx({319.0f, StartY}, {319.0f, EndY}, 1.0f, colors.border);
    DrawTexture(setting, StartX + 10, StartY + 18, colors.primaryAcc);
    
    DrawTextEx(font, "Controls", {StartX + 20 + setting.width, StartY + 25}, 24, 1.0f, colors.textPrimary);
    DrawLineEx({StartX, 142}, {319.0f, 142}, 1.0f, colors.border);

    DrawTextEx(miniTitleFont, "DATA OPERATIONS",{24, 142 + 24}, 20, 1.0f, colors.textPrimary);

    InitButton.Draw();
    AddButton.Draw();
    DeleteButton.Draw();
    UpdateButton.Draw();
    SearchButton.Draw();

    DrawTextEx(miniTitleFont, "PLAYBACK",{24, 580}, 20, 1.0f, colors.textPrimary);
    
    if (isPause)  
    {
        DrawTexture(play, 135, 616, colors.primaryAcc);
    }
    else
    {
        DrawTexture(pause, 135, 616, colors.primaryAcc);
    }

    Rectangle playPauseRect = {135.0f, 616.0f, (float)play.width, (float)play.height};
    if (CheckCollisionPointRec(GetMousePosition(), playPauseRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        isPause ^= 1;
    }
}
