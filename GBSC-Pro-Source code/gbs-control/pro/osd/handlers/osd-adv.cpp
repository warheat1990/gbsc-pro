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
    OSD_writeStringAtRow(1, 1, "Enable I2P/2X");
    OSD_drawDashRange(1, 14, 22);
    OSD_writeStringAtRow(2, 1, "Smooth", uopt->advI2P ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_drawDashRange(2, 7, 22, uopt->advI2P ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_writeStringAtRow(3, 1, "ACE Settings");
    OSD_writeCharAtRow(3, 0xFF, arrow_right_icon, (selectedMenuLine == 3) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
}

void handle_SVAVInput_Page1_Values(void)
{
    OSD_writeOnOff(1, uopt->advI2P);
    OSD_writeOnOff(2, uopt->advSmooth, uopt->advI2P ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
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
    OSD_displayNumber3DigitAtRow(1, uopt->advBrightness, 25, 24, 23);
    OSD_displayNumber3DigitAtRow(2, uopt->advContrast, 25, 24, 23);
    OSD_displayNumber3DigitAtRow(3, uopt->advSaturation, 25, 24, 23);
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

// ====================================================================================
// ACE Settings - Page 1 (Enable, Luma Gain, Chroma Gain)
// ====================================================================================

void handle_ACE_Page1(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(false, '1', true);
    OSD_writeStringAtRow(1, 1, "Enable");
    OSD_drawDashRange(1, 7, 22);
    OSD_writeStringAtRow(2, 1, "Luma Gain", uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_drawDashRange(2, 10, 23, uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_writeStringAtRow(3, 1, "Chroma Gain", uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_drawDashRange(3, 12, 23, uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
}

void handle_ACE_Page1_Values(void)
{
    OSD_writeOnOff(1, uopt->advACE);
    OSD_displayNumber2DigitAtRow(2, uopt->advACELumaGain, 25, 24, uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_displayNumber2DigitAtRow(3, uopt->advACEChromaGain, 25, 24, uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
}

// ====================================================================================
// ACE Settings - Page 2 (Chroma Max, Gamma Gain, Response Speed)
// ====================================================================================

void handle_ACE_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '2', true);
    OSD_writeStringAtRow(1, 1, "Chroma Max", uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_drawDashRange(1, 11, 23, uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_writeStringAtRow(2, 1, "Gamma Gain", uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_drawDashRange(2, 11, 23, uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_writeStringAtRow(3, 1, "Response Spd", uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_drawDashRange(3, 13, 23, uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
}

void handle_ACE_Page2_Values(void)
{
    OSD_displayNumber2DigitAtRow(1, uopt->advACEChromaMax, 25, 24, uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_displayNumber2DigitAtRow(2, uopt->advACEGammaGain, 25, 24, uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_displayNumber2DigitAtRow(3, uopt->advACEResponseSpeed, 25, 24, uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
}

// ====================================================================================
// ACE Settings - Page 3 (Default)
// ====================================================================================

void handle_ACE_Page3(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '3', false);
    OSD_writeStringAtRow(1, 1, "Default", uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_writeStringAtRow(2, 1, "");
    OSD_writeStringAtRow(3, 1, "");
}

void handle_ACE_Page3_Values(void)
{
    // No values to display on page 3
}
