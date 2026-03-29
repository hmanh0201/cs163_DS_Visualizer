#include "SLL.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <cstring>

SLL::SLL(int width, int height) 
{
    StartX = 0;
    StartY = height * 65 / 1080;
    EndX = width;
    EndY = height;
    curIndex = 0;
    TotalTime = 0.0f;
    isPause = false;
    typeQuery = -1;

    srand(time(NULL));

    Image image = LoadImage("assets/setting.png");
    double scale = height * 0.035f;
    int w = image.width * scale / image.height;
    int h = scale;
    ImageResize(&image, w, h);
    setting = LoadTextureFromImage(image);
    UnloadImage(image);

    image = LoadImage("assets/pause.png");
    scale = 48;
    w = image.width * scale / image.height;
    h = scale;
    ImageResize(&image, w, h);
    pause = LoadTextureFromImage(image);
    UnloadImage(image);

    image = LoadImage("assets/play.png");
    scale = 48;
    w = image.width * scale / image.height;
    h = scale;
    ImageResize(&image, w, h);
    play = LoadTextureFromImage(image);
    UnloadImage(image);

    image = LoadImage("assets/random.png"); 
    ImageResize(&image, 30, 30); 
    randomIcon = LoadTextureFromImage(image);
    UnloadImage(image);

    font = LoadFontEx("assets/fonts/Roboto/Roboto-Bold.ttf", 24, 0, 0);
    titleFont = LoadFontEx("assets/fonts/Roboto/Roboto-Bold.ttf", 36, 0, 0);
    miniTitleFont = LoadFontEx("assets/fonts/Roboto/Roboto-Medium.ttf", 20, 0, 0);
    vertexFont = LoadFontEx("assets/fonts/Roboto/Roboto-Medium.ttf", 30, 0, 0);
    SetTextureFilter(titleFont.texture, TEXTURE_FILTER_BILINEAR);   
    SetTextureFilter(miniTitleFont.texture, TEXTURE_FILTER_BILINEAR);   
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
}

SLL::~SLL()
{
    UnloadTexture(setting);
    UnloadTexture(pause);
    UnloadTexture(play);
    UnloadTexture(randomIcon);

    UnloadFont(font);
    UnloadFont(titleFont);
    UnloadFont(miniTitleFont);
    UnloadFont(vertexFont);
}

void SLL::SetCode(const std::vector<std::string>& code) {
    currentCodeLines = code;
}

void SLL::PushNewStep(const std::vector<VertexState>& nextNodes, const std::vector<EdgeState>& nextEdges, int codeLine) {
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
            uState.opacity = 0.0f;
            uState.endPos = uState.startPos; 
        }
        stepEdges.push_back({uState, nextEdges[i]});
    }

    currentNodes = nextNodes;
    currentEdges = nextEdges;
    AnimationNodes.push_back(stepNodes);
    AnimationEdges.push_back(stepEdges);
    stepCodeLines.push_back(codeLine);
}

// ================= CÁC THUẬT TOÁN =================

void SLL::GenerateInit(const std::vector<int>& values) {
    AnimationNodes.clear(); AnimationEdges.clear(); stepCodeLines.clear();
    currentNodes.clear(); currentEdges.clear();
    curIndex = 0; TotalTime = 0.0f; isPause = false;
    internalList = values;
    
    SetCode({
        "void Init() {",
        "  head = NULL;",
        "  for (int v : values)",
        "    AddTail(v);",
        "}"
    });

    std::vector<VertexState> targetNodes;
    std::vector<EdgeState> targetEdges;
    PushNewStep(targetNodes, targetEdges, 0); 
    
    for (size_t i = 0; i < values.size(); i++) {
        if (i > 0) {
            EdgeState edge;
            edge.startPos = { START_X +(i - 1) * SPACING, START_Y };
            edge.endPos = { START_X + i * SPACING, START_Y };
            edge.opacity = 1.0f;
            targetEdges.push_back(edge);
            PushNewStep(targetNodes, targetEdges, 3);
        }
        VertexState node;
        node.pos = {START_X + i * SPACING, START_Y};
        node.text = std::to_string(values[i]);
        node.opacity = 1.0f;
        targetNodes.push_back(node);
        PushNewStep(targetNodes, targetEdges, 3);
        currentNodes = targetNodes; 
        currentEdges = targetEdges;
    }
    PushNewStep(targetNodes, targetEdges, 4);
    typeQuery = -1; 
}

void SLL::GenerateAdd(int val) {
    AnimationNodes.clear(); AnimationEdges.clear(); stepCodeLines.clear();
    curIndex = 0; TotalTime = 0.0f; isPause = false;
    
    SetCode({
        "Node* newNode = new Node(v);",
        "newNode->next = head;",
        "head = newNode;"
    });

    int n = currentNodes.size();
    std::vector<VertexState> nextNodes = currentNodes;
    std::vector<EdgeState> nextEdges = currentEdges;

    VertexState newNode;
    newNode.pos = {START_X, START_Y + SPACING}; 
    newNode.text = std::to_string(val);
    newNode.opacity = 1.0f;
    newNode.radius = 30.0f;
    newNode.themeState = ElementTheme::HIGHLIGHT;
    nextNodes.push_back(newNode); 
    
    PushNewStep(nextNodes, nextEdges, 0); 

    if (n > 0) {
        EdgeState newEdge;
        newEdge.startPos = {START_X, START_Y + SPACING};
        newEdge.endPos = {START_X, START_Y}; 
        newEdge.opacity = 1.0f;
        newEdge.thickness = 4.0f;
        nextEdges.push_back(newEdge); 
    }
    PushNewStep(nextNodes, nextEdges, 1); 


    nextNodes[n].pos = {START_X, START_Y}; 
    nextNodes[n].themeState = ElementTheme::DEFAULT;
    
    for (int i = 0; i < n; i++) {
        nextNodes[i].pos = {START_X + (i + 1) * SPACING, START_Y};
    }
    
    for (int i = 0; i < currentEdges.size(); i++) {
        nextEdges[i].startPos = {START_X + (i + 1) * SPACING, START_Y};
        nextEdges[i].endPos   = {START_X + (i + 2) * SPACING, START_Y};
    }

    if (n > 0) {
        nextEdges[n - 1].startPos = {START_X, START_Y};
        nextEdges[n - 1].endPos = {START_X + SPACING, START_Y};
    }
    
    PushNewStep(nextNodes, nextEdges, 2); 


    std::vector<VertexState> reorderedNodes;
    reorderedNodes.push_back(nextNodes[n]); 
    for (int i = 0; i < n; i++) reorderedNodes.push_back(nextNodes[i]);
    
    std::vector<EdgeState> reorderedEdges;
    if (n > 0) {
        reorderedEdges.push_back(nextEdges[n - 1]); 
        for (int i = 0; i < n - 1; i++) reorderedEdges.push_back(nextEdges[i]);
    }
    
    currentNodes = reorderedNodes;
    currentEdges = reorderedEdges;

    internalList.insert(internalList.begin(), val);
    typeQuery = -1;
}

void SLL::GenerateDelete(int val) {
    AnimationNodes.clear(); AnimationEdges.clear(); stepCodeLines.clear();
    curIndex = 0; TotalTime = 0.0f; isPause = false;
    SetCode({
        "if (head == NULL) return;",
        "if (head->value == v) { head = head->next; return; }",
        "Node* temp = head;",
        "while (temp->next != NULL && temp->next->value != v)",
        "  temp = temp->next;",
        "if (temp->next != NULL) temp->next = temp->next->next;"
    });
    PushNewStep(currentNodes, currentEdges, 0);
    
    if (internalList.empty()) { typeQuery = -1; return; }

    int targetIdx = -1;
    for (int i=0; i<internalList.size(); i++) {
        currentNodes[i].themeState = ElementTheme::HIGHLIGHT;
        PushNewStep(currentNodes, currentEdges, (i==0) ? 1 : 3);
        
        if (internalList[i] == val) {
            targetIdx = i;
            currentNodes[i].themeState = ElementTheme::SUCCESS;
            PushNewStep(currentNodes, currentEdges, (i==0) ? 1 : 5);
            break;
        }
        currentNodes[i].themeState = ElementTheme::DEFAULT;
        if (i>0) PushNewStep(currentNodes, currentEdges, 4);
    }

    if (targetIdx != -1) {
        internalList.erase(internalList.begin() + targetIdx);
        
        currentNodes[targetIdx].opacity = 0.0f;
        if (targetIdx > 0) currentEdges[targetIdx-1].opacity = 0.0f;
        if (targetIdx < currentEdges.size()) currentEdges[targetIdx].opacity = 0.0f;
        PushNewStep(currentNodes, currentEdges, (targetIdx==0) ? 1 : 5);

        currentNodes.erase(currentNodes.begin() + targetIdx);
        currentEdges.clear();
        for (int i=0; i<currentNodes.size(); i++) {
            currentNodes[i].pos = {START_X + i * SPACING, START_Y};
            currentNodes[i].themeState = ElementTheme::DEFAULT;
            if (i < currentNodes.size() - 1) {
                EdgeState e;
                e.startPos = {START_X + i*SPACING, START_Y};
                e.endPos = {START_X + (i+1)*SPACING, START_Y};
                e.opacity = 1.0f;
                currentEdges.push_back(e);
            }
        }
        PushNewStep(currentNodes, currentEdges, -1);
    }
    typeQuery = -1;
}

void SLL::GenerateUpdate(int val1, int val2) {
    AnimationNodes.clear(); AnimationEdges.clear(); stepCodeLines.clear();
    curIndex = 0; TotalTime = 0.0f; isPause = false;
    SetCode({
        "Node* temp = head;",
        "while (temp != NULL && temp->value != v1)",
        "  temp = temp->next;",
        "if (temp != NULL) temp->value = v2;"
    });
    PushNewStep(currentNodes, currentEdges, 0);

    for (int i=0; i<internalList.size(); i++) {
        currentNodes[i].themeState = ElementTheme::HIGHLIGHT;
        PushNewStep(currentNodes, currentEdges, 1); 
        
        if (internalList[i] == val1) {
            currentNodes[i].themeState = ElementTheme::SUCCESS;
            PushNewStep(currentNodes, currentEdges, 3);
            
            internalList[i] = val2;
            currentNodes[i].text = std::to_string(val2);
            PushNewStep(currentNodes, currentEdges, 3);
            break;
        }
        
        PushNewStep(currentNodes, currentEdges, 2); 
        currentNodes[i].themeState = ElementTheme::DEFAULT;
    }
    
    for(auto& n : currentNodes) n.themeState = ElementTheme::DEFAULT;
    PushNewStep(currentNodes, currentEdges, -1);
    
    typeQuery = -1;
}

void SLL::GenerateSearch(int val) {
    AnimationNodes.clear(); AnimationEdges.clear(); stepCodeLines.clear();
    curIndex = 0; TotalTime = 0.0f; isPause = false;
    SetCode({
        "Node* temp = head;",
        "while (temp != NULL && temp->value != v)",
        "  temp = temp->next;",
        "if (temp != NULL) return temp;",
        "return NULL;"
    });
    PushNewStep(currentNodes, currentEdges, 0);
    
    bool found = false;
    for (int i=0; i<internalList.size(); i++) {
        currentNodes[i].themeState = ElementTheme::HIGHLIGHT;
        PushNewStep(currentNodes, currentEdges, 1); 
        
        if (internalList[i] == val) {
            currentNodes[i].themeState = ElementTheme::SUCCESS;
            PushNewStep(currentNodes, currentEdges, 3); 
            found = true;
            break;
        }
        
        PushNewStep(currentNodes, currentEdges, 2); 
        currentNodes[i].themeState = ElementTheme::DEFAULT;
    }
    if (!found) {
        PushNewStep(currentNodes, currentEdges, 4); 
    }
    for(auto& n : currentNodes) n.themeState = ElementTheme::DEFAULT;
    PushNewStep(currentNodes, currentEdges, -1);
    
    typeQuery = -1;
}

// ================= VÒNG LẶP CHÍNH =================

void SLL::Solve(const Palette &colors)
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

void SLL::Draw(const Palette &colors)
{  
    ClearBackground(colors.background);
    
    //Draw Controls
    DrawLineEx({319.0f, StartY}, {319.0f, EndY}, 1.0f, colors.border);
    DrawTexture(setting, StartX + 10, StartY + 18, colors.primaryAcc);
    
    DrawTextEx(font, "Controls", {StartX + 20 + (float)setting.width, StartY + 25}, 24, 1.0f, colors.textPrimary);
    DrawLineEx({StartX, 142}, {319.0f, 142}, 1.0f, colors.border);

    DrawTextEx(miniTitleFont, "DATA OPERATIONS",{24, 142 + 24}, 20, 1.0f, colors.textPrimary);

    InitButton.Draw();
    AddButton.Draw();
    DeleteButton.Draw();
    UpdateButton.Draw();
    SearchButton.Draw();

    if (InitButton.isPressed()) {
        isInsertDialogOpen = true; insertInputText[0] = '\0'; insertInputCount = 0; typeQuery = 0;
    }
    if (AddButton.isPressed()) {
        isInsertDialogOpen = true; insertInputText[0] = '\0'; insertInputCount = 0; typeQuery = 1;
    }   
    if (DeleteButton.isPressed()) {
        isInsertDialogOpen = true; insertInputText[0] = '\0'; insertInputCount = 0; typeQuery = 2;
    }
    if (UpdateButton.isPressed()) {
        isInsertDialogOpen = true; insertInputText[0] = '\0'; insertInputCount = 0; typeQuery = 3;
    }
    if (SearchButton.isPressed()) {
        isInsertDialogOpen = true; insertInputText[0] = '\0'; insertInputCount = 0; typeQuery = 4;
    }

    // --- PLAYBACK ---
    DrawTextEx(miniTitleFont, "PLAYBACK",{24, 580}, 20, 1.0f, colors.textPrimary);
    if (isPause) DrawTexture(play, 135, 616, colors.primaryAcc);
    else DrawTexture(pause, 135, 616, colors.primaryAcc);

    Rectangle playPauseRect = {135.0f, 616.0f, (float)play.width, (float)play.height};
    if (CheckCollisionPointRec(GetMousePosition(), playPauseRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        isPause ^= 1;
    }

    DrawTextEx(miniTitleFont, "Speed", {24, 690}, 20, 0.8f, colors.textPrimary);
    mySpeedSlider.Draw(colors);
    DrawTextEx(miniTitleFont, FloatToString(mySpeedSlider.getSpeed()).c_str(), {280, 690}, 20, 0.8f, colors.textPrimary);

    DrawLineEx({319, 128}, {1920, 128}, 1.0f, colors.border);
    DrawTextEx(titleFont, "SINGLY LINKED LIST", {319 + 32, 85}, 32, 0.8f, colors.textPrimary);

    // --- VISUALIZATION ---
    if (!AnimationEdges.empty() && !AnimationNodes.empty()) 
    {
        if (curIndex >= AnimationEdges.size()) {
            for (size_t i = 0; i < currentEdges.size(); i++) {
                EdgeTransform finalState = {currentEdges[i], currentEdges[i]};
                visualizer.RenderDirectedEdge(finalState, 1.0f, colors);
            }
        } else {
            for (auto& step : AnimationEdges[curIndex]) {
                visualizer.RenderDirectedEdge(step, TotalTime, colors);
            }
        }

        if (curIndex >= AnimationNodes.size()) {
            for (size_t i = 0; i < currentNodes.size(); i++) {
                VertexTransform finalState = {currentNodes[i], currentNodes[i]};
                visualizer.RenderVertex(finalState, 1.0f, vertexFont, colors);
            }
        } else {
            for (auto& step : AnimationNodes[curIndex]) {
                visualizer.RenderVertex(step, TotalTime, vertexFont, colors);
            }
        }
    }

    // --- CẬP NHẬT THỜI GIAN VÀ TIMELINE ---
    int maxStep = (AnimationNodes.size() > 0) ? AnimationNodes.size() - 1 : 0;
    if (!isPause) {
        TotalTime += GetFrameTime() * mySpeedSlider.getSpeed(); 
        if (TotalTime >= 1.0f) {
            if (curIndex < maxStep) {
                curIndex++;
                TotalTime -= 1.0f;
            } else {
                TotalTime = 1.0f; 
                isPause = true;   
            }
        }
    }

    DrawTextEx(miniTitleFont, "Timeline", {24, 750}, 20, 0.8f, colors.textPrimary);
    int prevIndex = curIndex;
    myTimeline.Draw({24, 780}, {295, 780}, maxStep, curIndex, colors);
    curIndex = myTimeline.getStep();
    if (curIndex != prevIndex) {
        TotalTime = 1.0f;
        isPause = true;
    }

    // --- VẼ EXECUTE LOG ---
    DrawLineEx({319, 830}, {1920, 830}, 1.0f, colors.border);
    DrawTextEx(titleFont, "EXECUTE LOG", {319 + 32, 840}, 32, 0.4f, colors.textPrimary);
    
    float codeStartY = 880.0f;
    int activeLine = (curIndex < stepCodeLines.size()) ? stepCodeLines[curIndex] : -1;
    
    for (int i = 0; i < currentCodeLines.size(); i++) {
        Color c = (i == activeLine) ? colors.textPrimary : colors.textSecondary;
        if (i == activeLine) {
            DrawRectangle(319 + 20, codeStartY + i * 25 - 2, 800, 25, Fade(colors.primaryAcc, 0.3f));
        }
        DrawTextEx(font, currentCodeLines[i].c_str(), {319 + 32, codeStartY + i * 25}, 20, 1.0f, c);
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
            while (cnt--) {
                input += std::to_string(rand() % 99 + 1) + (cnt > 0 ? " " : "");
            }
            strcpy(insertInputText, input.c_str());
            insertInputCount = input.length();
        }

        Rectangle textBoxRec = { dialogX + 20, dialogY + 80, dialogWidth - 40, 50 };
        DrawRectangleRec(textBoxRec, colors.background);
        DrawRectangleLinesEx(textBoxRec, 1.0f, colors.border);
        DrawTextEx(font, insertInputText, {textBoxRec.x + 10, textBoxRec.y + 13}, 24, 1.0f, colors.textPrimary);

        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (insertInputCount < 255)) {
                insertInputText[insertInputCount++] = (char)key;
                insertInputText[insertInputCount] = '\0';
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) && insertInputCount > 0) {
            insertInputText[--insertInputCount] = '\0';
        }

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
            int cur_num = 0;
            bool hasNum = false;
            myList.clear();
            for (int i = 0; i < insertInputCount; i++) {
                if (insertInputText[i] >= '0' && insertInputText[i] <= '9') {
                    cur_num = cur_num * 10 + (insertInputText[i] - '0');
                    hasNum = true;
                } else if (insertInputText[i] == ' ' && hasNum) {
                    myList.push_back(cur_num);
                    cur_num = 0; hasNum = false;
                }
            }
            if (hasNum) myList.push_back(cur_num);
            
            // Xử lý bảo vệ input
            if (myList.empty() || (typeQuery == 3 && myList.size() < 2)) typeQuery = -1;
        }
        else if (IsKeyPressed(KEY_ESCAPE)) {
            isInsertDialogOpen = false; 
            typeQuery = -1;
        }
    }
}

std::string SLL::FloatToString(float value)
{
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%.1fx", value);
    return std::string(buffer);
}