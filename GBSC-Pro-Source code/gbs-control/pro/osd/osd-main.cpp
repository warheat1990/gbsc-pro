// ====================================================================================
// osd-main.cpp
// TV OSD Handlers for Main Menu and Cursor
// ====================================================================================

#include "osd-common.h"

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
    OSD_writeStringAtRow(1, 1, "1 Input", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "2 Output Resolution", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "3 Screen Settings", OSD_getMenuLineColor(3));
    // Selection arrows at end of each menu item (indicates submenu)
    OSD_writeCharAtRow(1, 0x15, 8, (selectedMenuLine == 1) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
    OSD_writeCharAtRow(2, 0x15, 20, (selectedMenuLine == 2) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
    OSD_writeCharAtRow(3, 0x15, 18, (selectedMenuLine == 3) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
}

void handle_MainMenu_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '2', false);
    OSD_writeStringAtRow(1, 1, "4 System Settings", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "5 Picture Settings", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "6 Reset Settings", OSD_getMenuLineColor(3));
    // Selection arrows at end of each menu item (indicates submenu)
    OSD_writeCharAtRow(1, 0x15, 18, (selectedMenuLine == 1) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
    OSD_writeCharAtRow(2, 0x15, 19, (selectedMenuLine == 2) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
    OSD_writeCharAtRow(3, 0x15, 17, (selectedMenuLine == 3) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
}

// ====================================================================================
// Cursor Highlight Handlers
// ====================================================================================

void handle_HighlightRow1(void) { highlightRow(1); }
void handle_HighlightRow2(void) { highlightRow(2); }
void handle_HighlightRow3(void) { highlightRow(3); }
