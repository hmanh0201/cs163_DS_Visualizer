#pragma once
#include "UI.h"
#include "theme.h"
#include <vector>

struct Node 
{
    int value;
    Node* next;
    Node* prev; 
    Node(int val) : value(val), next(nullptr), prev(nullptr) {}
};

class SLL 
{
    public:
        SLL(int width, int height);
        ~SLL();
        void Solve(const Palette& colors);
    private:
        TextButton InitButton{{34.0f, 210.0f, 251.0f, 50.0f}, "INIT"};
        TextButton AddButton{{34.0f, 284.0f, 251.0f, 50.0f}, "ADD"};
        TextButton DeleteButton{{34.0f, 358.0f, 251.0f, 50.0f}, "DELETE"}; 
        TextButton UpdateButton{{34.0f, 432.0f, 251.0f, 50.0f}, "UPDATE"};
        TextButton SearchButton{{34.0f, 506.0f, 251.0f, 50.0f}, "SEARCH"};
        Font font;
        Font titleFont;
        Font miniTitleFont;
        Texture2D setting, pause, play;

        int StartX, StartY, EndX, EndY;
        void Draw(const Palette& colors);
        int curIndex; 
        bool isPause;
};