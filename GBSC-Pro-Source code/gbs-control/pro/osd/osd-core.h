// ====================================================================================
// osd-core.h
// OSD Core - Helper Functions and Dispatch
//
// This file contains:
// - Helper function declarations for TV OSD rendering
// - External references needed by OSD handlers
//
// Handler declarations and types are in osd-registry.h
// ====================================================================================

#pragma once

#include <Arduino.h>

#include "osd-registry.h"         // OsdCommand, MenuEntry, handler declarations
#include "../gbs-control-pro.h"   // Pro-specific declarations (uses osd-registry.h)
#include "../options-pro.h"
#include "../../options.h"
#include "../../tv5725.h"

#include "../drivers/stv9426.h"

// ====================================================================================
// External References
// ====================================================================================

typedef TV5725<GBS_ADDR> GBS;
extern struct runTimeOptions *rto;
extern struct userOptions *uopt;

extern boolean areScanLinesAllowed();
extern void saveUserPrefs();
extern void applyPresets(uint8_t videoMode);

// ====================================================================================
// Helper Function Declarations
// ====================================================================================

// Display "profile-N" at positions P15-P23 (9 characters)
void displayProfileName(uint8_t row, uint8_t index, uint8_t color = OSD_COLOR_AUTO);

// Get color for menu row (1-based)
uint8_t OSD_getMenuLineColor(uint8_t row);

// Set menu line colors based on selection
void OSD_setMenuLineColors(uint8_t selectedLine);

// Write page navigation icons at position P27
void OSD_writePageIcons(bool showUp, uint8_t pageChar, bool showDown);

// Unified row highlight function
void highlightRow(uint8_t row);

// Draw dashes on a row from startPos to endPos (logical positions 0-27)
void OSD_drawDashRange(uint8_t row, uint8_t startPos, uint8_t endPos, uint8_t color = OSD_COLOR_AUTO);

// Write ON or OFF indicator at end of row (position 25)
void OSD_writeOnOff(uint8_t row, bool isOn, uint8_t color = OSD_COLOR_AUTO);

// Show 4-direction adjustment icons on TV OSD row
void OSD_showAdjustArrows(uint8_t row, uint8_t pos = 22, uint8_t color = OSD_TEXT_SELECTED);

// Show text feedback on TV OSD row, then clear (blocking)
void OSD_showFeedback(uint8_t row, const char* text, uint8_t startPos, int iterations = 800);

// Convenience wrappers for common feedback types
void OSD_showLimitFeedback(uint8_t row, int iterations = 400);
void OSD_showOkFeedback(uint8_t row, int iterations = 800);
void OSD_showSavingFeedback(uint8_t row, uint8_t startPos = 19, int iterations = 800);

// Resolution/Input name helpers
const char* getOutputResolutionName(uint8_t presetID);
const char* getInputTypeName(uint8_t type);

