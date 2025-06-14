// --- START OF FILE opengl_ui_renderer.cpp ---

#define NOMINMAX
#include <windows.h>

#include "opengl_ui_renderer.h"
#include <gl/GLU.h>   // For gluOrtho2D
#include "../../renderer.h" // For existing glPrint and font bases
#include "font_manager.h" // Include FontManager
#include "../../tools.h"    // For s2ws
#include <algorithm> // For std::max/min

// If you have these in a header that opengl_ui_renderer.h includes, fine.
// Otherwise, you might need to re-declare or include the specific header that has them.
extern unsigned int labelBase;


OpenGLUIRenderer::OpenGLUIRenderer(HDC hdc_param) : hDC(hdc_param) {}


void OpenGLUIRenderer::BeginFrame(int screenWidth, int screenHeight) {
    currentScreenWidth = screenWidth;
    currentScreenHeight = screenHeight;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, screenWidth, 0.0, screenHeight); // Y is 0 at bottom

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_SCISSOR_TEST);
    clipRectStack.clear();
    glScissor(0, 0, screenWidth, screenHeight);
}

void OpenGLUIRenderer::EndFrame() {
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glDisable(GL_SCISSOR_TEST);
}

void OpenGLUIRenderer::DrawRect(const Rect& r, const Color& c, bool filled, float lineWidth) {
    glColor4f(c.r, c.g, c.b, c.a);
    if (filled) {
        glBegin(GL_QUADS);
        glVertex2i(r.x, currentScreenHeight - r.y);
        glVertex2i(r.x + r.width, currentScreenHeight - r.y);
        glVertex2i(r.x + r.width, currentScreenHeight - (r.y + r.height));
        glVertex2i(r.x, currentScreenHeight - (r.y + r.height));
        glEnd();
    }
    else {
        glLineWidth(lineWidth);
        glBegin(GL_LINE_LOOP);
        glVertex2i(r.x, currentScreenHeight - r.y);
        glVertex2i(r.x + r.width, currentScreenHeight - r.y);
        glVertex2i(r.x + r.width, currentScreenHeight - (r.y + r.height));
        glVertex2i(r.x, currentScreenHeight - (r.y + r.height));
        glEnd();
    }
}

void OpenGLUIRenderer::DrawText(const std::string& text, const Point& pos, const std::string& fontKey, const Color& c) {
    if (text.empty() || fontKey.empty()) return;

    FontData* fd = FontManager::Instance().GetFontData(fontKey);
    if (!fd || !fd->hFont || fd->glListBase == 0) {
        return; // Font not found or not ready
    }

    // --- VERTICAL CENTERING FIX IN RENDERER ---
    // The `pos.y` coordinate passed from the widget already represents the desired top Y coordinate of the text.
    // To correctly position it in OpenGL's bottom-up coordinate system, we need the font's ascent metric.
    // GetTextExtent gives the full height, but for raster positioning, we need the distance from the baseline to the top.

    TEXTMETRIC tm;
    HFONT oldFont = (HFONT)SelectObject(hDC, fd->hFont);
    GetTextMetrics(hDC, &tm);
    SelectObject(hDC, oldFont);
    int ascent = tm.tmAscent;

    // Correct Y position for glRasterPos:
    // It's the screen height minus the desired top Y, minus the font's ascent.
    // This places the baseline of the font such that its top aligns with `pos.y`.
    int glY = currentScreenHeight - (pos.y + ascent);

    // Set color and position
    glColor4f(c.r, c.g, c.b, c.a);
    glRasterPos2i(pos.x, glY);
    // --- END OF FIX ---

    // Escape '%' characters for glPrint's format string
    std::string escapedText = text;
    size_t p = escapedText.find('%');
    while (p != std::string::npos) {
        escapedText.insert(p, "%");
        p = escapedText.find('%', p + 2);
    }

    // Draw the text using the display list
    glPrint(escapedText.c_str(), &fd->glListBase);
}

Size OpenGLUIRenderer::GetTextExtent(const std::string& text, const std::string& fontKey) {
    if (text.empty() || fontKey.empty()) return { 0, 0 };

    FontData* fd = FontManager::Instance().GetFontData(fontKey);
    if (!fd || !fd->hFont || !hDC) {
        return { static_cast<int>(text.length()) * 8, 16 };
    }

    HFONT oldFont = (HFONT)SelectObject(hDC, fd->hFont);
    SIZE winSize;
    std::wstring wtext = s2ws(text);
    GetTextExtentPoint32W(hDC, wtext.c_str(), static_cast<int>(wtext.length()), &winSize);
    SelectObject(hDC, oldFont);
    return { static_cast<int>(winSize.cx), static_cast<int>(winSize.cy) };
}

void OpenGLUIRenderer::DrawLine(const Point& p1, const Point& p2, const Color& c, float lineWidth) {
    glColor4f(c.r, c.g, c.b, c.a);
    glLineWidth(lineWidth);
    glBegin(GL_LINES);
    glVertex2i(p1.x, currentScreenHeight - p1.y);
    glVertex2i(p2.x, currentScreenHeight - p2.y);
    glEnd();
}

void OpenGLUIRenderer::PushClipRect(const Rect& r) {
    GLint currentScissor[4];
    glGetIntegerv(GL_SCISSOR_BOX, currentScissor);
    clipRectStack.push_back({ currentScissor[0], currentScreenHeight - (currentScissor[1] + currentScissor[3]), currentScissor[2], currentScissor[3] });

    Rect newGLScissor;
    newGLScissor.x = r.x;
    newGLScissor.y = currentScreenHeight - (r.y + r.height);
    newGLScissor.width = r.width;
    newGLScissor.height = r.height;

    GLint finalX = std::max(currentScissor[0], newGLScissor.x);
    GLint finalY = std::max(currentScissor[1], newGLScissor.y);
    GLint finalRight = std::min(currentScissor[0] + currentScissor[2], newGLScissor.x + newGLScissor.width);
    GLint finalTop = std::min(currentScissor[1] + currentScissor[3], newGLScissor.y + newGLScissor.height);

    GLint finalWidth = std::max(0, finalRight - finalX);
    GLint finalHeight = std::max(0, finalTop - finalY);

    glScissor(finalX, finalY, finalWidth, finalHeight);
}

void OpenGLUIRenderer::PopClipRect() {
    if (!clipRectStack.empty()) {
        Rect prevScissorRect = clipRectStack.back();
        clipRectStack.pop_back();
        // Convert back to OpenGL's bottom-left Y coordinate system for glScissor
        glScissor(prevScissorRect.x, currentScreenHeight - (prevScissorRect.y + prevScissorRect.height), prevScissorRect.width, prevScissorRect.height);
    }
    else {
        glScissor(0, 0, currentScreenWidth, currentScreenHeight);
    }
}