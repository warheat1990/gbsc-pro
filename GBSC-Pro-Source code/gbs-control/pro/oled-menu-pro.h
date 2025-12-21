// ====================================================================================
// oled-menu-pro.h
// OLED Menu Navigation and IR Remote Handling
//
// This file contains declarations for:
// - IR_handleMenuSelection(): Main menu state machine
// - IR_handleInput(): IR remote input handler
// - OLED menu initialization and handlers
// ====================================================================================

#pragma once

#include <Arduino.h>

// Forward declarations
class OLEDMenuManager;
struct OLEDMenuItem;
enum class OLEDMenuNav;

// ====================================================================================
// IR Remote Functions
// ====================================================================================

void IR_handleMenuSelection(void);
void IR_handleInput(void);

// ====================================================================================
// OLED Menu Initialization Functions
// ====================================================================================

void OLED_initInputMenu(OLEDMenuItem *root);
void OLED_initSettingsMenu(OLEDMenuItem *root);

// ====================================================================================
// OLED Menu Handler Functions
// ====================================================================================

bool OLED_handleInputSelection(OLEDMenuManager *manager, OLEDMenuItem *item, OLEDMenuNav nav, bool isFirstTime);
bool OLED_handleSettingSelection(OLEDMenuManager *manager, OLEDMenuItem *item, OLEDMenuNav nav, bool isFirstTime);
bool OLED_handleTvModeSelection(OLEDMenuManager *manager, OLEDMenuItem *item, OLEDMenuNav nav, bool isFirstTime);
