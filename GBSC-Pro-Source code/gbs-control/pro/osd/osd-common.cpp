// ====================================================================================
// osd-common.cpp
// Shared helper functions for TV OSD rendering
// ====================================================================================

#include "osd-common.h"

// ====================================================================================
// Helper Functions
// ====================================================================================

// Display "profile-N" at positions P15-P23 (9 characters)
void displayProfileName(uint8_t row, uint8_t index, uint8_t color)
{
    OSD_writeStringAtRow(row, 15, "profile", color);
    if (index < 10) {
        OSD_writeCharAtRow(row, 22, '-', color);
        OSD_writeCharAtRow(row, 23, '0' + index, color);
    } else {
        OSD_writeCharAtRow(row, 22, '0' + (index / 10), color);
        OSD_writeCharAtRow(row, 23, '0' + (index % 10), color);
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
        OSD_writeCharAtRow(1, 27, arrow_up_icon, OSD_ICON_PAGE);
    OSD_writeCharAtRow(2, 27, pageChar, OSD_ICON_PAGE);
    if (showDown)
        OSD_writeCharAtRow(3, 27, arrow_down_icon, OSD_ICON_PAGE);
}

// Unified row highlight function
void highlightRow(uint8_t row)
{
    OSD_fillBackground();
    for (uint8_t r = 1; r <= 3; r++) {
        OSD_writeCharAtRow(r, 0, arrow_right_icon, (r == row) ? OSD_CURSOR_ACTIVE : OSD_BACKGROUND);
    }
    selectedMenuLine = row;
}
