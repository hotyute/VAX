#pragma once
#include "widget.h" // Base class
#include <vector>
#include <string>
#include <functional>

class ListBoxWidget : public Widget {
public:
    std::vector<std::string> items;
    int selectedIndex = -1;
    int hoverIndex = -1;
    int itemHeight = 20; // Or calculate from font

    int scrollOffsetY = 0; // Pixel offset for vertical scrolling

    std::function<void(ListBoxWidget*, int, const std::string&)> OnItemSelected; // Index, Text
    std::function<void(ListBoxWidget*)> OnDoubleClickItem; // (optional)

    ListBoxWidget();

    void AddItem(const std::string& item);
    void ClearItems();

    void Draw(UIRenderer& renderer, float inheritedAlpha = 1.0) override;
    Size Measure() override;
    void HandleEvent(UIEvent& event) override;

    std::function<void(ListBoxWidget*, int, const std::string&, Point)> OnRightClickItem;

    void SetScrollOffset(int newOffset);
    int GetContentHeight() const; // To inform scrollbar
};