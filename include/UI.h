#pragma once 
#include "raylib.h"
#include <string>
#include "theme.h"

class button 
{
    public:
        bool type = 0;
        button(const char *imagePath, const char *ChangeImagePath, const char *imagePath02, const char *ChangeImagePath02, Vector2 imagePosition, double scale);
        ~button();
        void Draw(Vector2 mousePos);
        bool isPressed(Vector2 mousePos, bool mousePressed);
        void SetPosition(Vector2 newPosition)
        {
            this->position = newPosition;
        }
    private:
        Texture2D texture;
        Vector2 position;
        Texture2D ChangeTexture;
        Texture2D texture02;
        Texture2D ChangeTexture02;
};

class WriteText 
{
    private:
        Font font;
        bool isLoaded;

    public:
        WriteText();
        ~WriteText();
        bool LoadMyFont(const std::string& filePath, int fontSize);
        void Draw(const std::string& text, Vector2 position, float fontSize, float spacing, Color color);
        void Unload();
};

class Topic 
{
    public:
        Topic(const std::string& iconPath, const std::string& titleText, int w = 300, int h = 180);
        ~Topic();
        void Draw(Vector2 pos, const Palette& palette);
        bool getClick();
    private:
        Texture2D iconTex;
        Font font;

        std::string title;
        int cardWidth;
        int cardHeight;

        Rectangle bounds;
};

class TextButton 
{
    public:
        // Thêm tham số fontPath với giá trị mặc định để dễ tùy biến
        TextButton(Rectangle bounds, const std::string& text, const std::string& fontPath = "assets/fonts/static/Inter-SemiBold.ttf");
        ~TextButton();

        void Draw();
        bool isPressed();
        void SetPosition(Vector2 newPos);

    private:
        Rectangle bounds;
        std::string text;
        Font font;
        
        float hoverProgress;
        float transitionSpeed;
        
        Color normalColor;
        Color hoverColor;
        Color textColor;
};