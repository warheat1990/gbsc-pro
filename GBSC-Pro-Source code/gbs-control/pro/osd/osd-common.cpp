// ====================================================================================
// osd-common.cpp
// Shared helper functions for TV OSD rendering
// ====================================================================================

#include "osd-common.h"

// ====================================================================================
// Helper Functions
// ====================================================================================

// Display 9-character profile name at positions P15-P23
void displayProfileName(uint8_t row, uint8_t color)
{
    for (uint8_t i = 0; i < 9; ++i) {
        OSD_writeCharAtRow(row, profileChars[i], 15 + i, color);
    }
}

// Get color for menu row (1-based)
uint8_t OSD_getMenuLineColor(uint8_t row) {
    if (row < 1 || row > OSD_MAX_MENU_ROWS) return OSD_TEXT_NORMAL;
    return menuLineColors[row - 1];
}

// Set menu line colors based on selection (scalable to OSD_MAX_MENU_ROWS)
void OSD_setMenuLineColors(uint8_t selectedLine) {
    for (uint8_t i = 0; i < OSD_MAX_MENU_ROWS; i++) {
        menuLineColors[i] = (i + 1 == selectedLine) ? OSD_TEXT_SELECTED : OSD_TEXT_NORMAL;
    }
}

// Set menu line colors with custom color for a specific line
void OSD_setMenuLineColorsCustom(uint8_t selectedLine, uint8_t customRow, uint8_t customColor) {
    OSD_setMenuLineColors(selectedLine);
    if (customRow >= 1 && customRow <= OSD_MAX_MENU_ROWS) {
        if (customRow == selectedLine && customColor != OSD_TEXT_DISABLED) {
            return;
        }
        menuLineColors[customRow - 1] = customColor;
    }
}

// Write page navigation icons at position P27
void OSD_writePageIcons(bool showUp, uint8_t pageChar, bool showDown)
{
    if (showUp)
        OSD_writeCharAtRow(1, icon5, 27, OSD_ICON_PAGE);
    OSD_writeCharAtRow(2, pageChar, 27, OSD_ICON_PAGE);
    if (showDown)
        OSD_writeCharAtRow(3, icon6, 27, OSD_ICON_PAGE);
}

// Unified row highlight function
void highlightRow(uint8_t row)
{
    OSD_fillBackground();
    for (uint8_t r = 1; r <= 3; r++) {
        OSD_writeCharAtRow(r, icon4, 0, (r == row) ? OSD_CURSOR_ACTIVE : OSD_BACKGROUND);
    }
    selectedMenuLine = row;
}

// Generate "profile-N" name directly into profileChars[]
void setProfileName(uint8_t index) {
    profileChars[0] = 'p';
    profileChars[1] = 'r';
    profileChars[2] = 'o';
    profileChars[3] = 'f';
    profileChars[4] = 'i';
    profileChars[5] = 'l';
    profileChars[6] = 'e';
    if (index < 10) {
        profileChars[7] = 0x3E;  // '-'
        profileChars[8] = '0' + index;
    } else {
        profileChars[7] = '0' + (index / 10);
        profileChars[8] = '0' + (index % 10);
    }
}
