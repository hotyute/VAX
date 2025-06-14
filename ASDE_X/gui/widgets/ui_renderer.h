#pragma once
#include "ui_common.h"
#include <string>

class UIRenderer {
public:
    virtual ~UIRenderer() = default;

    virtual void BeginFrame(int screenWidth, int screenHeight) = 0;
    virtual void EndFrame() = 0;

    virtual void DrawRect(const Rect& r, const Color& c, bool filled = true, float lineWidth = 1.0f) = 0;
    // Takes fontKey (string) now
    virtual void DrawText(const std::string& text, const Point& pos, const std::string& fontKey, const Color& c) = 0;
    virtual Size GetTextExtent(const std::string& text, const std::string& fontKey) = 0;
    virtual void DrawLine(const Point& p1, const Point& p2, const Color& c, float lineWidth = 1.0f) = 0;

    virtual void PushClipRect(const Rect& r) = 0;
    virtual void PopClipRect() = 0;
};