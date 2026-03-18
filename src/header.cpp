#include "header.h"

header::header(int w, int h, const char *closeImg, const char *closeHoverImg, const char *themeImg, const char *themeHoverImg, const char *logoImg, const char *logoImgHover)
    : closeButton(closeImg, closeHoverImg, closeImg, closeHoverImg, 
                  Vector2{(float)(w - h * 65.0f / 1080.0f * 1.2f), h * 10.0f / 1080.0f }, 
                  (h * 65.0f / 1080.0f * 0.8f)),
      themeLightButton(themeImg, themeImg, themeImg, themeImg, 
                  Vector2{(float)(w - h * 65.0f / 1080.0f * 3.0f), h * 15.0f / 1080.0f }, 
                  (h * 65.0f / 1080.0f * 0.6f)),
      themeDarkButton(themeHoverImg, themeHoverImg, themeHoverImg, themeHoverImg, 
                  Vector2{(float)(w - h * 65.0f / 1080.0f * 4.0f), h * 15.0f / 1080.0f }, 
                  (h * 65.0f / 1080.0f * 0.6f))
{
    width = w;
    height = h * 65 / 1080;
    
    // Tải và thay đổi kích thước cho Logo bình thường
    Image image = LoadImage(logoImg);
    ImageResize(&image, int(h * 50 / 1080), int(h * 50 / 1080));
    logoTexture = LoadTextureFromImage(image);
    UnloadImage(image);
    
    // Tải và thay đổi kích thước cho Logo khi Hover
    Image imageHover = LoadImage(logoImgHover);
    ImageResize(&imageHover, int(h * 50 / 1080), int(h * 50 / 1080));
    logoHoverTexture = LoadTextureFromImage(imageHover);
    UnloadImage(imageHover);
    
    titleFont = LoadFontEx("assets/fonts/Roboto/Roboto-Bold.ttf", 48, 0, 0);
    SetTextureFilter(titleFont.texture, TEXTURE_FILTER_BILINEAR);
    
    state = 1;
}

header::~header()
{
    UnloadTexture(logoTexture); 
    UnloadTexture(logoHoverTexture); // Đừng quên giải phóng ảnh Hover
    UnloadFont(titleFont); 
}

void header::Draw()
{
    DrawRectangle(0, 0, width, height, GetColor(0x0F172AFF));
    DrawLine(0, height, width, height, GetColor(0x334155FF));

    float paddingLeft = 60.0f; 
    float logoCenterY = (height - logoTexture.height) / 2.0f + height * 0.06f;

    const char* title = "DS Visualizer";
    float fontSize = height * 0.45f;
    Vector2 textSize = MeasureTextEx(titleFont, title, fontSize, 1.0f);
    
    float textCenterY = (height - textSize.y) / 2.0f + height * 0.06f;
    float gap = 15.0f; 
    
    // Tọa độ vẽ chữ
    Vector2 textPos = {paddingLeft + logoTexture.width + gap, textCenterY };
    
    // Khung xét va chạm chuột (Hover area)
    Rectangle rect = {paddingLeft, 0, textSize.x + height, (float)height};
    
    // Biến kiểm tra xem chuột có đang nằm trong khu vực Logo/Tiêu đề không
    bool isHovering = CheckCollisionPointRec(GetMousePosition(), rect);

    // Gộp logic vẽ Ảnh và Text vào chung một lệnh if/else
    if (isHovering) 
    {
        DrawTexture(logoHoverTexture, paddingLeft, logoCenterY, WHITE); // Vẽ ảnh hover
        DrawTextEx(titleFont, title, textPos, fontSize, 1.0f, GetColor(0x3B82F6FF)); // Chữ xanh
    } 
    else 
    {
        DrawTexture(logoTexture, paddingLeft, logoCenterY, WHITE); // Vẽ ảnh thường
        DrawTextEx(titleFont, title, textPos, fontSize, 1.0f, GetColor(0xF8FAFCFF)); // Chữ trắng
    }

    // Xử lý logic click để quay về menu
    if (isHovering && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) 
    {
        returnMenu = 1;
    }

    // 4. Vẽ các nút
    closeButton.Draw(GetMousePosition());
    themeDarkButton.Draw(GetMousePosition());
    themeLightButton.Draw(GetMousePosition());

    // 5. Xử lý Logic click các nút
    if (closeButton.isPressed(GetMousePosition(), IsMouseButtonDown(MOUSE_LEFT_BUTTON)))
    {
        state = 0; 
    }
    
    if (themeLightButton.isPressed(GetMousePosition(), IsMouseButtonDown(MOUSE_LEFT_BUTTON)))
    {
        themeState = 1;
    }

    if (themeDarkButton.isPressed(GetMousePosition(), IsMouseButtonDown(MOUSE_LEFT_BUTTON)))
    {
        themeState = 0;
    }
}

bool header::getState()
{
    return state;
}

bool header::getThemeState()
{
    return themeState;
}

bool header::getReturnMenu()
{
    return returnMenu;
}