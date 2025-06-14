#pragma once
#include "ui_renderer.h"
#include <windows.h> // For HDC
#include <gl/GL.h>   // For OpenGL types

// Forward declare from your existing code if needed
extern void glPrint(const char* fmt, unsigned int* base, ...); // From your renderer.h
extern unsigned int labelBase; // Example base, you'll need to manage this better
extern std::wstring s2ws(const std::string& s); // From your tools.h


class OpenGLUIRenderer : public UIRenderer {
public:
    OpenGLUIRenderer(HDC hdc); // Takes the HDC from your main application

    void BeginFrame(int screenWidth, int screenHeight) override;
    void EndFrame() override;

    void DrawRect(const Rect& r, const Color& c, bool filled = true, float lineWidth = 1.0f) override;
    void DrawText(const std::string& text, const Point& pos, const std::string& fontKey, const Color& c) override;
    Size GetTextExtent(const std::string& text, const std::string& fontKey) override;
    void DrawLine(const Point& p1, const Point& p2, const Color& c, float lineWidth = 1.0f) override;

    void PushClipRect(const Rect& r) override;
    void PopClipRect() override;

private:
    HDC hDC;
    int currentScreenWidth, currentScreenHeight;
    std::vector<Rect> clipRectStack;
};