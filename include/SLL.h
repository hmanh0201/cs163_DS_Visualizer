#pragma once
#include "UI.h"
#include "theme.h"
#include <vector>
#include <speedSlider.h>
#include <string>
#include <timeline.h>

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
        Texture2D setting, pause, play, randomIcon;
        speedSlider mySpeedSlider{{24.0f, 720.0f}, {295.0f, 720.0f}, 0.1f, 2.0f};
        Timeline myTimeline;

        int StartX, StartY, EndX, EndY;
        void Draw(const Palette& colors);
        int curIndex; 
        int size;
        bool isPause;

        std::string FloatToString(float value);

        bool isInsertDialogOpen = false; // Trạng thái bật/tắt hộp thoại
        char insertInputText[256] = "\0"; // Lưu trữ chuỗi người dùng gõ
        int insertInputCount = 0;        // Đếm số ký tự
        int framesCounter = 0;           // Dùng để nháy con trỏ chuột (cursor)
        int typeQuery; // 0:Init 1:Add 2:Delete 3:Update 4:Search
        std::vector<int> myList; // Lưu trữ dữ liệu của SLL
};