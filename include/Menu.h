#pragma once
#include "UI.h"
#include "theme.h"
#include <vector>

class DrawMenu
{
    public:
        // Constructor nhận vào chiều rộng và chiều cao của màn hình (ví dụ 1920x1080 hoặc 1280x720)
        DrawMenu(int w, int h);
        ~DrawMenu();
        
        // Hàm vẽ toàn bộ giao diện
        void Draw(const Palette& colors);

        // Hàm kiểm tra xem người dùng có click vào topic nào không, trả về index (0-5)
        int CheckTopicClick();

    private:
        WriteText mainTitle;
        WriteText subTitle;
        
        int width;
        int height;

        // Danh sách các thẻ Topic
        std::vector<Topic*> topics;
};