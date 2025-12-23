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
// OSD Dispatch Table (generated from X-macro)
// ====================================================================================

const MenuEntry osdDispatchTable[] = {
    #define DISPATCH_ENTRY(cmd, handler, saveable) {cmd, handler, saveable},
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
            if (osdDispatchTable[i].saveable) {
                lastOsdCommand = cmd;
            }
            osdDispatchTable[i].handler();
            return;
        }
    }
}
