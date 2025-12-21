// ====================================================================================
// oled-menu-pro.cpp
// OLED Menu Navigation and IR Remote Handling
//
// This file contains:
// - IR_handleMenuSelection(): Main menu state machine for IR remote navigation
// - IR_handleInput(): IR remote input handler and decoder
// - IR_handle*(): Sub-handlers for each menu section (Output, Screen, Color, etc.)
// - OLED_init*Menu(): OLED menu initialization functions
// - OLED_handle*Selection(): OLED menu selection handlers
// ====================================================================================

// Modular menu handlers (includes all common headers)
#include "menu/menu-common.h"

// Additional headers not in menu-common.h
#include "oled-menu-pro.h"
#include "../OLEDMenuManager.h"
#include "../OLEDMenuImplementation.h"
#include "../tv5725.h"
#include "../ntsc_720x480.h"

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <SSD1306Wire.h>

#include "drivers/pt2257.h"

// ====================================================================================
// External References - gbs-control.ino
// ====================================================================================

typedef TV5725<GBS_ADDR> GBS;
extern SSD1306Wire display;
extern struct runTimeOptions *rto;
extern struct userOptions *uopt;
extern char serialCommand;
extern char userCommand;

extern uint8_t getVideoMode();
extern void applyPresets(uint8_t videoMode);
extern void shiftVerticalUpIF();
extern void shiftVerticalDownIF();
extern void saveUserPrefs();
extern void disableMotionAdaptDeinterlace();
extern void disableScanlines();
extern boolean areScanLinesAllowed();
extern float getOutputFrameRate();
extern void writeProgramArrayNew(const uint8_t *programArray, boolean skipMDSection);
extern void doPostPresetLoadSteps();
extern void freezeVideo();

// ====================================================================================
// External References - OLEDMenuImplementation.cpp
// ====================================================================================

extern OLEDMenuManager oledMenu;
extern unsigned long oledMenuFreezeStartTime;
extern unsigned long oledMenuFreezeTimeoutInMS;

// ====================================================================================
// IR Menu Handlers
// Now in separate files under menu/ directory:
// - menu/menu-output.cpp: IR_handleOutputResolution()
// - menu/menu-screen.cpp: IR_handleScreenSettings()
// - menu/menu-color.cpp: IR_handleColorSettings()
// - menu/menu-system.cpp: IR_handleSystemSettings()
// - menu/menu-input.cpp: IR_handleInputSelection()
// - menu/menu-profile.cpp: IR_handleProfileManagement()
// - menu/menu-main.cpp: IR_handleMainMenu()
// - menu/menu-misc.cpp: IR_handleMiscSettings(), IR_handleInfoDisplay()
// ====================================================================================
// ====================================================================================
// IR_handleMenuSelection - Menu State Machine
// ====================================================================================

// Check if the IR key is a valid menu navigation key
static bool IR_isValidMenuKey(uint32_t key)
{
    switch (key) {
        case IR_KEY_MENU:
        case IR_KEY_SAVE:
        case IR_KEY_INFO:
        case IR_KEY_RIGHT:
        case IR_KEY_LEFT:
        case IR_KEY_UP:
        case IR_KEY_DOWN:
        case IR_KEY_OK:
        case IR_KEY_EXIT:
        case IR_KEY_MUTE:
        case IR_KEY_VOL_UP:
        case IR_KEY_VOL_DN:
            return true;
        default:
            return false;
    }
}

// Get the user command for a given resolution
static char IR_getResolutionCommand(uint8_t resolution)
{
    switch (resolution) {
        case Output960P:  return 'f';  // 1280x960
        case Output720P:  return 'g';  // 1280x720
        case Output480P:  return 'h';  // 480p/576p
        case Output1024P: return 'p';  // 1280x1024
        case Output1080P: return 's';  // 1920x1080
        default:          return 'g';  // Default to 720p
    }
}

// Handle resolution confirmation countdown display
static void IR_updateResolutionCountdown(void)
{
    if ((millis() - lastResolutionTime) < OSD_RESOLUTION_UP_TIME ||
        oled_menuItem != OLED_RetainedSettings) {
        return;
    }

    lastMenuItemTime = millis();
    lastResolutionTime = millis();

    uint8_t secondsRemaining = OSD_RESOLUTION_CLOSE_TIME / 1000 -
                               ((lastResolutionTime - resolutionStartTime) / 1000);

    // Display countdown timer
    if (secondsRemaining >= 10) {
        OSD_writeCharAtRow(2, (secondsRemaining / 10) + '0', 11, OSD_TEXT_NORMAL);
        OSD_writeCharAtRow(2, (secondsRemaining % 10) + '0', 12, OSD_TEXT_NORMAL);
        OSD_writeStringAtRow(2, 14, " s ", OSD_TEXT_NORMAL);
    } else {
        OSD_writeCharAtRow(2, '0', 12, OSD_BACKGROUND);
        OSD_writeCharAtRow(2, secondsRemaining + '0', 11, OSD_TEXT_NORMAL);
        OSD_writeStringAtRow(2, 13, " s ", OSD_TEXT_NORMAL);
    }

    // Countdown expired - apply resolution
    if ((lastResolutionTime - resolutionStartTime) >= OSD_RESOLUTION_CLOSE_TIME) {
        userCommand = IR_getResolutionCommand(tentativeResolution);
        OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
        OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
        oled_menuItem = OLED_OutputResolution_PassThrough;
    }
}

// Handle menu timeout and cleanup
static void IR_handleMenuTimeout(void)
{
    // Track menu item changes
    if (lastOledMenuItem != oled_menuItem && oled_menuItem != OLED_None) {
        lastMenuItemTime = millis();
        oledClearFlag = 1;
    }

    // Check for menu timeout
    if ((millis() - lastMenuItemTime) >= OSD_CLOSE_TIME && oled_menuItem != OLED_None) {
        // Restore display settings if Info Display was active
        if (isInfoDisplayActive) {
            GBS::VDS_DIS_HB_ST::write(horizontalBlankStart);
            GBS::VDS_DIS_HB_SP::write(horizontalBlankStop);
            isInfoDisplayActive = 0;
        }

        // Close menu
        oled_menuItem = OLED_None;
        lastOledMenuItem = OLED_None;
        OSD_clearAll();
        OSD_init();
    }

    lastOledMenuItem = oled_menuItem;
}

void IR_handleMenuSelection(void)
{
    NEW_OLED_MENU = (oled_menuItem == OLED_None);

    // Dispatch to appropriate handler
    IR_handleOutputResolution() ||
    IR_handleScreenSettings() ||
    IR_handleColorSettings() ||
    IR_handleSystemSettings() ||
    IR_handleInputSelection() ||
    IR_handleProfileManagement() ||
    IR_handleMainMenu() ||
    IR_handleMiscSettings() ||
    IR_handleInfoDisplay();

    // Reset activity timer on valid IR input
    if (IR_isValidMenuKey(results.value) && irDecodedFlag && oled_menuItem != OLED_None) {
        lastMenuItemTime = millis();
        irDecodedFlag = 0;
        resetOLEDScreenSaverTimer();
    }

    // Handle resolution confirmation countdown
    IR_updateResolutionCountdown();

    // Handle menu timeout
    IR_handleMenuTimeout();
}

// ====================================================================================
// IR_handleInput - IR Remote Input Handler
// ====================================================================================

// Display mute status on OSD and OLED
static void IR_displayMuteStatus(bool muted)
{
    const char* statusText = muted ? "MUTE ON" : "MUTE OFF";

    NEW_OLED_MENU = false;
    OSD_fillRowBackground(ROW_1, 9, OSD_BACKGROUND);

    // Display on OSD
    for (int i = 0; i <= 800; i++) {
        OSD_writeStringAtRow(1, 1, "MUTE", OSD_TEXT_SELECTED);
        if (muted) {
            OSD_writeStringAtRow(1, 6, "ON ", OSD_TEXT_NORMAL);  // Extra space to clear "OFF"
        } else {
            OSD_writeStringAtRow(1, 6, "OFF", OSD_TEXT_NORMAL);
        }

        // Display on OLED
        display.clear();
        if (muted) {
            display.flipScreenVertically();
        }
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(8, 15, statusText);
        display.display();
    }

    oled_menuItem = OLED_None;
    OSD_fillRowBackground(ROW_1, 9, OSD_BACKGROUND);
    OSD_clearRowColors(ROW_1);
    OSD_init();
}

// Handle mute toggle
static void IR_handleMuteToggle(void)
{
    lastMenuItemTime = millis();

    if (audioMuted) {
        PT2257_mute(false);  // Unmute
        IR_displayMuteStatus(false);
        audioMuted = 0;
    } else {
        PT2257_mute(true);  // Mute
        IR_displayMuteStatus(true);
        audioMuted = 1;
    }
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
    OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
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

// Handle Volume keys - opens volume adjust menu
static void IR_handleVolumeKeyPress(void)
{
    lastMenuItemTime = millis();
    NEW_OLED_MENU = false;
    OSD_fillRowBackground(ROW_1, 25, OSD_BACKGROUND);
    oled_menuItem = OLED_Volume_Adjust;
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
            IR_handleVolumeKeyPress();
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
        "RGBs", "RGsB", "VGA", "YPBPR", "SV", "AV"
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

#ifdef ACE
    const char *settingLabels[6] = {
        "Smooth_Off", "Smooth_On ",
        "Compatibility_Off", "Compatibility_On ",
        "ACE_Off", "ACE_On "
    };
    uint8_t settingTags[6] = {
        MT_SMOOTH_OFF, MT_SMOOTH_ON,
        MT_COMPATIBILITY_OFF, MT_COMPATIBILITY_ON,
        MT_ACE_OFF, MT_ACE_ON
    };
    const size_t settingCount = 6;
#else
    const char *settingLabels[4] = {
        "Smooth_Off", "Smooth_On ",
        "Compatibility_Off", "Compatibility_On "
    };
    uint8_t settingTags[4] = {
        MT_SMOOTH_OFF, MT_SMOOTH_ON,
        MT_COMPATIBILITY_OFF, MT_COMPATIBILITY_ON
    };
    const size_t settingCount = 4;
#endif

    for (size_t i = 0; i < settingCount; ++i) {
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
    SETTING_PresetPreference preset = SETTING_PresetPreference::MT_7391_1X;

    switch (item->tag) {
        case MT_7391_1X:          preset = SETTING_PresetPreference::MT_7391_1X;          break;
        case MT_7391_2X:          preset = SETTING_PresetPreference::MT_7391_2X;          break;
        case MT_SMOOTH_OFF:       preset = SETTING_PresetPreference::MT_SMOOTH_OFF;       break;
        case MT_SMOOTH_ON:        preset = SETTING_PresetPreference::MT_SMOOTH_ON;        break;
        case MT_COMPATIBILITY_OFF: preset = SETTING_PresetPreference::MT_COMPATIBILITY_OFF; break;
        case MT_COMPATIBILITY_ON:  preset = SETTING_PresetPreference::MT_COMPATIBILITY_ON;  break;
#ifdef ACE
        case MT_ACE_OFF:          preset = SETTING_PresetPreference::MT_ACE_OFF;          break;
        case MT_ACE_ON:           preset = SETTING_PresetPreference::MT_ACE_ON;           break;
#endif
        default: break;
    }

    uopt->SETTING_presetPreference = preset;

    switch (preset) {
        case SETTING_PresetPreference::MT_7391_1X:
            ADV_sendLine1X();
            break;
        case SETTING_PresetPreference::MT_7391_2X:
            ADV_sendLine2X();
            break;
        case SETTING_PresetPreference::MT_SMOOTH_OFF:
            ADV_sendSmoothOff();
            break;
        case SETTING_PresetPreference::MT_SMOOTH_ON:
            ADV_sendSmoothOn();
            break;
        case SETTING_PresetPreference::MT_COMPATIBILITY_OFF:
            rgbComponentMode = COMPATIBILITY_OFF;
            ADV_sendCompatibility(rgbComponentMode);
            break;
        case SETTING_PresetPreference::MT_COMPATIBILITY_ON:
            rgbComponentMode = COMPATIBILITY_ON;
            ADV_sendCompatibility(rgbComponentMode);
            break;
#ifdef ACE
        case SETTING_PresetPreference::MT_ACE_OFF:
            {
                unsigned char Adv_ACE_OFF[7] = {0x41, 0x44, 'S', 0x81};
                Serial.write(Adv_ACE_OFF, 7);
            }
            break;
        case SETTING_PresetPreference::MT_ACE_ON:
            {
                unsigned char Adv_ACE_ON[7] = {0x41, 0x44, 'S', 0x80};
                Serial.write(Adv_ACE_ON, 7);
            }
            break;
#endif
        default:
            break;
    }

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
    if (inputType == InputTypeAV) {
        AVModeOption = 0;
        saveUserPrefs();
    } else if (inputType == InputTypeSV) {
        SVModeOption = 0;
        saveUserPrefs();
    }

    // Send TV mode command if in SV or AV mode
    if (inputType == InputTypeSV || inputType == InputTypeAV) {
        ADV_sendVideoFormat(ADV_VideoFormats[preset]);
    }

    return false;
}
