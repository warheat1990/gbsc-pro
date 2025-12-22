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
    OSD_writeStringAtRow(1, 1, "Red");
    OSD_drawDashRange(1, 4, 22);
    OSD_writeStringAtRow(2, 1, "Green");
    OSD_drawDashRange(2, 6, 22);
    OSD_writeStringAtRow(3, 1, "Blue");
    OSD_drawDashRange(3, 5, 22);
}

void handle_ColorSettings_Page1_Values(void)
{
    // Display R, G, B values at P23-P25 on each row
    OSD_displayNumber3DigitAtRow(1, R_VAL, 25, 24, 23);
    OSD_displayNumber3DigitAtRow(2, G_VAL, 25, 24, 23);
    OSD_displayNumber3DigitAtRow(3, B_VAL, 25, 24, 23);
}

// Page 2: ADC gain, Scanlines, Line filter
void handle_ColorSettings_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '2', true);
    OSD_writeStringAtRow(1, 1, "ADC gain");
    OSD_drawDashRange(1, 9, 18);
    OSD_drawDashRange(1, 22, 22);
    OSD_writeStringAtRow(2, 1, "Scanlines", areScanLinesAllowed() ? OSD_TEXT_NORMAL : OSD_TEXT_DISABLED);
    OSD_drawDashRange(2, 10, 19, areScanLinesAllowed() ? OSD_TEXT_NORMAL : OSD_TEXT_DISABLED);
    OSD_drawDashRange(2, 22, 22, areScanLinesAllowed() ? OSD_TEXT_NORMAL : OSD_TEXT_DISABLED);
    OSD_writeStringAtRow(3, 1, "Line filter");
    OSD_drawDashRange(3, 12, 22);
}

void handle_ColorSettings_Page2_Values(void)
{
    OSD_displayNumber3DigitAtRow(1, 255 - GBS::ADC_RGCTRL::read(), 21, 20, 19);
    OSD_writeOnOff(1, uopt->enableAutoGain != 0);
    OSD_drawDashRange(2, 10, 19, areScanLinesAllowed() ? OSD_TEXT_NORMAL : OSD_TEXT_DISABLED);
    OSD_writeCharAtRow(2, 21, '0', areScanLinesAllowed() ? OSD_TEXT_NORMAL : OSD_TEXT_DISABLED);
    OSD_writeCharAtRow(2, 20, '0' + (uopt->scanlineStrength >> 4), areScanLinesAllowed() ? OSD_TEXT_NORMAL : OSD_TEXT_DISABLED);
    OSD_drawDashRange(2, 22, 22, areScanLinesAllowed() ? OSD_TEXT_NORMAL : OSD_TEXT_DISABLED);
    OSD_writeOnOff(2, uopt->wantScanlines, areScanLinesAllowed() ? OSD_TEXT_NORMAL : OSD_TEXT_DISABLED);
    OSD_writeOnOff(3, uopt->wantVdsLineFilter);
}

// Page 3: Sharpness, Peaking, Step response
void handle_ColorSettings_Page3(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '3', true);
    OSD_writeStringAtRow(1, 1, "Sharpness");
    OSD_drawDashRange(1, 10, 22);
    OSD_writeStringAtRow(2, 1, "Peaking");
    OSD_drawDashRange(2, 8, 22);
    OSD_writeStringAtRow(3, 1, "Step response");
    OSD_drawDashRange(3, 14, 22);
}

void handle_ColorSettings_Page3_Values(void)
{
    OSD_writeOnOff(1, GBS::VDS_PK_LB_GAIN::read() != 0x16);

    if (isPeakingLocked()) {
        OSD_writeStringAtRow(2, 20, "LOCKED");
    } else {
        OSD_drawDashRange(2, 20, 22);
        OSD_writeOnOff(2, uopt->wantPeaking != 0);
    }

    OSD_writeOnOff(3, uopt->wantStepResponse);
}

// Page 4: Y Gain, Color, Default Color
void handle_ColorSettings_Page4(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '4', false);
    OSD_writeStringAtRow(1, 1, "Y Gain");
    OSD_drawDashRange(1, 7, 22);
    OSD_writeStringAtRow(2, 1, "Color");
    OSD_drawDashRange(2, 6, 14);
    OSD_writeStringAtRow(3, 1, "Default Color");
}

void handle_ColorSettings_Page4_Values(void)
{
    OSD_displayNumber3DigitAtRow(1, GBS::VDS_Y_GAIN::read(), 25, 24, 23);
    OSD_writeStringAtRow(2, 15, "U:");
    OSD_displayNumber3DigitAtRow(2, GBS::VDS_UCOS_GAIN::read(), 19, 18, 17);
    OSD_writeStringAtRow(2, 20, "/V:");
    OSD_displayNumber3DigitAtRow(2, GBS::VDS_VCOS_GAIN::read(), 25, 24, 23);
}
