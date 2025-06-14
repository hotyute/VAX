#pragma once
#include <string>

// Define all the font keys your application will use.
// These names are up to you, but make them descriptive.
// They should correspond to the fonts you were previously creating with BuildFont.

// From your renderer.h:
// extern HFONT callSignFont, topBtnFont, confFont, legendFont, titleFont, labelFont, errorFont;
// extern unsigned int callSignBase, topButtonBase, confBase, legendBase, titleBase, labelBase, errorBase;

const std::string FONT_KEY_LEGEND = "LegendFont";      // Was legendFont, legendBase
const std::string FONT_KEY_TOP_BUTTON = "TopButtonFont";   // Was topBtnFont, topButtonBase
const std::string FONT_KEY_TITLE = "TitleFont";       // Was titleFont, titleBase
const std::string FONT_KEY_CONFIG = "ConfigFont";      // Was confFont, confBase
const std::string FONT_KEY_CALLSIGN = "CallsignFont";    // Was callSignFont, callSignBase
const std::string FONT_KEY_UI_LABEL = "UILabelFont";     // Was labelFont, labelBase (renamed to avoid confusion with general labels)
const std::string FONT_KEY_ERROR = "ErrorFont";       // Was errorFont, errorBase

// You might want more specific keys if a single font name/size is used with different weights/styles
// e.g., FONT_KEY_UI_LABEL_BOLD