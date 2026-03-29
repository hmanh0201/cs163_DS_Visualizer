#include "Visualizer.h"
#include <cmath>

Visualizer::Visualizer() {}
Visualizer::~Visualizer() {}

// --- NỘI SUY (LINEAR INTERPOLATION) ---
float Visualizer::Lerp(float start, float end, float progress) {
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;
    return start + (end - start) * progress;
}

Vector2 Visualizer::LerpVector2(Vector2 start, Vector2 end, float progress) {
    return { Lerp(start.x, end.x, progress), Lerp(start.y, end.y, progress) };
}

Color Visualizer::LerpColor(Color start, Color end, float progress) {
    return {
        (unsigned char)Lerp(start.r, end.r, progress),
        (unsigned char)Lerp(start.g, end.g, progress),
        (unsigned char)Lerp(start.b, end.b, progress),
        (unsigned char)Lerp(start.a, end.a, progress)
    };
}

Color Visualizer::GetNodeColor(ElementTheme state, const Palette& palette) {
    switch (state) {
        case ElementTheme::HIGHLIGHT: return palette.nodeHighlight;
        case ElementTheme::SUCCESS:   return palette.nodeSuccess;
        default:                      return palette.nodeDefault;
    }
}

Color Visualizer::GetEdgeColor(ElementTheme state, const Palette& palette) {
    switch (state) {
        case ElementTheme::HIGHLIGHT: return palette.edgeHighlight;
        case ElementTheme::SUCCESS:   return palette.edgeHighlight; 
        default:                      return palette.edgeDefault;
    }
}

// --- HÀM VẼ ---
void Visualizer::RenderEdge(const EdgeTransform& trans, float progress, const Palette& palette) {
    float currentOpacity = Lerp(trans.u.opacity, trans.v.opacity, progress);
    if (currentOpacity <= 0.05f) return; 

    Vector2 currentStart = LerpVector2(trans.u.startPos, trans.v.startPos, progress);
    Vector2 currentEnd = LerpVector2(trans.u.endPos, trans.v.endPos, progress);
    float currentThickness = Lerp(trans.u.thickness, trans.v.thickness, progress);

    Color uColor = GetEdgeColor(trans.u.themeState, palette);
    Color vColor = GetEdgeColor(trans.v.themeState, palette);
    Color currentColor = LerpColor(uColor, vColor, progress);
    
    currentColor = Fade(currentColor, currentOpacity);
    DrawLineEx(currentStart, currentEnd, currentThickness, currentColor);
}

void Visualizer::RenderDirectedEdge(const EdgeTransform& trans, float progress, const Palette& palette, float destRadius, float arrowSize) {
    RenderEdge(trans, progress, palette); // Vẽ thân cạnh

    float currentOpacity = Lerp(trans.u.opacity, trans.v.opacity, progress);
    if (currentOpacity <= 0.05f) return; 

    Vector2 currentStart = LerpVector2(trans.u.startPos, trans.v.startPos, progress);
    Vector2 currentEnd = LerpVector2(trans.u.endPos, trans.v.endPos, progress);
    Color currentColor = LerpColor(GetEdgeColor(trans.u.themeState, palette), GetEdgeColor(trans.v.themeState, palette), progress);
    currentColor = Fade(currentColor, currentOpacity);

    float dx = currentEnd.x - currentStart.x;
    float dy = currentEnd.y - currentStart.y;
    float angle = atan2(dy, dx);

    // Điểm mũi tên dừng lại ở viền Node đích
    Vector2 arrowTip = { 
        currentEnd.x - destRadius * cos(angle), 
        currentEnd.y - destRadius * sin(angle) 
    };

    Vector2 p2 = { arrowTip.x - arrowSize * cos(angle - PI/6), arrowTip.y - arrowSize * sin(angle - PI/6) };
    Vector2 p3 = { arrowTip.x - arrowSize * cos(angle + PI/6), arrowTip.y - arrowSize * sin(angle + PI/6) };

    DrawTriangle(arrowTip, p2, p3, currentColor);
}

void Visualizer::RenderVertex(const VertexTransform& trans, float progress, Font font, const Palette& palette) {
    float currentOpacity = Lerp(trans.u.opacity, trans.v.opacity, progress);
    if (currentOpacity <= 0.05f) return; 

    Vector2 currentPos = LerpVector2(trans.u.pos, trans.v.pos, progress);
    float currentRadius = Lerp(trans.u.radius, trans.v.radius, progress);
    if (currentRadius <= 0.1f) currentRadius = 30.0f; 

    Color currentBgColor = LerpColor(GetNodeColor(trans.u.themeState, palette), GetNodeColor(trans.v.themeState, palette), progress);
    Color currentBorderColor = palette.border; 
    Color currentTextColor = palette.nodeText;

    currentBgColor = Fade(currentBgColor, currentOpacity);
    currentBorderColor = Fade(currentBorderColor, currentOpacity);
    currentTextColor = Fade(currentTextColor, currentOpacity);

    DrawCircleV(currentPos, currentRadius, currentBgColor);
    DrawRing(currentPos, currentRadius - 3.0f, currentRadius, 0, 360, 32, currentBorderColor);

    std::string currentText = (progress < 0.5f) ? trans.u.text : trans.v.text; 
    if (!currentText.empty()) {
        float fontSize = currentRadius * 1.0f; 
        if (currentText.length() >= 3) fontSize = currentRadius * 0.8f; 

        float spacing = 1.0f; 
        Vector2 textSize = MeasureTextEx(font, currentText.c_str(), fontSize, spacing);
        Vector2 textPos = { currentPos.x - (textSize.x / 2.0f), currentPos.y - (textSize.y / 2.0f) };
        DrawTextEx(font, currentText.c_str(), textPos, fontSize, spacing, currentTextColor);
    }
}