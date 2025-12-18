// ====================================================================================
// OLEDMenuImplementation-pro.h
// GBSC-Pro OLED Menu Handlers
//
// This file contains Pro-specific declarations for:
// - OLED menu initialization
// - OLED menu handlers
//
// Note: Constants and core functions are in gbs-control-pro.h
// ====================================================================================

#ifndef OLED_MENU_IMPLEMENTATION_PRO_H_
#define OLED_MENU_IMPLEMENTATION_PRO_H_

#include "OLEDMenuManager.h"

// ====================================================================================
// Menu Initialization Functions
// ====================================================================================

void OLED_initInputMenu(OLEDMenuItem *root);
void OLED_initSettingsMenu(OLEDMenuItem *root);

// ====================================================================================
// Menu Handler Functions
// ====================================================================================

bool OLED_handleInputSelection(OLEDMenuManager *manager, OLEDMenuItem *item, OLEDMenuNav nav, bool isFirstTime);
bool OLED_handleSettingSelection(OLEDMenuManager *manager, OLEDMenuItem *item, OLEDMenuNav nav, bool isFirstTime);
bool OLED_handleTvModeSelection(OLEDMenuManager *manager, OLEDMenuItem *item, OLEDMenuNav nav, bool isFirstTime);

#endif // OLED_MENU_IMPLEMENTATION_PRO_H_
