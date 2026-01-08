// ====================================================================================
// menu-core.h
// Menu Core - Helper Functions, Navigation, and Dispatch
//
// This file contains:
// - OLED display helper function declarations
// - IR receiver helper declarations
// - Profile navigation helpers
//
// Types, enums, and handler declarations are in menu-registry.h
// ====================================================================================

#pragma once

#include "menu-registry.h"        // Types, enums, handler declarations
#include "../osd/osd-core.h"      // For OSD helper functions (includes osd-registry.h)
#include "../drivers/ir_remote.h" // For IR keys

// Forward declarations
class SSD1306Wire;
class OLEDMenuManager;
struct OLEDMenuItem;
enum class OLEDMenuNav;

// ====================================================================================
// OLED Display Helpers
// ====================================================================================

// Prepare OLED display for drawing (clear if needed, set defaults)
void oledPrepare();

// Display menu item on OLED (title at top, label centered)
void showMenu(const char* title, const char* label);

// Display single centered text on OLED
void showMenuCentered(const char* text, uint8_t x = 8, uint8_t y = 15);

// Display menu with ON/OFF toggle
void showMenuToggle(const char* title, const char* label, bool isOn);

// Display menu with custom value text
void showMenuValue(const char* title, const char* label, const char* value);

// Exit menu completely
void exitMenu();

// ====================================================================================
// IR Receiver Helpers
// ====================================================================================

// Decode IR input and set flag
bool irDecode();

// Resume IR receiver after handling
void irResume();

// ====================================================================================
// OLED Menu Feedback Helpers
// ====================================================================================

// Show selection feedback on OLED with "Loaded" indicator, then freeze menu
void showSelectionFeedback(OLEDMenuManager *manager, const char* label);

// Check if freeze timeout expired and unfreeze if so
bool checkFreezeTimeout(OLEDMenuManager *manager);

// ====================================================================================
// Profile Navigation Helpers
// ====================================================================================

// Check if state is in Load range (Load1-Load20), return index 0-19 or -1
int getLoadSlotIndex(int state);

// Check if state is in Save range (Save1-Save20), return index 0-19 or -1
int getSaveSlotIndex(int state);

// Circular navigation: next slot (wraps 19 -> 0)
OLED_MenuState getNextSlot(OLED_MenuState base, int idx);

// Circular navigation: prev slot (wraps 0 -> 19)
OLED_MenuState getPrevSlot(OLED_MenuState base, int idx);

// Get slot character 'A'-'T' from index 0-19
char getSlotChar(int idx);

// Handle profile row navigation (unified for Load and Save rows)
bool handleProfileRow(bool isLoadRow);

// ====================================================================================
// IR Key Helpers
// ====================================================================================

// Check if IR key is a valid menu navigation key
bool IR_isValidMenuKey(uint32_t key);

// Get user command character for resolution
char IR_getResolutionCommand(uint8_t resolution);

// ====================================================================================
// IR Key Repeat Helper
// ====================================================================================

// Default repeat interval for value adjustment menus (ms)
#define IR_REPEAT_INTERVAL_MS 125

// Process IR input with key repeat support using shared global state.
// Use this for menus where user holds a key to adjust values (color, screen, etc.)
// Call after irDecode() returns true. Returns key to process, or 0 if repeat ignored.
// Usage:
//   if (irDecode()) {
//       uint32_t key = IR_getKeyRepeat();
//       if (key) { /* process key */ }
//       irResume();
//   }
uint32_t IR_getKeyRepeat(void);

// Clear the stored repeat key state (call when navigating away or on non-repeat keys)
void IR_clearRepeatKey(void);

