#pragma once
#include "UI.h"
#include "theme.h"
#include <vector>
#include <string>
#include <map>
#include <speedSlider.h>
#include <timeline.h>
#include <Visualizer.h>

struct TrieNode {
    char val;
    bool isEnd;
    bool active;
    int parent;
    std::map<char, int> children;
    Vector2 targetPos;
};

class Trie 
{
    public:
        Trie(int width, int height);
        ~Trie();
        void Solve(const Palette& colors);
    private:
        TextButton InitButton{{34.0f, 210.0f, 251.0f, 50.0f}, "INIT"};
        TextButton AddButton{{34.0f, 284.0f, 251.0f, 50.0f}, "ADD"};
        TextButton DeleteButton{{34.0f, 358.0f, 251.0f, 50.0f}, "DELETE"}; 
        TextButton UpdateButton{{34.0f, 432.0f, 251.0f, 50.0f}, "UPDATE"}; // Đã khôi phục nút UPDATE
        TextButton SearchButton{{34.0f, 506.0f, 251.0f, 50.0f}, "SEARCH"};
        
        Font font, titleFont, miniTitleFont, vertexFont;
        Texture2D setting, pause, play, randomIcon;
        speedSlider mySpeedSlider{{24.0f, 720.0f}, {295.0f, 720.0f}, 0.1f, 2.0f};
        Timeline myTimeline;

        float StartX, StartY, EndX, EndY;
        void Draw(const Palette& colors);
        int curIndex; 
        bool isPause;
        float TotalTime;

        std::string FloatToString(float value);

        bool isInsertDialogOpen = false; 
        bool isWarningOpen = false; 
        char insertInputText[256] = "\0"; 
        int insertInputCount = 0;        
        int framesCounter = 0;           
        int typeQuery; // 0:Init 1:Add 2:Delete 4:Search
        
        std::vector<std::string> stringQueryList; 
        
        std::vector<TrieNode> tree; 
        int rootIndex;

        Visualizer visualizer; 
        std::vector<VertexState> currentNodes; 
        std::vector<EdgeState> currentEdges; 

        // --- HỆ THỐNG CODE HIGHLIGHT ---
        std::vector<std::string> currentCodeLines;
        std::vector<int> stepCodeLines; 
        void SetCode(const std::vector<std::string>& code);
        void PushNewStep(const std::vector<VertexState>& nextNodes, const std::vector<EdgeState>& nextEdges, int codeLine);
        
        // --- HÀM TÍNH TOÁN & VẼ TRIE ---
        void CalculatePositions(int node, int depth, float leftX, float rightX);
        void GetTreeState(std::vector<VertexState>& nextNodes, std::vector<EdgeState>& nextEdges); 

        // --- THUẬT TOÁN TRIE ---
        void GenerateInit(const std::vector<std::string>& values);
        void GenerateAdd(std::string word);
        void GenerateDelete(std::string word);
        void GenerateSearch(std::string word);

        std::vector<std::vector<VertexTransform>> AnimationNodes;
        std::vector<std::vector<EdgeTransform>> AnimationEdges;
};