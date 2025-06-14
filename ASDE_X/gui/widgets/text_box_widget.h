#pragma once
#include "widget.h"
#include <vector>
#include <string>
#include <deque>

struct StyledTextLine {
    std::string text;
    Color color;
    // Optional: Store pre-wrapped segments if text is long
    std::vector<std::string> wrappedSegments;
    bool needsWrapping = true;

    StyledTextLine(std::string t, Color c) : text(std::move(t)), color(c) {}
};

class TextBoxWidget : public Widget {
public:
    TextBoxWidget();

    void AddLine(const std::string& text, const Color& color);
    void AddLineFront(const std::string& text, const Color& color);
    void Clear();

    void SetMaxHistory(int max);
    void SetReadOnly(bool ro); // For potential future editing

    void ScrollToLine(int lineIndex); // Scroll to make a specific original line visible
    void ScrollDelta(int deltaPixels); // Scroll by pixel amount

    // For DisplayBox features
    bool pruneTop = false; // If true, remove from top when history is full
    int numBlocksToDisplay = 0; // If > 0, try to fit this many "original" lines via scrolling/wrapping
                                // This is complex with wrapping. Simpler to scroll by lines/pixels.

    void Draw(UIRenderer& renderer, float inheritedAlpha = 1.0) override;
    Size Measure() override;
    void Arrange(const Rect& finalRect) override; // Will trigger re-wrapping
    void HandleEvent(UIEvent& event) override;

    int GetLineHeight() const; // Declaration

    std::vector<std::string> GetAllLogicalLinesText() const;
    std::string GetConcatenatedText(const std::string& delimiter = "\n") const;

private:
    std::deque<StyledTextLine> logicalLines; // Original lines added
    std::vector<std::pair<const StyledTextLine*, int>> displayLines; // Pointers to logical lines and segment index

    int maxHistory = 100;
    bool readOnly = true;
    int scrollOffsetY = 0; // Pixel offset for vertical scrolling
    int contentHeight = 0; // Total pixel height of all wrapped lines
    int calculatedLineHeight = 16;
    int lineSpacing = 2;

    void CalculateLineHeight(UIRenderer* renderer);
    void ReWrapAllLines(UIRenderer* renderer, int availableWidth);
    void UpdateDisplayLines(); // Populates displayLines from logicalLines based on wrapping
    void EnsureScrollBounds();
};