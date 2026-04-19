#include "Heap.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <cstring>
#include <cmath>

Heap::Heap(int width, int height) 
{
    StartX = 0; StartY = height * 65 / 1080;
    EndX = width; EndY = height;
    curIndex = 0; TotalTime = 0.0f;
    isPause = false; typeQuery = -1;

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
    vertexFont = LoadFontEx("assets/fonts/Roboto/Roboto-Medium.ttf", 30, 0, 0);
    SetTextureFilter(titleFont.texture, TEXTURE_FILTER_BILINEAR);   
    SetTextureFilter(miniTitleFont.texture, TEXTURE_FILTER_BILINEAR);   
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
}

Heap::~Heap() {
    UnloadTexture(setting); UnloadTexture(pause); UnloadTexture(play); UnloadTexture(randomIcon);
    UnloadFont(font); UnloadFont(titleFont); UnloadFont(miniTitleFont); UnloadFont(vertexFont);
}

void Heap::SetCode(const std::vector<std::string>& code) {
    currentCodeLines = code;
}

void Heap::PushNewStep(const std::vector<VertexState>& nextNodes, const std::vector<EdgeState>& nextEdges, int codeLine) {
    std::vector<VertexTransform> stepNodes;
    std::vector<EdgeTransform> stepEdges;

    for (size_t i = 0; i < nextNodes.size(); i++) {
        VertexState uState = (i < currentNodes.size()) ? currentNodes[i] : nextNodes[i];
        if (i >= currentNodes.size()) uState.opacity = 0.0f; 
        stepNodes.push_back({uState, nextNodes[i]});
    }

    for (size_t i = 0; i < nextEdges.size(); i++) {
        EdgeState uState = (i < currentEdges.size()) ? currentEdges[i] : nextEdges[i];
        if (i >= currentEdges.size()) {
            uState.opacity = 0.0f; uState.endPos = uState.startPos; 
        }
        stepEdges.push_back({uState, nextEdges[i]});
    }

    currentNodes = nextNodes; currentEdges = nextEdges;
    AnimationNodes.push_back(stepNodes); AnimationEdges.push_back(stepEdges);
    stepCodeLines.push_back(codeLine);
}

// ================= TÍNH TOÁN TỌA ĐỘ CÂY =================
Vector2 Heap::GetNodePos(int index) {
    if (index == 0) return {(float)GetScreenWidth() / 2.0f + 150.0f, 200.0f}; // Root
    
    int depth = 0;
    int temp = index + 1;
    while (temp > 1) { temp >>= 1; depth++; } // Tính log2(index+1)

    int offset = (index + 1) - (1 << depth);
    float screenW = 1200.0f; // Vùng rộng tối đa của cây
    float startX = ((float)GetScreenWidth() / 2.0f + 150.0f) - screenW / 2.0f;
    float step = screenW / (1 << depth);
    
    float x = startX + step * (offset + 0.5f);
    float y = 200.0f + depth * 120.0f; // Khoảng cách dọc các tầng
    return {x, y};
}

void Heap::SyncTreeState() {
    currentNodes.clear(); currentEdges.clear();
    for (int i = 0; i < internalHeap.size(); i++) {
        VertexState node;
        node.pos = GetNodePos(i); node.text = std::to_string(internalHeap[i]);
        node.opacity = 1.0f; node.radius = 30.0f; node.themeState = ElementTheme::DEFAULT;
        currentNodes.push_back(node);

        if (i > 0) {
            EdgeState edge;
            edge.startPos = GetNodePos((i - 1) / 2); edge.endPos = GetNodePos(i);
            edge.opacity = 1.0f; edge.thickness = 4.0f;
            currentEdges.push_back(edge);
        }
    }
}

// ================= CÁC THUẬT TOÁN MIN-HEAP =================

void Heap::GenerateInit(const std::vector<int>& values) {
    AnimationNodes.clear(); AnimationEdges.clear(); stepCodeLines.clear();
    currentNodes.clear(); currentEdges.clear(); internalHeap.clear();
    curIndex = 0; TotalTime = 0.0f; isPause = false;
    
    SetCode({
        "void Init() {",
        "  heap.clear();",
        "  for (int v : values)",
        "    Insert(v);",
        "}"
    });

    PushNewStep(currentNodes, currentEdges, 0); 
    PushNewStep(currentNodes, currentEdges, 1); 

    for (int val : values) {
        internalHeap.push_back(val);
        SyncTreeState();
        int i = internalHeap.size() - 1;
        currentNodes[i].themeState = ElementTheme::HIGHLIGHT;
        PushNewStep(currentNodes, currentEdges, 3); 
        
        HeapifyUp(i);
        PushNewStep(currentNodes, currentEdges, 2); 
    }
    
    for(auto& n : currentNodes) n.themeState = ElementTheme::DEFAULT;
    PushNewStep(currentNodes, currentEdges, 4);
    typeQuery = -1; 
}

void Heap::GenerateAdd(int val) {
    AnimationNodes.clear(); AnimationEdges.clear(); stepCodeLines.clear();
    curIndex = 0; TotalTime = 0.0f; isPause = false;
    
    SetCode({
        "void Insert(v) {",
        "  heap.push_back(v);",
        "  int i = heap.size() - 1;",
        "  while (i > 0 && heap[Parent(i)] > heap[i]) {",
        "    swap(heap[i], heap[Parent(i)]);",
        "    i = Parent(i);",
        "  }",
        "}"
    });

    internalHeap.push_back(val);
    SyncTreeState();
    int i = internalHeap.size() - 1;
    currentNodes[i].themeState = ElementTheme::SUCCESS;
    PushNewStep(currentNodes, currentEdges, 1); 
    PushNewStep(currentNodes, currentEdges, 2); 

    HeapifyUp(i);
    typeQuery = -1;
}

void Heap::HeapifyUp(int i) {
    while (i > 0) {
        int p = (i - 1) / 2;
        currentNodes[i].themeState = ElementTheme::HIGHLIGHT;
        currentNodes[p].themeState = ElementTheme::HIGHLIGHT;
        PushNewStep(currentNodes, currentEdges, 3); // while...

        if (internalHeap[p] > internalHeap[i]) {
            std::swap(internalHeap[p], internalHeap[i]);
            SyncTreeState(); // Cập nhật lại số trên node
            
            // Đổi màu để nhận diện đã swap
            currentNodes[i].themeState = ElementTheme::SUCCESS;
            currentNodes[p].themeState = ElementTheme::SUCCESS;
            PushNewStep(currentNodes, currentEdges, 4); // swap(...)
            
            currentNodes[i].themeState = ElementTheme::DEFAULT;
            currentNodes[p].themeState = ElementTheme::HIGHLIGHT;
            i = p;
            PushNewStep(currentNodes, currentEdges, 5); // i = Parent(i)
        } else {
            currentNodes[i].themeState = ElementTheme::DEFAULT;
            currentNodes[p].themeState = ElementTheme::DEFAULT;
            break;
        }
    }
    for(auto& n : currentNodes) n.themeState = ElementTheme::DEFAULT;
    PushNewStep(currentNodes, currentEdges, 7);
}

void Heap::GenerateDelete(int val) {
    AnimationNodes.clear(); AnimationEdges.clear(); stepCodeLines.clear();
    curIndex = 0; TotalTime = 0.0f; isPause = false;
    SetCode({
        "void Delete(v) {",
        "  int i = Search(v);",
        "  if (i == -1) return;",
        "  swap(heap[i], heap[last]);",
        "  heap.pop_back();",
        "  HeapifyDown(i); // OR HeapifyUp(i)",
        "}"
    });
    PushNewStep(currentNodes, currentEdges, 0);
    
    int targetIdx = -1;
    for (int i = 0; i < internalHeap.size(); i++) {
        currentNodes[i].themeState = ElementTheme::HIGHLIGHT;
        PushNewStep(currentNodes, currentEdges, 1);
        if (internalHeap[i] == val) {
            targetIdx = i;
            currentNodes[i].themeState = ElementTheme::SUCCESS;
            PushNewStep(currentNodes, currentEdges, 1);
            break;
        }
        currentNodes[i].themeState = ElementTheme::DEFAULT;
    }

    if (targetIdx == -1) {
        PushNewStep(currentNodes, currentEdges, 2);
        typeQuery = -1; return;
    }

    int lastIdx = internalHeap.size() - 1;
    if (targetIdx != lastIdx) {
        currentNodes[targetIdx].themeState = ElementTheme::HIGHLIGHT;
        currentNodes[lastIdx].themeState = ElementTheme::HIGHLIGHT;
        PushNewStep(currentNodes, currentEdges, 3);
        
        std::swap(internalHeap[targetIdx], internalHeap[lastIdx]);
        SyncTreeState();
        currentNodes[targetIdx].themeState = ElementTheme::SUCCESS;
        currentNodes[lastIdx].themeState = ElementTheme::SUCCESS;
        PushNewStep(currentNodes, currentEdges, 3);
    }

    internalHeap.pop_back();
    SyncTreeState();
    if (targetIdx < internalHeap.size()) currentNodes[targetIdx].themeState = ElementTheme::HIGHLIGHT;
    PushNewStep(currentNodes, currentEdges, 4);

    if (targetIdx < internalHeap.size()) {
        HeapifyDown(targetIdx);
        // Đôi khi cần HeapifyUp nếu số được thế vào lại nhỏ hơn parent
        if (targetIdx > 0 && internalHeap[(targetIdx - 1) / 2] > internalHeap[targetIdx]) {
            HeapifyUp(targetIdx);
        }
    }
    typeQuery = -1;
}

void Heap::HeapifyDown(int i) {
    int n = internalHeap.size();
    while (true) {
        int smallest = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        if (left < n && internalHeap[left] < internalHeap[smallest]) smallest = left;
        if (right < n && internalHeap[right] < internalHeap[smallest]) smallest = right;

        if (smallest != i) {
            currentNodes[i].themeState = ElementTheme::HIGHLIGHT;
            currentNodes[smallest].themeState = ElementTheme::HIGHLIGHT;
            PushNewStep(currentNodes, currentEdges, 5); // Đang so sánh

            std::swap(internalHeap[i], internalHeap[smallest]);
            SyncTreeState();
            
            currentNodes[i].themeState = ElementTheme::SUCCESS;
            currentNodes[smallest].themeState = ElementTheme::SUCCESS;
            PushNewStep(currentNodes, currentEdges, 5); // Đã Swap
            
            currentNodes[i].themeState = ElementTheme::DEFAULT;
            i = smallest;
        } else {
            break;
        }
    }
    for(auto& n : currentNodes) n.themeState = ElementTheme::DEFAULT;
    PushNewStep(currentNodes, currentEdges, 6);
}

void Heap::GenerateUpdate(int val1, int val2) {
    AnimationNodes.clear(); AnimationEdges.clear(); stepCodeLines.clear();
    curIndex = 0; TotalTime = 0.0f; isPause = false;
    SetCode({
        "void Update(oldV, newV) {",
        "  int i = Search(oldV);",
        "  if (i == -1) return;",
        "  heap[i] = newV;",
        "  if (newV < oldV) HeapifyUp(i);",
        "  else HeapifyDown(i);",
        "}"
    });
    PushNewStep(currentNodes, currentEdges, 0);

    int targetIdx = -1;
    for (int i = 0; i < internalHeap.size(); i++) {
        currentNodes[i].themeState = ElementTheme::HIGHLIGHT;
        PushNewStep(currentNodes, currentEdges, 1);
        if (internalHeap[i] == val1) {
            targetIdx = i;
            currentNodes[i].themeState = ElementTheme::SUCCESS;
            PushNewStep(currentNodes, currentEdges, 1);
            break;
        }
        currentNodes[i].themeState = ElementTheme::DEFAULT;
    }

    if (targetIdx == -1) { PushNewStep(currentNodes, currentEdges, 2); typeQuery = -1; return; }

    internalHeap[targetIdx] = val2;
    SyncTreeState();
    currentNodes[targetIdx].themeState = ElementTheme::SUCCESS;
    PushNewStep(currentNodes, currentEdges, 3);

    if (val2 < val1) {
        PushNewStep(currentNodes, currentEdges, 4);
        HeapifyUp(targetIdx);
    } else {
        PushNewStep(currentNodes, currentEdges, 5);
        HeapifyDown(targetIdx);
    }
    typeQuery = -1;
}

void Heap::GenerateSearch(int val) {
    AnimationNodes.clear(); AnimationEdges.clear(); stepCodeLines.clear();
    curIndex = 0; TotalTime = 0.0f; isPause = false;
    SetCode({
        "int Search(v) {",
        "  for (int i = 0; i < heap.size(); i++) {",
        "    if (heap[i] == v) return i;",
        "  }",
        "  return -1;",
        "}"
    });
    PushNewStep(currentNodes, currentEdges, 0);
    
    bool found = false;
    for (int i=0; i<internalHeap.size(); i++) {
        currentNodes[i].themeState = ElementTheme::HIGHLIGHT;
        PushNewStep(currentNodes, currentEdges, 1); 
        
        if (internalHeap[i] == val) {
            currentNodes[i].themeState = ElementTheme::SUCCESS;
            PushNewStep(currentNodes, currentEdges, 2); 
            found = true;
            break;
        }
        currentNodes[i].themeState = ElementTheme::DEFAULT;
    }
    if (!found) PushNewStep(currentNodes, currentEdges, 4); 
    
    for(auto& n : currentNodes) n.themeState = ElementTheme::DEFAULT;
    PushNewStep(currentNodes, currentEdges, -1);
    
    typeQuery = -1;
}

// ================= VÒNG LẶP CHÍNH =================

void Heap::Solve(const Palette &colors)
{
    if (!myList.empty() && typeQuery != -1)
    {
        if (typeQuery == 0) GenerateInit(myList);
        else if (typeQuery == 1) GenerateAdd(myList[0]);
        else if (typeQuery == 2) GenerateDelete(myList[0]);
        else if (typeQuery == 3) {
            if (myList.size() >= 2) GenerateUpdate(myList[0], myList[1]);
            else typeQuery = -1; 
        }
        else if (typeQuery == 4) GenerateSearch(myList[0]);

        myList.clear(); 
    }

    Draw(colors);
}

void Heap::Draw(const Palette &colors)
{  
    ClearBackground(colors.background);
    
    //Draw Controls
    DrawLineEx({319.0f, StartY}, {319.0f, EndY}, 1.0f, colors.border);
    DrawTexture(setting, StartX + 10, StartY + 18, colors.primaryAcc);
    
    DrawTextEx(font, "Controls", {StartX + 20 + (float)setting.width, StartY + 25}, 24, 1.0f, colors.textPrimary);
    DrawLineEx({StartX, 142}, {319.0f, 142}, 1.0f, colors.border);

    DrawTextEx(miniTitleFont, "DATA OPERATIONS",{24, 142 + 24}, 20, 1.0f, colors.textPrimary);

    InitButton.Draw(); AddButton.Draw(); DeleteButton.Draw(); UpdateButton.Draw(); SearchButton.Draw();

    if (InitButton.isPressed()) { isInsertDialogOpen = true; insertInputText[0] = '\0'; insertInputCount = 0; typeQuery = 0; }
    if (AddButton.isPressed()) { isInsertDialogOpen = true; insertInputText[0] = '\0'; insertInputCount = 0; typeQuery = 1; }   
    if (DeleteButton.isPressed()) { isInsertDialogOpen = true; insertInputText[0] = '\0'; insertInputCount = 0; typeQuery = 2; }
    if (UpdateButton.isPressed()) { isInsertDialogOpen = true; insertInputText[0] = '\0'; insertInputCount = 0; typeQuery = 3; }
    if (SearchButton.isPressed()) { isInsertDialogOpen = true; insertInputText[0] = '\0'; insertInputCount = 0; typeQuery = 4; }

    // --- PLAYBACK ---
    DrawTextEx(miniTitleFont, "PLAYBACK",{24, 580}, 20, 1.0f, colors.textPrimary);
    if (isPause) DrawTexture(play, 135, 616, colors.primaryAcc);
    else DrawTexture(pause, 135, 616, colors.primaryAcc);

    Rectangle playPauseRect = {135.0f, 616.0f, (float)play.width, (float)play.height};
    if (CheckCollisionPointRec(GetMousePosition(), playPauseRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) isPause ^= 1;

    DrawTextEx(miniTitleFont, "Speed", {24, 690}, 20, 0.8f, colors.textPrimary);
    mySpeedSlider.Draw(colors);
    DrawTextEx(miniTitleFont, FloatToString(mySpeedSlider.getSpeed()).c_str(), {280, 690}, 20, 0.8f, colors.textPrimary);

    DrawLineEx({319, 128}, {1920, 128}, 1.0f, colors.border);
    DrawTextEx(titleFont, "MIN-HEAP TREE", {319 + 32, 85}, 32, 0.8f, colors.textPrimary);

    // --- VISUALIZATION ---
    if (!AnimationEdges.empty() && !AnimationNodes.empty()) 
    {
        if (curIndex >= AnimationEdges.size()) {
            for (size_t i = 0; i < currentEdges.size(); i++) {
                EdgeTransform finalState = {currentEdges[i], currentEdges[i]};
                visualizer.RenderDirectedEdge(finalState, 1.0f, colors);
            }
        } else {
            for (auto& step : AnimationEdges[curIndex]) visualizer.RenderDirectedEdge(step, TotalTime, colors);
        }

        if (curIndex >= AnimationNodes.size()) {
            for (size_t i = 0; i < currentNodes.size(); i++) {
                VertexTransform finalState = {currentNodes[i], currentNodes[i]};
                visualizer.RenderVertex(finalState, 1.0f, vertexFont, colors);
            }
        } else {
            for (auto& step : AnimationNodes[curIndex]) visualizer.RenderVertex(step, TotalTime, vertexFont, colors);
        }
    }

    // --- CẬP NHẬT THỜI GIAN VÀ TIMELINE ---
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
    
    // 1. Chỉ gọi hàm Draw để vẽ
    myTimeline.Draw({24, 780}, {295, 780}, maxStep, curIndex, colors);
    
    // 2. Dùng getStep() để cập nhật giá trị
    curIndex = myTimeline.getStep();

    if (curIndex != prevIndex) { TotalTime = 1.0f; isPause = true; }
// --- VẼ EXECUTE LOG TỰ ĐỘNG CUỘN ---
    DrawLineEx({319, 830}, {1920, 830}, 1.0f, colors.border);
    DrawTextEx(titleFont, "EXECUTE LOG", {319 + 32, 840}, 32, 0.4f, colors.textPrimary);
    
    float codeStartY = 880.0f;
    int activeLine = (curIndex < stepCodeLines.size()) ? stepCodeLines[curIndex] : -1;
    
    int maxVisibleLines = 7; 
    int totalLines = currentCodeLines.size();
    
    int startLine = 0;
    if (activeLine >= 0) {
        startLine = activeLine - (maxVisibleLines / 2); 
    }
    
    if (startLine < 0) startLine = 0;
    if (totalLines > maxVisibleLines && startLine > totalLines - maxVisibleLines) {
        startLine = totalLines - maxVisibleLines;
    }

    int displayIndex = 0; 
    
    for (int i = startLine; i < totalLines && displayIndex < maxVisibleLines; i++) {
        Color c = (i == activeLine) ? colors.textPrimary : colors.textSecondary;
        
        float currentY = codeStartY + displayIndex * 25;
        
        if (i == activeLine) {
            DrawRectangle(319 + 20, currentY - 2, 800, 25, Fade(colors.primaryAcc, 0.3f));
        }
        DrawTextEx(font, currentCodeLines[i].c_str(), {319 + 32, currentY}, 20, 1.0f, c);
        
        displayIndex++;
    }

    // --- DIALOG ---
    if (isInsertDialogOpen)
    {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.6f));
        float dialogWidth = 1000.0f; float dialogHeight = 200.0f;
        float dialogX = (GetScreenWidth() - dialogWidth) / 2.0f; float dialogY = (GetScreenHeight() - dialogHeight) / 2.0f;
        Rectangle dialogRec = { dialogX, dialogY, dialogWidth, dialogHeight };

        DrawRectangleRec(dialogRec, colors.surface);
        DrawRectangleLinesEx(dialogRec, 2.0f, colors.primaryAcc);
        DrawTextEx(titleFont, "Input Data", {dialogX + 20, dialogY + 20}, 30, 1.0f, colors.textPrimary);

        Rectangle randomBtnRec = { dialogX + dialogWidth - 50, dialogY + 20, 30.0f, 30.0f };
        bool isRandomHovered = CheckCollisionPointRec(GetMousePosition(), randomBtnRec);
        
        if (isRandomHovered) {
            DrawRectangleRec(randomBtnRec, Fade(colors.primaryAcc, 0.2f));
            DrawTextEx(miniTitleFont, "Random", {randomBtnRec.x - 65, randomBtnRec.y + 5}, 16, 1.0f, colors.textSecondary);
        }
        DrawTexture(randomIcon, randomBtnRec.x, randomBtnRec.y, WHITE);

        if (isRandomHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int cnt = (typeQuery == 0) ? rand()%5 + 5 : ((typeQuery == 3) ? 2 : 1);
            std::string input;
            while (cnt--) input += std::to_string(rand() % 99 + 1) + (cnt > 0 ? " " : "");
            strcpy(insertInputText, input.c_str()); insertInputCount = input.length();
        }

        Rectangle textBoxRec = { dialogX + 20, dialogY + 80, dialogWidth - 40, 50 };
        DrawRectangleRec(textBoxRec, colors.background);
        DrawRectangleLinesEx(textBoxRec, 1.0f, colors.border);
        DrawTextEx(font, insertInputText, {textBoxRec.x + 10, textBoxRec.y + 13}, 24, 1.0f, colors.textPrimary);

        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (insertInputCount < 255)) {
                insertInputText[insertInputCount++] = (char)key; insertInputText[insertInputCount] = '\0';
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) && insertInputCount > 0) insertInputText[--insertInputCount] = '\0';

        framesCounter++;
        if ((framesCounter / 30) % 2 == 0) {
            Vector2 textSize = MeasureTextEx(font, insertInputText, 24, 1.0f);
            DrawLineEx({ textBoxRec.x + 12 + textSize.x, textBoxRec.y + 10 }, { textBoxRec.x + 12 + textSize.x, textBoxRec.y + 40 }, 2.0f, colors.primaryAcc);
        }

        const char* hint = "Type, Random, or Drag & Drop a .txt file here";
        if (typeQuery == 3) hint = "Type target value and new value (e.g. 15 99) separated by space";
        DrawTextEx(miniTitleFont, hint, { dialogX + 20, dialogY + 140 }, 16, 1.0f, colors.textSecondary);
        DrawTextEx(miniTitleFont, "Press ENTER to confirm | ESC to cancel", { dialogX + 20, dialogY + 165 }, 18, 1.0f, colors.textSecondary);

        if (IsKeyPressed(KEY_ENTER)) {
            isInsertDialogOpen = false; 
            int cur_num = 0; bool hasNum = false; myList.clear();
            for (int i = 0; i < insertInputCount; i++) {
                if (insertInputText[i] >= '0' && insertInputText[i] <= '9') {
                    cur_num = cur_num * 10 + (insertInputText[i] - '0'); hasNum = true;
                } else if (insertInputText[i] == ' ' && hasNum) {
                    myList.push_back(cur_num); cur_num = 0; hasNum = false;
                }
            }
            if (hasNum) myList.push_back(cur_num);
            if (myList.empty() || (typeQuery == 3 && myList.size() < 2)) typeQuery = -1;
        }
        else if (IsKeyPressed(KEY_ESCAPE)) { isInsertDialogOpen = false; typeQuery = -1; }
    }
}

std::string Heap::FloatToString(float value) {
    char buffer[10]; snprintf(buffer, sizeof(buffer), "%.1fx", value); return std::string(buffer);
}