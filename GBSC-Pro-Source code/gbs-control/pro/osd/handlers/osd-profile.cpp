// ====================================================================================
// osd-profile.cpp
// TV OSD Handlers for Profile Management
// ====================================================================================

#include "../osd-core.h"

// ====================================================================================
// External References
// ====================================================================================

extern String slotIndexMap;
extern bool loadSlotSettings();

// ====================================================================================
// Profile Handlers
// ====================================================================================

void handle_Profile_SaveLoad(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writeStringAtRow(1, 1, "Loadprofile:");
    OSD_writeStringAtRow(2, 1, "Saveprofile:");
    OSD_writeStringAtRow(3, 1, "Active save:", OSD_TEXT_SELECTED);
}

void handle_Profile_SlotDisplay(void)
{
    // Row 1: Load profile display (Load1-Load36 → index 1-36)
    int loadIdx = oled_menuItem - OLED_Profile_Load1;
    if (loadIdx >= 0 && loadIdx < 36) {
        displayProfileName(1, loadIdx + 1);
    }

    // Row 3: Active save slot (presetSlot mapped via slotIndexMap)
    int activeIdx = slotIndexMap.indexOf(uopt->presetSlot);
    if (activeIdx >= 0 && activeIdx < 36) {
        displayProfileName(3, activeIdx + 1, OSD_TEXT_SELECTED);
    }

    // Row 2: Save profile display (Save1-Save36 → index 1-36)
    int saveIdx = oled_menuItem - OLED_Profile_Save1;
    if (saveIdx >= 0 && saveIdx < 36) {
        displayProfileName(2, saveIdx + 1);
    }
}

void handle_Profile_SlotRow1(void)
{
    uopt->presetPreference = OutputCustomized;

    // Load slot settings and apply preset (doPostPresetLoadSteps applies ADV/GBS colors)
    loadSlotSettings();
    applyPresets(rto->videoStandardInput);
    saveUserPrefs();
}
