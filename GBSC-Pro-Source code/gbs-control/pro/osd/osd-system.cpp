// ====================================================================================
// osd-system.cpp
// TV OSD Handlers for System Settings and Developer Options
// ====================================================================================

#include "osd-common.h"

// ====================================================================================
// System Settings - SV/AV Input
// ====================================================================================

void handle_SVAVInput_Page1(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(false, '1', true);
    // Line 2 (Smooth) disabled when lineOption is false
    uint8_t line2Color = lineOption ? OSD_TEXT_NORMAL : OSD_TEXT_DISABLED;
    OSD_setMenuLineColorsCustom(selectedMenuLine, 2, line2Color);
    OSD_writeStringAtRow(1, 1, "DoubleLine", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "Smooth", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "Bright", OSD_getMenuLineColor(3));
}

void handle_SVAVInput_Page1_Values(void)
{
    if (lineOption) {
        OSD_writeStringAtRow(1, 24, "2X");
    } else {
        OSD_writeStringAtRow(1, 24, "1X");
        smoothOption = false;
    }

    OSD_writeOnOff(2, smoothOption);
    OSD_displayNumber3DigitAtRow(3, brightness, 25, 24, 23, OSD_TEXT_NORMAL);
}

void handle_SVAVInput_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '2', false);
    OSD_writeStringAtRow(1, 1, "Contrast", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "Saturation", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "Default", OSD_getMenuLineColor(3));
}

void handle_SVAVInput_Page2_Values(void)
{
    OSD_displayNumber3DigitAtRow(1, contrast, 25, 24, 23, OSD_TEXT_NORMAL);
    OSD_displayNumber3DigitAtRow(2, saturation, 25, 24, 23, OSD_TEXT_NORMAL);
}

// ====================================================================================
// System Settings - General Pages
// ====================================================================================

void handle_SysSettings_Page1(void)
{
    // Line 1 (SV/AV Input Settings) disabled when not SV/AV input
    bool isSvAvInput = (inputType == InputTypeSV) || (inputType == InputTypeAV);
    uint8_t line1Color = isSvAvInput ? OSD_TEXT_NORMAL : OSD_TEXT_DISABLED;
    OSD_setMenuLineColorsCustom(selectedMenuLine, 1, line1Color);
    OSD_writeCharAtRow(1, 21, arrow_right_icon, (selectedMenuLine == 1) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
    OSD_writePageIcons(false, '1', true);
    OSD_writeStringAtRow(1, 1, "AV/SV Input Settings", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "Compatibility Mode", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "Matched presets", OSD_getMenuLineColor(3));
}

void handle_SysSettings_Page1_Values(void)
{
    OSD_drawDashRange(2, 19, 22);  // Row 2: P19-P22
    OSD_writeOnOff(2, rgbComponentMode == 1);
    OSD_drawDashRange(3, 16, 22);  // Row 3: P16-P22
    OSD_writeOnOff(3, uopt->matchPresetSource);
}

void handle_SysSettings_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '2', true);
    OSD_writeStringAtRow(1, 1, "Deinterlace", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "Force 50Hz to 60Hz", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "Lock method", OSD_getMenuLineColor(3));
}

void handle_SysSettings_Page2_Values(void)
{
    OSD_drawDashRange(1, 12, 17);  // Row 1: P12-P17

    if (uopt->deintMode == 0) {
        OSD_writeStringAtRow(1, 18, "Adaptive");
    } else {
        OSD_drawDashRange(1, 18, 22);  // Row 1: P18-P22
        OSD_writeStringAtRow(1, 23, "Bob");
    }

    OSD_drawDashRange(2, 19, 22);  // Row 2: P19-P22
    OSD_drawDashRange(3, 12, 13);  // Row 3: P12-P13
    OSD_writeOnOff(2, uopt->PalForce60);

    if (uopt->frameTimeLockMethod == 0) {
        OSD_writeStringAtRow(3, 14, "0Vtotal<VSST");
    } else {
        OSD_writeStringAtRow(3, 14, "1Vtotal only");
    }
}

void handle_SysSettings_Page4(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '3', false);
    OSD_writeStringAtRow(1, 1, "ADC calibration", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "Frame Time lock", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "EnableFrameTimeLock", OSD_getMenuLineColor(3));
}

void handle_SysSettings_Page4_Values(void)
{
    OSD_drawDashRange(1, 16, 22);  // Row 1: P16-P22
    OSD_drawDashRange(2, 16, 22);  // Row 2: P16-P22
    OSD_drawDashRange(3, 20, 22);  // Row 3: P20-P22
    OSD_writeOnOff(1, uopt->enableCalibrationADC);
    OSD_writeOnOff(2, uopt->enableFrameTimeLock);
    OSD_writeOnOff(3, !uopt->disableExternalClockGenerator);
}

void handle_SysSettings_Page5(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, 'I', true);
    OSD_writeStringAtRow(1, 1, "Enable OTA", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "Restart", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "Reset defaults", OSD_getMenuLineColor(3));
}

void handle_SysSettings_Page5_Values(void)
{
    OSD_drawDashRange(1, 11, 22);  // Row 1: P11-P22
    OSD_writeOnOff(1, rto->allowUpdatesOTA);
}

// ====================================================================================
// Developer Options
// ====================================================================================

void handle_Developer_Memory(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, 'I', true);
    OSD_writeStringAtRow(1, 1, "MEM left/right", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "HS left/right", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "HTotal", OSD_getMenuLineColor(3));
}

void handle_Developer_Memory_Values(void)
{
    OSD_drawDashRange(1, 15, 22);  // Row 1: pos 15-22
    OSD_writeCharAtRow(1, 23, horizontal_scale_part1_icon, OSD_CURSOR_ACTIVE);
    OSD_writeCharAtRow(1, 24, horizontal_scale_part2_icon, OSD_CURSOR_ACTIVE);
    OSD_drawDashRange(2, 14, 22);  // Row 2: pos 14-22
    OSD_writeCharAtRow(2, 23, horizontal_scale_part1_icon, OSD_CURSOR_ACTIVE);
    OSD_writeCharAtRow(2, 24, horizontal_scale_part2_icon, OSD_CURSOR_ACTIVE);
    OSD_drawDashRange(3, 7, 22);   // Row 3: pos 7-22
    OSD_displayNumber3DigitAtRow(3, GBS::VDS_HSYNC_RST::read(), 25, 24, 23, OSD_TEXT_NORMAL);
}

void handle_Developer_Debug(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, 'I', true);
    OSD_writeStringAtRow(1, 1, "Debug view", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "ADC filter", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "Freeze capture", OSD_getMenuLineColor(3));
}

void handle_Developer_Debug_Values(void)
{
    OSD_drawDashRange(1, 11, 22);  // Row 1: P11-P22
    OSD_drawDashRange(2, 11, 22);  // Row 2: P11-P22
    OSD_drawDashRange(3, 15, 22);  // Row 3: P15-P22
    OSD_writeOnOff(1, GBS::ADC_UNUSED_62::read() != 0x00);
    OSD_writeOnOff(2, GBS::ADC_FLTR::read() > 0);
    OSD_writeOnOff(3, GBS::CAPTURE_ENABLE::read() == 0);  // Inverted: ON when frozen
}

void handle_Restart(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '4', false);
    OSD_writeStringAtRow(1, 1, "Matched presets", OSD_getMenuLineColor(1));
}
