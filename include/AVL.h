#pragma once
#include "UI.h"
#include "theme.h"
#include <vector>
#include <speedSlider.h>
#include <string>
#include <timeline.h>
#include <Visualizer.h>

struct AVLNode {
    int val;
    int left;
    int right;
    int height;
    bool active;
    Vector2 targetPos;
};

class AVL 
{
    public:
        AVL(int width, int height);
        ~AVL();
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
        bool isWarningOpen = false; // Popup cảnh báo cho thao tác không điển hình (Chỉ Update)
        char insertInputText[256] = "\0"; 
        int insertInputCount = 0;        
        int framesCounter = 0;           
        int typeQuery; // 0:Init 1:Add 2:Delete 3:Update 4:Search
        
        std::vector<int> myList; 
        std::vector<AVLNode> tree; 
        int rootIndex;

        Visualizer visualizer; 
        std::vector<VertexState> currentNodes; 
        std::vector<EdgeState> currentEdges; 

        // --- HỆ THỐNG CODE HIGHLIGHT ---
        std::vector<std::string> currentCodeLines;
        std::vector<int> stepCodeLines; 
        void SetCode(const std::vector<std::string>& code);
        void PushNewStep(const std::vector<VertexState>& nextNodes, const std::vector<EdgeState>& nextEdges, int codeLine);
        
        // --- HÀM TÍNH TOÁN & VẼ AVL ---
        int GetHeight(int node);
        int GetBalance(int node);
        void UpdateHeight(int node);
        void CalculatePositions(int node, int depth, float leftX, float rightX);
        void GetTreeState(std::vector<VertexState>& nextNodes, std::vector<EdgeState>& nextEdges); 

        // --- THUẬT TOÁN AVL ---
        int RightRotate(int y, int &animCode);
        int LeftRotate(int x, int &animCode);
        int InsertNode(int node, int key, int parentIndex); 
        int DeleteNode(int node, int key); // <--- BỔ SUNG HÀM DELETE
        void SearchNode(int node, int key);

        void GenerateInit(const std::vector<int>& values);
        void GenerateAdd(int val);
        void GenerateDelete(int val);
        void GenerateSearch(int val);

        std::vector<std::vector<VertexTransform>> AnimationNodes;
        std::vector<std::vector<EdgeTransform>> AnimationEdges;
};