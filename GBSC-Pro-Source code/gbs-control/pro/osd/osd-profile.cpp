// ====================================================================================
// osd-profile.cpp
// TV OSD Handlers for Profile Management
// ====================================================================================

#include "osd-common.h"

// ====================================================================================
// Profile Handlers
// ====================================================================================

void handle_Profile_SaveLoad(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writeStringAtRow(1, 1, "Loadprofile:", OSD_getMenuLineColor(1));
    OSD_writeStringAtRow(2, 1, "Saveprofile:", OSD_getMenuLineColor(2));
    OSD_writeStringAtRow(3, 1, "Active save:", OSD_TEXT_SELECTED);
}

void handle_Profile_SlotDisplay(void)
{
    // Row 1: Load profile display (Load1-Load20 → index 1-20)
    int loadIdx = oled_menuItem - OLED_Profile_Load1;
    if (loadIdx >= 0 && loadIdx < 20) {
        setProfileName(loadIdx + 1);
        displayProfileName(1, OSD_TEXT_NORMAL);
    }

    // Row 3: Active save slot (presetSlot 'A'-'T' → index 1-20)
    if (uopt->presetSlot >= 'A' && uopt->presetSlot <= 'T') {
        setProfileName(uopt->presetSlot - 'A' + 1);
        displayProfileName(3, OSD_TEXT_SELECTED);
    }

    // Row 2: Save profile display (Save1-Save20 → index 1-20)
    int saveIdx = oled_menuItem - OLED_Profile_Save1;
    if (saveIdx >= 0 && saveIdx < 20) {
        setProfileName(saveIdx + 1);
        displayProfileName(2, OSD_TEXT_NORMAL);
    }
}

void handle_Profile_SlotRow1(void)
{
    uopt->presetPreference = OutputCustomized;
    saveUserPrefs();
    uopt->presetPreference = OutputCustomized;

    if (rto->videoStandardInput == 14) {
        rto->videoStandardInput = 15;
    } else {
        applyPresets(rto->videoStandardInput);
    }

    saveUserPrefs();
}
