// ====================================================================================
// osd-render-pro.h
// TV OSD Menu Rendering - Core Declarations
//
// This file contains declarations for:
// - menuTable[]: Command dispatch table
// - OSD_handleCommand(): Menu command dispatcher
//
// Note: MenuEntry type is defined in gbs-control-pro.h
// Note: Helper and handler function declarations are in osd/osd-common.h
// ====================================================================================

#pragma once

#include <Arduino.h>

// ====================================================================================
// External Variables - Menu Table
// ====================================================================================

extern const MenuEntry menuTable[];
extern const size_t menuTableSize;

// ====================================================================================
// Function Declarations - Menu Dispatcher
// ====================================================================================

void OSD_handleCommand(OsdCommand cmd);
