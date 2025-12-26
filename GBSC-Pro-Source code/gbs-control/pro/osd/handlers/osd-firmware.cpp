// ====================================================================================
// osd-firmware.cpp
// TV OSD Handler for Firmware Version Display
// ====================================================================================

#include "../osd-core.h"

// GBS_FW_VERSION and ADV_FW_VERSION are defined in gbs-control.ino

// ====================================================================================
// Firmware Version Handler
// ====================================================================================

void handle_FirmwareVersion(void)
{
    // No row selection - this is an informational screen only
    OSD_setMenuLineColors(0);

    // Row 1: Title/Header
    OSD_writeStringAtRow(1, 1, "--- GBS-C Pro Versions ---");

    // Row 2: GBS Control version
    OSD_writeStringAtRow(2, 1, "GBS Control");
    OSD_writeStringAtRow(2, 17, GBS_FW_VERSION);

    // Row 3: ADV Controller version
    OSD_writeStringAtRow(3, 1, "ADV Control");
    OSD_writeStringAtRow(3, 17, ADV_FW_VERSION);
}
