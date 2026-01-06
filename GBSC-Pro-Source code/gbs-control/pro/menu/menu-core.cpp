// ====================================================================================
// menu-core.cpp
// Menu Core - Navigation, Mapping, and IR Dispatch Infrastructure
//
// This file contains ONLY menu infrastructure:
// - oledToOsdMap[]: OLED state to OSD page mapping (X-macro generated)
// - Menu_navigateTo(): Navigation function
// - IR_handleMenuSelection(): Main IR dispatcher
// - IR_handleInput(): IR input handler entry point
//
// Helper functions are in: helpers/menu-helpers.cpp
// IR handlers are in: handlers/menu-*.cpp
// ====================================================================================

#include "menu-core.h"
#include "../../tv5725.h"
#include "../../ntsc_720x480.h"
#include "../../OLEDMenuManager.h"
#include "../../OLEDMenuImplementation.h"

#include <SSD1306Wire.h>

#include "../drivers/pt2257.h"

// ====================================================================================
// External References
// ====================================================================================

typedef TV5725<GBS_ADDR> GBS;

extern SSD1306Wire display;
extern struct runTimeOptions *rto;
extern struct userOptions *uopt;
extern char userCommand;
extern void saveUserPrefs();

extern void writeProgramArrayNew(const uint8_t *programArray, boolean skipMDSection);
extern void doPostPresetLoadSteps();
extern void freezeVideo();

extern OLEDMenuManager oledMenu;

// ====================================================================================
// Menu Item to OSD Mapping Table (generated from X-macro)
// ====================================================================================

// Stores in flash memory (PROGMEM) to save RAM on ESP8266
// Maps OLED_MenuState -> {OsdCommand pageCmd, uint8_t row}
const MenuItemMapping oledToOsdMap[] PROGMEM = {
    #define MENU_ITEM(oled, osd, row) {oled, osd, row},
    ALL_MAPPED_MENU_ITEMS
    #undef MENU_ITEM
};

const size_t oledToOsdMapSize = sizeof(oledToOsdMap) / sizeof(oledToOsdMap[0]);

// ====================================================================================
// Menu Navigation
// ====================================================================================

// Find mapping for menu item (returns nullptr if not found)
static const MenuItemMapping* findOledToOsdMapping(OLED_MenuState item) {
    for (size_t i = 0; i < oledToOsdMapSize; i++) {
        // Read from PROGMEM
        uint16_t mapItem = pgm_read_word(&oledToOsdMap[i].item);
        if (mapItem == (uint16_t)item) {
            return &oledToOsdMap[i];
        }
    }
    return nullptr;
}

void Menu_navigateTo(OLED_MenuState newItem) {
    OLED_MenuState oldItem = (OLED_MenuState)oled_menuItem;

    const MenuItemMapping* newMap = findOledToOsdMapping(newItem);
    if (!newMap) {
        // Item not mapped, just assign state
        oled_menuItem = newItem;
        return;
    }

    // Read new mapping from PROGMEM
    OsdCommand newPageCmd = (OsdCommand)pgm_read_byte(&newMap->pageCmd);
    uint8_t newRow = pgm_read_byte(&newMap->row);

    const MenuItemMapping* oldMap = findOledToOsdMapping(oldItem);
    bool pageChanged = true;
    if (oldMap) {
        OsdCommand oldPageCmd = (OsdCommand)pgm_read_byte(&oldMap->pageCmd);
        pageChanged = (oldPageCmd != newPageCmd);
    }

    if (pageChanged) {
        // Page change: fill background + render new page
        OSD_handleCommand((OsdCommand)(OSD_CMD_PAGE_CHANGE_ROW1 + (newRow - 1)));
        OSD_handleCommand(newPageCmd);
    } else {
        // Same page: just update row selection
        selectedMenuLine = newRow;
        OSD_handleCommand(newPageCmd);
    }

    oled_menuItem = newItem;
}

// ====================================================================================
// IR Menu Timeout Helper (private)
// ====================================================================================

// Handle menu timeout and cleanup
static void IR_handleMenuTimeout(void)
{
    // Track menu item changes
    if (lastOledMenuItem != oled_menuItem && oled_menuItem != OLED_None) {
        lastMenuItemTime = millis();
        oledClearFlag = 1;
    }

    // Check for menu timeout (use shorter timeout for mute/volume)
    unsigned long timeout = OSD_CLOSE_TIME;
    if (oled_menuItem == OLED_Mute_Display) {
        timeout = OSD_MUTE_CLOSE_TIME;
    } else if (oled_menuItem == OLED_Volume_Adjust) {
        timeout = OSD_VOLUME_CLOSE_TIME;
    }
    if ((millis() - lastMenuItemTime) >= timeout && oled_menuItem != OLED_None) {
        // Restore display settings if Info Display was active
        if (isInfoDisplayActive) {
            GBS::VDS_DIS_HB_ST::write(horizontalBlankStart);
            GBS::VDS_DIS_HB_SP::write(horizontalBlankStop);
            isInfoDisplayActive = 0;
        }

        // Close menu - use fillBackground for volume to avoid glitches
        int lastCmd = oled_menuItem;
        oled_menuItem = OLED_None;
        lastOledMenuItem = OLED_None;

        // Clear OSD using fillRowBackground to avoid glitches (OSD_clearAll causes visual artifacts)
        if (lastCmd == OLED_Mute_Display) {
            // Mute uses ROW_1 only (9 chars)
            OSD_fillRowBackground(ROW_1, 9, OSD_BACKGROUND);
            OSD_clearRowColors(ROW_1);
        } else if (lastCmd == OLED_Volume_Adjust) {
            // Volume uses ROW_1 only (25 chars)
            OSD_fillRowBackground(ROW_1, 25, OSD_BACKGROUND);
            OSD_clearRowColors(ROW_1);
        } else if (lastCmd == OLED_Info_Display) {
            // Info display uses ROW_1 and ROW_2 (28 chars each)
            OSD_fillRowBackground(ROW_1, 28, OSD_BACKGROUND);
            OSD_fillRowBackground(ROW_2, 28, OSD_BACKGROUND);
            OSD_clearRowColors(ROW_1);
            OSD_clearRowColors(ROW_2);
        } else {
            // Standard menus use all 3 rows (28 chars each)
            OSD_fillRowBackground(ROW_1, 28, OSD_BACKGROUND);
            OSD_fillRowBackground(ROW_2, 28, OSD_BACKGROUND);
            OSD_fillRowBackground(ROW_3, 28, OSD_BACKGROUND);
            OSD_clearRowColors(ROW_1);
            OSD_clearRowColors(ROW_2);
            OSD_clearRowColors(ROW_3);
        }
        OSD_init();
    }

    lastOledMenuItem = oled_menuItem;
}

// ====================================================================================
// IR_handleMenuSelection - Menu State Machine
// ====================================================================================

void IR_handleMenuSelection(void)
{
    NEW_OLED_MENU = (oled_menuItem == OLED_None);

    // Dispatch to appropriate handler
    IR_handleOutputResolution() ||
    IR_handleScreenSettings() ||
    IR_handleColorSettings() ||
    IR_handleSystemSettings() ||
    IR_handleADVSettings() ||
    IR_handleACESettings() ||
    IR_handleVideoFiltersSettings() ||
    IR_handlePreferencesMenu() ||
    IR_handleDeveloperMenu() ||
    IR_handleInputSelection() ||
    IR_handleProfileManagement() ||
    IR_handleMainMenu() ||
    IR_handleMuteDisplay() ||
    IR_handleMiscSettings() ||
    IR_handleInfoDisplay();

    // Reset activity timer on valid IR input
    if (IR_isValidMenuKey(results.value) && irDecodedFlag && oled_menuItem != OLED_None) {
        lastMenuItemTime = millis();
        irDecodedFlag = 0;
        resetOLEDScreenSaverTimer();
    }

    // Handle menu timeout
    IR_handleMenuTimeout();
}

// ====================================================================================
// IR_handleInput - IR Remote Input Handler
// ====================================================================================

// Display mute status on OLED display only
static void IR_displayMuteOnOLED(bool muted)
{
    const char* statusText = muted ? "MUTE ON" : "MUTE OFF";

    display.clear();
    if (muted) {
        display.flipScreenVertically();
    }
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
    display.drawString(8, 15, statusText);
    display.display();
}

// Handle mute toggle
static void IR_handleMuteToggle(void)
{
    lastMenuItemTime = millis();
    NEW_OLED_MENU = false;

    // Toggle mute state
    if (uopt->audioMuted) {
        PT2257_mute(false);  // Unmute
        uopt->audioMuted = 0;
    } else {
        PT2257_mute(true);  // Mute
        uopt->audioMuted = 1;
    }

    // Display on OLED
    IR_displayMuteOnOLED(uopt->audioMuted);

    // Set up OSD display with timeout (like volume)
    OSD_fillRowBackground(ROW_1, 10, OSD_BACKGROUND);
    oled_menuItem = OLED_Mute_Display;
}

// Handle Menu key press - opens main menu or info display
static void IR_handleMenuKeyPress(void)
{
    lastMenuItemTime = millis();
    NEW_OLED_MENU = false;

    // Check if source is disconnected or board has no power
    bool noSignal = rto->sourceDisconnected ||
                    !rto->boardHasPower ||
                    GBS::PAD_CKIN_ENZ::read();

    if (noSignal) {
        // Show info display when no signal
        OSD_fillRowBackground(ROW_1, 28, OSD_BACKGROUND);
        OSD_fillRowBackground(ROW_2, 28, OSD_BACKGROUND);
        oled_menuItem = OLED_Info_Display;

        // Save horizontal blank values before modifying
        isInfoDisplayActive = 1;
        horizontalBlankStart = GBS::VDS_DIS_HB_ST::read();
        horizontalBlankStop = GBS::VDS_DIS_HB_SP::read();

        // Initialize display for info
        writeProgramArrayNew(ntsc_720x480, false);
        doPostPresetLoadSteps();
        GBS::VDS_DIS_HB_ST::write(0x00);
        GBS::VDS_DIS_HB_SP::write(0xffff);
        freezeVideo();
        GBS::SP_CLAMP_MANUAL::write(1);
    } else {
        // Open main input menu
        selectedMenuLine = 1;
        OSD_handleCommand(OSD_CMD_INIT);
        OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
        OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
        oled_menuItem = OLED_Input;
        display.clear();
    }
}

// Handle Save key press - opens profile menu
static void IR_handleSaveKeyPress(void)
{
    lastMenuItemTime = millis();
    NEW_OLED_MENU = false;
    OSD_handleCommand(OSD_CMD_INIT);
    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
    OSD_handleCommand(OSD_CMD_PROFILE_SAVELOAD);
    oled_menuItem = OLED_Profile_Load1;
}

// Handle Info key press - opens info display
static void IR_handleInfoKeyPress(void)
{
    lastMenuItemTime = millis();
    NEW_OLED_MENU = false;
    OSD_fillRowBackground(ROW_1, 28, OSD_BACKGROUND);
    OSD_fillRowBackground(ROW_2, 28, OSD_BACKGROUND);
    oled_menuItem = OLED_Info_Display;
}

// Handle Volume keys - opens volume adjust menu and applies first change
static void IR_handleVolumeKeyPress(uint32_t key)
{
    lastMenuItemTime = millis();
    NEW_OLED_MENU = false;
    OSD_fillRowBackground(ROW_1, 23, OSD_BACKGROUND);
    oled_menuItem = OLED_Volume_Adjust;

    // Set initial key and apply first volume change immediately
    Volume_setInitialKey(key);
}

void IR_handleInput()
{
    if (!irDecode()) {
        return;
    }

    switch (results.value) {
        case IR_KEY_MENU:
            IR_handleMenuKeyPress();
            break;
        case IR_KEY_SAVE:
            IR_handleSaveKeyPress();
            break;
        case IR_KEY_INFO:
            IR_handleInfoKeyPress();
            break;
        case IR_KEY_MUTE:
            IR_handleMuteToggle();
            break;
        case IR_KEY_VOL_UP:
        case IR_KEY_VOL_DN:
            IR_handleVolumeKeyPress(results.value);
            break;
    }

    irResume();
    delay(5);
}

// ====================================================================================
// OLED Menu Initialization - Input Menu
// ====================================================================================

void OLED_initInputMenu(OLEDMenuItem *root) {
    OLEDMenuItem *advMenu = oledMenu.registerItem(root, MT_NULL, IMAGE_ITEM(OM_ADVINPUT));

    const char *inputLabels[6] = {
        "RGBs", "RGsB", "VGA", "YPbPr", "SV", "AV"
    };

    uint8_t inputTags[6] = {
        MT_RGBs, MT_RGsB, MT_VGA, MT_YPBPR, MT_SV, MT_AV
    };

    for (size_t i = 0; i < 6; ++i) {
        oledMenu.registerItem(advMenu, inputTags[i], inputLabels[i], OLED_handleInputSelection);
    }
}

// ====================================================================================
// OLED Menu Initialization - Settings Menu
// ====================================================================================

void OLED_initSettingsMenu(OLEDMenuItem *root) {
    OLEDMenuItem *settingMenu = oledMenu.registerItem(root, MT_NULL, IMAGE_ITEM(OM_SETTING));

    const char *settingLabels[8] = {
        "I2P_Off", "I2P_On ",
        "Smooth_Off", "Smooth_On ",
        "Compatibility_Off", "Compatibility_On ",
        "ACE_Off", "ACE_On "
    };
    uint8_t settingTags[8] = {
        MT_I2P_OFF, MT_I2P_ON,
        MT_SMOOTH_OFF, MT_SMOOTH_ON,
        MT_COMPATIBILITY_OFF, MT_COMPATIBILITY_ON,
        MT_ACE_OFF, MT_ACE_ON
    };

    for (size_t i = 0; i < 8; ++i) {
        oledMenu.registerItem(settingMenu, settingTags[i], settingLabels[i], OLED_handleSettingSelection);
    }

    // TV Mode submenu
    OLEDMenuItem *tvModeMenu = oledMenu.registerItem(settingMenu, MT_NULL, IMAGE_ITEM(OM_TVMODE));

    const char *tvModeLabels[12] = {
        "AUTO", "PAL", "NTSC-M", "PAL-60", "NTSC443", "NTSC-J",
        "PAL-N w/ p", "PAL-M w/o p", "PAL-M", "PAL Cmb -N", "PAL Cmb -N w/ p", "SECAM"
    };

    uint8_t tvModeTags[12] = {
        MT_MODE_AUTO, MT_MODE_PAL, MT_MODE_NTSCM, MT_MODE_PAL60,
        MT_MODE_NTSC443, MT_MODE_NTSCJ, MT_MODE_PALNwp, MT_MODE_PALMwop,
        MT_MODE_PALM, MT_MODE_PALCmbN, MT_MODE_PALCmbNwp, MT_MODE_SECAM
    };

    for (size_t i = 0; i < 12; ++i) {
        oledMenu.registerItem(tvModeMenu, tvModeTags[i], tvModeLabels[i], OLED_handleTvModeSelection);
    }
}

// ====================================================================================
// OLED Menu Handlers - Input Selection
// ====================================================================================

bool OLED_handleInputSelection(OLEDMenuManager *manager, OLEDMenuItem *item, OLEDMenuNav, bool isFirstTime) {
    if (!isFirstTime) {
        checkFreezeTimeout(manager);
        return false;
    }

    showSelectionFeedback(manager, item->str);

    // Apply input selection
    INPUT_PresetPreference preset = INPUT_PresetPreference::MT_RGBs;

    switch (item->tag) {
        case MT_RGBs:  preset = INPUT_PresetPreference::MT_RGBs;  break;
        case MT_RGsB:  preset = INPUT_PresetPreference::MT_RGsB;  break;
        case MT_VGA:   preset = INPUT_PresetPreference::MT_VGA;   break;
        case MT_YPBPR: preset = INPUT_PresetPreference::MT_YPBPR; break;
        case MT_SV:    preset = INPUT_PresetPreference::MT_SV;    break;
        case MT_AV:    preset = INPUT_PresetPreference::MT_AV;    break;
        default: break;
    }

    uopt->INPUT_presetPreference = preset;

    switch (preset) {
        case INPUT_PresetPreference::MT_RGBs:  InputRGBs(); break;
        case INPUT_PresetPreference::MT_RGsB:  InputRGsB(); break;
        case INPUT_PresetPreference::MT_VGA:   InputVGA();  break;
        case INPUT_PresetPreference::MT_YPBPR: InputYUV();  break;
        case INPUT_PresetPreference::MT_SV:    InputSV();   break;
        case INPUT_PresetPreference::MT_AV:    InputAV();   break;
        default: break;
    }

    return false;
}

// ====================================================================================
// OLED Menu Handlers - Settings Selection
// ====================================================================================

bool OLED_handleSettingSelection(OLEDMenuManager *manager, OLEDMenuItem *item, OLEDMenuNav, bool isFirstTime) {
    if (!isFirstTime) {
        checkFreezeTimeout(manager);
        return false;
    }

    showSelectionFeedback(manager, item->str);

    // Apply setting
    SETTING_PresetPreference preset = SETTING_PresetPreference::MT_I2P_OFF;

    switch (item->tag) {
        case MT_I2P_OFF:           preset = SETTING_PresetPreference::MT_I2P_OFF;           break;
        case MT_I2P_ON:            preset = SETTING_PresetPreference::MT_I2P_ON;            break;
        case MT_SMOOTH_OFF:        preset = SETTING_PresetPreference::MT_SMOOTH_OFF;        break;
        case MT_SMOOTH_ON:         preset = SETTING_PresetPreference::MT_SMOOTH_ON;         break;
        case MT_COMPATIBILITY_OFF: preset = SETTING_PresetPreference::MT_COMPATIBILITY_OFF; break;
        case MT_COMPATIBILITY_ON:  preset = SETTING_PresetPreference::MT_COMPATIBILITY_ON;  break;
        case MT_ACE_OFF:           preset = SETTING_PresetPreference::MT_ACE_OFF;           break;
        case MT_ACE_ON:            preset = SETTING_PresetPreference::MT_ACE_ON;            break;
        default: break;
    }

    uopt->SETTING_presetPreference = preset;

    switch (preset) {
        case SETTING_PresetPreference::MT_I2P_OFF:
            uopt->advI2P = 0;
            uopt->advSmooth = 0;  // Smooth requires I2P
            ADV_sendI2P(false);
            break;
        case SETTING_PresetPreference::MT_I2P_ON:
            uopt->advI2P = 1;
            ADV_sendI2P(true);
            break;
        case SETTING_PresetPreference::MT_SMOOTH_OFF:
            uopt->advSmooth = 0;
            ADV_sendSmooth(false);
            break;
        case SETTING_PresetPreference::MT_SMOOTH_ON:
            if (uopt->advI2P) {  // Smooth only works with I2P enabled
                uopt->advSmooth = 1;
                ADV_sendSmooth(true);
            }
            break;
        case SETTING_PresetPreference::MT_COMPATIBILITY_OFF:
            uopt->advCompatibility = 0;
            ADV_sendCompatibility(uopt->advCompatibility);
            break;
        case SETTING_PresetPreference::MT_COMPATIBILITY_ON:
            uopt->advCompatibility = 1;
            ADV_sendCompatibility(uopt->advCompatibility);
            break;
        case SETTING_PresetPreference::MT_ACE_OFF:
            uopt->advACE = 0;
            ADV_sendACE(false);
            break;
        case SETTING_PresetPreference::MT_ACE_ON:
            uopt->advACE = 1;
            ADV_sendACE(true);
            break;
        default:
            break;
    }

    saveUserPrefs();
    return false;
}

// ====================================================================================
// OLED Menu Handlers - TV Mode Selection
// ====================================================================================

bool OLED_handleTvModeSelection(OLEDMenuManager *manager, OLEDMenuItem *item, OLEDMenuNav, bool isFirstTime) {
    if (!isFirstTime) {
        checkFreezeTimeout(manager);
        return false;
    }

    showSelectionFeedback(manager, item->str);

    // Apply TV mode selection
    TVMODE_PresetPreference preset = TVMODE_PresetPreference::MT_MODE_AUTO;

    switch (item->tag) {
        case MT_MODE_AUTO:      preset = TVMODE_PresetPreference::MT_MODE_AUTO;      break;
        case MT_MODE_PAL:       preset = TVMODE_PresetPreference::MT_MODE_PAL;       break;
        case MT_MODE_NTSCM:     preset = TVMODE_PresetPreference::MT_MODE_NTSCM;     break;
        case MT_MODE_PAL60:     preset = TVMODE_PresetPreference::MT_MODE_PAL60;     break;
        case MT_MODE_NTSC443:   preset = TVMODE_PresetPreference::MT_MODE_NTSC443;   break;
        case MT_MODE_NTSCJ:     preset = TVMODE_PresetPreference::MT_MODE_NTSCJ;     break;
        case MT_MODE_PALNwp:    preset = TVMODE_PresetPreference::MT_MODE_PALNwp;    break;
        case MT_MODE_PALMwop:   preset = TVMODE_PresetPreference::MT_MODE_PALMwop;   break;
        case MT_MODE_PALM:      preset = TVMODE_PresetPreference::MT_MODE_PALM;      break;
        case MT_MODE_PALCmbN:   preset = TVMODE_PresetPreference::MT_MODE_PALCmbN;   break;
        case MT_MODE_PALCmbNwp: preset = TVMODE_PresetPreference::MT_MODE_PALCmbNwp; break;
        case MT_MODE_SECAM:     preset = TVMODE_PresetPreference::MT_MODE_SECAM;     break;
        default: break;
    }

    uopt->TVMODE_presetPreference = preset;

    // Update AV/SV mode options if applicable
    if (uopt->activeInputType == InputTypeAV) {
        uopt->avVideoFormat = 0;
        saveUserPrefs();
    } else if (uopt->activeInputType == InputTypeSV) {
        uopt->svVideoFormat = 0;
        saveUserPrefs();
    }

    // Send TV mode command if in SV or AV mode
    if (uopt->activeInputType == InputTypeSV || uopt->activeInputType == InputTypeAV) {
        ADV_sendVideoFormat(ADV_VideoFormats[preset]);
    }

    return false;
}
