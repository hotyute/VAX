#pragma once
#include "ui_common.h"
#include <string> // For fontKey

struct Style {
    Color foregroundColor = { 0.805f, 0.805f, 0.805f, 1.0f };
    Color backgroundColor = { 0.447f, 0.447f, 0.447f, 1.0f };
    Color borderColor = { 0.227f, 0.227f, 0.227f, 1.0f };

    std::string fontKey; // Key to retrieve FontData from FontManager
    // e.g., "DefaultLabel", "TopButtonFont_Bold_12"

    Padding padding;
    Margin margin;
    float borderWidth = 1.0f;
    bool hasBorder = true;

    static Style DefaultButton();
    static Style DefaultLabel();
    static Style DefaultInputField();
    static Style DefaultWindow();
    static Style DefaultDisplayBox();
};