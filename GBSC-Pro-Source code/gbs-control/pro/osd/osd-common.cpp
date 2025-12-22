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
// Also updates the cursor arrow at position 0 for each row
void OSD_setMenuLineColors(uint8_t selectedLine) {
    for (uint8_t i = 0; i < OSD_MAX_MENU_ROWS; i++) {
        uint8_t row = i + 1;
        bool isSelected = (row == selectedLine);
        menuLineColors[i] = isSelected ? OSD_TEXT_SELECTED : OSD_TEXT_NORMAL;
        // Update cursor arrow at position 0
        OSD_writeCharAtRow(row, 0, arrow_right_icon, isSelected ? OSD_CURSOR_ACTIVE : OSD_BACKGROUND);
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
    selectedMenuLine = row;
    OSD_setMenuLineColors(row);
}

// Draw dashes on a row from startPos to endPos (logical positions 0-27)
void OSD_drawDashRange(uint8_t row, uint8_t startPos, uint8_t endPos, uint8_t color) {
    for (uint8_t p = startPos; p <= endPos; p++) {
        OSD_writeCharAtRow(row, p, '-', color);
    }
}

// Write ON or OFF indicator at end of row (position 23)
void OSD_writeOnOff(uint8_t row, bool isOn, uint8_t color) {
    OSD_writeStringAtRow(row, 23, isOn ? "-ON" : "OFF", color);
}

// Highlight menu icon at position (1=top, 2=mid, 3=bottom)
void OSD_highlightIcon(uint8_t pos) {
    OSD_writeCharAtRow(1, 0, arrow_right_icon, pos == 1 ? OSD_CURSOR_ACTIVE : OSD_CURSOR_INACTIVE);
    OSD_writeCharAtRow(2, 0, arrow_right_icon, pos == 2 ? OSD_CURSOR_ACTIVE : OSD_CURSOR_INACTIVE);
    OSD_writeCharAtRow(3, 0, arrow_right_icon, pos == 3 ? OSD_CURSOR_ACTIVE : OSD_CURSOR_INACTIVE);
}

// Show 4-direction adjustment icons on TV OSD row
void OSD_showAdjustArrows(uint8_t row, uint8_t pos, uint8_t color) {
    OSD_writeCharAtRow(row, pos,     horizontal_scale_part1_icon, color);
    OSD_writeCharAtRow(row, pos + 1, vertical_scale_part1_icon, color);
    OSD_writeCharAtRow(row, pos + 2, vertical_scale_part2_icon, color);
    OSD_writeCharAtRow(row, pos + 3, horizontal_scale_part2_icon, color);
}

// Show "limit" feedback on TV OSD row, then clear (blocking)
void OSD_showLimitFeedback(uint8_t row, int iterations) {
    uint8_t logicalRow = OSD_bankToRow(row);
    for (int p = 0; p <= iterations; p++) {
        OSD_writeStringAtRow(logicalRow, 20, "limit", OSD_TEXT_DISABLED);
        OSD_writeCharAtRow(logicalRow, 25, enable_icon, OSD_TEXT_DISABLED);
    }
    OSD_writeStringAtRow(logicalRow, 20, "limit", OSD_BACKGROUND);
    OSD_writeCharAtRow(logicalRow, 25, enable_icon, OSD_BACKGROUND);
}

// Show "OK" feedback on TV OSD row, then clear (blocking)
void OSD_showOkFeedback(uint8_t row, int iterations) {
    uint8_t logicalRow = OSD_bankToRow(row);
    for (int p = 0; p <= iterations; p++) {
        OSD_writeStringAtRow(logicalRow, 25, "OK", OSD_TEXT_DISABLED);
    }
    OSD_writeStringAtRow(logicalRow, 25, "OK", OSD_BACKGROUND);
}

// Show "saving" feedback on TV OSD row, then clear (blocking)
void OSD_showSavingFeedback(uint8_t row, uint8_t startPos, int iterations) {
    uint8_t logicalRow = OSD_bankToRow(row);
    for (int p = 0; p <= iterations; p++) {
        OSD_writeStringAtRow(logicalRow, startPos, "saving", OSD_TEXT_DISABLED);
    }
    OSD_writeStringAtRow(logicalRow, startPos, "saving", OSD_BACKGROUND);
}
