// ====================================================================================
// OLEDMenuImplementation-pro.cpp
// GBSC-Pro OLED Menu Handlers
//
// This file contains Pro-specific OLED menu implementations:
// - Menu initialization (Input, Settings, TV Mode)
// - Menu handlers (OLED_handleInputSelection, OLED_handleSettingSelection, OLED_handleTvModeSelection)
//
// Note: ADV communication and input switching functions are in gbs-control-pro.cpp
// ====================================================================================

#include "OLEDMenuImplementation-pro.h"
#include "OLEDMenuImplementation.h"
#include "gbs-control-pro.h"
#include "options-pro.h"
#include "options.h"
#include "tv5725.h"
#include <Arduino.h>

// ====================================================================================
// External References - gbs-control.ino
// ====================================================================================

typedef TV5725<GBS_ADDR> GBS;
extern struct runTimeOptions *rto;
extern struct userOptions *uopt;
extern void saveUserPrefs();

// ====================================================================================
// External References - OLEDMenuImplementation.cpp
// ====================================================================================

extern OLEDMenuManager oledMenu;
extern unsigned long oledMenuFreezeStartTime;
extern unsigned long oledMenuFreezeTimeoutInMS;

// ====================================================================================
// Menu Initialization - Input Menu
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
// Menu Initialization - Settings Menu
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
// Menu Handlers - Input Selection Handler
// ====================================================================================

bool OLED_handleInputSelection(OLEDMenuManager *manager, OLEDMenuItem *item, OLEDMenuNav, bool isFirstTime) {
    if (!isFirstTime) {
        if (millis() - oledMenuFreezeStartTime >= oledMenuFreezeTimeoutInMS) {
            manager->unfreeze();
        }
        return false;
    }

    // Show selection feedback
    oledMenuFreezeTimeoutInMS = 1000;
    oledMenuFreezeStartTime = millis();
    OLEDDisplay *display = manager->getDisplay();
    display->clear();
    display->setColor(OLEDDISPLAY_COLOR::WHITE);
    display->setFont(ArialMT_Plain_16);
    display->setTextAlignment(OLEDDISPLAY_TEXT_ALIGNMENT::TEXT_ALIGN_CENTER);
    display->drawString(OLED_MENU_WIDTH / 2, 16, item->str);
    display->drawXbm((OLED_MENU_WIDTH - TEXT_LOADED_WIDTH) / 2, OLED_MENU_HEIGHT / 2, IMAGE_ITEM(TEXT_LOADED));
    display->display();

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

    manager->freeze();
    return false;
}

// ====================================================================================
// Menu Handlers - Settings Handler
// ====================================================================================

bool OLED_handleSettingSelection(OLEDMenuManager *manager, OLEDMenuItem *item, OLEDMenuNav, bool isFirstTime) {
    if (!isFirstTime) {
        if (millis() - oledMenuFreezeStartTime >= oledMenuFreezeTimeoutInMS) {
            manager->unfreeze();
        }
        return false;
    }

    // Show selection feedback
    oledMenuFreezeTimeoutInMS = 1000;
    oledMenuFreezeStartTime = millis();
    OLEDDisplay *display = manager->getDisplay();
    display->clear();
    display->setColor(OLEDDISPLAY_COLOR::WHITE);
    display->setFont(ArialMT_Plain_16);
    display->setTextAlignment(OLEDDISPLAY_TEXT_ALIGNMENT::TEXT_ALIGN_CENTER);
    display->drawString(OLED_MENU_WIDTH / 2, 16, item->str);
    display->drawXbm((OLED_MENU_WIDTH - TEXT_LOADED_WIDTH) / 2, OLED_MENU_HEIGHT / 2, IMAGE_ITEM(TEXT_LOADED));
    display->display();

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
                // Send via wrapper - but ACE packets are local
                // For now, define locally and use direct serial
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

    manager->freeze();
    return false;
}

// ====================================================================================
// Menu Handlers - TV Mode Handler
// ====================================================================================

bool OLED_handleTvModeSelection(OLEDMenuManager *manager, OLEDMenuItem *item, OLEDMenuNav, bool isFirstTime) {
    if (!isFirstTime) {
        if (millis() - oledMenuFreezeStartTime >= oledMenuFreezeTimeoutInMS) {
            manager->unfreeze();
        }
        return false;
    }

    // Show selection feedback
    oledMenuFreezeTimeoutInMS = 1000;
    oledMenuFreezeStartTime = millis();
    OLEDDisplay *display = manager->getDisplay();
    display->clear();
    display->setColor(OLEDDISPLAY_COLOR::WHITE);
    display->setFont(ArialMT_Plain_16);
    display->setTextAlignment(OLEDDISPLAY_TEXT_ALIGNMENT::TEXT_ALIGN_CENTER);
    display->drawString(OLED_MENU_WIDTH / 2, 16, item->str);
    display->drawXbm((OLED_MENU_WIDTH - TEXT_LOADED_WIDTH) / 2, OLED_MENU_HEIGHT / 2, IMAGE_ITEM(TEXT_LOADED));
    display->display();

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

    manager->freeze();
    return false;
}
