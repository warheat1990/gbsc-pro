// ====================================================================================
// osd-render-pro.h
// TV OSD Menu Rendering - Core Declarations
//
// This file contains declarations for:
// - menuTable[]: Command dispatch table
// - OSD_handleCommand(): Menu command dispatcher
// - OSD feedback functions
//
// Note: MenuEntry type is defined in gbs-control-pro.h
// Note: Handler function declarations are in osd/osd-common.h
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

// ====================================================================================
// Function Declarations - TV OSD Feedback
// ====================================================================================

void OSD_showLimitFeedback(uint8_t row, int iterations = 400);
void OSD_showOkFeedback(uint8_t row, int iterations = 800);
void OSD_showSavingFeedback(uint8_t row, uint8_t startPos = 19, int iterations = 800);
void OSD_showAdjustArrows(uint8_t row, uint8_t dashStart = 8);
void OSD_highlightIcon(uint8_t pos);
