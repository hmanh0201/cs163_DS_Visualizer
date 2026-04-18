#include "AVL.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <string>
#include <cstring>
#include <algorithm>

AVL::AVL(int width, int height) 
{
    StartX = 0; StartY = height * 65 / 1080; EndX = width; EndY = height;
    curIndex = 0; TotalTime = 0.0f; isPause = false; typeQuery = -1; rootIndex = -1;

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

AVL::~AVL() {
    UnloadTexture(setting); UnloadTexture(pause); UnloadTexture(play); UnloadTexture(randomIcon);
    UnloadFont(font); UnloadFont(titleFont); UnloadFont(miniTitleFont); UnloadFont(vertexFont);
}

void AVL::SetCode(const std::vector<std::string>& code) { currentCodeLines = code; }

void AVL::PushNewStep(const std::vector<VertexState>& nextNodes, const std::vector<EdgeState>& nextEdges, int codeLine) {
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

// ================= TÍNH TOÁN CẤU TRÚC AVL =================

int AVL::GetHeight(int node) { return (node == -1) ? 0 : tree[node].height; }
int AVL::GetBalance(int node) { return (node == -1) ? 0 : GetHeight(tree[node].left) - GetHeight(tree[node].right); }
void AVL::UpdateHeight(int node) { if (node != -1) tree[node].height = 1 + std::max(GetHeight(tree[node].left), GetHeight(tree[node].right)); }

void AVL::CalculatePositions(int node, int depth, float leftX, float rightX) {
    if (node == -1 || !tree[node].active) return;
    float midX = (leftX + rightX) / 2.0f;
    float midY = StartY + 120.0f + depth * 90.0f; 
    tree[node].targetPos = {midX, midY};
    CalculatePositions(tree[node].left, depth + 1, leftX, midX);
    CalculatePositions(tree[node].right, depth + 1, midX, rightX);
}

void AVL::GetTreeState(std::vector<VertexState>& nextNodes, std::vector<EdgeState>& nextEdges) {
    float workspaceLeft = 319.0f + 50.0f; 
    float workspaceRight = (float)GetScreenWidth() - 50.0f;
    CalculatePositions(rootIndex, 0, workspaceLeft, workspaceRight);

    nextNodes.assign(tree.size(), VertexState());
    nextEdges.assign(tree.size(), EdgeState());

    for (int i = 0; i < tree.size(); i++) {
        if (tree[i].active) {
            nextNodes[i].pos = tree[i].targetPos;
            nextNodes[i].text = std::to_string(tree[i].val);
            nextNodes[i].opacity = 1.0f;
            nextNodes[i].radius = 28.0f;
            if (i < currentNodes.size()) nextNodes[i].themeState = currentNodes[i].themeState;
            else nextNodes[i].themeState = ElementTheme::DEFAULT;
        } else {
            nextNodes[i].opacity = 0.0f;
        }
    }

    for (int i = 0; i < tree.size(); i++) {
        if (tree[i].active) {
            if (tree[i].left != -1 && tree[tree[i].left].active) {
                int child = tree[i].left;
                nextEdges[child].startPos = tree[i].targetPos;
                nextEdges[child].endPos = tree[child].targetPos;
                nextEdges[child].opacity = 1.0f;
                nextEdges[child].thickness = 3.0f;
            }
            if (tree[i].right != -1 && tree[tree[i].right].active) {
                int child = tree[i].right;
                nextEdges[child].startPos = tree[i].targetPos;
                nextEdges[child].endPos = tree[child].targetPos;
                nextEdges[child].opacity = 1.0f;
                nextEdges[child].thickness = 3.0f;
            }
        }
    }
    
    for (int i = 0; i < tree.size(); i++) {
        if (i < currentEdges.size()) nextEdges[i].themeState = currentEdges[i].themeState;
    }
}

// ================= THUẬT TOÁN AVL =================

int AVL::RightRotate(int y, int &animCode) {
    int x = tree[y].left; int T2 = tree[x].right;
    tree[x].right = y; tree[y].left = T2;
    UpdateHeight(y); UpdateHeight(x);
    
    std::vector<VertexState> nextNodes; std::vector<EdgeState> nextEdges;
    GetTreeState(nextNodes, nextEdges);
    PushNewStep(nextNodes, nextEdges, animCode); 
    return x;
}

int AVL::LeftRotate(int x, int &animCode) {
    int y = tree[x].right; int T2 = tree[y].left;
    tree[y].left = x; tree[x].right = T2;
    UpdateHeight(x); UpdateHeight(y);
    
    std::vector<VertexState> nextNodes; std::vector<EdgeState> nextEdges;
    GetTreeState(nextNodes, nextEdges);
    PushNewStep(nextNodes, nextEdges, animCode); 
    return y;
}

int AVL::InsertNode(int node, int key, int parentIndex) {
    if (node == -1) {
        tree.push_back({key, -1, -1, 1, true, {0,0}});
        int newIdx = tree.size() - 1;

        std::vector<VertexState> nextNodes; std::vector<EdgeState> nextEdges;
        GetTreeState(nextNodes, nextEdges);

        VertexState startNode = nextNodes[newIdx]; EdgeState startEdge = nextEdges[newIdx];
        if (parentIndex != -1) {
            startNode.pos = tree[parentIndex].targetPos;
            startEdge.startPos = tree[parentIndex].targetPos; startEdge.endPos = tree[parentIndex].targetPos;
        } else {
            startNode.pos = { (319.0f + 50.0f + GetScreenWidth() - 50.0f) / 2.0f, StartY }; 
        }
        startNode.opacity = 0.0f; startEdge.opacity = 0.0f;
        
        while(currentNodes.size() <= newIdx) currentNodes.push_back(startNode);
        while(currentEdges.size() <= newIdx) currentEdges.push_back(startEdge);
        currentNodes[newIdx] = startNode; currentEdges[newIdx] = startEdge;

        nextNodes[newIdx].themeState = ElementTheme::SUCCESS;
        PushNewStep(nextNodes, nextEdges, 1);
        return newIdx;
    }

    std::vector<VertexState> nextNodes; std::vector<EdgeState> nextEdges;
    GetTreeState(nextNodes, nextEdges);
    nextNodes[node].themeState = ElementTheme::HIGHLIGHT; // Vàng đang xét
    PushNewStep(nextNodes, nextEdges, 0);

    if (key < tree[node].val) {
        GetTreeState(nextNodes, nextEdges); nextNodes[node].themeState = ElementTheme::HIGHLIGHT; 
        PushNewStep(nextNodes, nextEdges, 3);
        tree[node].left = InsertNode(tree[node].left, key, node);
    } else if (key > tree[node].val) {
        GetTreeState(nextNodes, nextEdges); nextNodes[node].themeState = ElementTheme::HIGHLIGHT;
        PushNewStep(nextNodes, nextEdges, 5);
        tree[node].right = InsertNode(tree[node].right, key, node);
    } else {
        GetTreeState(nextNodes, nextEdges); PushNewStep(nextNodes, nextEdges, 6);
        return node;
    }

    GetTreeState(nextNodes, nextEdges);
    nextNodes[node].themeState = ElementTheme::HIGHLIGHT;
    UpdateHeight(node); PushNewStep(nextNodes, nextEdges, 7);

    int balance = GetBalance(node); PushNewStep(nextNodes, nextEdges, 8);

    int codeL = 9, codeR = 10, codeLR = 12, codeRL = 16;
    if (balance > 1 && key < tree[tree[node].left].val) {
        int res = RightRotate(node, codeL);
        // Vá lỗi dính màu sau khi xoay
        GetTreeState(nextNodes, nextEdges); 
        nextNodes[node].themeState = ElementTheme::DEFAULT; 
        nextNodes[res].themeState = ElementTheme::DEFAULT; 
        PushNewStep(nextNodes, nextEdges, 19);
        return res;
    }
    if (balance < -1 && key > tree[tree[node].right].val) {
        int res = LeftRotate(node, codeR);
        GetTreeState(nextNodes, nextEdges); 
        nextNodes[node].themeState = ElementTheme::DEFAULT; 
        nextNodes[res].themeState = ElementTheme::DEFAULT; 
        PushNewStep(nextNodes, nextEdges, 19);
        return res;
    }
    if (balance > 1 && key > tree[tree[node].left].val) {
        int leftChild = tree[node].left;
        tree[node].left = LeftRotate(tree[node].left, codeLR);
        int finalAnim = 13; int res = RightRotate(node, finalAnim);
        GetTreeState(nextNodes, nextEdges); 
        nextNodes[node].themeState = ElementTheme::DEFAULT; 
        nextNodes[res].themeState = ElementTheme::DEFAULT; 
        nextNodes[leftChild].themeState = ElementTheme::DEFAULT;
        PushNewStep(nextNodes, nextEdges, 19);
        return res;
    }
    if (balance < -1 && key < tree[tree[node].right].val) {
        int rightChild = tree[node].right;
        tree[node].right = RightRotate(tree[node].right, codeRL);
        int finalAnim = 17; int res = LeftRotate(node, finalAnim);
        GetTreeState(nextNodes, nextEdges); 
        nextNodes[node].themeState = ElementTheme::DEFAULT; 
        nextNodes[res].themeState = ElementTheme::DEFAULT; 
        nextNodes[rightChild].themeState = ElementTheme::DEFAULT;
        PushNewStep(nextNodes, nextEdges, 19);
        return res;
    }

    GetTreeState(nextNodes, nextEdges);
    nextNodes[node].themeState = ElementTheme::DEFAULT;
    PushNewStep(nextNodes, nextEdges, 19);
    return node;
}

int AVL::DeleteNode(int node, int key) {
    if (node == -1) return node;

    std::vector<VertexState> nextNodes; std::vector<EdgeState> nextEdges;
    GetTreeState(nextNodes, nextEdges);
    nextNodes[node].themeState = ElementTheme::HIGHLIGHT;
    PushNewStep(nextNodes, nextEdges, 1);

    if (key < tree[node].val) {
        GetTreeState(nextNodes, nextEdges); nextNodes[node].themeState = ElementTheme::HIGHLIGHT;
        PushNewStep(nextNodes, nextEdges, 2);
        tree[node].left = DeleteNode(tree[node].left, key);
    } else if (key > tree[node].val) {
        GetTreeState(nextNodes, nextEdges); nextNodes[node].themeState = ElementTheme::HIGHLIGHT;
        PushNewStep(nextNodes, nextEdges, 3);
        tree[node].right = DeleteNode(tree[node].right, key);
    } else {
        GetTreeState(nextNodes, nextEdges); nextNodes[node].themeState = ElementTheme::SUCCESS;
        PushNewStep(nextNodes, nextEdges, 4);

        if (tree[node].left == -1 || tree[node].right == -1) {
            GetTreeState(nextNodes, nextEdges); PushNewStep(nextNodes, nextEdges, 5);
            int temp = tree[node].left != -1 ? tree[node].left : tree[node].right;

            if (temp == -1) { 
                tree[node].active = false; node = -1;
            } else { 
                tree[node].active = false; node = temp;
            }
            GetTreeState(nextNodes, nextEdges); PushNewStep(nextNodes, nextEdges, 8); 
        } else {
            GetTreeState(nextNodes, nextEdges); PushNewStep(nextNodes, nextEdges, 9);
            int temp = tree[node].right;
            while (tree[temp].left != -1) temp = tree[temp].left;

            GetTreeState(nextNodes, nextEdges); nextNodes[temp].themeState = ElementTheme::SUCCESS;
            PushNewStep(nextNodes, nextEdges, 10); 

            tree[node].val = tree[temp].val; 
            GetTreeState(nextNodes, nextEdges); PushNewStep(nextNodes, nextEdges, 11);

            tree[node].right = DeleteNode(tree[node].right, tree[temp].val);
        }
    }

    if (node == -1) return node;

    UpdateHeight(node); 
    GetTreeState(nextNodes, nextEdges); nextNodes[node].themeState = ElementTheme::HIGHLIGHT;
    PushNewStep(nextNodes, nextEdges, 16); 

    int balance = GetBalance(node);
    PushNewStep(nextNodes, nextEdges, 17); 

    int codeLL = 18, codeLR = 19, codeRR = 20, codeRL = 21;
    if (balance > 1 && GetBalance(tree[node].left) >= 0) {
        int res = RightRotate(node, codeLL);
        GetTreeState(nextNodes, nextEdges); nextNodes[node].themeState = ElementTheme::DEFAULT; nextNodes[res].themeState = ElementTheme::DEFAULT; PushNewStep(nextNodes, nextEdges, 22);
        return res;
    }
    if (balance > 1 && GetBalance(tree[node].left) < 0) {
        int leftChild = tree[node].left;
        tree[node].left = LeftRotate(tree[node].left, codeLR);
        int finalAnim = 19; int res = RightRotate(node, finalAnim);
        GetTreeState(nextNodes, nextEdges); nextNodes[node].themeState = ElementTheme::DEFAULT; nextNodes[res].themeState = ElementTheme::DEFAULT; nextNodes[leftChild].themeState = ElementTheme::DEFAULT; PushNewStep(nextNodes, nextEdges, 22);
        return res;
    }
    if (balance < -1 && GetBalance(tree[node].right) <= 0) {
        int res = LeftRotate(node, codeRR);
        GetTreeState(nextNodes, nextEdges); nextNodes[node].themeState = ElementTheme::DEFAULT; nextNodes[res].themeState = ElementTheme::DEFAULT; PushNewStep(nextNodes, nextEdges, 22);
        return res;
    }
    if (balance < -1 && GetBalance(tree[node].right) > 0) {
        int rightChild = tree[node].right;
        tree[node].right = RightRotate(tree[node].right, codeRL);
        int finalAnim = 21; int res = LeftRotate(node, finalAnim);
        GetTreeState(nextNodes, nextEdges); nextNodes[node].themeState = ElementTheme::DEFAULT; nextNodes[res].themeState = ElementTheme::DEFAULT; nextNodes[rightChild].themeState = ElementTheme::DEFAULT; PushNewStep(nextNodes, nextEdges, 22);
        return res;
    }

    GetTreeState(nextNodes, nextEdges); nextNodes[node].themeState = ElementTheme::DEFAULT;
    PushNewStep(nextNodes, nextEdges, 22);
    return node;
}

// ================= CÁC LỆNH GIAO TIẾP UI =================

void AVL::GenerateInit(const std::vector<int>& values) {
    AnimationNodes.clear(); AnimationEdges.clear(); stepCodeLines.clear();
    tree.clear(); rootIndex = -1; curIndex = 0; TotalTime = 0.0f; isPause = false;
    currentNodes.clear(); currentEdges.clear();
    
    SetCode({
        "void Init() {",
        "  root = NULL;",
        "  for (int v : values)",
        "    root = InsertNode(root, v);",
        "}"
    });

    std::vector<VertexState> nextNodes; std::vector<EdgeState> nextEdges;
    GetTreeState(nextNodes, nextEdges);
    PushNewStep(nextNodes, nextEdges, 0); 
    
    for (int val : values) {
        rootIndex = InsertNode(rootIndex, val, -1);
        
        // Quét sạch màu dư thừa (Trở về 1 màu đồng nhất) sau mỗi lần Insert
        GetTreeState(nextNodes, nextEdges);
        for(auto& n : nextNodes) n.themeState = ElementTheme::DEFAULT;
        PushNewStep(nextNodes, nextEdges, 3); 
    }
    
    // Đảm bảo bước kết thúc Init toàn bộ node sạch màu 100%
    GetTreeState(nextNodes, nextEdges);
    for(auto& n : nextNodes) n.themeState = ElementTheme::DEFAULT;
    PushNewStep(nextNodes, nextEdges, 4); 
    
    typeQuery = -1; 
}

void AVL::GenerateAdd(int val) {
    AnimationNodes.clear(); AnimationEdges.clear(); stepCodeLines.clear();
    curIndex = 0; TotalTime = 0.0f; isPause = false;
    
    SetCode({
/* 0*/  "Node* insert(Node* node, int key) {",
/* 1*/  "  if (node == NULL) return new Node(key);",
/* 2*/  "  if (key < node->key)",
/* 3*/  "    node->left = insert(node->left, key);",
/* 4*/  "  else if (key > node->key)",
/* 5*/  "    node->right = insert(node->right, key);",
/* 6*/  "  else return node;",
/* 7*/  "  node->height = 1 + max(height(node->left), height(node->right));",
/* 8*/  "  int balance = getBalance(node);",
/* 9*/  "  if (balance > 1 && key < node->left->key) return rightRotate(node);",
/*10*/  "  if (balance < -1 && key > node->right->key) return leftRotate(node);",
/*11*/  "  if (balance > 1 && key > node->left->key) {",
/*12*/  "    node->left = leftRotate(node->left);",
/*13*/  "    return rightRotate(node);",
/*14*/  "  }",
/*15*/  "  if (balance < -1 && key < node->right->key) {",
/*16*/  "    node->right = rightRotate(node->right);",
/*17*/  "    return leftRotate(node);",
/*18*/  "  }",
/*19*/  "  return node;",
/*20*/  "}"
    });

    std::vector<VertexState> nextNodes; std::vector<EdgeState> nextEdges;
    GetTreeState(nextNodes, nextEdges);
    PushNewStep(nextNodes, nextEdges, 0); 
    
    rootIndex = InsertNode(rootIndex, val, -1);
    
    // Quét sạch màu ở frame cuối
    GetTreeState(nextNodes, nextEdges);
    for(auto& n : nextNodes) n.themeState = ElementTheme::DEFAULT;
    PushNewStep(nextNodes, nextEdges, 20); 
    typeQuery = -1;
}

void AVL::GenerateDelete(int val) {
    AnimationNodes.clear(); AnimationEdges.clear(); stepCodeLines.clear();
    curIndex = 0; TotalTime = 0.0f; isPause = false;

    SetCode({
/* 0*/  "Node* deleteNode(Node* root, int key) {",
/* 1*/  "  if (root == NULL) return root;",
/* 2*/  "  if (key < root->key) root->left = deleteNode(root->left, key);",
/* 3*/  "  else if (key > root->key) root->right = deleteNode(root->right, key);",
/* 4*/  "  else {",
/* 5*/  "    if ((root->left == NULL) || (root->right == NULL)) {",
/* 6*/  "      Node *temp = root->left ? root->left : root->right;",
/* 7*/  "      if (temp == NULL) { temp = root; root = NULL; } else *root = *temp;",
/* 8*/  "      free(temp);",
/* 9*/  "    } else {",
/*10*/  "      Node* temp = minValueNode(root->right);",
/*11*/  "      root->key = temp->key;",
/*12*/  "      root->right = deleteNode(root->right, temp->key);",
/*13*/  "    }",
/*14*/  "  }",
/*15*/  "  if (root == NULL) return root;",
/*16*/  "  root->height = 1 + max(height(root->left), height(root->right));",
/*17*/  "  int balance = getBalance(root);",
/*18*/  "  if (balance > 1 && getBalance(root->left) >= 0) return rightRotate(root);",
/*19*/  "  if (balance > 1 && getBalance(root->left) < 0) { root->left = leftRotate(root->left); return rightRotate(root); }",
/*20*/  "  if (balance < -1 && getBalance(root->right) <= 0) return leftRotate(root);",
/*21*/  "  if (balance < -1 && getBalance(root->right) > 0) { root->right = rightRotate(root->right); return leftRotate(root); }",
/*22*/  "  return root;",
/*23*/  "}"
    });

    std::vector<VertexState> nextNodes; std::vector<EdgeState> nextEdges;
    GetTreeState(nextNodes, nextEdges);
    PushNewStep(nextNodes, nextEdges, 0);

    rootIndex = DeleteNode(rootIndex, val);

    // Quét sạch màu ở frame cuối
    GetTreeState(nextNodes, nextEdges);
    for(auto& n : nextNodes) n.themeState = ElementTheme::DEFAULT;
    PushNewStep(nextNodes, nextEdges, 23);
    typeQuery = -1;
}

void AVL::GenerateSearch(int val) {
    AnimationNodes.clear(); AnimationEdges.clear(); stepCodeLines.clear();
    curIndex = 0; TotalTime = 0.0f; isPause = false;
    
    SetCode({
/* 0*/  "Node* search(Node* node, int key) {",
/* 1*/  "  if (node == NULL || node->key == key)",
/* 2*/  "    return node;",
/* 3*/  "  if (node->key < key)",
/* 4*/  "    return search(node->right, key);",
/* 5*/  "  return search(node->left, key);",
/* 6*/  "}"
    });

    std::vector<VertexState> nextNodes; std::vector<EdgeState> nextEdges;
    GetTreeState(nextNodes, nextEdges);
    PushNewStep(nextNodes, nextEdges, 0);

    int curr = rootIndex;
    bool found = false;

    while (curr != -1) {
        GetTreeState(nextNodes, nextEdges);
        nextNodes[curr].themeState = ElementTheme::HIGHLIGHT;
        PushNewStep(nextNodes, nextEdges, 1);
        
        if (tree[curr].val == val) {
            GetTreeState(nextNodes, nextEdges);
            nextNodes[curr].themeState = ElementTheme::SUCCESS;
            PushNewStep(nextNodes, nextEdges, 2);
            found = true; break;
        } else if (tree[curr].val < val) {
            PushNewStep(nextNodes, nextEdges, 4);
            curr = tree[curr].right;
        } else {
            PushNewStep(nextNodes, nextEdges, 5);
            curr = tree[curr].left;
        }
    }
    
    if (!found) { GetTreeState(nextNodes, nextEdges); PushNewStep(nextNodes, nextEdges, 2); }
    
    // Quét sạch màu ở frame cuối
    GetTreeState(nextNodes, nextEdges);
    for(auto& n : nextNodes) n.themeState = ElementTheme::DEFAULT;
    PushNewStep(nextNodes, nextEdges, 6);
    typeQuery = -1;
}

// ================= VÒNG LẶP CHÍNH =================

void AVL::Solve(const Palette &colors) {
    if (!myList.empty() && typeQuery != -1) {
        if (typeQuery == 0) GenerateInit(myList);
        else if (typeQuery == 1) GenerateAdd(myList[0]);
        else if (typeQuery == 2) GenerateDelete(myList[0]);
        else if (typeQuery == 4) GenerateSearch(myList[0]);

        myList.clear(); 
    }
    Draw(colors);
}

void AVL::Draw(const Palette &colors) {  
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
    
    // UPDATE VẪN LÀ THAO TÁC BỊ CHẶN (Không điển hình)
    if (UpdateButton.isPressed()) { isWarningOpen = true; }

    DrawTextEx(miniTitleFont, "PLAYBACK",{24, 580}, 20, 1.0f, colors.textPrimary);
    if (isPause) DrawTexture(play, 135, 616, colors.primaryAcc); else DrawTexture(pause, 135, 616, colors.primaryAcc);
    Rectangle playPauseRect = {135.0f, 616.0f, (float)play.width, (float)play.height};
    if (CheckCollisionPointRec(GetMousePosition(), playPauseRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) isPause ^= 1;

    DrawTextEx(miniTitleFont, "Speed", {24, 690}, 20, 0.8f, colors.textPrimary);
    mySpeedSlider.Draw(colors);
    DrawTextEx(miniTitleFont, FloatToString(mySpeedSlider.getSpeed()).c_str(), {280, 690}, 20, 0.8f, colors.textPrimary);

    DrawLineEx({319, 128}, {1920, 128}, 1.0f, colors.border);
    DrawTextEx(titleFont, "AVL TREE", {319 + 32, 85}, 32, 0.8f, colors.textPrimary);

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

    DrawTextEx(miniTitleFont, "Timeline", {24, 750}, 20, 0.8f, colors.textPrimary);
    int prevIndex = curIndex;
    myTimeline.Draw({24, 780}, {295, 780}, maxStep, curIndex, colors);
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

    // --- DIALOG NHẬP LIỆU ---
    if (isInsertDialogOpen) {
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.6f));
        float dialogWidth = 600.0f; float dialogHeight = 200.0f;
        float dialogX = (GetScreenWidth() - dialogWidth) / 2.0f; float dialogY = (GetScreenHeight() - dialogHeight) / 2.0f;
        Rectangle dialogRec = { dialogX, dialogY, dialogWidth, dialogHeight };

        DrawRectangleRec(dialogRec, colors.surface); DrawRectangleLinesEx(dialogRec, 2.0f, colors.primaryAcc);
        DrawTextEx(titleFont, "Input Data", {dialogX + 20, dialogY + 20}, 30, 1.0f, colors.textPrimary);

        Rectangle randomBtnRec = { dialogX + dialogWidth - 50, dialogY + 20, 30.0f, 30.0f };
        bool isRandomHovered = CheckCollisionPointRec(GetMousePosition(), randomBtnRec);
        if (isRandomHovered) { DrawRectangleRec(randomBtnRec, Fade(colors.primaryAcc, 0.2f)); DrawTextEx(miniTitleFont, "Random", {randomBtnRec.x - 65, randomBtnRec.y + 5}, 16, 1.0f, colors.textSecondary); }
        DrawTexture(randomIcon, randomBtnRec.x, randomBtnRec.y, WHITE);

        if (isRandomHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int cnt = (typeQuery == 0) ? rand()%5 + 5 : 1; std::string input;
            while (cnt--) input += std::to_string(rand() % 99 + 1) + (cnt > 0 ? " " : "");
            strcpy(insertInputText, input.c_str()); insertInputCount = input.length();
        }

        Rectangle textBoxRec = { dialogX + 20, dialogY + 80, dialogWidth - 40, 50 };
        DrawRectangleRec(textBoxRec, colors.background); DrawRectangleLinesEx(textBoxRec, 1.0f, colors.border);
        DrawTextEx(font, insertInputText, {textBoxRec.x + 10, textBoxRec.y + 13}, 24, 1.0f, colors.textPrimary);

        int key = GetCharPressed();
        while (key > 0) { if ((key >= 32) && (key <= 125) && (insertInputCount < 255)) { insertInputText[insertInputCount++] = (char)key; insertInputText[insertInputCount] = '\0'; } key = GetCharPressed(); }
        if (IsKeyPressed(KEY_BACKSPACE) && insertInputCount > 0) insertInputText[--insertInputCount] = '\0';
        
        framesCounter++;
        if ((framesCounter / 30) % 2 == 0) {
            Vector2 textSize = MeasureTextEx(font, insertInputText, 24, 1.0f);
            DrawLineEx({ textBoxRec.x + 12 + textSize.x, textBoxRec.y + 10 }, { textBoxRec.x + 12 + textSize.x, textBoxRec.y + 40 }, 2.0f, colors.primaryAcc);
        }

        DrawTextEx(miniTitleFont, "Press ENTER to confirm | ESC to cancel", { dialogX + 20, dialogY + 145 }, 18, 1.0f, colors.textSecondary);

        if (IsKeyPressed(KEY_ENTER)) {
            isInsertDialogOpen = false; int cur_num = 0; bool hasNum = false; myList.clear();
            for (int i = 0; i < insertInputCount; i++) {
                if (insertInputText[i] >= '0' && insertInputText[i] <= '9') { cur_num = cur_num * 10 + (insertInputText[i] - '0'); hasNum = true; } 
                else if (insertInputText[i] == ' ' && hasNum) { myList.push_back(cur_num); cur_num = 0; hasNum = false; }
            }
            if (hasNum) myList.push_back(cur_num);
            if (myList.empty()) typeQuery = -1;
        }
        else if (IsKeyPressed(KEY_ESCAPE)) { isInsertDialogOpen = false; typeQuery = -1; }
    }

    // --- DIALOG CẢNH BÁO ---
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

std::string AVL::FloatToString(float value) { char buffer[10]; snprintf(buffer, sizeof(buffer), "%.1fx", value); return std::string(buffer); }