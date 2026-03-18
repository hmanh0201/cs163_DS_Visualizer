#include <iostream>
#include <string>
#include <raylib.h>
#include "UI.h" 
#include "header.h"
#include "theme.h"

int WindowWidth = 1920;
int WindowHeight = 1080;

int main()
{
    InitWindow(WindowWidth, WindowHeight, "Data Structure Visualizer");
    int monitor = GetCurrentMonitor();

    // Lấy kích thước màn hình thực tế
    WindowWidth = GetMonitorWidth(monitor);
    WindowHeight = GetMonitorHeight(monitor);
    
    SetWindowSize(WindowWidth, WindowHeight);
    ToggleFullscreen();

    SetTargetFPS(60);

    ThemeManager& themeManager = ThemeManager::GetInstance();
    
    header myHeader(WindowWidth, WindowHeight, 
                    "assets/close.png", "assets/close_hover.png", 
                    "assets/day-mode.png", "assets/night-mode.png", 
                    "assets/home-logo.png", "assets/home-logo-2.png");

    std::string MainState = "MENU";

    while (!WindowShouldClose())
    {
        BeginDrawing();

        const Palette& colors = themeManager.GetColors();
        ClearBackground(colors.background);

        myHeader.Draw();
        
        if (!myHeader.getState())
        {
            break; 
        }

        if (myHeader.getThemeState())
        {
            themeManager.SetTheme(ThemeMode::LIGHT);
        }
        else
        {
            themeManager.SetTheme(ThemeMode::DARK);
        }

        if (myHeader.getReturnMenu())
        {
            MainState = "MENU";
        }

        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}