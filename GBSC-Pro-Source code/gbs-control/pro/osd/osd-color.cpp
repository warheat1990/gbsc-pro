// ====================================================================================
// osd-color.cpp
// TV OSD Handlers for Color Settings
// ====================================================================================

#include "osd-common.h"

// ====================================================================================
// Color Settings Handlers
// ====================================================================================

void handle_ColorSettings_Page1(void)
{
    // Line 2 (Scanlines) disabled when not allowed
    uint8_t line2Color = areScanLinesAllowed() ? OSD_TEXT_NORMAL : OSD_TEXT_DISABLED;
    OSD_setMenuLineColorsCustom(selectedMenuLine, 2, line2Color);
    OSD_writePageIcons(true, '2', true);
    OSD_writeStringAtRow(1, 1, "ADC gain", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "Scanlines", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "Line filter", OSD_getMenuLineColor(3));
}

void handle_ColorSettings_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '3', true);
    OSD_writeStringAtRow(1, 1, "Sharpness", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "Peaking", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "Step response", OSD_getMenuLineColor(3));
}

void handle_ColorSettings_Page3(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '4', false);
    OSD_writeStringAtRow(1, 1, "Default Color", OSD_getMenuLineColor(1));
}

void handle_ColorSettings_RGB_Labels(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(false, '1', true);
    OSD_writeStringAtRow(1, 1, "R", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "G", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "B", OSD_getMenuLineColor(3));
}

void handle_ColorSettings_Page1_Values(void)
{
    OSD_drawDashRange(1, 9, 18);   // Row 1: pos 9-18
    OSD_writeCharAtRow(1, 0x3E, 22, OSD_TEXT_NORMAL);

    // Only show dashes for scanlines if they are available
    if (areScanLinesAllowed()) {
        OSD_drawDashRange(2, 10, 19);  // Row 2: pos 10-19
        OSD_writeCharAtRow(2, 0x3E, 22, OSD_TEXT_NORMAL);
        OSD_writeOnOff(2, uopt->wantScanlines);
        // Display scanline strength (0x00-0x50 → 00-05)
        osdDisplayValue = uopt->scanlineStrength;
        OSD_writeCharAtRow(2, '0', 21, OSD_TEXT_NORMAL);
        OSD_writeCharAtRow(2, '0' + (osdDisplayValue >> 4), 20, OSD_TEXT_NORMAL);
    }

    OSD_drawDashRange(3, 12, 22);  // Row 3: pos 12-22
    OSD_writeOnOff(3, uopt->wantVdsLineFilter);
    osdDisplayValue = 255 - GBS::ADC_RGCTRL::read();  // Inverted value
    OSD_displayNumber3DigitAtRow(1, osdDisplayValue, 21, 20, 19, OSD_TEXT_NORMAL);
    OSD_writeOnOff(1, uopt->enableAutoGain != 0);
}

void handle_ColorSettings_Page2_Values(void)
{
    OSD_drawDashRange(1, 10, 22);  // Row 1: P10-P22
    OSD_drawDashRange(2, 8, 19);   // Row 2: P8-P19
    if (!isPeakingLocked()) {
        OSD_drawDashRange(2, 20, 22);  // Row 2: P20-P22 (when not locked)
    }

    OSD_drawDashRange(3, 14, 22);  // Row 3: P14-P22
    OSD_writeOnOff(1, GBS::VDS_PK_LB_GAIN::read() != 0x16);

    if (isPeakingLocked()) {
        // Locked state - overwrite dashes and ON/OFF with LOCKED
        OSD_writeStringAtRow(2, 20, "LOCKED");
    } else {
        OSD_writeOnOff(2, uopt->wantPeaking != 0);
    }

    OSD_writeOnOff(3, uopt->wantStepResponse);
}

void handle_ColorSettings_RGB_Values(void)
{
    OSD_drawDashRange(1, 5, 22);  // Row 1: P5-P22
    OSD_drawDashRange(2, 5, 22);  // Row 2: P5-P22
    OSD_drawDashRange(3, 5, 22);  // Row 3: P5-P22
    // Display R, G, B values at P23-P25 on each row
    OSD_displayNumber3DigitAtRow(1, R_VAL, 25, 24, 23, OSD_TEXT_NORMAL);
    OSD_displayNumber3DigitAtRow(2, G_VAL, 25, 24, 23, OSD_TEXT_NORMAL);
    OSD_displayNumber3DigitAtRow(3, B_VAL, 25, 24, 23, OSD_TEXT_NORMAL);
}
