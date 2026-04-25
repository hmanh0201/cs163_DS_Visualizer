#include "find_cc.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <cstring>
#include <fstream>
#include <sstream>

static const Color ccColors[10] = {
    { 239, 68, 68, 255 },  
    { 245, 158, 11, 255 }, 
    { 16, 185, 129, 255 }, 
    { 59, 130, 246, 255 }, 
    { 139, 92, 246, 255 }, 
    { 236, 72, 153, 255 }, 
    { 20, 184, 166, 255 }, 
    { 132, 204, 22, 255 }, 
    { 99, 102, 241, 255 }, 
    { 217, 70, 239, 255 }  
};

static void DrawCustomBezierQuad(Vector2 start, Vector2 end, Vector2 control, float thick, Color color) {
    int segments = 24;
    Vector2 prev = start;
    for (int i = 1; i <= segments; i++) {
        float t = (float)i / segments;
        float u = 1.0f - t;
        Vector2 next = {
            u * u * start.x + 2.0f * u * t * control.x + t * t * end.x,
            u * u * start.y + 2.0f * u * t * control.y + t * t * end.y
        };
        DrawLineEx(prev, next, thick, color);
        prev = next;
    }
}

FindCC::FindCC(int width, int height) 
{
    StartX = 0.0f; StartY = height * 65.0f / 1080.0f; EndX = width; EndY = height;
    curIndex = 0; TotalTime = 0.0f; isPause = false; typeQuery = -1;
    draggingNodeIndex = -1; dragOffset = {0.0f, 0.0f};

    srand(time(NULL));

    Image image = LoadImage("assets/setting.png");
    double scale = height * 0.035f;
    int w = image.width * scale / image.height;
    ImageResize(&image, w, scale); setting = LoadTextureFromImage(image); UnloadImage(image);

    image = LoadImage("assets/pause.png");
    ImageResize(&image, 48 * image.width / image.height, 48); pause = LoadTextureFromImage(image); UnloadImage(image);

    image = LoadImage("assets/play.png");
    ImageResize(&image, 48 * image.width / image.height, 48); play = LoadTextureFromImage(image); UnloadImage(image);

    image = LoadImage("assets/random.png"); 
    ImageResize(&image, 30, 30); randomIcon = LoadTextureFromImage(image); UnloadImage(image);

    font = LoadFontEx("assets/fonts/Roboto/Roboto-Bold.ttf", 24, 0, 0);
    titleFont = LoadFontEx("assets/fonts/Roboto/Roboto-Bold.ttf", 36, 0, 0);
    miniTitleFont = LoadFontEx("assets/fonts/Roboto/Roboto-Medium.ttf", 20, 0, 0);
    vertexFont = LoadFontEx("assets/fonts/Roboto/Roboto-Medium.ttf", 24, 0, 0);
    SetTextureFilter(titleFont.texture, TEXTURE_FILTER_BILINEAR);   
    SetTextureFilter(miniTitleFont.texture, TEXTURE_FILTER_BILINEAR);   
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
}

FindCC::~FindCC() {
    UnloadTexture(setting); UnloadTexture(pause); UnloadTexture(play); UnloadTexture(randomIcon);
    UnloadFont(font); UnloadFont(titleFont); UnloadFont(miniTitleFont); UnloadFont(vertexFont);
}

void FindCC::SetCode(const std::vector<std::string>& code) { currentCodeLines = code; }

void FindCC::PushNewStep(const std::vector<CCVertexState>& nextNodes, const std::vector<CCEdgeState>& nextEdges, int codeLine) {
    std::vector<CCVertexTransform> stepNodes; std::vector<CCEdgeTransform> stepEdges;
    for (size_t i = 0; i < nextNodes.size(); i++) {
        CCVertexState uState = (i < currentNodes.size()) ? currentNodes[i] : nextNodes[i];
        if (i >= currentNodes.size()) uState.opacity = 0.0f; 
        stepNodes.push_back({uState, nextNodes[i]});
    }
    for (size_t i = 0; i < nextEdges.size(); i++) {
        CCEdgeState uState = (i < currentEdges.size()) ? currentEdges[i] : nextEdges[i];
        if (i >= currentEdges.size()) { uState.opacity = 0.0f; uState.endPos = uState.startPos; }
        stepEdges.push_back({uState, nextEdges[i]});
    }
    currentNodes = nextNodes; currentEdges = nextEdges;
    AnimationNodes.push_back(stepNodes); AnimationEdges.push_back(stepEdges);
    stepCodeLines.push_back(codeLine);
}

void FindCC::UpdatePositionsAfterDrag() {
    if (internalNodes.empty()) return;
    for (size_t i = 0; i < internalNodes.size(); i++) {
        if (i < currentNodes.size()) currentNodes[i].pos = internalNodes[i].pos;
    }
    for (size_t j = 0; j < internalEdges.size(); j++) {
        if (j < currentEdges.size()) {
            currentEdges[j].startPos = internalNodes[internalEdges[j].u].pos;
            currentEdges[j].endPos = internalNodes[internalEdges[j].v].pos;
        }
    }
    for (auto& frame : AnimationNodes) {
        for (size_t i = 0; i < frame.size(); i++) {
            if (i < internalNodes.size()) {
                frame[i].u.pos = internalNodes[i].pos;
                frame[i].v.pos = internalNodes[i].pos;
            }
        }
    }
    for (auto& frame : AnimationEdges) {
        for (size_t j = 0; j < frame.size(); j++) {
            if (j < internalEdges.size()) {
                frame[j].u.startPos = internalNodes[internalEdges[j].u].pos;
                frame[j].u.endPos = internalNodes[internalEdges[j].v].pos;
                frame[j].v.startPos = internalNodes[internalEdges[j].u].pos;
                frame[j].v.endPos = internalNodes[internalEdges[j].v].pos;
            }
        }
    }
}

void FindCC::GenerateInit(int n, const std::vector<CCEdge>& edges) {
    AnimationNodes.clear(); AnimationEdges.clear(); stepCodeLines.clear();
    internalNodes.clear(); internalEdges.clear(); adjList.clear();
    currentNodes.clear(); currentEdges.clear();
    curIndex = 0; TotalTime = 0.0f; isPause = false;

    SetCode({
        "void InitGraph() {",
        "  Build graph from input",
        "  with " + std::to_string(n) + " vertices.",
        "}"
    });

    float workspaceLeft = 319.0f + 100.0f; 
    float workspaceRight = (float)GetScreenWidth() - 100.0f;
    float workspaceTop = StartY + 100.0f;
    float workspaceBottom = 800.0f;

    for (int i = 0; i < n; i++) {
        CCNode node;
        node.id = i;
        bool valid;
        int attempts = 0;
        do {
            valid = true;
            node.pos.x = workspaceLeft + (rand() % (int)(workspaceRight - workspaceLeft));
            node.pos.y = workspaceTop + (rand() % (int)(workspaceBottom - workspaceTop));
            
            for (int j = 0; j < i; j++) {
                float dx = node.pos.x - internalNodes[j].pos.x;
                float dy = node.pos.y - internalNodes[j].pos.y;
                if (sqrt(dx*dx + dy*dy) < 120.0f) { valid = false; break; }
            }
            attempts++;
        } while (!valid && attempts < 100);
        internalNodes.push_back(node);
    }

    internalEdges = edges;
    adjList.assign(n, std::vector<int>());
    for (auto& e : internalEdges) {
        adjList[e.u].push_back(e.v);
        adjList[e.v].push_back(e.u);
    }

    std::vector<CCVertexState> nextNodes; std::vector<CCEdgeState> nextEdges;
    for (int i = 0; i < n; i++) {
        CCVertexState vState;
        vState.pos = internalNodes[i].pos; vState.text = std::to_string(i);
        vState.radius = 28.0f; vState.opacity = 1.0f; 
        vState.themeState = ElementTheme::DEFAULT; vState.ccColorIndex = -1;
        nextNodes.push_back(vState);
    }
    for (auto& e : internalEdges) {
        CCEdgeState eState;
        eState.startPos = internalNodes[e.u].pos; eState.endPos = internalNodes[e.v].pos;
        eState.thickness = 4.0f; eState.opacity = 1.0f; 
        eState.themeState = ElementTheme::DEFAULT; eState.ccColorIndex = -1;
        nextEdges.push_back(eState);
    }

    PushNewStep(nextNodes, nextEdges, 0);
    PushNewStep(nextNodes, nextEdges, 1);
    PushNewStep(nextNodes, nextEdges, 2);
    typeQuery = -1;
}

void FindCC::DFS(int u, int ccIndex, std::vector<bool>& visited, std::vector<CCVertexState>& nextNodes, std::vector<CCEdgeState>& nextEdges) {
    visited[u] = true;
    nextNodes[u].themeState = ElementTheme::SUCCESS;
    nextNodes[u].ccColorIndex = ccIndex;
    PushNewStep(nextNodes, nextEdges, 8);

    for (int v : adjList[u]) {
        int edgeIdx = -1;
        for (size_t i = 0; i < internalEdges.size(); i++) {
            if ((internalEdges[i].u == u && internalEdges[i].v == v) || (internalEdges[i].u == v && internalEdges[i].v == u)) {
                edgeIdx = i; break;
            }
        }

        if (edgeIdx != -1 && nextEdges[edgeIdx].themeState != ElementTheme::SUCCESS) {
            nextEdges[edgeIdx].themeState = ElementTheme::HIGHLIGHT;
            PushNewStep(nextNodes, nextEdges, 9);

            if (!visited[v]) {
                PushNewStep(nextNodes, nextEdges, 10);
                nextEdges[edgeIdx].themeState = ElementTheme::SUCCESS;
                nextEdges[edgeIdx].ccColorIndex = ccIndex;
                PushNewStep(nextNodes, nextEdges, 11);
                DFS(v, ccIndex, visited, nextNodes, nextEdges);
            } else {
                nextEdges[edgeIdx].themeState = ElementTheme::SUCCESS;
                nextEdges[edgeIdx].ccColorIndex = ccIndex;
                PushNewStep(nextNodes, nextEdges, 10);
            }
        }
    }
}

void FindCC::GenerateFindCC() {
    if (internalNodes.empty()) { typeQuery = -1; return; }
    AnimationNodes.clear(); AnimationEdges.clear(); stepCodeLines.clear();
    curIndex = 0; TotalTime = 0.0f; isPause = false;

    SetCode({
/* 0*/  "void FindConnectedComponents() {",
/* 1*/  "  for (int i = 0; i < V; i++)",
/* 2*/  "    visited[i] = false;",
/* 3*/  "  int ccIndex = 0;",
/* 4*/  "  for (int i = 0; i < V; i++) {",
/* 5*/  "    if (!visited[i]) {",
/* 6*/  "      DFS(i, ccIndex);",
/* 7*/  "      ccIndex++;",
/* 8*/  "    }",
/* 9*/  "  }",
/*10*/  "}",
/*11*/  "",
/*12*/  "void DFS(int u, int ccIndex) {",
/*13*/  "  visited[u] = true;",
/*14*/  "  for (int v : adj[u]) {",
/*15*/  "    if (!visited[v]) {",
/*16*/  "      DFS(v, ccIndex);",
/*17*/  "    }",
/*18*/  "  }",
/*19*/  "}"
    });

    std::vector<CCVertexState> nextNodes = currentNodes;
    std::vector<CCEdgeState> nextEdges = currentEdges;
    PushNewStep(nextNodes, nextEdges, 0);

    for(auto& n : nextNodes) { n.themeState = ElementTheme::DEFAULT; n.ccColorIndex = -1; }
    for(auto& e : nextEdges) { e.themeState = ElementTheme::DEFAULT; e.ccColorIndex = -1; e.opacity = 0.2f; }
    PushNewStep(nextNodes, nextEdges, 1);
    PushNewStep(nextNodes, nextEdges, 2);

    int n = internalNodes.size();
    std::vector<bool> visited(n, false);
    int ccIndex = 0;
    PushNewStep(nextNodes, nextEdges, 3);

    for (int i = 0; i < n; i++) {
        PushNewStep(nextNodes, nextEdges, 4);
        
        nextNodes[i].themeState = ElementTheme::HIGHLIGHT;
        PushNewStep(nextNodes, nextEdges, 5);

        if (!visited[i]) {
            PushNewStep(nextNodes, nextEdges, 6);
            DFS(i, ccIndex, visited, nextNodes, nextEdges);
            ccIndex++;
            PushNewStep(nextNodes, nextEdges, 7);
        } else {
            nextNodes[i].themeState = ElementTheme::SUCCESS;
        }
    }

    PushNewStep(nextNodes, nextEdges, 10);
    typeQuery = -1;
}

void FindCC::Solve(const Palette &colors) {
    if (typeQuery == 0 && numVerticesToGen > 0) { GenerateInit(numVerticesToGen, customEdgesToGen); numVerticesToGen = 0; customEdgesToGen.clear(); }
    else if (typeQuery == 1) { GenerateFindCC(); }
    Draw(colors);
}

void FindCC::Draw(const Palette &colors) {  
    ClearBackground(colors.background);

    if (isInitDialogOpen && IsFileDropped()) {
        FilePathList droppedFiles = LoadDroppedFiles();
        if (droppedFiles.count > 0) {
            std::ifstream file(droppedFiles.paths[0]);
            if (file.is_open()) {
                std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                strncpy(inputText, content.c_str(), 4095);
                inputText[4095] = '\0';
                inputCount = strlen(inputText);
            }
        }
        UnloadDroppedFiles(droppedFiles);
    }
    
    DrawLineEx({319.0f, StartY}, {319.0f, EndY}, 1.0f, colors.border);
    DrawTexture(setting, StartX + 10, StartY + 18, colors.primaryAcc);
    DrawTextEx(font, "Controls", {StartX + 20 + (float)setting.width, StartY + 25}, 24, 1.0f, colors.textPrimary);
    DrawLineEx({StartX, 142}, {319.0f, 142}, 1.0f, colors.border);
    DrawTextEx(miniTitleFont, "DATA OPERATIONS",{24, 142 + 24}, 20, 1.0f, colors.textPrimary);

    InitButton.Draw(); FindCCButton.Draw();

    if (InitButton.isPressed()) { isInitDialogOpen = true; inputText[0] = '\0'; inputCount = 0; typeQuery = -1; }
    if (FindCCButton.isPressed()) { typeQuery = 1; }   

    DrawTextEx(miniTitleFont, "PLAYBACK",{24, 580}, 20, 1.0f, colors.textPrimary);
    if (isPause) DrawTexture(play, 135, 616, colors.primaryAcc); else DrawTexture(pause, 135, 616, colors.primaryAcc);
    Rectangle playPauseRect = {135.0f, 616.0f, (float)play.width, (float)play.height};
    if (CheckCollisionPointRec(GetMousePosition(), playPauseRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) isPause ^= 1;

    DrawTextEx(miniTitleFont, "Speed", {24, 690}, 20, 0.8f, colors.textPrimary);
    mySpeedSlider.Draw(colors);
    DrawTextEx(miniTitleFont, FloatToString(mySpeedSlider.getSpeed()).c_str(), {280, 690}, 20, 0.8f, colors.textPrimary);

    DrawLineEx({319, 128}, {1920, 128}, 1.0f, colors.border);
    DrawTextEx(titleFont, "CONNECTED COMPONENTS", {319 + 32, 85}, 32, 0.8f, colors.textPrimary);

    if (!isInitDialogOpen) {
        Vector2 mousePos = GetMousePosition();
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            for (int i = (int)internalNodes.size() - 1; i >= 0; i--) {
                if (CheckCollisionPointCircle(mousePos, internalNodes[i].pos, 28.0f)) {
                    draggingNodeIndex = i;
                    dragOffset.x = mousePos.x - internalNodes[i].pos.x;
                    dragOffset.y = mousePos.y - internalNodes[i].pos.y;
                    break;
                }
            }
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            draggingNodeIndex = -1;
        }
        if (draggingNodeIndex != -1) {
            float workspaceLeft = 319.0f + 30.0f;
            float workspaceRight = (float)GetScreenWidth() - 30.0f;
            float workspaceTop = StartY + 30.0f;
            float workspaceBottom = (float)GetScreenHeight() - 30.0f;
            
            float newX = mousePos.x - dragOffset.x;
            float newY = mousePos.y - dragOffset.y;
            
            if (newX < workspaceLeft) newX = workspaceLeft;
            if (newX > workspaceRight) newX = workspaceRight;
            if (newY < workspaceTop) newY = workspaceTop;
            if (newY > workspaceBottom) newY = workspaceBottom;

            internalNodes[draggingNodeIndex].pos.x = newX;
            internalNodes[draggingNodeIndex].pos.y = newY;
            UpdatePositionsAfterDrag();
        }
    }

    if (!AnimationEdges.empty() && !AnimationNodes.empty()) {
        auto DrawFrame = [&](const std::vector<CCEdgeTransform>& eFrame, const std::vector<CCVertexTransform>& vFrame, float gTime, bool isFinal) {
            for (size_t i = 0; i < eFrame.size(); i++) {
                auto state = isFinal ? eFrame[i].v : eFrame[i].u;
                float currentOpacity = isFinal ? state.opacity : eFrame[i].u.opacity + (eFrame[i].v.opacity - eFrame[i].u.opacity) * gTime;
                
                Color c = colors.edgeDefault;
                if(state.themeState == ElementTheme::HIGHLIGHT) c = colors.edgeHighlight;
                if(state.themeState == ElementTheme::SUCCESS) c = state.ccColorIndex >= 0 ? ccColors[state.ccColorIndex % 10] : colors.edgeDefault;

                Vector2 start = state.startPos;
                Vector2 end = state.endPos;
                Vector2 dir = { end.x - start.x, end.y - start.y };
                float len = sqrt(dir.x*dir.x + dir.y*dir.y);
                Vector2 controlPos = { (start.x + end.x)/2.0f, (start.y + end.y)/2.0f };
                if (len > 0) {
                    Vector2 normal = { -dir.y/len, dir.x/len };
                    if (start.x > end.x || (start.x == end.x && start.y > end.y)) { normal.x *= -1; normal.y *= -1; }
                    controlPos.x += normal.x * 40.0f; controlPos.y += normal.y * 40.0f;
                }
                DrawCustomBezierQuad(start, end, controlPos, state.thickness, Fade(c, currentOpacity));
            }

            for (size_t i = 0; i < vFrame.size(); i++) {
                auto state = isFinal ? vFrame[i].v : vFrame[i].u;
                float currentOpacity = isFinal ? state.opacity : vFrame[i].u.opacity + (vFrame[i].v.opacity - vFrame[i].u.opacity) * gTime;
                if (currentOpacity <= 0.0f) continue;

                Color bg = colors.surface;
                Color borderC = colors.nodeDefault;
                Color textC = colors.textPrimary;
                
                if (state.themeState == ElementTheme::HIGHLIGHT) {
                    bg = colors.nodeHighlight; borderC = colors.nodeHighlight; textC = colors.nodeText;
                } else if (state.themeState == ElementTheme::SUCCESS) {
                    bg = state.ccColorIndex >= 0 ? ccColors[state.ccColorIndex % 10] : colors.nodeSuccess;
                    borderC = bg; textC = colors.nodeText;
                }

                DrawCircleV(state.pos, state.radius, Fade(bg, currentOpacity));
                DrawCircleLines(state.pos.x, state.pos.y, state.radius, Fade(borderC, currentOpacity));
                
                Vector2 tSize = MeasureTextEx(vertexFont, state.text.c_str(), 20, 1.0f);
                DrawTextEx(vertexFont, state.text.c_str(), { state.pos.x - tSize.x/2.0f, state.pos.y - tSize.y/2.0f }, 20, 1.0f, Fade(textC, currentOpacity));
            }
        };

        if (curIndex >= AnimationEdges.size()) {
            DrawFrame(AnimationEdges.back(), AnimationNodes.back(), 1.0f, true);
        } else {
            DrawFrame(AnimationEdges[curIndex], AnimationNodes[curIndex], TotalTime, false);
        }
    }

    int maxStep = (AnimationNodes.size() > 0) ? AnimationNodes.size() - 1 : 0;
    if (!isPause) {
        TotalTime += GetFrameTime() * mySpeedSlider.getSpeed(); 
        if (TotalTime >= 1.0f) {
            if (curIndex < maxStep) { curIndex++; TotalTime -= 1.0f; } 
            else { TotalTime = 1.0f; isPause = true; }
        }
    }

    DrawTextEx(miniTitleFont, "Timeline", {24, 750}, 20, 0.8f, colors.textPrimary);
    int prevIndex = curIndex;
    myTimeline.Draw({24, 780}, {295, 780}, maxStep, curIndex, colors);
    curIndex = myTimeline.getStep();
    if (curIndex != prevIndex) { TotalTime = 1.0f; isPause = true; }

    DrawLineEx({319, 830}, {1920, 830}, 1.0f, colors.border);
    DrawTextEx(titleFont, "EXECUTE LOG", {319 + 32, 840}, 32, 0.4f, colors.textPrimary);
    
    float codeStartY = 880.0f;
    int activeLine = (curIndex < stepCodeLines.size()) ? stepCodeLines[curIndex] : -1;
    
    int maxVisibleLines = 7; 
    int totalLines = currentCodeLines.size();
    
    int startLine = 0;
    if (activeLine >= 0) startLine = activeLine - (maxVisibleLines / 2); 
    if (startLine < 0) startLine = 0;
    if (totalLines > maxVisibleLines && startLine > totalLines - maxVisibleLines) startLine = totalLines - maxVisibleLines;

    int displayIndex = 0; 
    for (int i = startLine; i < totalLines && displayIndex < maxVisibleLines; i++) {
        Color c = (i == activeLine) ? colors.textPrimary : colors.textSecondary;
        float currentY = codeStartY + displayIndex * 25;
        if (i == activeLine) DrawRectangle(319 + 20, currentY - 2, 800, 25, Fade(colors.primaryAcc, 0.3f));
        DrawTextEx(font, currentCodeLines[i].c_str(), {319 + 32, currentY}, 20, 1.0f, c);
        displayIndex++;
    }

    if (isInitDialogOpen) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.6f));
        float dialogWidth = 600.0f; float dialogHeight = 500.0f;
        float dialogX = (GetScreenWidth() - dialogWidth) / 2.0f; float dialogY = (GetScreenHeight() - dialogHeight) / 2.0f;
        Rectangle dialogRec = { dialogX, dialogY, dialogWidth, dialogHeight };

        DrawRectangleRec(dialogRec, colors.surface); DrawRectangleLinesEx(dialogRec, 2.0f, colors.primaryAcc);
        DrawTextEx(titleFont, "Input Unweighted Graph", {dialogX + 20, dialogY + 20}, 30, 1.0f, colors.textPrimary);

        Rectangle randomBtnRec = { dialogX + dialogWidth - 50, dialogY + 20, 30.0f, 30.0f };
        bool isRandomHovered = CheckCollisionPointRec(GetMousePosition(), randomBtnRec);
        if (isRandomHovered) { DrawRectangleRec(randomBtnRec, Fade(colors.primaryAcc, 0.2f)); DrawTextEx(miniTitleFont, "Random", {randomBtnRec.x - 65, randomBtnRec.y + 5}, 16, 1.0f, colors.textSecondary); }
        DrawTexture(randomIcon, randomBtnRec.x, randomBtnRec.y, WHITE);

        if (isRandomHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int n = rand() % 8 + 5; 
            std::string s = std::to_string(n) + "\n";
            
            int numComponents = rand() % 3 + 1;
            std::vector<int> comp(n);
            for(int i = 0; i < n; i++) comp[i] = rand() % numComponents;
            
            std::vector<std::vector<int>> comps(numComponents);
            for(int i = 0; i < n; i++) comps[comp[i]].push_back(i);
            
            for(int c = 0; c < numComponents; c++) {
                if(comps[c].size() > 1) {
                    for(size_t i = 0; i < comps[c].size() - 1; i++) {
                        s += std::to_string(comps[c][i]) + " " + std::to_string(comps[c][i+1]) + "\n";
                    }
                }
            }
            
            for(int i = 0; i < n; i++) {
                for(int j = i + 1; j < n; j++) {
                    if(comp[i] == comp[j] && (rand() % 100 < 30)) {
                        s += std::to_string(i) + " " + std::to_string(j) + "\n";
                    }
                }
            }

            strncpy(inputText, s.c_str(), 4095);
            inputCount = strlen(inputText);
        }

        Rectangle textBoxRec = { dialogX + 20, dialogY + 80, dialogWidth - 40, 320 };
        DrawRectangleRec(textBoxRec, colors.background); DrawRectangleLinesEx(textBoxRec, 1.0f, colors.border);
        DrawTextEx(font, inputText, {textBoxRec.x + 10, textBoxRec.y + 13}, 24, 1.0f, colors.textPrimary);

        int key = GetCharPressed();
        while (key > 0) { 
            if (((key >= '0' && key <= '9') || key == ' ') && (inputCount < 4095)) { 
                inputText[inputCount++] = (char)key; inputText[inputCount] = '\0'; 
            } 
            key = GetCharPressed(); 
        }
        if (IsKeyPressed(KEY_ENTER) && inputCount < 4095) {
            inputText[inputCount++] = '\n'; inputText[inputCount] = '\0';
        }
        if (IsKeyPressed(KEY_BACKSPACE) && inputCount > 0) inputText[--inputCount] = '\0';
        
        framesCounter++;
        if ((framesCounter / 30) % 2 == 0) {
            int lineCount = 0;
            int lastNewlineIdx = -1;
            for (int i = 0; i < inputCount; i++) {
                if (inputText[i] == '\n') {
                    lineCount++;
                    lastNewlineIdx = i;
                }
            }
            const char* lastLine = (lastNewlineIdx == -1) ? inputText : (inputText + lastNewlineIdx + 1);
            Vector2 lastLineSize = MeasureTextEx(font, lastLine, 24, 1.0f);
            float lineHeight = MeasureTextEx(font, "A", 24, 1.0f).y;
            float cursorX = textBoxRec.x + 10 + lastLineSize.x + 2;
            float cursorY = textBoxRec.y + 13 + lineCount * lineHeight;
            DrawLineEx({ cursorX, cursorY }, { cursorX, cursorY + 24 }, 2.0f, colors.primaryAcc);
        }

        DrawTextEx(miniTitleFont, "Format: Line 1 = n. Next Lines = u v.  File drag-and-drop supported.", { dialogX + 20, dialogY + 410 }, 14, 1.0f, colors.textSecondary);

        Rectangle okBtnRec = { dialogX + dialogWidth / 2 - 60, dialogY + 440, 120, 40 };
        bool isOkHovered = CheckCollisionPointRec(GetMousePosition(), okBtnRec);
        DrawRectangleRec(okBtnRec, isOkHovered ? colors.primaryAcc : colors.border);
        DrawTextEx(miniTitleFont, "CONFIRM", {okBtnRec.x + 20, okBtnRec.y + 10}, 18, 1.0f, colors.textPrimary);

        if ((isOkHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))) {
            isInitDialogOpen = false; 
            std::vector<int> tokens;
            int cur_num = 0; bool hasNum = false;
            for(int i = 0; i <= inputCount; i++) {
                if(inputText[i] >= '0' && inputText[i] <= '9') {
                    cur_num = cur_num * 10 + (inputText[i] - '0');
                    hasNum = true;
                } else if(hasNum) {
                    tokens.push_back(cur_num);
                    cur_num = 0; hasNum = false;
                }
            }
            if (tokens.size() > 0) {
                int n = tokens[0];
                if (n < 3) n = 3; if (n > 20) n = 20;
                std::vector<CCEdge> customEdges;
                int idx = 0;
                for(size_t i = 1; i + 1 < tokens.size(); i += 2) {
                    int u = tokens[i]; int v = tokens[i+1];
                    if (u < n && v < n && u != v) {
                        bool dup = false;
                        for(auto& e : customEdges) {
                            if ((e.u == u && e.v == v) || (e.u == v && e.v == u)) { dup = true; break; }
                        }
                        if (!dup) customEdges.push_back({u, v, idx++});
                    }
                }
                typeQuery = 0; numVerticesToGen = n; customEdgesToGen = customEdges;
            } else { typeQuery = -1; }
        }
        else if (IsKeyPressed(KEY_ESCAPE)) { isInitDialogOpen = false; typeQuery = -1; }
    }
}

std::string FindCC::FloatToString(float value) { char buffer[10]; snprintf(buffer, sizeof(buffer), "%.1fx", value); return std::string(buffer); }