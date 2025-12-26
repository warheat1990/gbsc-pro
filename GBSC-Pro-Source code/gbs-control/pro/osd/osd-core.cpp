// ====================================================================================
// osd-core.cpp
// OSD Core - Dispatch Table and Command Handler
//
// This file contains ONLY OSD infrastructure:
// - osdDispatchTable[]: OSD command to handler mapping (X-macro generated)
// - OSD_handleCommand(): Menu command dispatcher
//
// Helper functions are in: helpers/osd-helpers.cpp
// OSD handlers are in: handlers/osd-*.cpp
// ====================================================================================

#include "osd-core.h"

// ====================================================================================
// OSD Theme Variables (shared across all files via extern in stv9426.h)
// ====================================================================================

// Current theme ID
uint8_t OSD_currentTheme = OSD_THEME_CLASSIC;

// Text colors - initialized to Classic theme (blue background)
uint8_t OSD_TEXT_NORMAL      = OSD_COLOR(OSD_FG_WHITE, OSD_BG_BLUE);   // 0x17
uint8_t OSD_TEXT_SELECTED    = OSD_COLOR(OSD_FG_YELLOW, OSD_BG_BLUE);  // 0x16
uint8_t OSD_TEXT_DISABLED    = OSD_COLOR(OSD_FG_RED, OSD_BG_BLUE);     // 0x14

// Navigation elements
uint8_t OSD_ICON_PAGE        = OSD_COLOR(OSD_FG_GREEN, OSD_BG_BLUE);   // 0x12
uint8_t OSD_CURSOR_ACTIVE    = OSD_COLOR(OSD_FG_BLACK, OSD_BG_YELLOW); // 0x60
uint8_t OSD_CURSOR_INACTIVE  = OSD_COLOR(OSD_FG_BLUE, OSD_BG_BLUE);    // 0x11

// Background
uint8_t OSD_BACKGROUND       = OSD_COLOR(OSD_FG_BLUE, OSD_BG_BLUE);    // 0x11

// ====================================================================================
// OSD Dispatch Table (generated from X-macro)
// ====================================================================================

const MenuEntry osdDispatchTable[] = {
    #define DISPATCH_ENTRY(cmd, handler) {cmd, handler},
    OSD_DISPATCH_ENTRIES
    #undef DISPATCH_ENTRY
};

const size_t osdDispatchTableSize = sizeof(osdDispatchTable) / sizeof(osdDispatchTable[0]);

// ====================================================================================
// OSD Command Dispatcher
// ====================================================================================

void OSD_handleCommand(OsdCommand cmd)
{
    for (size_t i = 0; i < osdDispatchTableSize; i++) {
        if (osdDispatchTable[i].cmd == cmd) {
            osdDispatchTable[i].handler();
            return;
        }
    }
}
