// ====================================================================================
// OLEDMenuImplementation-pro.cpp
// GBSC-Pro Extensions for OLEDMenuImplementation
//
// This file contains Pro-specific implementations for:
// - HC32 controller communication (PacketSender class)
// - ADV7280/ADV7391 processor control
// - Input source switching
// - Pro-specific OLED menu handlers
// ====================================================================================

#include "OLEDMenuImplementation-pro.h"
#include "OLEDMenuImplementation.h"
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
extern void resetSyncProcessor();

// ====================================================================================
// External References - gbs-control-pro.cpp
// ====================================================================================

extern uint8_t rgbComponentMode;
extern uint8_t selectedInputSource;
extern uint8_t brightnessOrContrastOption;
extern uint8_t Info;

// ====================================================================================
// External References - OLEDMenuImplementation.cpp
// ====================================================================================

extern OLEDMenuManager oledMenu;
extern unsigned long oledMenuFreezeStartTime;
extern unsigned long oledMenuFreezeTimeoutInMS;
extern void ChangeAVModeOption(uint8_t num);
extern void ChangeSVModeOption(uint8_t num);

// ====================================================================================
// HC32 Communication - Packet Constants (internal)
// ====================================================================================

static const unsigned char RGBs[7]       = {0x41, 0x44, 'S', 0x40};
static const unsigned char RGsB[7]       = {0x41, 0x44, 'S', 0x50};
static const unsigned char VGA[7]        = {0x41, 0x44, 'S', 0x60};
static const unsigned char Ypbpr[7]      = {0x41, 0x44, 'S', 0x70};
static const unsigned char Adv_7391_SV[7] = {0x41, 0x44, 'S', 0x10};
static const unsigned char Adv_7391_AV[7] = {0x41, 0x44, 'S', 0x20};

static unsigned char TvMode[7]              = {0x41, 0x44, 'T'};
static unsigned char Adv_2X[7]              = {0x41, 0x44, 'S', 0x30};
static unsigned char Adv_1X[7]              = {0x41, 0x44, 'S', 0x31};
static unsigned char Adv_SM_ON[7]           = {0x41, 0x44, 'S', 0x90};
static unsigned char Adv_SM_OFF[7]          = {0x41, 0x44, 'S', 0x91};
static unsigned char Adv_COMPATIBILITY_ON[7]  = {0x41, 0x44, 'S', 0xA0};
static unsigned char Adv_COMPATIBILITY_OFF[7] = {0x41, 0x44, 'S', 0xA1};
static unsigned char Adv_BCSH[7]            = {0x41, 0x44, 'N'};

// ====================================================================================
// HC32 Communication - TV Mode Mapping Table
// ====================================================================================

const uint8_t modes[12] = {
    0x04,  // 0: Auto
    0x84,  // 1: Pal
    0x54,  // 2: Ntsc_M
    0x64,  // 3: Pal_60
    0x74,  // 4: Ntsc443
    0x44,  // 5: Ntsc_J
    0x94,  // 6: Pal_N_wp
    0xA4,  // 7: Pal_M_wop
    0xB4,  // 8: Pal_M
    0xC4,  // 9: Pal_Cmb_N
    0xD4,  // 10: Pal_Cmb_N_wp
    0xE4   // 11: Secam
};

// ====================================================================================
// HC32 Communication - PacketSender Class
// ====================================================================================

class PacketSender {
public:
    explicit PacketSender(HardwareSerial& serial = Serial) : m_serial(serial) {
        randomSeed(analogRead(A0));
    }

    void send(const unsigned char* buff, uint8_t mode = 0) {
        unsigned char buff_lin[7];
        buff_lin[0] = buff[0];
        buff_lin[1] = buff[1];
        buff_lin[2] = buff[2];
        buff_lin[3] = buff[3] | (mode & 0x0f);
        buff_lin[4] = random(254);
        buff_lin[5] = 0xfe;
        buff_lin[6] = buff_lin[0] + buff_lin[1] + buff_lin[2] + buff_lin[3] + buff_lin[4] + buff_lin[5];
        m_serial.write(buff_lin, sizeof(buff_lin));
    }

    void writeReg(const unsigned char* buff, unsigned char reg, unsigned char val) {
        unsigned char buff_lin[7];
        for (int i = 0; i < 4; ++i) buff_lin[i] = buff[i];
        buff_lin[3] = reg;
        buff_lin[4] = val;
        buff_lin[5] = 0xFE;
        unsigned char sum = 0;
        for (int i = 0; i < 6; ++i) sum += buff_lin[i];
        buff_lin[6] = sum;
        m_serial.write(buff_lin, sizeof(buff_lin));
    }

private:
    HardwareSerial& m_serial;
};

static PacketSender packetSender;

// ====================================================================================
// HC32 Communication Functions
// ====================================================================================

void SetReg(unsigned char reg, unsigned char val) {
    packetSender.writeReg(Adv_BCSH, reg, val);
}

// ====================================================================================
// ADV Processor Control Functions
// ====================================================================================

void Send_TvMode(uint8_t Mode) {
    TvMode[3] = Mode;
    packetSender.send(TvMode);
    saveUserPrefs();
}

void Send_Line(bool line) {
    if (line)
        packetSender.send(Adv_2X);
    else
        packetSender.send(Adv_1X);
    saveUserPrefs();
}

void Send_Smooth(bool Smooth) {
    if (Smooth)
        packetSender.send(Adv_SM_ON);
    else
        packetSender.send(Adv_SM_OFF);
    saveUserPrefs();
}

void Send_Compatibility(bool Com) {
    if (!Com)
        packetSender.send(Adv_COMPATIBILITY_ON);
    else
        packetSender.send(Adv_COMPATIBILITY_OFF);
    saveUserPrefs();
}

// ====================================================================================
// Input Source Switching - Basic Functions
// ====================================================================================

void InputRGBs(void) {
    packetSender.send(RGBs);
    selectedInputSource = S_RGBs;
    Info = InfoRGBs;
    resetSyncProcessor();
    GBS::ADC_SOGEN::write(RGB1);
    GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
    GBS::ADC_INPUT_SEL::write(RGB1);
    brightnessOrContrastOption = 0;
    rto->sourceDisconnected = true;
    saveUserPrefs();
}

void InputRGsB(void) {
    packetSender.send(RGsB);
    selectedInputSource = S_RGBs;
    Info = InfoRGsB;
    resetSyncProcessor();
    GBS::ADC_SOGEN::write(RGB1);
    GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
    GBS::ADC_INPUT_SEL::write(RGB1);
    brightnessOrContrastOption = 0;
    rto->sourceDisconnected = true;
    saveUserPrefs();
}

void InputVGA(void) {
    packetSender.send(VGA, 1);
    selectedInputSource = S_VGA;
    Info = InfoVGA;
    resetSyncProcessor();
    GBS::ADC_SOGEN::write(RGB1);
    GBS::SP_EXT_SYNC_SEL::write(HV_Enable);
    GBS::ADC_INPUT_SEL::write(RGB1);
    brightnessOrContrastOption = 0;
    rto->sourceDisconnected = true;
    saveUserPrefs();
}

void InputYUV(void) {
    packetSender.send(Ypbpr);
    selectedInputSource = S_YUV;
    Info = InfoYUV;
    resetSyncProcessor();
    brightnessOrContrastOption = 1;
    rto->sourceDisconnected = true;
    saveUserPrefs();
}

void InputSV(void) {
    packetSender.send(Adv_7391_SV);
    selectedInputSource = S_YUV;
    Info = InfoSV;
    resetSyncProcessor();
    GBS::ADC_SOGEN::write(YUV0);
    GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
    GBS::ADC_INPUT_SEL::write(YUV0);
    brightnessOrContrastOption = 2;
    rto->sourceDisconnected = true;
    rto->isInLowPowerMode = false;
    saveUserPrefs();
}

void InputAV(void) {
    packetSender.send(Adv_7391_AV);
    selectedInputSource = S_YUV;
    Info = InfoAV;
    resetSyncProcessor();
    GBS::ADC_SOGEN::write(YUV0);
    GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
    GBS::ADC_INPUT_SEL::write(YUV0);
    brightnessOrContrastOption = 2;
    rto->sourceDisconnected = true;
    rto->isInLowPowerMode = false;
    saveUserPrefs();
}

// ====================================================================================
// Input Source Switching - Functions with Mode Parameter
// ====================================================================================

void InputRGBs_mode(uint8_t mode) {
    packetSender.send(RGBs, !mode);
    selectedInputSource = S_RGBs;
    Info = InfoRGBs;
    resetSyncProcessor();
    GBS::ADC_SOGEN::write(RGB1);
    GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
    GBS::ADC_INPUT_SEL::write(RGB1);
    brightnessOrContrastOption = 0;
    rto->sourceDisconnected = true;
    saveUserPrefs();
}

void InputRGsB_mode(uint8_t mode) {
    packetSender.send(RGsB, !mode);
    selectedInputSource = S_RGBs;
    Info = InfoRGsB;
    resetSyncProcessor();
    GBS::ADC_SOGEN::write(RGB1);
    GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
    GBS::ADC_INPUT_SEL::write(RGB1);
    brightnessOrContrastOption = 0;
    rto->sourceDisconnected = true;
    saveUserPrefs();
}

void InputVGA_mode(uint8_t mode) {
    packetSender.send(VGA, !mode);
    selectedInputSource = S_VGA;
    Info = InfoVGA;
    resetSyncProcessor();
    GBS::ADC_SOGEN::write(RGB1);
    GBS::SP_EXT_SYNC_SEL::write(HV_Enable);
    GBS::ADC_INPUT_SEL::write(RGB1);
    brightnessOrContrastOption = 0;
    rto->sourceDisconnected = true;
    saveUserPrefs();
}

void InputSV_mode(uint8_t mode) {
    packetSender.send(Adv_7391_SV, mode);
    selectedInputSource = S_YUV;
    Info = InfoSV;
    resetSyncProcessor();
    GBS::ADC_SOGEN::write(YUV0);
    GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
    GBS::ADC_INPUT_SEL::write(YUV0);
    brightnessOrContrastOption = 2;
    rto->sourceDisconnected = true;
    rto->isInLowPowerMode = false;
    saveUserPrefs();
}

void InputAV_mode(uint8_t mode) {
    packetSender.send(Adv_7391_AV, mode);
    selectedInputSource = S_YUV;
    Info = InfoAV;
    resetSyncProcessor();
    GBS::ADC_SOGEN::write(YUV0);
    GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
    GBS::ADC_INPUT_SEL::write(YUV0);
    brightnessOrContrastOption = 2;
    rto->sourceDisconnected = true;
    rto->isInLowPowerMode = false;
    saveUserPrefs();
}

// ====================================================================================
// Input Source Switching - Boot/Restore Function
// ====================================================================================

void applySavedInputSource(void) {
    // Apply hardware configuration based on selectedInputSource
    // Note: Don't call saveUserPrefs() here as we're loading from file
    switch (selectedInputSource) {
        case S_RGBs:
            Info = InfoRGBs;
            resetSyncProcessor();
            GBS::ADC_SOGEN::write(RGB1);
            GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
            GBS::ADC_INPUT_SEL::write(RGB1);
            brightnessOrContrastOption = 0;
            rto->sourceDisconnected = true;
            break;

        case S_VGA:
            Info = InfoVGA;
            resetSyncProcessor();
            GBS::ADC_SOGEN::write(RGB1);
            GBS::SP_EXT_SYNC_SEL::write(HV_Enable);
            GBS::ADC_INPUT_SEL::write(RGB1);
            brightnessOrContrastOption = 0;
            rto->sourceDisconnected = true;
            break;

        case S_YUV:
            // Check which YUV mode is active based on Info
            if (Info == InfoYUV) {
                resetSyncProcessor();
                brightnessOrContrastOption = 1;
                rto->sourceDisconnected = true;
            } else if (Info == InfoSV) {
                resetSyncProcessor();
                GBS::ADC_SOGEN::write(YUV0);
                GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
                GBS::ADC_INPUT_SEL::write(YUV0);
                brightnessOrContrastOption = 2;
                rto->sourceDisconnected = true;
            } else if (Info == InfoAV) {
                resetSyncProcessor();
                GBS::ADC_SOGEN::write(YUV0);
                GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
                GBS::ADC_INPUT_SEL::write(YUV0);
                brightnessOrContrastOption = 2;
                rto->sourceDisconnected = true;
            } else if (Info == InfoRGsB) {
                resetSyncProcessor();
                GBS::ADC_SOGEN::write(RGB1);
                GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
                GBS::ADC_INPUT_SEL::write(RGB1);
                brightnessOrContrastOption = 0;
                rto->sourceDisconnected = true;
            }
            break;

        default:
            // Unknown input source, set to a safe default (RGBs)
            selectedInputSource = S_RGBs;
            Info = InfoRGBs;
            resetSyncProcessor();
            GBS::ADC_SOGEN::write(RGB1);
            GBS::SP_EXT_SYNC_SEL::write(HV_Disable);
            GBS::ADC_INPUT_SEL::write(RGB1);
            brightnessOrContrastOption = 0;
            rto->sourceDisconnected = true;
            break;
    }
}

// ====================================================================================
// Menu Initialization - Input Menu
// ====================================================================================

void initOLEDMenuProInput(OLEDMenuItem *root) {
    OLEDMenuItem *advMenu = oledMenu.registerItem(root, MT_NULL, IMAGE_ITEM(OM_ADVINPUT));

    const char *inputLabels[6] = {
        "RGBs", "RGsB", "VGA", "YPBPR", "SV", "AV"
    };

    uint8_t inputTags[6] = {
        MT_RGBs, MT_RGsB, MT_VGA, MT_YPBPR, MT_SV, MT_AV
    };

    for (size_t i = 0; i < 6; ++i) {
        oledMenu.registerItem(advMenu, inputTags[i], inputLabels[i], InputSwHandler);
    }
}

// ====================================================================================
// Menu Initialization - Settings Menu
// ====================================================================================

void initOLEDMenuProSettings(OLEDMenuItem *root) {
    OLEDMenuItem *settingMenu = oledMenu.registerItem(root, MT_NULL, IMAGE_ITEM(OM_SETTING));

    // Settings options
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
        oledMenu.registerItem(settingMenu, settingTags[i], settingLabels[i], SettingHandler);
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
        oledMenu.registerItem(tvModeMenu, tvModeTags[i], tvModeLabels[i], Adv7391TvModeSwHandler);
    }
}

// ====================================================================================
// Menu Handlers - Input Selection Handler
// ====================================================================================

bool InputSwHandler(OLEDMenuManager *manager, OLEDMenuItem *item, OLEDMenuNav, bool isFirstTime) {
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

bool SettingHandler(OLEDMenuManager *manager, OLEDMenuItem *item, OLEDMenuNav, bool isFirstTime) {
    // ACE packet constants (local to this handler)
    unsigned char Adv_ACE_ON[7] = {0x41, 0x44, 'S', 0x80};
    unsigned char Adv_ACE_OFF[7] = {0x41, 0x44, 'S', 0x81};

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
            packetSender.send(Adv_1X);
            break;
        case SETTING_PresetPreference::MT_7391_2X:
            packetSender.send(Adv_2X);
            break;
        case SETTING_PresetPreference::MT_SMOOTH_OFF:
            packetSender.send(Adv_SM_OFF);
            break;
        case SETTING_PresetPreference::MT_SMOOTH_ON:
            packetSender.send(Adv_SM_ON);
            break;
        case SETTING_PresetPreference::MT_COMPATIBILITY_OFF:
            rgbComponentMode = COMPATIBILITY_OFF;
            Send_Compatibility(rgbComponentMode);
            break;
        case SETTING_PresetPreference::MT_COMPATIBILITY_ON:
            rgbComponentMode = COMPATIBILITY_ON;
            Send_Compatibility(rgbComponentMode);
            break;
#ifdef ACE
        case SETTING_PresetPreference::MT_ACE_OFF:
            packetSender.send(Adv_ACE_OFF);
            break;
        case SETTING_PresetPreference::MT_ACE_ON:
            packetSender.send(Adv_ACE_ON);
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

bool Adv7391TvModeSwHandler(OLEDMenuManager *manager, OLEDMenuItem *item, OLEDMenuNav, bool isFirstTime) {
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
    if (Info == InfoAV) {
        ChangeAVModeOption(0);
    } else if (Info == InfoSV) {
        ChangeSVModeOption(0);
    }

    // Send TV mode command if in SV or AV mode
    if (Info == InfoSV || Info == InfoAV) {
        TvMode[3] = modes[preset];
        packetSender.send(TvMode);
    }

    manager->freeze();
    return false;
}
