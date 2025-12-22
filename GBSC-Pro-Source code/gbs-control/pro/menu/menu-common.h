// ====================================================================================
// menu-common.h
// Shared helper functions for IR menu handlers
// ====================================================================================

#pragma once

#include "../osd/osd-common.h"   // For OSD helper functions (includes gbs-control-pro.h, stv9426.h, options.h)
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
// IR Menu Handler Function Declarations
// ====================================================================================

bool IR_handleMainMenu();
bool IR_handleInputSelection();
bool IR_handleOutputResolution();
bool IR_handleScreenSettings();
bool IR_handleColorSettings();
bool IR_handleSystemSettings();
bool IR_handleProfileManagement();
bool IR_handleMiscSettings();
bool IR_handleInfoDisplay();

// ====================================================================================
// Main IR/Menu Dispatchers and OLED Handlers
// (Declared in oled-menu-pro.h, included via gbs-control-pro.h)
// ====================================================================================
