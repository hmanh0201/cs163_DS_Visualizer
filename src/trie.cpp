#include "Trie.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <cstring>
#include <algorithm>

Trie::Trie(int width, int height) 
{
    StartX = 0; StartY = height * 65 / 1080; EndX = width; EndY = height;
    curIndex = 0; TotalTime = 0.0f; isPause = false; typeQuery = -1;
    
    // Khởi tạo Root
    tree.push_back({'*', false, true, -1, {}, {0,0}});
    rootIndex = 0;

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

Trie::~Trie() {
    UnloadTexture(setting); UnloadTexture(pause); UnloadTexture(play); UnloadTexture(randomIcon);
    UnloadFont(font); UnloadFont(titleFont); UnloadFont(miniTitleFont); UnloadFont(vertexFont);
}

void Trie::SetCode(const std::vector<std::string>& code) { currentCodeLines = code; }

void Trie::PushNewStep(const std::vector<VertexState>& nextNodes, const std::vector<EdgeState>& nextEdges, int codeLine) {
    std::vector<VertexTransform> stepNodes; std::vector<EdgeTransform> stepEdges;

    for (size_t i = 0; i < nextNodes.size(); i++) {
        VertexState uState = (i < currentNodes.size()) ? currentNodes[i] : nextNodes[i];
        if (i >= currentNodes.size()) uState.opacity = 0.0f; 
        stepNodes.push_back({uState, nextNodes[i]});
    }
    for (size_t i = 0; i < nextEdges.size(); i++) {
        EdgeState uState = (i < currentEdges.size()) ? currentEdges[i] : nextEdges[i];
        if (i >= currentEdges.size()) { uState.opacity = 0.0f; uState.endPos = uState.startPos; }
        stepEdges.push_back({uState, nextEdges[i]});
    }

    currentNodes = nextNodes; currentEdges = nextEdges;
    AnimationNodes.push_back(stepNodes); AnimationEdges.push_back(stepEdges);
    stepCodeLines.push_back(codeLine);
}

void Trie::CalculatePositions(int node, int depth, float leftX, float rightX) {
    if (node == -1 || node >= tree.size() || !tree[node].active) return;

    float midX = (leftX + rightX) / 2.0f;
    float midY = StartY + 120.0f + depth * 80.0f; 
    tree[node].targetPos = {midX, midY};

    int activeCount = 0;
    for (auto const& [c, childIdx] : tree[node].children) {
        if (tree[childIdx].active) activeCount++;
    }

    if (activeCount == 0) return;

    float step = (rightX - leftX) / activeCount;
    float curLeft = leftX;

    for (auto const& [c, childIdx] : tree[node].children) {
        if (tree[childIdx].active) {
            CalculatePositions(childIdx, depth + 1, curLeft, curLeft + step);
            curLeft += step;
        }
    }
}

void Trie::GetTreeState(std::vector<VertexState>& nextNodes, std::vector<EdgeState>& nextEdges) {
    float workspaceLeft = 319.0f + 50.0f; 
    float workspaceRight = (float)GetScreenWidth() - 50.0f;
    CalculatePositions(rootIndex, 0, workspaceLeft, workspaceRight);

    nextNodes.assign(tree.size(), VertexState());
    nextEdges.assign(tree.size(), EdgeState());

    for (int i = 0; i < tree.size(); i++) {
        if (tree[i].active) {
            nextNodes[i].pos = tree[i].targetPos;
            
            std::string textVal = std::string(1, tree[i].val);
            if (tree[i].isEnd && tree[i].val != '*') textVal = "[" + textVal + "]";
            nextNodes[i].text = textVal;
            
            nextNodes[i].opacity = 1.0f;
            nextNodes[i].radius = 22.0f; 
            if (i < currentNodes.size()) nextNodes[i].themeState = currentNodes[i].themeState;
            else nextNodes[i].themeState = ElementTheme::DEFAULT;
        } else {
            nextNodes[i].opacity = 0.0f;
        }
    }

    for (int i = 0; i < tree.size(); i++) {
        if (tree[i].active) {
            for (auto const& [c, childIdx] : tree[i].children) {
                if (tree[childIdx].active) {
                    nextEdges[childIdx].startPos = tree[i].targetPos;
                    nextEdges[childIdx].endPos = tree[childIdx].targetPos;
                    nextEdges[childIdx].opacity = 1.0f;
                    nextEdges[childIdx].thickness = 3.0f;
                }
            }
        }
    }
    
    for (int i = 0; i < tree.size(); i++) {
        if (i < currentEdges.size()) nextEdges[i].themeState = currentEdges[i].themeState;
    }
}

void Trie::GenerateInit(const std::vector<std::string>& values) {
    AnimationNodes.clear(); AnimationEdges.clear(); stepCodeLines.clear();
    tree.clear(); tree.push_back({'*', false, true, -1, {}, {0,0}}); rootIndex = 0;
    curIndex = 0; TotalTime = 0.0f; isPause = false;
    currentNodes.clear(); currentEdges.clear();
    
    SetCode({
        "void Init() {",
        "  root = new Node('*');",
        "  for (string word : values)",
        "    insert(word);",
        "}"
    });

    std::vector<VertexState> nextNodes; std::vector<EdgeState> nextEdges;
    GetTreeState(nextNodes, nextEdges);
    PushNewStep(nextNodes, nextEdges, 0); 
    PushNewStep(nextNodes, nextEdges, 1); 
    
    for (std::string word : values) {
        GenerateAdd(word); 
    }
    
    GetTreeState(nextNodes, nextEdges);
    for(auto& n : nextNodes) n.themeState = ElementTheme::DEFAULT;
    PushNewStep(nextNodes, nextEdges, 4); 
    
    typeQuery = -1; 
}

void Trie::GenerateAdd(std::string word) {
    if (typeQuery != 0) { AnimationNodes.clear(); AnimationEdges.clear(); stepCodeLines.clear(); }
    curIndex = 0; TotalTime = 0.0f; isPause = false;
    
    SetCode({
/* 0*/  "void insert(string word) {",
/* 1*/  "  Node* curr = root;",
/* 2*/  "  for (char c : word) {",
/* 3*/  "    if (curr->children[c] == NULL)",
/* 4*/  "      curr->children[c] = new Node(c);",
/* 5*/  "    curr = curr->children[c];",
/* 6*/  "  }",
/* 7*/  "  curr->isEnd = true;",
/* 8*/  "}"
    });

    std::vector<VertexState> nextNodes; std::vector<EdgeState> nextEdges;
    GetTreeState(nextNodes, nextEdges);
    if(typeQuery != 0) PushNewStep(nextNodes, nextEdges, 0); 
    
    int curr = rootIndex;
    GetTreeState(nextNodes, nextEdges);
    nextNodes[curr].themeState = ElementTheme::HIGHLIGHT;
    PushNewStep(nextNodes, nextEdges, 1);

    for (char c : word) {
        PushNewStep(nextNodes, nextEdges, 2);
        
        if (tree[curr].children.find(c) == tree[curr].children.end() || !tree[tree[curr].children[c]].active) {
            PushNewStep(nextNodes, nextEdges, 3);
            
            TrieNode newNode;
            newNode.val = c; newNode.isEnd = false; newNode.active = true; newNode.parent = curr;
            tree.push_back(newNode);
            int newIdx = tree.size() - 1;
            tree[curr].children[c] = newIdx;

            GetTreeState(nextNodes, nextEdges); 
            
            VertexState startNode = nextNodes[newIdx];
            EdgeState startEdge = nextEdges[newIdx];
            startNode.pos = tree[curr].targetPos;
            startEdge.startPos = tree[curr].targetPos; startEdge.endPos = tree[curr].targetPos;
            startNode.opacity = 0.0f; startEdge.opacity = 0.0f;

            while(currentNodes.size() <= newIdx) currentNodes.push_back(startNode);
            while(currentEdges.size() <= newIdx) currentEdges.push_back(startEdge);
            currentNodes[newIdx] = startNode; currentEdges[newIdx] = startEdge;

            nextNodes[newIdx].themeState = ElementTheme::SUCCESS;
            PushNewStep(nextNodes, nextEdges, 4);
        }

        int nextCurr = tree[curr].children[c];
        GetTreeState(nextNodes, nextEdges);
        nextNodes[curr].themeState = ElementTheme::DEFAULT;
        nextNodes[nextCurr].themeState = ElementTheme::HIGHLIGHT;
        PushNewStep(nextNodes, nextEdges, 5);
        curr = nextCurr;
    }

    tree[curr].isEnd = true;
    GetTreeState(nextNodes, nextEdges);
    nextNodes[curr].themeState = ElementTheme::SUCCESS;
    PushNewStep(nextNodes, nextEdges, 7);

    GetTreeState(nextNodes, nextEdges);
    for(auto& n : nextNodes) n.themeState = ElementTheme::DEFAULT;
    PushNewStep(nextNodes, nextEdges, 8); 
    
    if(typeQuery != 0) typeQuery = -1;
}

void Trie::GenerateSearch(std::string word) {
    AnimationNodes.clear(); AnimationEdges.clear(); stepCodeLines.clear();
    curIndex = 0; TotalTime = 0.0f; isPause = false;
    
    SetCode({
/* 0*/  "bool search(string word) {",
/* 1*/  "  Node* curr = root;",
/* 2*/  "  for (char c : word) {",
/* 3*/  "    if (curr->children[c] == NULL)",
/* 4*/  "      return false;",
/* 5*/  "    curr = curr->children[c];",
/* 6*/  "  }",
/* 7*/  "  return curr->isEnd;",
/* 8*/  "}"
    });

    std::vector<VertexState> nextNodes; std::vector<EdgeState> nextEdges;
    GetTreeState(nextNodes, nextEdges);
    PushNewStep(nextNodes, nextEdges, 0);

    int curr = rootIndex;
    GetTreeState(nextNodes, nextEdges);
    nextNodes[curr].themeState = ElementTheme::HIGHLIGHT;
    PushNewStep(nextNodes, nextEdges, 1);

    bool found = true;
    for (char c : word) {
        PushNewStep(nextNodes, nextEdges, 2);
        
        if (tree[curr].children.find(c) == tree[curr].children.end() || !tree[tree[curr].children[c]].active) {
            PushNewStep(nextNodes, nextEdges, 3);
            PushNewStep(nextNodes, nextEdges, 4);
            found = false; break;
        }

        int nextCurr = tree[curr].children[c];
        GetTreeState(nextNodes, nextEdges);
        nextNodes[curr].themeState = ElementTheme::DEFAULT;
        nextNodes[nextCurr].themeState = ElementTheme::HIGHLIGHT;
        PushNewStep(nextNodes, nextEdges, 5);
        curr = nextCurr;
    }

    if (found) {
        if (tree[curr].isEnd) nextNodes[curr].themeState = ElementTheme::SUCCESS;
        PushNewStep(nextNodes, nextEdges, 7);
    }

    GetTreeState(nextNodes, nextEdges);
    for(auto& n : nextNodes) n.themeState = ElementTheme::DEFAULT;
    PushNewStep(nextNodes, nextEdges, 8);
    typeQuery = -1;
}

void Trie::GenerateDelete(std::string word) {
    AnimationNodes.clear(); AnimationEdges.clear(); stepCodeLines.clear();
    curIndex = 0; TotalTime = 0.0f; isPause = false;

    SetCode({
/* 0*/  "void deleteWord(string word) {",
/* 1*/  "  Node* curr = searchNode(word);",
/* 2*/  "  if (curr == NULL || !curr->isEnd) return;",
/* 3*/  "  curr->isEnd = false;",
/* 4*/  "  while (curr != root && curr->children.empty() && !curr->isEnd) {",
/* 5*/  "    Node* p = curr->parent;",
/* 6*/  "    p->children.erase(curr->val);",
/* 7*/  "    delete curr;",
/* 8*/  "    curr = p;",
/* 9*/  "  }",
/*10*/  "}"
    });

    std::vector<VertexState> nextNodes; std::vector<EdgeState> nextEdges;
    GetTreeState(nextNodes, nextEdges);
    PushNewStep(nextNodes, nextEdges, 0);

    int curr = rootIndex;
    bool exists = true;
    for (char c : word) {
        if (tree[curr].children.find(c) == tree[curr].children.end() || !tree[tree[curr].children[c]].active) { exists = false; break; }
        curr = tree[curr].children[c];
    }
    
    if (!exists || !tree[curr].isEnd) {
        PushNewStep(nextNodes, nextEdges, 1);
        PushNewStep(nextNodes, nextEdges, 2);
        typeQuery = -1; return;
    }

    GetTreeState(nextNodes, nextEdges);
    nextNodes[curr].themeState = ElementTheme::HIGHLIGHT;
    PushNewStep(nextNodes, nextEdges, 1);

    tree[curr].isEnd = false;
    GetTreeState(nextNodes, nextEdges);
    nextNodes[curr].themeState = ElementTheme::HIGHLIGHT; 
    PushNewStep(nextNodes, nextEdges, 3);

    while (curr != rootIndex && !tree[curr].isEnd) {
        PushNewStep(nextNodes, nextEdges, 4);
        
        bool hasActiveChild = false;
        for (auto const& [c, childIdx] : tree[curr].children) {
            if (tree[childIdx].active) { hasActiveChild = true; break; }
        }
        
        if (hasActiveChild) break; 
        
        int p = tree[curr].parent;
        PushNewStep(nextNodes, nextEdges, 5);
        
        tree[curr].active = false; 
        GetTreeState(nextNodes, nextEdges);
        nextNodes[curr].themeState = ElementTheme::SUCCESS; 
        PushNewStep(nextNodes, nextEdges, 6);
        PushNewStep(nextNodes, nextEdges, 7);
        
        curr = p;
        GetTreeState(nextNodes, nextEdges);
        nextNodes[curr].themeState = ElementTheme::HIGHLIGHT;
        PushNewStep(nextNodes, nextEdges, 8);
    }

    GetTreeState(nextNodes, nextEdges);
    for(auto& n : nextNodes) n.themeState = ElementTheme::DEFAULT;
    PushNewStep(nextNodes, nextEdges, 10);
    typeQuery = -1;
}


void Trie::Solve(const Palette &colors) {
    if (!stringQueryList.empty() && typeQuery != -1) {
        if (typeQuery == 0) GenerateInit(stringQueryList);
        else if (typeQuery == 1) GenerateAdd(stringQueryList[0]);
        else if (typeQuery == 2) GenerateDelete(stringQueryList[0]);
        else if (typeQuery == 4) GenerateSearch(stringQueryList[0]);

        stringQueryList.clear(); 
    }
    Draw(colors);
}

void Trie::Draw(const Palette &colors) {  
    ClearBackground(colors.background);
    
    DrawLineEx({319.0f, StartY}, {319.0f, EndY}, 1.0f, colors.border);
    DrawTexture(setting, StartX + 10, StartY + 18, colors.primaryAcc);
    DrawTextEx(font, "Controls", {StartX + 20 + (float)setting.width, StartY + 25}, 24, 1.0f, colors.textPrimary);
    DrawLineEx({StartX, 142}, {319.0f, 142}, 1.0f, colors.border);
    DrawTextEx(miniTitleFont, "DATA OPERATIONS",{24, 142 + 24}, 20, 1.0f, colors.textPrimary);

    InitButton.Draw(); AddButton.Draw(); DeleteButton.Draw(); UpdateButton.Draw(); SearchButton.Draw();

    if (InitButton.isPressed()) { isInsertDialogOpen = true; insertInputText[0] = '\0'; insertInputCount = 0; typeQuery = 0; }
    if (AddButton.isPressed()) { isInsertDialogOpen = true; insertInputText[0] = '\0'; insertInputCount = 0; typeQuery = 1; }   
    if (DeleteButton.isPressed()) { isInsertDialogOpen = true; insertInputText[0] = '\0'; insertInputCount = 0; typeQuery = 2; }
    if (SearchButton.isPressed()) { isInsertDialogOpen = true; insertInputText[0] = '\0'; insertInputCount = 0; typeQuery = 4; }
    
    if (UpdateButton.isPressed()) { isWarningOpen = true; }

    DrawTextEx(miniTitleFont, "PLAYBACK",{24, 580}, 20, 1.0f, colors.textPrimary); 
    if (isPause) DrawTexture(play, 135, 616, colors.primaryAcc); else DrawTexture(pause, 135, 616, colors.primaryAcc);
    Rectangle playPauseRect = {135.0f, 616.0f, (float)play.width, (float)play.height};
    if (CheckCollisionPointRec(GetMousePosition(), playPauseRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) isPause ^= 1;

    DrawTextEx(miniTitleFont, "Speed", {24, 690}, 20, 0.8f, colors.textPrimary);
    mySpeedSlider.Draw(colors);
    DrawTextEx(miniTitleFont, FloatToString(mySpeedSlider.getSpeed()).c_str(), {280, 690}, 20, 0.8f, colors.textPrimary);

    DrawLineEx({319, 128}, {1920, 128}, 1.0f, colors.border);
    DrawTextEx(titleFont, "TRIE", {319 + 32, 85}, 32, 0.8f, colors.textPrimary);

    // --- VISUALIZATION ---
    if (!AnimationEdges.empty() && !AnimationNodes.empty()) {
        if (curIndex >= AnimationEdges.size()) {
            for (size_t i = 0; i < currentEdges.size(); i++) {
                EdgeTransform finalState = {currentEdges[i], currentEdges[i]}; visualizer.RenderDirectedEdge(finalState, 1.0f, colors);
            }
        } else {
            for (auto& step : AnimationEdges[curIndex]) visualizer.RenderDirectedEdge(step, TotalTime, colors);
        }

        if (curIndex >= AnimationNodes.size()) {
            for (size_t i = 0; i < currentNodes.size(); i++) {
                if(currentNodes[i].opacity > 0.0f) {
                    VertexTransform finalState = {currentNodes[i], currentNodes[i]}; visualizer.RenderVertex(finalState, 1.0f, vertexFont, colors);
                }
            }
        } else {
            for (auto& step : AnimationNodes[curIndex]) {
                if(step.v.opacity > 0.0f || step.u.opacity > 0.0f) visualizer.RenderVertex(step, TotalTime, vertexFont, colors);
            }
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

    // Khôi phục Y = 750 cho Timeline
    DrawTextEx(miniTitleFont, "Timeline", {24, 750}, 20, 0.8f, colors.textPrimary);
    int prevIndex = curIndex;
    myTimeline.Draw({24, 780}, {295, 780}, maxStep, curIndex, colors);
    curIndex = myTimeline.getStep();
    if (curIndex != prevIndex) { TotalTime = 1.0f; isPause = true; }

    // --- VẼ EXECUTE LOG TỰ ĐỘNG CUỘN (SLIDING WINDOW) ---
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

    // --- DIALOG NHẬP LIỆU (NHẬP STRING) ---
    if (isInsertDialogOpen) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.6f));
        float dialogWidth = 600.0f; float dialogHeight = 200.0f;
        float dialogX = (GetScreenWidth() - dialogWidth) / 2.0f; float dialogY = (GetScreenHeight() - dialogHeight) / 2.0f;
        Rectangle dialogRec = { dialogX, dialogY, dialogWidth, dialogHeight };

        DrawRectangleRec(dialogRec, colors.surface); DrawRectangleLinesEx(dialogRec, 2.0f, colors.primaryAcc);
        DrawTextEx(titleFont, "Input Word(s)", {dialogX + 20, dialogY + 20}, 30, 1.0f, colors.textPrimary);

        Rectangle randomBtnRec = { dialogX + dialogWidth - 50, dialogY + 20, 30.0f, 30.0f };
        bool isRandomHovered = CheckCollisionPointRec(GetMousePosition(), randomBtnRec);
        if (isRandomHovered) { DrawRectangleRec(randomBtnRec, Fade(colors.primaryAcc, 0.2f)); DrawTextEx(miniTitleFont, "Random", {randomBtnRec.x - 65, randomBtnRec.y + 5}, 16, 1.0f, colors.textSecondary); }
        DrawTexture(randomIcon, randomBtnRec.x, randomBtnRec.y, WHITE);

        if (isRandomHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int cnt = (typeQuery == 0) ? rand()%3 + 2 : 1; std::string input;
            while (cnt--) {
                int wordLen = rand() % 3 + 3; // Random từ 3-5 ký tự
                for(int i=0; i<wordLen; i++) input += (char)('a' + rand() % 26);
                if (cnt > 0) input += " ";
            }
            strcpy(insertInputText, input.c_str()); insertInputCount = input.length();
        }

        Rectangle textBoxRec = { dialogX + 20, dialogY + 80, dialogWidth - 40, 50 };
        DrawRectangleRec(textBoxRec, colors.background); DrawRectangleLinesEx(textBoxRec, 1.0f, colors.border);
        DrawTextEx(font, insertInputText, {textBoxRec.x + 10, textBoxRec.y + 13}, 24, 1.0f, colors.textPrimary);

        int key = GetCharPressed();
        while (key > 0) { 
            // Chỉ nhận chữ cái a-z, A-Z và dấu cách
            if (((key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z') || key == ' ') && (insertInputCount < 255)) { 
                insertInputText[insertInputCount++] = (char)tolower(key); 
                insertInputText[insertInputCount] = '\0'; 
            } 
            key = GetCharPressed(); 
        }
        if (IsKeyPressed(KEY_BACKSPACE) && insertInputCount > 0) insertInputText[--insertInputCount] = '\0';
        
        framesCounter++;
        if ((framesCounter / 30) % 2 == 0) {
            Vector2 textSize = MeasureTextEx(font, insertInputText, 24, 1.0f);
            DrawLineEx({ textBoxRec.x + 12 + textSize.x, textBoxRec.y + 10 }, { textBoxRec.x + 12 + textSize.x, textBoxRec.y + 40 }, 2.0f, colors.primaryAcc);
        }

        DrawTextEx(miniTitleFont, "Press ENTER to confirm | ESC to cancel", { dialogX + 20, dialogY + 145 }, 18, 1.0f, colors.textSecondary);

        if (IsKeyPressed(KEY_ENTER)) {
            isInsertDialogOpen = false; 
            stringQueryList.clear();
            std::string currentStr = "";
            for (int i = 0; i < insertInputCount; i++) {
                if (insertInputText[i] == ' ') {
                    if (!currentStr.empty()) { stringQueryList.push_back(currentStr); currentStr = ""; }
                } else {
                    currentStr += insertInputText[i];
                }
            }
            if (!currentStr.empty()) stringQueryList.push_back(currentStr);
            if (stringQueryList.empty()) typeQuery = -1;
        }
        else if (IsKeyPressed(KEY_ESCAPE)) { isInsertDialogOpen = false; typeQuery = -1; }
    }

    if (isWarningOpen) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.7f));
        float warnW = 600.0f; float warnH = 150.0f;
        float warnX = (GetScreenWidth() - warnW) / 2.0f; float warnY = (GetScreenHeight() - warnH) / 2.0f;
        Rectangle warnRec = { warnX, warnY, warnW, warnH };

        DrawRectangleRec(warnRec, colors.surface);
        DrawRectangleLinesEx(warnRec, 2.0f, RED); 

        const char* warnText = "Operation doesn't fit to specific data structures";
        Vector2 warnTextSize = MeasureTextEx(titleFont, warnText, 24, 1.0f);
        DrawTextEx(titleFont, warnText, {warnX + (warnW - warnTextSize.x)/2.0f, warnY + 40}, 24, 1.0f, RED);
        
        DrawTextEx(miniTitleFont, "Press ESC or CLICK to dismiss", {warnX + 160, warnY + 100}, 18, 1.0f, colors.textSecondary);

        if (IsKeyPressed(KEY_ESCAPE) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            isWarningOpen = false;
        }
    }
}

std::string Trie::FloatToString(float value) { char buffer[10]; snprintf(buffer, sizeof(buffer), "%.1fx", value); return std::string(buffer); }