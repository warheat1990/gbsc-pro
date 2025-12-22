// ====================================================================================
// osd-input.cpp
// TV OSD Handlers for Input Menu and ADC Calibration
// ====================================================================================

#include "osd-common.h"

// ====================================================================================
// Input Menu Handlers
// ====================================================================================

void handle_InputMenu_Page1(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(false, '1', true);
    OSD_writeStringAtRow(1, 1, "RGBs", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "RGsB", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "VGA", OSD_getMenuLineColor(3));
}

void handle_InputMenu_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '2', false);
    OSD_writeStringAtRow(1, 1, "YPBPR", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "SV", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "AV", OSD_getMenuLineColor(3));
}

void handle_InputMenu_Page2_Values(void)
{
    bool isSV = (oled_menuItem == OLED_Input_SV);
    bool isAV = (oled_menuItem == OLED_Input_AV);

    // Clear or show "Format:" label
    if (isSV) {
        OSD_writeStringAtRow(2, 4, "Format:");
        OSD_writeStringAtRow(3, 4, "                      ");
    } else if (isAV) {
        OSD_writeStringAtRow(3, 4, "Format:");
        OSD_writeStringAtRow(2, 4, "                      ");
    } else {
        OSD_writeStringAtRow(2, 4, "                      ");
        OSD_writeStringAtRow(3, 4, "                      ");
    }

    // Display format name for SV or AV
    if (isSV) {
        // Pad format name to fixed width to overwrite previous text
        char padded[16];
        snprintf(padded, sizeof(padded), "%-15s", getVideoFormatName(SVModeOption));
        OSD_writeStringAtRow(2, 11, padded);
    } else if (isAV) {
        char padded[16];
        snprintf(padded, sizeof(padded), "%-15s", getVideoFormatName(AVModeOption));
        OSD_writeStringAtRow(3, 11, padded);
    }
}

void handle_InputInfo(void)
{
    // All rows normal (no selection highlight in info screen)
    OSD_setMenuLineColors(0);
    OSD_writeStringAtRow(1, 0, "Whether to keep the settings", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 0, "Restore in ", OSD_getMenuLineColor(2));

    // Show checkmark on selected option (Changes or Recover)
    OSD_writeCharAtRow(3, 2, arrow_right_icon, keepSettings ? OSD_TEXT_SELECTED : OSD_BACKGROUND);
    OSD_writeStringAtRow(3, 3, "Changes", OSD_getMenuLineColor(3));
    OSD_writeCharAtRow(3, 13, arrow_right_icon, keepSettings ? OSD_BACKGROUND : OSD_TEXT_SELECTED);
    OSD_writeStringAtRow(3, 0xFF, "    Recover", OSD_getMenuLineColor(3));
}

void handle_InfoDisplay_Source(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, 'I', true);
    OSD_writeStringAtRow(1, 1, "Setting", OSD_getMenuLineColor(1));
}
