// ====================================================================================
// osd-screen.cpp
// TV OSD Handlers for Screen Settings
// ====================================================================================

#include "../osd-core.h"

// ====================================================================================
// Screen Settings Handlers
// ====================================================================================

void handle_ScreenSettings_Page1(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(false, '1', true);
    OSD_writeStringAtRow(1, 1, "Move");
    OSD_writeStringAtRow(2, 1, "Scale");
    OSD_writeStringAtRow(3, 1, "Borders");
}

void handle_ScreenSettings_Page1_Values(void)
{
    uint16_t hPos;
    if (rto->videoStandardInput == 1 || rto->videoStandardInput == 2) {
        hPos = GBS::IF_HBIN_SP::read();
    } else {
        hPos = GBS::IF_HB_SP2::read();
    }
    OSD_displayNumber4DigitAtRow(1, hPos, 12, 11, 10, 9);
    OSD_writeCharAtRow(1, 13, '/');
    OSD_displayNumber4DigitAtRow(1, GBS::IF_VB_ST::read(), 17, 16, 15, 14);
    OSD_displayNumber4DigitAtRow(2, GBS::VDS_HSCALE::read(), 12, 11, 10, 9);
    OSD_writeCharAtRow(2, 13, '/');
    OSD_displayNumber4DigitAtRow(2, GBS::VDS_VSCALE::read(), 17, 16, 15, 14);
    OSD_displayNumber4DigitAtRow(3, GBS::VDS_DIS_HB_ST::read(), 12, 11, 10, 9);
    OSD_writeCharAtRow(3, 13, '/');
    OSD_displayNumber4DigitAtRow(3, GBS::VDS_DIS_VB_ST::read(), 17, 16, 15, 14);
}

void handle_ScreenSettings_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '2', false);
    OSD_writeStringAtRow(1, 1, "Full height");
    OSD_drawDashRange(1, 12, 22);
}

void handle_ScreenSettings_Page2_Values(void)
{
    OSD_writeOnOff(1, uopt->wantFullHeight);
}
