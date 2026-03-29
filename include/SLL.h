#pragma once
#include "UI.h"
#include "theme.h"
#include <vector>
#include <speedSlider.h>
#include <string>
#include <timeline.h>
#include <Visualizer.h>

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
        Font font, titleFont, miniTitleFont, vertexFont;
        Texture2D setting, pause, play, randomIcon;
        speedSlider mySpeedSlider{{24.0f, 720.0f}, {295.0f, 720.0f}, 0.1f, 2.0f};
        Timeline myTimeline;

        int StartX, StartY, EndX, EndY;
        void Draw(const Palette& colors);
        int curIndex; 
        bool isPause;
        float TotalTime;

        std::string FloatToString(float value);

        bool isInsertDialogOpen = false; 
        char insertInputText[256] = "\0"; 
        int insertInputCount = 0;        
        int framesCounter = 0;           
        int typeQuery; // 0:Init 1:Add 2:Delete 3:Update 4:Search
        std::vector<int> myList; // Buffer chứa số liệu gõ từ bàn phím
        std::vector<int> internalList; // Mảng CHÍNH THỨC lưu trữ SLL

        Visualizer visualizer; 
        std::vector<VertexState> currentNodes; 
        std::vector<EdgeState> currentEdges; 

        // --- HỆ THỐNG CODE HIGHLIGHT ---
        std::vector<std::string> currentCodeLines;
        std::vector<int> stepCodeLines; 
        void SetCode(const std::vector<std::string>& code);

        const float START_X = 500.0f;
        const float START_Y = 500.0f;
        const float SPACING = 120.0f; // Khoảng cách giãn ra chút cho đẹp
        
        // Khai báo hàm truyền thêm dòng code bị bôi đen (codeLine)
        void PushNewStep(const std::vector<VertexState>& nextNodes, const std::vector<EdgeState>& nextEdges, int codeLine);
        
        // --- CÁC HÀM THUẬT TOÁN ---
        void GenerateInit(const std::vector<int>& values);
        void GenerateAdd(int val);
        void GenerateDelete(int val);
        void GenerateUpdate(int val1, int val2);
        void GenerateSearch(int val);

        std::vector<std::vector<VertexTransform>> AnimationNodes;
        std::vector<std::vector<EdgeTransform>> AnimationEdges;
};