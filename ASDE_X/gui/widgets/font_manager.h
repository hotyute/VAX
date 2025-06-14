#pragma once
#include <windows.h> // For HFONT, HDC
#include <string>
#include <unordered_map>
#include <memory> // For std::unique_ptr
#include <vector> // For BuildFontInternal if it needs to return multiple bases (not typical for this use)

#include "ui_renderer.h"

// Your existing BuildFont function signature from renderer.h:
// void BuildFont(LPCWSTR, int, bool, unsigned int*, HFONT*);
// We'll adapt this.

struct FontData {
    HFONT hFont = NULL;
    unsigned int glListBase = 0; // For glPrint compatibility
    // You could add cached metrics here if desired, e.g., avg char width, line height
};

class FontManager {
public:
    static FontManager& Instance();

    // Call once, from the OpenGL thread after context is current
    void Initialize(HDC hdc);
    void Cleanup(); // Call before GL context is destroyed

    // Get FontData (contains HFONT and list base)
    // Widgets/Styles will store the string key.
    FontData* GetFontData(const std::string& fontKey);

    // Load a specific font and cache it with a given key.
    // Returns true on success.
    bool LoadFont(const std::string& fontKey, const std::wstring& fontName, int height, bool bold, bool italic = false);

    // Helper to get HFONT directly if only that is needed by some legacy code
    HFONT GetHFONT(const std::string& fontKey);

    unsigned int GetListBase(HFONT hFont);

    Size GetTextExtent(const std::string& text, const std::string& fontKey);
    Size GetTextExtent(const std::wstring& wtext, const std::string& fontKey);


private:
    FontManager() = default;
    ~FontManager(); // Manages cleanup of HFONTs and GL lists
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;

    // Internal version of your BuildFont
    bool BuildFontInternal(const std::wstring& fontName, int height, bool bold, bool italic,
        unsigned int& outListBase, HFONT& outHFont);

    friend class LabelWidget; // Allow specific widgets access if truly needed (still a bit of a smell)
    friend class ButtonWidget; // This is getting messy, shows the need for a better pattern or public metrics
    friend class TextBoxWidget;
    friend class RadioButtonWidget;

    HDC m_hdc = NULL; // DC valid for wglUseFontBitmaps
    std::unordered_map<std::string, std::unique_ptr<FontData>> fontCache_;
};