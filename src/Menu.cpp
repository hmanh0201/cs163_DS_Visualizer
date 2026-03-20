#pragma once
#include "Menu.h"


DrawMenu::DrawMenu(int w, int h)
{
    width = w;
    height = h;

    mainTitle.LoadMyFont("assets/fonts/Roboto/Roboto-Bold.ttf", 128);
    subTitle.LoadMyFont("assets/fonts/Roboto/Roboto-Regular.ttf", 28);

    int cardW = 380;
    int cardH = 160;

    topics.push_back(new Topic("assets/list.png", "Singly Linked List", cardW, cardH));
    topics.push_back(new Topic("assets/heap.png", "Heap", cardW, cardH));
    topics.push_back(new Topic("assets/avl.png", "AVL tree", cardW, cardH));
    
    topics.push_back(new Topic("assets/trie.png", "Trie", cardW, cardH));
    topics.push_back(new Topic("assets/mst.png", "Minimum Spanning Tree", cardW, cardH));
    topics.push_back(new Topic("assets/connected.png", "Find Connected Component", cardW, cardH));
}

DrawMenu::~DrawMenu()
{
    for (auto topic : topics) {
        delete topic;
    }
    topics.clear();
}

void DrawMenu::Draw(const Palette &colors)
{
    const char* titleStr = "Data Structure Visualizer";
    float titleX = (width / 2.0f) - 355.0f; // Ước lượng dịch trái 400px
    mainTitle.Draw(titleStr, { titleX, 120 }, 72, 1.5f, colors.textPrimary);

    const char* sub1 = "Interactive visualizations for common data structures. Select a topic to explore!";
    float sub1X = (width / 2.0f) - 390.0f;
    subTitle.Draw(sub1, { sub1X, 220 }, 26, 1.0f, colors.textSecondary);

    int cols = 3;                  
    float spacingX = 40.0f;        
    float spacingY = 40.0f;        
    float cardW = 380.0f;
    float cardH = 160.0f;

    float totalGridWidth = (cols * cardW) + ((cols - 1) * spacingX);
    
    float startX = (width - totalGridWidth) / 2.0f; 
    float startY = 350.0f;                         

    for (int i = 0; i < topics.size(); ++i) {
        int row = i / cols; 
        int col = i % cols; 

        Vector2 pos = {
            startX + col * (cardW + spacingX),
            startY + row * (cardH + spacingY)
        };

        topics[i]->Draw(pos, colors);
    }
}

int DrawMenu::CheckTopicClick()
{
    for (int i = 0; i < topics.size(); ++i) {
        if (topics[i]->getClick()) {
            return i; 
        }
    }
    return -1; 
}
