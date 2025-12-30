// ====================================================================================
// osd-developer.cpp
// TV OSD Handlers for Developer Menu (6 pages)
// ====================================================================================

#include "../osd-core.h"

extern float getOutputFrameRate();

// ====================================================================================
// Developer Page 1: MEM, HS, HTotal
// ====================================================================================

void handle_Developer_Page1(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(false, '1', true);
    OSD_writeStringAtRow(1, 1, "MEM left/right");
    OSD_drawDashRange(1, 15, 23);
    OSD_writeStringAtRow(2, 1, "HS left/right");
    OSD_drawDashRange(2, 14, 23);
    OSD_writeStringAtRow(3, 1, "HTotal");
    OSD_drawDashRange(3, 7, 21);
}

void handle_Developer_Page1_Values(void)
{
    OSD_writeCharAtRow(1, 24, horizontal_scale_part1_icon);
    OSD_writeCharAtRow(1, 25, horizontal_scale_part2_icon);
    OSD_writeCharAtRow(2, 24, horizontal_scale_part1_icon);
    OSD_writeCharAtRow(2, 25, horizontal_scale_part2_icon);
    OSD_displayNumber4DigitAtRow(3, GBS::VDS_HSYNC_RST::read(), 25, 24, 23, 22);
}

// ====================================================================================
// Developer Page 2: Debug, ADC, Freeze
// ====================================================================================

void handle_Developer_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '2', true);
    OSD_writeStringAtRow(1, 1, "Debug view");
    OSD_drawDashRange(1, 11, 22);
    OSD_writeStringAtRow(2, 1, "ADC filter");
    OSD_drawDashRange(2, 11, 22);
    OSD_writeStringAtRow(3, 1, "Freeze capture");
    OSD_drawDashRange(3, 15, 22);
}

void handle_Developer_Page2_Values(void)
{
    OSD_writeOnOff(1, GBS::ADC_UNUSED_62::read() != 0x00);
    OSD_writeOnOff(2, GBS::ADC_FLTR::read() > 0);
    OSD_writeOnOff(3, GBS::CAPTURE_ENABLE::read() == 0);
}

// ====================================================================================
// Developer Page 3: Resync HTotal, Cycle SDRAM, PLL Divider
// ====================================================================================

void handle_Developer_Page3(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '3', true);
    OSD_writeStringAtRow(1, 1, "Resync HTotal");
    OSD_writeStringAtRow(2, 1, "Cycle SDRAM");
    OSD_drawDashRange(2, 12, 19);
    OSD_writeStringAtRow(3, 1, "PLL div++");
    OSD_drawDashRange(3, 10, 21);
}

void handle_Developer_Page3_Values(void)
{
    // Show SDRAM clock value based on PLL_MS
    uint8_t pllMs = GBS::PLL_MS::read();
    const char* clockStr;
    switch (pllMs) {
        case 0: clockStr = "129MHz"; break;
        case 1: clockStr = "144MHz"; break;
        case 2: clockStr = "162MHz"; break;
        case 3: clockStr = "185MHz"; break;
        case 4: clockStr = "216MHz"; break;
        case 5: clockStr = "108MHz"; break;
        default: clockStr = "----FB"; break;  // Feedback clock
    }
    OSD_writeStringAtRow(2, 20, clockStr);

    // Show PLL divider value
    OSD_displayNumber4DigitAtRow(3, GBS::PLLAD_MD::read(), 25, 24, 23, 22);
}

// ====================================================================================
// Developer Page 4: Invert Sync, SyncWatcher, SyncProcessor
// ====================================================================================

void handle_Developer_Page4(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '4', true);
    OSD_writeStringAtRow(1, 1, "Invert Sync");
    OSD_writeStringAtRow(2, 1, "SyncWatcher");
    OSD_drawDashRange(2, 12, 22);
    OSD_writeStringAtRow(3, 1, "SyncProcessor");
}

void handle_Developer_Page4_Values(void)
{
    OSD_writeOnOff(2, rto->syncWatcherEnabled);
}

// ====================================================================================
// Developer Page 5: Oversampling, Snap Frame Rate, IF Auto Offset
// ====================================================================================

void handle_Developer_Page5(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '5', true);
    OSD_writeStringAtRow(1, 1, "Oversampling");
    OSD_drawDashRange(1, 13, 23);
    OSD_writeStringAtRow(2, 1, "60/50Hz HDMI");
    OSD_drawDashRange(2, 13, 21);
    OSD_writeStringAtRow(3, 1, "IF Auto Offset");
    OSD_drawDashRange(3, 15, 22);
}

void handle_Developer_Page5_Values(void)
{
    // Show OSR value: 1x, 2x, or 4x
    OSD_writeCharAtRow(1, 24, '0' + rto->osr);
    OSD_writeCharAtRow(1, 25, 'x');

    // Show snap target: 50Hz or 60Hz based on current output frame rate
    float ofr = getOutputFrameRate();
    if (ofr > 56.5f && ofr < 64.5f) {
        OSD_writeStringAtRow(2, 22, "60Hz");
    } else if (ofr > 46.5f && ofr < 54.5f) {
        OSD_writeStringAtRow(2, 22, "50Hz");
    } else {
        OSD_writeStringAtRow(2, 22, "----");
    }

    // Show IF Auto Offset ON/OFF
    OSD_writeOnOff(3, GBS::IF_AUTO_OFST_EN::read() != 0);
}

// ====================================================================================
// Developer Page 6: SOG Level, Reset Chip
// ====================================================================================

void handle_Developer_Page6(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '6', false);
    OSD_writeStringAtRow(1, 1, "SOG Level--");
    OSD_drawDashRange(1, 12, 16);
    OSD_writeStringAtRow(2, 1, "Reset Chip");
}

void handle_Developer_Page6_Values(void)
{
    // Show Phase and SOG: "P:xx-S:xx"
    OSD_writeStringAtRow(1, 17, "P:");
    OSD_displayNumber2DigitAtRow(1, rto->phaseSP, 20, 19);
    OSD_writeCharAtRow(1, 21, '-');
    OSD_writeStringAtRow(1, 22, "S:");
    OSD_displayNumber2DigitAtRow(1, rto->currentLevelSOG, 25, 24);
}
