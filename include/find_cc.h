#pragma once
#include "UI.h"
#include "theme.h"
#include <vector>
#include <speedSlider.h>
#include <string>
#include <timeline.h>
#include <Visualizer.h>

struct CCNode {
    Vector2 pos;
    int id;
};

struct CCEdge {
    int u, v;
    int visualIndex; 
};

struct CCVertexState {
    Vector2 pos;
    std::string text;
    float radius;
    float opacity;
    ElementTheme themeState;
    int ccColorIndex;
};

struct CCEdgeState {
    Vector2 startPos;
    Vector2 endPos;
    float thickness;
    float opacity;
    ElementTheme themeState;
    int ccColorIndex;
};

struct CCVertexTransform {
    CCVertexState u, v;
};

struct CCEdgeTransform {
    CCEdgeState u, v;
};

class FindCC 
{
    public:
        FindCC(int width, int height);
        ~FindCC();
        void Solve(const Palette& colors);
    private:
        TextButton InitButton{{34.0f, 210.0f, 251.0f, 50.0f}, "INIT GRAPH"};
        TextButton FindCCButton{{34.0f, 284.0f, 251.0f, 50.0f}, "FIND CC"};
        
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

        bool isInitDialogOpen = false; 
        char inputText[4096] = "\0"; 
        int inputCount = 0;        
        int framesCounter = 0;           
        int typeQuery; 
        
        int numVerticesToGen = 0; 
        std::vector<CCEdge> customEdgesToGen;
        
        std::vector<CCNode> internalNodes;
        std::vector<CCEdge> internalEdges;
        std::vector<std::vector<int>> adjList;

        std::vector<CCVertexState> currentNodes; 
        std::vector<CCEdgeState> currentEdges; 

        std::vector<std::string> currentCodeLines;
        std::vector<int> stepCodeLines; 
        void SetCode(const std::vector<std::string>& code);
        void PushNewStep(const std::vector<CCVertexState>& nextNodes, const std::vector<CCEdgeState>& nextEdges, int codeLine);
        
        void GenerateInit(int n, const std::vector<CCEdge>& edges);
        void GenerateFindCC();
        void DFS(int u, int ccIndex, std::vector<bool>& visited, std::vector<CCVertexState>& nextNodes, std::vector<CCEdgeState>& nextEdges);

        int draggingNodeIndex = -1;
        Vector2 dragOffset = {0.0f, 0.0f};
        void UpdatePositionsAfterDrag();

        std::vector<std::vector<CCVertexTransform>> AnimationNodes;
        std::vector<std::vector<CCEdgeTransform>> AnimationEdges;
};