// ====================================================================================
// osd-developer.cpp
// TV OSD Handlers for Developer Menu
// ====================================================================================

#include "../osd-core.h"

// ====================================================================================
// Developer Memory Page
// ====================================================================================

void handle_Developer_Memory(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(false, '1', true);
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

// ====================================================================================
// Developer Debug Page
// ====================================================================================

void handle_Developer_Debug(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(false, '2', false);
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
    OSD_writeOnOff(3, GBS::CAPTURE_ENABLE::read() == 0);
}
