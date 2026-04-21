#pragma once
#include "UI.h"
#include "theme.h"
#include <vector>
#include <speedSlider.h>
#include <string>
#include <timeline.h>
#include <Visualizer.h>
#include <algorithm>

struct MNode {
    Vector2 pos;
    int id;
};

struct MEdge {
    int u, v;
    int weight;
    int visualIndex; 
};

class MST 
{
    public:
        MST(int width, int height);
        ~MST();
        void Solve(const Palette& colors);
    private:
        TextButton InitButton{{34.0f, 210.0f, 251.0f, 50.0f}, "INIT GRAPH"};
        TextButton FindMSTButton{{34.0f, 284.0f, 251.0f, 50.0f}, "FIND MST"};
        
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
        std::vector<MEdge> customEdgesToGen;
        
        std::vector<MNode> internalNodes;
        std::vector<MEdge> internalEdges;

        Visualizer visualizer; 
        std::vector<VertexState> currentNodes; 
        std::vector<EdgeState> currentEdges; 

        std::vector<std::string> currentCodeLines;
        std::vector<int> stepCodeLines; 
        void SetCode(const std::vector<std::string>& code);
        void PushNewStep(const std::vector<VertexState>& nextNodes, const std::vector<EdgeState>& nextEdges, int codeLine);
        
        void GenerateInit(int n, const std::vector<MEdge>& edges);
        void GenerateMST();
        int findSet(int i, std::vector<int>& parent);

        int draggingNodeIndex = -1;
        Vector2 dragOffset = {0.0f, 0.0f};
        void UpdatePositionsAfterDrag();

        std::vector<std::vector<VertexTransform>> AnimationNodes;
        std::vector<std::vector<EdgeTransform>> AnimationEdges;
};