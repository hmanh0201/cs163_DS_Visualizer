#include <iostream>
#include <string>
#include <raylib.h>
#include "UI.h" 
#include "header.h"
#include "theme.h"
#include "Menu.h"
#include "SLL.h"
#include "Heap.h"
#include <AVL.h>
#include <trie.h>
#include <mst.h>
#include <find_cc.h>

int WindowWidth = 1920;
int WindowHeight = 1080;

int main()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(WindowWidth, WindowHeight, "Data Structure Visualizer");
    int monitor = GetCurrentMonitor();

    WindowWidth = GetMonitorWidth(monitor);
    WindowHeight = GetMonitorHeight(monitor);
    
    SetWindowSize(WindowWidth, WindowHeight);
    SetWindowPosition(0, 0);

    SetTargetFPS(60);

    SetExitKey(0);

    std::string MainState = "MENU";

    ThemeManager& themeManager = ThemeManager::GetInstance();

    header myHeader(WindowWidth, WindowHeight, "assets/close.png", "assets/close_hover.png", "assets/day-mode.png", "assets/night-mode.png", "assets/home-logo.png", "assets/home-logo-2.png");

    DrawMenu myMenu(WindowWidth, WindowHeight);

    SLL mySLL(WindowWidth, WindowHeight);
    Heap myHeap(WindowWidth, WindowHeight);
    AVL myAVL(WindowWidth, WindowHeight);
    Trie myTrie(WindowWidth, WindowHeight);
    MST myMST(WindowWidth, WindowHeight);
    FindCC myCC(WindowWidth, WindowHeight);

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

        if (MainState == "MENU")
        {
            myMenu.Draw(colors);
            int clickedTopic = myMenu.CheckTopicClick();
            if (clickedTopic == 0) {
                MainState = "SLL";
            } else if (clickedTopic == 1) {
                MainState = "HEAP";
            } else if (clickedTopic == 2) {
                MainState = "AVL";
            } else if (clickedTopic == 3) {
                MainState = "TRIE";
            } else if (clickedTopic == 4) {
                MainState = "MST";
            } else if (clickedTopic == 5) {
                MainState = "CONNECTED";
            }
        }

        if (MainState == "SLL")
        {
            mySLL.Solve(colors);
        }
        if (MainState == "HEAP")
        {
            myHeap.Solve(colors);
        }
        if (MainState == "AVL")
        {
            myAVL.Solve(colors);
        }
        if (MainState == "TRIE")
        {
            myTrie.Solve(colors);
        }
        if (MainState == "MST")
        {
            myMST.Solve(colors);
        }
        if (MainState == "CONNECTED")
        {
            myCC.Solve(colors);
        }

        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
//fix font topic