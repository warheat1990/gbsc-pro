// ====================================================================================
// osd-adv.cpp
// TV OSD Handlers for ADV7280 Settings (SV/AV Input)
// ====================================================================================

#include "../osd-core.h"

// ====================================================================================
// SV/AV Input Settings - Page 1 (I2P, Smooth, ACE)
// ====================================================================================

void handle_SVAVInput_Page1(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(false, '1', true);
    OSD_setMenuLineColorsCustom(selectedMenuLine, 2, advI2P ? OSD_TEXT_NORMAL : OSD_TEXT_DISABLED);
    OSD_writeStringAtRow(1, 1, "Enable I2P/2X");
    OSD_drawDashRange(1, 14, 22);
    OSD_writeStringAtRow(2, 1, "Smooth");
    OSD_drawDashRange(2, 7, 22);
    OSD_writeStringAtRow(3, 1, "ACE");
    OSD_drawDashRange(3, 4, 22);
}

void handle_SVAVInput_Page1_Values(void)
{
    OSD_writeOnOff(1, advI2P);
    OSD_writeOnOff(2, advSmooth);
    OSD_writeOnOff(3, advACE);
}

// ====================================================================================
// SV/AV Input Settings - Page 2 (Brightness, Contrast, Saturation)
// ====================================================================================

void handle_SVAVInput_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '2', true);
    OSD_writeStringAtRow(1, 1, "Brightness");
    OSD_drawDashRange(1, 11, 22);
    OSD_writeStringAtRow(2, 1, "Contrast");
    OSD_drawDashRange(2, 9, 22);
    OSD_writeStringAtRow(3, 1, "Saturation");
    OSD_drawDashRange(3, 11, 22);
}

void handle_SVAVInput_Page2_Values(void)
{
    OSD_displayNumber3DigitAtRow(1, advBrightness, 25, 24, 23);
    OSD_displayNumber3DigitAtRow(2, advContrast, 25, 24, 23);
    OSD_displayNumber3DigitAtRow(3, advSaturation, 25, 24, 23);
}

// ====================================================================================
// SV/AV Input Settings - Page 3 (Default)
// ====================================================================================

void handle_SVAVInput_Page3(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '3', false);
    OSD_writeStringAtRow(1, 1, "Default");
    OSD_writeStringAtRow(2, 1, "");
    OSD_writeStringAtRow(3, 1, "");
}

void handle_SVAVInput_Page3_Values(void)
{
    // No values to display on page 3
}
