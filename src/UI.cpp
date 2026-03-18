#pragma once
#include <raylib.h>
#include <string>   
#include <UI.h>

// Button
button::button(const char *imagePath, const char *ChangeImagePath, const char *imagePath02, const char *ChangeImagePath02, Vector2 imagePosition, double scale)
{
    Image image = LoadImage(imagePath);
    
    int w = image.width * scale / image.height;
    int h = scale;

    ImageResize(&image, w, h);
    texture = LoadTextureFromImage(image);
    UnloadImage(image);

    image = LoadImage(ChangeImagePath);
    ImageResize(&image, w, h);
    ChangeTexture = LoadTextureFromImage(image);
    UnloadImage(image);

    position = imagePosition;

    image = LoadImage(imagePath02);

    ImageResize(&image, w, h);
    texture02 = LoadTextureFromImage(image);
    UnloadImage(image);

    image = LoadImage(ChangeImagePath02);
    ImageResize(&image, w, h);
    ChangeTexture02 = LoadTextureFromImage(image);
    UnloadImage(image);
}

button::~button()
{
    UnloadTexture(texture);
    UnloadTexture(ChangeTexture);
    UnloadTexture(texture02);
    UnloadTexture(ChangeTexture02);
}

void button::Draw(Vector2 mousePos)
{
    Rectangle rect = {position.x, position.y, static_cast<double>(texture.width), static_cast<double>(texture.height)};
    if (!type)
    {
        if (CheckCollisionPointRec(mousePos, rect)) DrawTextureV(ChangeTexture, position, WHITE);
        else DrawTextureV(texture, position, WHITE);
    }
    else 
    {
        if (CheckCollisionPointRec(mousePos, rect)) DrawTextureV(ChangeTexture02, position, WHITE);
        else DrawTextureV(texture02, position, WHITE);
    }
}

bool button::isPressed(Vector2 mousePos, bool mousePressed)
{
    Rectangle rect = {position.x, position.y, static_cast<double>(texture.width), static_cast<double>(texture.height)};
    if (CheckCollisionPointRec(mousePos, rect) && mousePressed) return true;
    return false;
}

WriteText::WriteText() {
    isLoaded = false;
    font = GetFontDefault(); 
}

WriteText::~WriteText() {
    Unload();
}

bool WriteText::LoadMyFont(const std::string& filePath, int fontSize) {
    if (isLoaded) {
        UnloadFont(font);
    }

    font = LoadFontEx(filePath.c_str(), fontSize, 0, 0);
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);

    if (font.texture.id == 0) {
        isLoaded = false;
        return false;
    }

    isLoaded = true;
    return true;
}

void WriteText::Draw(const std::string& text, Vector2 position, float fontSize, float spacing, Color color) {
    if (isLoaded) {
        DrawTextEx(font, text.c_str(), position, fontSize, spacing, color);
    } else {
        DrawText(text.c_str(), (int)position.x, (int)position.y, (int)fontSize, color);
    }
}

void WriteText::Unload() {
    if (isLoaded) {
        UnloadFont(font);
        isLoaded = false;
    }
}

Topic::Topic(const std::string& iconPath, const std::string& titleText, int w, int h) {
    iconTex = LoadTexture(iconPath.c_str());

    font = LoadFontEx("assets/fonts/static/Inter-SemiBold.ttf", 28, 0, 0); 
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);

    title = titleText;
    cardWidth = w;
    cardHeight = h;
    bounds = {0, 0, (float)w, (float)h}; 
}

Topic::~Topic() {
    UnloadTexture(iconTex);
    UnloadFont(font);
}

void Topic::Draw(Vector2 pos, const Palette& palette) {
    this->bounds = {pos.x, pos.y, (float)cardWidth, (float)cardHeight};

    Vector2 mousePos = GetMousePosition();
    bool isHovered = CheckCollisionPointRec(mousePos, this->bounds);
    
    float liftOffset = 0.0f; 
    Color borderColor = palette.border; 
    // 1. Khai báo màu chữ mặc định
    Color textColor = palette.textPrimary; 

    if (isHovered) {
        liftOffset = -10.0f; 
        borderColor = palette.primaryAcc; 
        // 2. Cập nhật màu chữ sang màu chủ đạo (xanh) khi hover
        textColor = palette.primaryAcc; 
    }

    Vector2 drawnPos = {pos.x, pos.y + liftOffset};
    
    DrawRectangleRounded({drawnPos.x, drawnPos.y, (float)cardWidth, (float)cardHeight}, 0.05f, 10, palette.surface);
    // Lưu ý: DrawRectangleRoundedLinesEx thay vì DrawRectangleRoundedLines
    DrawRectangleRoundedLinesEx({drawnPos.x, drawnPos.y, (float)cardWidth, (float)cardHeight}, 0.05f, 10, 2.0f, borderColor);

    float marginX = 30.0f; 
    float marginY = 30.0f; 
    float iconSquareSize = 48.0f; 
    float textYOffset = 25.0f;    

    Vector2 iconBgPos = {drawnPos.x + marginX, drawnPos.y + marginY};
    Color iconBgColor = {palette.primaryAcc.r, palette.primaryAcc.g, palette.primaryAcc.b, 30}; 
    DrawRectangleRounded({iconBgPos.x, iconBgPos.y, iconSquareSize, iconSquareSize}, 0.2f, 10, iconBgColor);
    
    Vector2 iconCenter = {iconBgPos.x + iconSquareSize / 2.0f, iconBgPos.y + iconSquareSize / 2.0f};
    float iconFitSize = 28.0f; 
    Rectangle sourceRect = {0, 0, (float)iconTex.width, (float)iconTex.height};
    Rectangle destRect = {iconCenter.x - iconFitSize / 2.0f, iconCenter.y - iconFitSize / 2.0f, iconFitSize, iconFitSize};

    DrawTexturePro(iconTex, sourceRect, destRect, {0, 0}, 0.0f, palette.primaryAcc);

    Vector2 textPos = {drawnPos.x + marginX, drawnPos.y + marginY + iconSquareSize + textYOffset};
    
    // 3. Sử dụng biến textColor ở đây thay vì cố định palette.textPrimary
    DrawTextEx(font, title.c_str(), textPos, 28.0f, 1.5f, textColor); 
}

bool Topic::getClick() {
    Vector2 mousePos = GetMousePosition();
    if (CheckCollisionPointRec(mousePos, this->bounds) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        return true;
    }
    return false;
}