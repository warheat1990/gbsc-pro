/**
 * Virtual Preset Menu for GBSC-Pro
 *
 * This implements a memory-efficient virtual menu for preset selection.
 * Instead of allocating 36 OLEDMenuItem objects (36 × 176 = 6,336 bytes),
 * it uses a compact PresetEntry array (36 × 26 = 936 bytes) and handles
 * navigation internally.
 */
#ifndef MENU_PRESETS_H_
#define MENU_PRESETS_H_

#include "../../OLEDMenuManager.h"

// Virtual menu handler for presets - handles all navigation internally
bool presetsVirtualMenuHandler(OLEDMenuManager *manager, OLEDMenuItem *item, OLEDMenuNav nav, bool isFirstTime);

#endif // MENU_PRESETS_H_
