// ====================================================================================
// osd-output.cpp
// TV OSD Handlers for Output Resolution
// ====================================================================================

#include "osd-common.h"

// ====================================================================================
// Output Resolution Handlers
// ====================================================================================

void handle_OutputRes_1080_1024_960(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(false, '1', true);
    OSD_writeStringAtRow(1, 1, "1920x1080", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "1280x1024", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "1280x960", OSD_getMenuLineColor(3));
}

void handle_OutputRes_720_480(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '2', false);
    OSD_writeStringAtRow(1, 1, "1280x720", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "480p/576p", OSD_getMenuLineColor(2));
}

// void handle_OutputRes_PassThrough(void)
// {
//     OSD_setMenuLineColors(selectedMenuLine);
//     OSD_writePageIcons(true, 'I', true);
//     OSD_writeStringAtRow(1, 1, "Pass through", OSD_getMenuLineColor(1));
// }
