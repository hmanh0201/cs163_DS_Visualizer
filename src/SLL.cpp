#include "SLL.h"
#include <iostream>
#include <cstdlib> // Thư viện dùng cho hàm rand() sinh số ngẫu nhiên
#include <ctime>   // Thư viện dùng cho hàm time() tạo seed ngẫu nhiên
#include <string>
#include <cstring>

SLL::SLL(int width, int height) 
{
    StartX = 0;
    StartY = height * 65 / 1080;
    EndX = width;
    EndY = height;
    curIndex = size = 0;

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
}

void SLL::Solve(const Palette &colors)
{
    Draw(colors);
    if (isPause) return;
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
        isInsertDialogOpen = true;
        insertInputText[0] = '\0';
        insertInputCount = 0;
        typeQuery = 0;
    }
    if (AddButton.isPressed()) {
        isInsertDialogOpen = true;
        insertInputText[0] = '\0';
        insertInputCount = 0;
        typeQuery = 1;
    }   
    if (DeleteButton.isPressed()) {
        isInsertDialogOpen = true;
        insertInputText[0] = '\0';
        insertInputCount = 0;
        typeQuery = 2;
    }
    if (UpdateButton.isPressed()) {
        isInsertDialogOpen = true;
        insertInputText[0] = '\0';
        insertInputCount = 0;
        typeQuery = 3;
    }
    if (SearchButton.isPressed()) {
        isInsertDialogOpen = true;
        insertInputText[0] = '\0';
        insertInputCount = 0;
        typeQuery = 4;
    }
    
    if (isInsertDialogOpen)
    {
        // a. Lớp nền mờ (Overlay) bao phủ toàn màn hình
        DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.6f));

        // b. Kích thước và vị trí hộp thoại (Căn giữa màn hình)
        float dialogWidth = 1000.0f;
        float dialogHeight = 200.0f;
        float dialogX = (GetScreenWidth() - dialogWidth) / 2.0f;
        float dialogY = (GetScreenHeight() - dialogHeight) / 2.0f;
        Rectangle dialogRec = { dialogX, dialogY, dialogWidth, dialogHeight };

        // Vẽ nền hộp thoại và viền
        DrawRectangleRec(dialogRec, colors.surface);
        DrawRectangleLinesEx(dialogRec, 2.0f, colors.primaryAcc);

        // Tiêu đề hộp thoại
        DrawTextEx(titleFont, "Input Data", {dialogX + 20, dialogY + 20}, 30, 1.0f, colors.textPrimary);

        // --- VẼ NÚT RANDOM Ở GÓC TRÊN BÊN PHẢI HỘP THOẠI ---
        Rectangle randomBtnRec = { dialogX + dialogWidth - 50, dialogY + 20, 30.0f, 30.0f };
        bool isRandomHovered = CheckCollisionPointRec(GetMousePosition(), randomBtnRec);
        
        if (isRandomHovered) {
            // Vẽ nền mờ khi di chuột vào và chữ tooltip nhỏ
            DrawRectangleRec(randomBtnRec, Fade(colors.primaryAcc, 0.2f));
            DrawTextEx(miniTitleFont, "Random", {randomBtnRec.x - 65, randomBtnRec.y + 5}, 16, 1.0f, colors.textSecondary);
        }
        
        // Vẽ icon Random
        DrawTexture(randomIcon, randomBtnRec.x, randomBtnRec.y, colors.primaryAcc);

        // Xử lý Click nút Random
        if (isRandomHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) 
        {
            int cnt = 1;
            if (typeQuery == 0) cnt = rand()%5 + 5;

            std::string input;

            while (cnt--)
            {
                int randomValue = rand() % 99 + 1;
                std::string randStr = std::to_string(randomValue);
                input += randStr + (cnt > 0 ? " " : "");
            }
            strcpy(insertInputText, input.c_str());
            insertInputCount = input.length();
        }
        // --------------------------------------------------

        // c. Vẽ ô nhập liệu (Text Box) bên trong hộp thoại
        Rectangle textBoxRec = { dialogX + 20, dialogY + 80, dialogWidth - 40, 50 };
        DrawRectangleRec(textBoxRec, colors.background);
        DrawRectangleLinesEx(textBoxRec, 1.0f, colors.border);

        // Hiển thị chữ đang gõ
        DrawTextEx(font, insertInputText, {textBoxRec.x + 10, textBoxRec.y + 13}, 24, 1.0f, colors.textPrimary);

        // d. Xử lý Logic Gõ Phím
        int key = GetCharPressed();
        while (key > 0)
        {
            if ((key >= 32) && (key <= 125) && (insertInputCount < 255))
            {
                insertInputText[insertInputCount] = (char)key;
                insertInputText[insertInputCount + 1] = '\0';
                insertInputCount++;
            }
            key = GetCharPressed();
        }

        // Xóa ký tự (Backspace)
        if (IsKeyPressed(KEY_BACKSPACE) && insertInputCount > 0)
        {
            insertInputCount--;
            insertInputText[insertInputCount] = '\0';
        }

        // e. Hiệu ứng con trỏ nhấp nháy
        framesCounter++;
        if ((framesCounter / 30) % 2 == 0) 
        {
            Vector2 textSize = MeasureTextEx(font, insertInputText, 24, 1.0f);
            DrawLineEx(
                { textBoxRec.x + 12 + textSize.x, textBoxRec.y + 10 }, 
                { textBoxRec.x + 12 + textSize.x, textBoxRec.y + 40 }, 
                2.0f, colors.primaryAcc);
        }

// --- XỬ LÝ KÉO THẢ FILE (DRAG & DROP) BẰNG RAYLIB GỐC ---
        if (IsFileDropped())
        {
            // Lấy danh sách các file được thả vào cửa sổ
            FilePathList droppedFiles = LoadDroppedFiles();

            // Chỉ xử lý nếu có ít nhất 1 file được thả vào
            if (droppedFiles.count > 0)
            {
                // Lấy đường dẫn của file đầu tiên
                const char* filePath = droppedFiles.paths[0];

                // (Tùy chọn) Kiểm tra xem có đúng là file .txt không
                if (IsFileExtension(filePath, ".txt"))
                {
                    // Đọc nội dung file
                    char* fileContent = LoadFileText(filePath);
                    if (fileContent != nullptr)
                    {
                        // Copy nội dung file vào ô nhập liệu (giới hạn 254 ký tự để tránh tràn mảng)
                        strncpy(insertInputText, fileContent, 254);
                        insertInputText[254] = '\0'; // Đảm bảo đóng chuỗi an toàn
                        
                        // Cập nhật lại số lượng ký tự
                        insertInputCount = strlen(insertInputText);

                        UnloadFileText(fileContent); // Giải phóng nội dung file
                    }
                }
            }
            
            // QUAN TRỌNG: Giải phóng bộ nhớ danh sách file đã thả
            UnloadDroppedFiles(droppedFiles); 
        }

        // f. Hướng dẫn sử dụng
        DrawTextEx(miniTitleFont, "Type, Random, or Drag & Drop a .txt file here", { dialogX + 20, dialogY + 140 }, 16, 1.0f, colors.textSecondary);
        DrawTextEx(miniTitleFont, "Press ENTER to confirm | ESC to cancel", { dialogX + 20, dialogY + 165 }, 18, 1.0f, colors.textSecondary);

        // g. Xử lý Xác nhận (ENTER) hoặc Hủy (ESC)
        if (IsKeyPressed(KEY_ENTER))
        {
            isInsertDialogOpen = false; 
            // Xử lý dữ liệu ở đây
            int cur_num = 0;
            myList.clear();
            for (int i = 0; i < insertInputCount; i++)
            {
                if (insertInputText[i] >= '0' && insertInputText[i] <= '9')
                {
                    cur_num = cur_num * 10 + (insertInputText[i] - '0');
                }
                else if (insertInputText[i] == ' ')
                {
                    myList.push_back(cur_num);
                    cur_num = 0;
                }
            }
            myList.push_back(cur_num);
        }
        else if (IsKeyPressed(KEY_ESCAPE))
        {
            isInsertDialogOpen = false; 
        }
    }

    // --- VẼ CÁC THÀNH PHẦN KHÁC (PLAYBACK, SPEED, TIMELINE) ---
    DrawTextEx(miniTitleFont, "PLAYBACK",{24, 580}, 20, 1.0f, colors.textPrimary);
    
    if (isPause)  
    {
        DrawTexture(play, 135, 616, colors.primaryAcc);
    }
    else
    {
        DrawTexture(pause, 135, 616, colors.primaryAcc);
    }

    Rectangle playPauseRect = {135.0f, 616.0f, (float)play.width, (float)play.height};
    if (CheckCollisionPointRec(GetMousePosition(), playPauseRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        isPause ^= 1;
    }

    DrawTextEx(miniTitleFont, "Speed", {24, 690}, 20, 0.8f, colors.textPrimary);
    mySpeedSlider.Draw(colors);
    DrawTextEx(miniTitleFont, FloatToString(mySpeedSlider.getSpeed()).c_str(), {280, 690}, 20, 0.8f, colors.textPrimary);

    DrawTextEx(miniTitleFont, "Timeline", {24, 750}, 20, 0.8f, colors.textPrimary);
    myTimeline.Draw({24, 780}, {295, 780}, size, curIndex, colors);
}

std::string SLL::FloatToString(float value)
{
    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%.1fx", value);
    return std::string(buffer);
}