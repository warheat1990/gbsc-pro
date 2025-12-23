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
