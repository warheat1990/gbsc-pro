// ====================================================================================
// osd-color.cpp
// TV OSD Handlers for Color Settings (Picture Settings menu)
// ====================================================================================

#include "osd-common.h"

// ====================================================================================
// Color Settings Handlers
// ====================================================================================

// Page 1: R, G, B
void handle_ColorSettings_Page1(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(false, '1', true);
    OSD_writeStringAtRow(1, 1, "R", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "G", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "B", OSD_getMenuLineColor(3));
}

void handle_ColorSettings_Page1_Values(void)
{
    OSD_drawDashRange(1, 5, 22);  // Row 1: P5-P22
    OSD_drawDashRange(2, 5, 22);  // Row 2: P5-P22
    OSD_drawDashRange(3, 5, 22);  // Row 3: P5-P22
    // Display R, G, B values at P23-P25 on each row
    OSD_displayNumber3DigitAtRow(1, R_VAL, 25, 24, 23, OSD_TEXT_NORMAL);
    OSD_displayNumber3DigitAtRow(2, G_VAL, 25, 24, 23, OSD_TEXT_NORMAL);
    OSD_displayNumber3DigitAtRow(3, B_VAL, 25, 24, 23, OSD_TEXT_NORMAL);
}

// Page 2: ADC gain, Scanlines, Line filter
void handle_ColorSettings_Page2(void)
{
    // Line 2 (Scanlines) disabled when not allowed
    uint8_t line2Color = areScanLinesAllowed() ? OSD_TEXT_NORMAL : OSD_TEXT_DISABLED;
    OSD_setMenuLineColorsCustom(selectedMenuLine, 2, line2Color);
    OSD_writePageIcons(true, '2', true);
    OSD_writeStringAtRow(1, 1, "ADC gain", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "Scanlines", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "Line filter", OSD_getMenuLineColor(3));
}

void handle_ColorSettings_Page2_Values(void)
{
    OSD_drawDashRange(1, 9, 18);   // Row 1: pos 9-18
    OSD_writeCharAtRow(1, 22, '-', OSD_TEXT_NORMAL);

    // Only show dashes for scanlines if they are available
    if (areScanLinesAllowed()) {
        OSD_drawDashRange(2, 10, 19);  // Row 2: pos 10-19
        OSD_writeCharAtRow(2, 22, '-', OSD_TEXT_NORMAL);
        OSD_writeOnOff(2, uopt->wantScanlines);
        // Display scanline strength (0x00-0x50 → 00-05)
        osdDisplayValue = uopt->scanlineStrength;
        OSD_writeCharAtRow(2, 21, '0', OSD_TEXT_NORMAL);
        OSD_writeCharAtRow(2, 20, '0' + (osdDisplayValue >> 4), OSD_TEXT_NORMAL);
    }

    OSD_drawDashRange(3, 12, 22);  // Row 3: pos 12-22
    OSD_writeOnOff(3, uopt->wantVdsLineFilter);
    osdDisplayValue = 255 - GBS::ADC_RGCTRL::read();  // Inverted value
    OSD_displayNumber3DigitAtRow(1, osdDisplayValue, 21, 20, 19, OSD_TEXT_NORMAL);
    OSD_writeOnOff(1, uopt->enableAutoGain != 0);
}

// Page 3: Sharpness, Peaking, Step response
void handle_ColorSettings_Page3(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '3', true);
    OSD_writeStringAtRow(1, 1, "Sharpness", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "Peaking", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "Step response", OSD_getMenuLineColor(3));
}

void handle_ColorSettings_Page3_Values(void)
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

// Page 4: Y Gain, Color, Default Color
void handle_ColorSettings_Page4(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '4', false);
    OSD_writeStringAtRow(1, 1, "Y Gain", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "Color", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "Default Color", OSD_getMenuLineColor(3));
}

void handle_ColorSettings_Page4_Values(void)
{
    OSD_displayNumber3DigitAtRow(1, GBS::VDS_Y_GAIN::read(), 25, 24, 23, OSD_TEXT_NORMAL);
    // Show both UCOS and VCOS values in format U:xxx-V:xxx (positions 15-25)
    OSD_writeStringAtRow(2, 15, "U:");
    OSD_displayNumber3DigitAtRow(2, GBS::VDS_UCOS_GAIN::read(), 19, 18, 17, OSD_TEXT_NORMAL);
    OSD_writeStringAtRow(2, 20, "-V:");
    OSD_displayNumber3DigitAtRow(2, GBS::VDS_VCOS_GAIN::read(), 25, 24, 23, OSD_TEXT_NORMAL);
}
