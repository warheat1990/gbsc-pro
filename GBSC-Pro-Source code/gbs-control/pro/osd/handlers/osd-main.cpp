// ====================================================================================
// osd-main.cpp
// TV OSD Handlers for Main Menu and Cursor
// ====================================================================================

#include "../osd-core.h"

// ====================================================================================
// OSD Initialization
// ====================================================================================

// Fill background once when menu opens (called before any menu handler)
void handle_OSD_Init(void)
{
    OSD_fillBackground();
}

// ====================================================================================
// Main Menu Handlers
// ====================================================================================

void handle_MainMenu_Page1(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(false, '1', true);
    OSD_writeStringAtRow(1, 1, "Input");
    OSD_writeCharAtRow(1, 0xFF, arrow_right_icon, (selectedMenuLine == 1) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
    OSD_writeStringAtRow(2, 1, "Output Resolution");
    OSD_writeCharAtRow(2, 0xFF, arrow_right_icon, (selectedMenuLine == 2) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
    OSD_writeStringAtRow(3, 1, "Screen Settings");
    OSD_writeCharAtRow(3, 0xFF, arrow_right_icon, (selectedMenuLine == 3) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
}

void handle_MainMenu_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '2', false);
    OSD_writeStringAtRow(1, 1, "System Settings");
    OSD_writeCharAtRow(1, 0xFF, arrow_right_icon, (selectedMenuLine == 1) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
    OSD_writeStringAtRow(2, 1, "Picture Settings");
    OSD_writeCharAtRow(2, 0xFF, arrow_right_icon, (selectedMenuLine == 2) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
    OSD_writeStringAtRow(3, 1, "Reset Settings");
    OSD_writeCharAtRow(3, 0xFF, arrow_right_icon, (selectedMenuLine == 3) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
}

// ====================================================================================
// Cursor Highlight Handlers
// ====================================================================================

void handle_HighlightRow1(void) { highlightRow(1); }
void handle_HighlightRow2(void) { highlightRow(2); }
void handle_HighlightRow3(void) { highlightRow(3); }
