#pragma once
#include "Menu.h"


DrawMenu::DrawMenu(int w, int h)
{
    width = w;
    height = h;

    // 1. Tải Font chữ (Lưu ý: Hãy chắc chắn đường dẫn font của bạn hợp lệ)
    mainTitle.LoadMyFont("assets/fonts/Roboto/Roboto-Bold.ttf", 128);
    subTitle.LoadMyFont("assets/fonts/Roboto/Roboto-Regular.ttf", 28);

    // 2. Kích thước thẻ Card (chỉnh lại cho dẹt và rộng giống ảnh thiết kế)
    int cardW = 380;
    int cardH = 160;

    // 3. Khởi tạo 6 Topics theo đúng hình ảnh
    // LƯU Ý: Bạn cần chuẩn bị các file ảnh icon này trong thư mục assets
    topics.push_back(new Topic("assets/icons/list.png", "Singly Linked List", cardW, cardH));
    topics.push_back(new Topic("assets/icons/heap.png", "Heap", cardW, cardH));
    topics.push_back(new Topic("assets/icons/avl.png", "AVL tree", cardW, cardH));
    
    topics.push_back(new Topic("assets/icons/trie.png", "Trie", cardW, cardH));
    topics.push_back(new Topic("assets/icons/mst.png", "Minimum Spanning Tree", cardW, cardH));
    topics.push_back(new Topic("assets/icons/connected.png", "Find Connected Component", cardW, cardH));
}

DrawMenu::~DrawMenu()
{
    // Giải phóng bộ nhớ động
    for (auto topic : topics) {
        delete topic;
    }
    topics.clear();
}

void DrawMenu::Draw(const Palette &colors)
{

    // ==========================================
    // 2. VẼ TIÊU ĐỀ (Title & Subtitle)
    // ==========================================
    // Vì WriteText hiện tại chưa có hàm đo Text width, tôi dùng công thức ước lượng 
    // để căn giữa màn hình cho khớp với thiết kế.
    
    // Căn giữa Title lớn
    const char* titleStr = "Data Structure Visualizer";
    float titleX = (width / 2.0f) - 400.0f; // Ước lượng dịch trái 400px
    mainTitle.Draw(titleStr, { titleX, 120 }, 72, 1.5f, colors.textPrimary);

    // Căn giữa Subtitle
    const char* sub1 = "Interactive visualizations for common data structures. Select a topic to explore!";
    float sub1X = (width / 2.0f) - 340.0f;
    subTitle.Draw(sub1, { sub1X, 220 }, 26, 1.0f, colors.textSecondary);


    // ==========================================
    // 3. VẼ GRID CÁC TOPICS (Căn giữa hoàn toàn)
    // ==========================================
    int cols = 3;                  // Số cột
    float spacingX = 40.0f;        // Khoảng cách ngang giữa các thẻ
    float spacingY = 40.0f;        // Khoảng cách dọc giữa các hàng
    float cardW = 380.0f;
    float cardH = 160.0f;

    // Tính toán tổng chiều rộng của lưới để căn giữa
    float totalGridWidth = (cols * cardW) + ((cols - 1) * spacingX);
    
    float startX = (width - totalGridWidth) / 2.0f; // Tọa độ X bắt đầu vẽ
    float startY = 350.0f;                          // Tọa độ Y bắt đầu vẽ lưới

    for (int i = 0; i < topics.size(); ++i) {
        int row = i / cols; // Xác định hàng hiện tại
        int col = i % cols; // Xác định cột hiện tại

        Vector2 pos = {
            startX + col * (cardW + spacingX),
            startY + row * (cardH + spacingY)
        };

        // Hàm Draw của Topic tự động xử lý Hover Effect (nhích lên, đổi màu viền)
        topics[i]->Draw(pos, colors);
    }
}

// Hàm để bạn gọi trong vòng lặp game chính (main loop) để bắt sự kiện
int DrawMenu::CheckTopicClick()
{
    for (int i = 0; i < topics.size(); ++i) {
        if (topics[i]->getClick()) {
            return i; // Trả về vị trí của topic được click (0 đến 5)
        }
    }
    return -1; // Không có topic nào được click
}
