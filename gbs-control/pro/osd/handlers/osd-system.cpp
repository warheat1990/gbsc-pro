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
    // Sync Stripper only applies to RGB inputs (not AV/SV)
    bool syncStripperAvailable = (uopt->activeInputType != InputTypeSV) &&
                                  (uopt->activeInputType != InputTypeAV);
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(false, '1', true);
    OSD_writeStringAtRow(1, 1, "Sync Stripper", syncStripperAvailable ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_drawDashRange(1, 14, 22, syncStripperAvailable ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_writeStringAtRow(2, 1, "Matched presets");
    OSD_drawDashRange(2, 16, 22);
    OSD_writeStringAtRow(3, 1, "Deinterlace");
    OSD_drawDashRange(3, 12, 17);
}

void handle_SysSettings_Page1_Values(void)
{
    // Sync Stripper only applies to RGB inputs (not AV/SV)
    bool syncStripperAvailable = (uopt->activeInputType != InputTypeSV) &&
                                  (uopt->activeInputType != InputTypeAV);
    OSD_writeOnOff(1, uopt->advSyncStripper == 1, syncStripperAvailable ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_writeOnOff(2, uopt->matchPresetSource);
    if (uopt->deintMode == 0) {
        OSD_writeStringAtRow(3, 18, "Adaptive");
    } else {
        OSD_writeStringAtRow(3, 18, "-----Bob");
    }
}

void handle_SysSettings_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '2', true);
    OSD_writeStringAtRow(1, 1, "Force 50Hz to 60Hz");
    OSD_drawDashRange(1, 19, 22);
    OSD_writeStringAtRow(2, 1, "Lock method");
    OSD_drawDashRange(2, 12, 13);
    OSD_writeStringAtRow(3, 1, "ADC calibration");
    OSD_drawDashRange(3, 16, 22);
}

void handle_SysSettings_Page2_Values(void)
{
    OSD_writeOnOff(1, uopt->PalForce60);

    if (uopt->frameTimeLockMethod == 0) {
        OSD_writeStringAtRow(2, 14, "0Vtotal<VSST");
    } else {
        OSD_writeStringAtRow(2, 14, "1Vtotal only");
    }

    OSD_writeOnOff(3, uopt->enableCalibrationADC);
}

void handle_SysSettings_Page4(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '3', false);
    OSD_writeStringAtRow(1, 1, "Frame Time lock");
    OSD_drawDashRange(1, 16, 22);
    OSD_writeStringAtRow(2, 1, "EnableFrameTimeLock");
    OSD_drawDashRange(2, 20, 22);
    OSD_writeStringAtRow(3, 1, "HDMI Limited Range");
    OSD_drawDashRange(3, 19, 22);
}

void handle_SysSettings_Page4_Values(void)
{
    OSD_writeOnOff(1, uopt->enableFrameTimeLock);
    OSD_writeOnOff(2, !uopt->disableExternalClockGenerator);
    // HDMI Limited Range: 0=OFF, 1=HD, 2=SD, 3=ALL
    switch (uopt->hdmiLimitedRange) {
        case 0: OSD_writeStringAtRow(3, 23, "OFF"); break;
        case 1: OSD_writeStringAtRow(3, 23, "-HD"); break;
        case 2: OSD_writeStringAtRow(3, 23, "-SD"); break;
        case 3: OSD_writeStringAtRow(3, 23, "ALL"); break;
    }
}

void handle_SysSettings_Page5(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '4', false);
    OSD_writeStringAtRow(1, 1, "Keep output on no signal");
    OSD_drawDashRange(1, 12, 22);
}

void handle_SysSettings_Page5_Values(void)
{
    OSD_writeOnOff(1, uopt->keepOutputOnNoSignal);
}
