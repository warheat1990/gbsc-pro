// ====================================================================================
// osd-system.cpp
// TV OSD Handlers for System Settings and Developer Options
// ====================================================================================

#include "../osd-core.h"

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
    OSD_writeStringAtRow(1, 1, "Enable I2P/2X");
    OSD_drawDashRange(1, 14, 22);
    OSD_writeStringAtRow(2, 1, "Smooth");
    OSD_drawDashRange(2, 7, 22);
    OSD_writeStringAtRow(3, 1, "Bright");
    OSD_drawDashRange(3, 7, 22);

}

void handle_SVAVInput_Page1_Values(void)
{
    OSD_writeOnOff(1, lineOption);
    OSD_writeOnOff(2, smoothOption);
    OSD_displayNumber3DigitAtRow(3, brightness, 25, 24, 23);
}

void handle_SVAVInput_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '2', false);
    OSD_writeStringAtRow(1, 1, "Contrast");
    OSD_drawDashRange(1, 9, 22);
    OSD_writeStringAtRow(2, 1, "Saturation");
    OSD_drawDashRange(2, 11, 22);
    OSD_writeStringAtRow(3, 1, "Default");
}

void handle_SVAVInput_Page2_Values(void)
{
    OSD_displayNumber3DigitAtRow(1, contrast, 25, 24, 23);
    OSD_displayNumber3DigitAtRow(2, saturation, 25, 24, 23);
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
    OSD_writeStringAtRow(1, 1, "AV/SV Input Settings");
    OSD_writeStringAtRow(2, 1, "Compatibility Mode");
    OSD_drawDashRange(2, 19, 22);
    OSD_writeStringAtRow(3, 1, "Matched presets");
    OSD_drawDashRange(3, 16, 22);
}

void handle_SysSettings_Page1_Values(void)
{
    OSD_writeOnOff(2, rgbComponentMode == 1);
    OSD_writeOnOff(3, uopt->matchPresetSource);
}

void handle_SysSettings_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '2', true);
    OSD_writeStringAtRow(1, 1, "Deinterlace");
    OSD_drawDashRange(1, 12, 17);
    OSD_writeStringAtRow(2, 1, "Force 50Hz to 60Hz");
    OSD_drawDashRange(2, 19, 22);
    OSD_writeStringAtRow(3, 1, "Lock method");
    OSD_drawDashRange(3, 12, 13);
}

void handle_SysSettings_Page2_Values(void)
{
    if (uopt->deintMode == 0) {
        OSD_writeStringAtRow(1, 18, "Adaptive");
    } else {
        OSD_writeStringAtRow(1, 18, "-----Bob");
    }
    
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
    OSD_writeStringAtRow(1, 1, "ADC calibration");
    OSD_drawDashRange(1, 16, 22);
    OSD_writeStringAtRow(2, 1, "Frame Time lock");
    OSD_drawDashRange(2, 16, 22);
    OSD_writeStringAtRow(3, 1, "EnableFrameTimeLock");
    OSD_drawDashRange(3, 20, 22);
}

void handle_SysSettings_Page4_Values(void)
{
    OSD_writeOnOff(1, uopt->enableCalibrationADC);
    OSD_writeOnOff(2, uopt->enableFrameTimeLock);
    OSD_writeOnOff(3, !uopt->disableExternalClockGenerator);
}

void handle_SysSettings_Page5(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, 'I', true);
    OSD_writeStringAtRow(1, 1, "Enable OTA");
    OSD_drawDashRange(1, 11, 22);
    OSD_writeStringAtRow(2, 1, "Restart");
    OSD_writeStringAtRow(3, 1, "Reset defaults");
}

void handle_SysSettings_Page5_Values(void)
{
    OSD_writeOnOff(1, rto->allowUpdatesOTA);
}

// ====================================================================================
// Developer Options
// ====================================================================================

void handle_Developer_Memory(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, 'I', true);
    OSD_writeStringAtRow(1, 1, "MEM left/right");
    OSD_drawDashRange(1, 15, 22);
    OSD_writeStringAtRow(2, 1, "HS left/right");
    OSD_drawDashRange(2, 14, 22);
    OSD_writeStringAtRow(3, 1, "HTotal");
    OSD_drawDashRange(3, 7, 22);
}

void handle_Developer_Memory_Values(void)
{
    OSD_writeCharAtRow(1, 23, horizontal_scale_part1_icon, OSD_CURSOR_ACTIVE);
    OSD_writeCharAtRow(1, 24, horizontal_scale_part2_icon, OSD_CURSOR_ACTIVE);
    OSD_writeCharAtRow(2, 23, horizontal_scale_part1_icon, OSD_CURSOR_ACTIVE);
    OSD_writeCharAtRow(2, 24, horizontal_scale_part2_icon, OSD_CURSOR_ACTIVE);
    OSD_displayNumber3DigitAtRow(3, GBS::VDS_HSYNC_RST::read(), 25, 24, 23);
}

void handle_Developer_Debug(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, 'I', true);
    OSD_writeStringAtRow(1, 1, "Debug view");
    OSD_drawDashRange(1, 11, 22);
    OSD_writeStringAtRow(2, 1, "ADC filter");
    OSD_drawDashRange(2, 11, 22);
    OSD_writeStringAtRow(3, 1, "Freeze capture");
    OSD_drawDashRange(3, 15, 22);
}

void handle_Developer_Debug_Values(void)
{
    OSD_writeOnOff(1, GBS::ADC_UNUSED_62::read() != 0x00);
    OSD_writeOnOff(2, GBS::ADC_FLTR::read() > 0);
    OSD_writeOnOff(3, GBS::CAPTURE_ENABLE::read() == 0);  // Inverted: ON when frozen
}

void handle_Restart(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '4', false);
    OSD_writeStringAtRow(1, 1, "Matched presets");
}
