// ====================================================================================
// osd-preferences.cpp
// TV OSD Handlers for Preferences Menu (Theme, Volume, Mute)
// ====================================================================================

#include "../osd-core.h"

// ====================================================================================
// External References
// ====================================================================================

extern uint8_t volume;
extern boolean audioMuted;

// ====================================================================================
// Preferences Menu - Page 1 (Theme, Volume, Mute)
// ====================================================================================

void handle_Preferences_Page1(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(false, '1', false);
    OSD_writeStringAtRow(1, 1, "Theme");
    OSD_drawDashRange(1, 6, 18);
    OSD_writeStringAtRow(2, 1, "Volume");
    OSD_drawDashRange(2, 7, 23);
    OSD_writeStringAtRow(3, 1, "Mute");
    OSD_drawDashRange(3, 5, 22);
}

void handle_Preferences_Page1_Values(void)
{
    // Theme value (right-aligned with padding)
    uint8_t themeId = OSD_getTheme();
    switch (themeId) {
        case OSD_THEME_CLASSIC: OSD_writeStringAtRow(1, 19, "Classic"); break;
        case OSD_THEME_DARK:    OSD_writeStringAtRow(1, 19, "---Dark"); break;
        case OSD_THEME_LIGHT:   OSD_writeStringAtRow(1, 19, "--Light"); break;
        case OSD_THEME_RETRO:   OSD_writeStringAtRow(1, 19, "--Retro"); break;
        default:                OSD_writeStringAtRow(1, 19, "Unknown"); break;
    }

    // Volume value (0-50 displayed as 2-digit number)
    uint8_t displayVolume = 50 - volume;  // 0=max(50), 50=min(0)
    OSD_displayNumber2DigitAtRow(2, displayVolume, 25, 24);

    // Mute status (ON/OFF)
    OSD_writeOnOff(3, audioMuted);
}
