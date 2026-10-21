// ====================================================================================
// osd-output.cpp
// TV OSD Handlers for Output Resolution
// ====================================================================================

#include "../osd-core.h"

// ====================================================================================
// Output Resolution Handlers
// ====================================================================================

void handle_OutputRes_1200_1080_1024(void)
{
    const bool isPalGroup = rto->videoStandardInput == 2 || rto->videoStandardInput == 4;
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(false, '1', true);
    OSD_writeStringAtRow(1, 1, "1920x1200", isPalGroup ? OSD_TEXT_DISABLED : OSD_TEXT_NORMAL);
    OSD_writeStringAtRow(2, 1, "1920x1080");
    OSD_writeStringAtRow(3, 1, "1280x1024");
}

void handle_OutputRes_960_720_768(void)
{
    const bool isPalGroup = rto->videoStandardInput == 2 || rto->videoStandardInput == 4;
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '2', true);
    OSD_writeStringAtRow(1, 1, "1280x960");
    OSD_writeStringAtRow(2, 1, "1280x720");
    OSD_writeStringAtRow(3, 1, "1024x768", isPalGroup ? OSD_TEXT_DISABLED : OSD_TEXT_NORMAL);
}

void handle_OutputRes_480(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '3', false);
    OSD_writeStringAtRow(1, 1, "480p/576p");
}

// void handle_OutputRes_1080_1024_960(void)
// {
//     OSD_setMenuLineColors(selectedMenuLine);
//     OSD_writePageIcons(false, '1', true);
//     OSD_writeStringAtRow(1, 1, "1920x1080");
//     OSD_writeStringAtRow(2, 1, "1280x1024");
//     OSD_writeStringAtRow(3, 1, "1280x960");
// }

// void handle_OutputRes_720_480(void)
// {
//     OSD_setMenuLineColors(selectedMenuLine);
//     OSD_writePageIcons(true, '2', false);
//     OSD_writeStringAtRow(1, 1, "1280x720");
//     OSD_writeStringAtRow(2, 1, "480p/576p");
// }
