// ====================================================================================
// osd-system.cpp
// TV OSD Handlers for System Settings
// ====================================================================================

#include "../osd-core.h"

// ====================================================================================
// System Settings - General Pages
// ====================================================================================

void handle_SysSettings_Page1(void)
{
    // SV/AV Input Settings disabled when not SV/AV input
    bool isSvAvInput = (uopt->activeInputType == InputTypeSV) || (uopt->activeInputType == InputTypeAV);
    OSD_setMenuLineColorsCustom(selectedMenuLine, 1, isSvAvInput ? OSD_TEXT_NORMAL : OSD_TEXT_DISABLED);
    // Arrow also disabled when not SV/AV input
    uint8_t arrowColor = isSvAvInput ? ((selectedMenuLine == 1) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE) : OSD_TEXT_DISABLED;
    OSD_writeCharAtRow(1, 21, arrow_right_icon, arrowColor);
    OSD_writePageIcons(false, '1', true);
    OSD_writeStringAtRow(1, 1, "AV/SV Input Settings");
    OSD_writeStringAtRow(2, 1, "Compatibility Mode");
    OSD_drawDashRange(2, 19, 22);
    OSD_writeStringAtRow(3, 1, "Matched presets");
    OSD_drawDashRange(3, 16, 22);
}

void handle_SysSettings_Page1_Values(void)
{
    OSD_writeOnOff(2, uopt->advCompatibility == 1);
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
