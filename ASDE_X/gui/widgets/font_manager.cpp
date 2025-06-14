#include "font_manager.h"
#include <gl/GL.h> // For glGenLists, glDeleteLists

// Assuming your s2ws is in tools.h, adjust if necessary
#include "../../tools.h" // For s2ws if needed for keys, though wstring fontName is better

FontManager& FontManager::Instance() {
    static FontManager instance;
    return instance;
}

FontManager::~FontManager() {
    Cleanup();
}

void FontManager::Initialize(HDC hdc) {
    m_hdc = hdc;
    // It's important that this HDC has an active GL rendering context
    // when BuildFontInternal (which calls wglUseFontBitmaps) is used.
}

void FontManager::Cleanup() {
    for (auto const& [key, val] : fontCache_) {
        if (val) {
            if (val->hFont) {
                DeleteObject(val->hFont);
            }
            if (val->glListBase != 0) {
                glDeleteLists(val->glListBase, 96); // Assuming 96 characters from your old BuildFont
            }
        }
    }
    fontCache_.clear();
    m_hdc = NULL;
}

bool FontManager::BuildFontInternal(const std::wstring& fontNameLPCWSTR, int height, bool bold, bool italic,
                                   unsigned int& outListBase, HFONT& outHFont) {
    if (!m_hdc) return false; // Must be initialized

    HFONT font;
    unsigned int listBase = glGenLists(96);
    if (listBase == 0) { // Failed to generate list
        return false;
    }

    font = CreateFontW( // Use CreateFontW for LPCWSTR
        height,              // Height Of Font
        0,                   // Width Of Font
        0,                   // Angle Of Escapement
        0,                   // Orientation Angle
        bold ? FW_BOLD : FW_NORMAL, // Font Weight (FW_NORMAL not FW_REGULAR)
        italic ? TRUE : FALSE, // Italic
        FALSE,               // Underline
        FALSE,               // Strikeout
        DEFAULT_CHARSET,     // Character Set Identifier
        OUT_TT_PRECIS,       // Output Precision (TT_PRECIS for TrueType quality)
        CLIP_DEFAULT_PRECIS, // Clipping Precision
        ANTIALIASED_QUALITY, // Output Quality (ANTIALIASED_QUALITY for smoother fonts)
        DEFAULT_PITCH | FF_DONTCARE, // Family And Pitch
        fontNameLPCWSTR.c_str());    // Font Name

    if (!font) {
        glDeleteLists(listBase, 96);
        return false;
    }

    HFONT oldFont = (HFONT)SelectObject(m_hdc, font);
    BOOL success = wglUseFontBitmapsW(m_hdc, 32, 96, listBase); // Use W version for Unicode
    SelectObject(m_hdc, oldFont); // Restore old font

    if (!success) {
        DeleteObject(font);
        glDeleteLists(listBase, 96);
        return false;
    }

    outHFont = font;
    outListBase = listBase;
    return true;
}

bool FontManager::LoadFont(const std::string& fontKey, const std::wstring& fontName, int height, bool bold, bool italic) {
    if (fontCache_.count(fontKey)) {
        return true; // Already loaded
    }
    if (!m_hdc) {
        // Log error: FontManager not initialized
        return false;
    }

    auto fontData = std::make_unique<FontData>();
    if (BuildFontInternal(fontName, height, bold, italic, fontData->glListBase, fontData->hFont)) {
        fontCache_[fontKey] = std::move(fontData);
        return true;
    }
    return false;
}

FontData* FontManager::GetFontData(const std::string& fontKey) {
    auto it = fontCache_.find(fontKey);
    if (it != fontCache_.end()) {
        return it->second.get();
    }
    // Optionally, log a warning if font key not found
    return nullptr;
}

HFONT FontManager::GetHFONT(const std::string& fontKey) {
    FontData* fd = GetFontData(fontKey);
    return fd ? fd->hFont : NULL;
}

// This is less ideal as it requires iterating the cache.
// Better to store HFONT -> glListBase mapping if this is frequently needed.
// For now, it's just for very specific transitional cases.
unsigned int FontManager::GetListBase(HFONT hFont) {
    for (auto const& [key, dataPtr] : fontCache_) {
        if (dataPtr && dataPtr->hFont == hFont) {
            return dataPtr->glListBase;
        }
    }
    return 0; // Not found or not managed by FontManager
}

Size FontManager::GetTextExtent(const std::string& text, const std::string& fontKey) {
    if (text.empty() || fontKey.empty()) return { 0, 0 };
    std::wstring wtext = s2ws(text); // Convert once
    return GetTextExtent(wtext, fontKey);
}

Size FontManager::GetTextExtent(const std::wstring& wtext, const std::string& fontKey) {
    if (wtext.empty() || fontKey.empty()) return { 0, 0 };

    FontData* fd = GetFontData(fontKey);
    SIZE winSize = { 0, 0 };

    if (fd && fd->hFont && m_hdc) { // Use the FontManager's stored HDC
        HFONT oldFont = (HFONT)SelectObject(m_hdc, fd->hFont);
        GetTextExtentPoint32W(m_hdc, wtext.c_str(), static_cast<int>(wtext.length()), &winSize);
        SelectObject(m_hdc, oldFont);
        return { static_cast<int>(winSize.cx), static_cast<int>(winSize.cy) };
    }
    else {
        // Fallback if FontManager not initialized, font not loaded, or text empty
        // This fallback is very rough.
        return { static_cast<int>(wtext.length() * 8), 16 }; // Estimate
    }
}