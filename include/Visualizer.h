#pragma once
#include <raylib.h>
#include <string>
#include "theme.h"

enum class ElementTheme 
{
    DEFAULT,    
    HIGHLIGHT,  
    SUCCESS     
};

struct VertexState 
{
    Vector2 pos;
    float radius;
    ElementTheme themeState; 
    float opacity;           
    std::string text;
    
    VertexState() : pos{0, 0}, radius(30.0f), themeState(ElementTheme::DEFAULT), opacity(1.0f), text("") {}
};

struct EdgeState 
{
    Vector2 startPos;
    Vector2 endPos;
    ElementTheme themeState;
    float thickness;
    float opacity;
    
    EdgeState() : startPos{0, 0}, endPos{0, 0}, themeState(ElementTheme::DEFAULT), thickness(4.0f), opacity(1.0f) {}
};

struct VertexTransform { VertexState u; VertexState v; };
struct EdgeTransform { EdgeState u; EdgeState v; };

class Visualizer 
{
    public:
        Visualizer();
        ~Visualizer();

        void RenderEdge(const EdgeTransform& trans, float progress, const Palette& palette);
        void RenderDirectedEdge(const EdgeTransform& trans, float progress, const Palette& palette, float destRadius = 30.0f, float arrowSize = 12.0f);
        void RenderVertex(const VertexTransform& trans, float progress, Font font, const Palette& palette);
    private:
        static float Lerp(float start, float end, float progress);
        static Vector2 LerpVector2(Vector2 start, Vector2 end, float progress);
        static Color LerpColor(Color start, Color end, float progress);

        static Color GetNodeColor(ElementTheme state, const Palette& palette);
        static Color GetEdgeColor(ElementTheme state, const Palette& palette);
};