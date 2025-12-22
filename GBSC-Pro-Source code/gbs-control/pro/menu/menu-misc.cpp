// ====================================================================================
// menu-misc.cpp
// IR Menu Handler for Volume and Info Display
// ====================================================================================

#include "menu-common.h"
#include "../drivers/pt2257.h"
#include "../../tv5725.h"

// ====================================================================================
// External References
// ====================================================================================

typedef TV5725<GBS_ADDR> GBS;
extern struct runTimeOptions *rto;
extern void saveUserPrefs();
extern float getOutputFrameRate();

// ====================================================================================
// Helper Functions
// ====================================================================================

// Get output resolution string by presetID
static const char* getOutputResolutionName(uint8_t presetID) {
    switch (presetID) {
        case 0x01: case 0x11: return "1280x960";
        case 0x02: case 0x12: return "1280x1024";
        case 0x03: case 0x13: return "1280x720";
        case 0x05: case 0x15: return "1920x1080";
        case 0x06: case 0x16: return "Downscale";
        case 0x04:            return "720x480";
        case 0x14:            return "768x576";
        default:              return "Bypass";
    }
}

// Get input type string
static const char* getInputTypeName(uint8_t type) {
    switch (type) {
        case InputTypeRGBs: return "RGBs";
        case InputTypeRGsB: return "RGsB";
        case InputTypeVGA:  return "VGA";
        case InputTypeYUV:  return "YPBPR";
        case InputTypeSV:   return "SV";
        case InputTypeAV:   return "AV";
        default:            return "";
    }
}

// ====================================================================================
// IR_handleMiscSettings - Volume Adjustment Handler
// ====================================================================================

bool IR_handleMiscSettings()
{
    // OLED_Volume_Adjust
    if (oled_menuItem == OLED_Volume_Adjust) {
        showMenuCentered("Volume - / + dB");

        // TV OSD display
        osdDisplayValue = 50 - volume;
        OSD_writeStringAtRow(1, 1, "Line input volume", OSD_TEXT_SELECTED);
        // Display 2-digit volume value at positions 20-21 (0-50 range)
        OSD_writeCharAtRow(1, 20, '0' + (osdDisplayValue / 10), OSD_TEXT_NORMAL);  // tens
        OSD_writeCharAtRow(1, 21, '0' + (osdDisplayValue % 10), OSD_TEXT_NORMAL);  // units

        if (irDecode()) {
            switch (results.value) {
                case IR_KEY_VOL_UP:
                    volume = MAX(volume - 1, 0);
                    osdDisplayValue = 50 - volume;
                    PT2257_setAttenuation(volume);
                    break;
                case IR_KEY_VOL_DN:
                    volume = MIN(volume + 1, 50);
                    osdDisplayValue = 50 - volume;
                    PT2257_setAttenuation(volume);
                    break;
                case IR_KEY_MENU:
                case IR_KEY_EXIT:
                    exitMenu();
                    break;
                case IR_KEY_OK:
                    saveUserPrefs();
                    OSD_showSavingFeedback(ROW_1);
                    break;
            }
            irResume();
        }
        return true;
    }

    // NOTE: EnableOTA, ResetDefaults handlers moved to menu-unused.cpp

    return false;
}

// ====================================================================================
// IR_handleInfoDisplay - Info Display Handler
// ====================================================================================

bool IR_handleInfoDisplay()
{
    if (oled_menuItem != OLED_Info_Display) {
        return false;
    }

    showMenu("Menu-", "Info");

    boolean vsyncActive = 0;
    boolean hsyncActive = 0;
    float ofr = getOutputFrameRate();
    uint8_t currentInput = GBS::ADC_INPUT_SEL::read();
    rto->presetID = GBS::GBS_PRESET_ID::read();

    // Row 1: Info header
    OSD_writeStringAtRow(1, 0, "Info:", OSD_CURSOR_ACTIVE);
    OSD_writeStringAtRow(1, 26, "Hz", OSD_TEXT_NORMAL);

    // Row 1: Output resolution (positions 6-14) - clear then write
    OSD_clearRowContent(ROW_1, 15, 6);
    OSD_writeStringAtRow(1, 6, getOutputResolutionName(rto->presetID), OSD_TEXT_NORMAL);

    // Row 1: Input type (positions 17-22) - clear then write
    OSD_clearRowContent(ROW_1, 23, 17);
    OSD_writeStringAtRow(1, 18, getInputTypeName(inputType), OSD_TEXT_NORMAL);

    // Row 1: Frame rate
    // Display frame rate (2 digits, 0-99 Hz range)
    osdDisplayValue = ofr;
    OSD_writeCharAtRow(1, 24, '0' + (osdDisplayValue / 10), OSD_TEXT_NORMAL);  // tens
    OSD_writeCharAtRow(1, 25, '0' + (osdDisplayValue % 10), OSD_TEXT_NORMAL);  // units

    OSD_writeStringAtRow(2, 0, "Current:", OSD_CURSOR_ACTIVE);
    OSD_writeStringAtRow(2, 0xFF, " ", OSD_TEXT_NORMAL);

    if ((rto->sourceDisconnected || !rto->boardHasPower || isInfoDisplayActive == 1)) {
        OSD_writeStringAtRow(2, 0xFF, "No Input", OSD_TEXT_NORMAL);
    } else if (((currentInput == 1) || (inputType == InputTypeRGBs || inputType == InputTypeRGsB || inputType == InputTypeVGA))) {
        OSD_writeCharAtRow(2, 16, 'B', OSD_BACKGROUND);
        OSD_writeStringAtRow(2, 0xFF, "RGB ", OSD_TEXT_NORMAL);
        vsyncActive = GBS::STATUS_SYNC_PROC_VSACT::read();
        if (vsyncActive) {
            hsyncActive = GBS::STATUS_SYNC_PROC_HSACT::read();
            if (hsyncActive) {
                OSD_writeStringAtRow(2, 0xFF, "HV   ", OSD_TEXT_NORMAL);
            }
        } else if ((inputType == InputTypeVGA) && ((!vsyncActive || !hsyncActive))) {
            OSD_writeCharAtRow(2, 11, 'B', OSD_BACKGROUND);
            OSD_writeStringAtRow(2, 0x09, "No Input", OSD_TEXT_NORMAL);
        }
    } else if ((rto->continousStableCounter > 35 || currentInput != 1) || (inputType == InputTypeYUV || inputType == InputTypeSV || inputType == InputTypeAV)) {
        OSD_writeCharAtRow(2, 16, 'B', OSD_BACKGROUND);
        if (inputType == InputTypeYUV)
            OSD_writeStringAtRow(2, 0xFF, "  YPBPR  ", OSD_TEXT_NORMAL);
        else if (inputType == InputTypeSV)
            OSD_writeStringAtRow(2, 0xFF, "   SV    ", OSD_TEXT_NORMAL);
        else if (inputType == InputTypeAV)
            OSD_writeStringAtRow(2, 0xFF, "   AV    ", OSD_TEXT_NORMAL);
    } else {
        OSD_writeStringAtRow(2, 0xFF, "No Input", OSD_TEXT_NORMAL);
    }

    // Show resolution only if input is connected
    if (!rto->sourceDisconnected && rto->boardHasPower) {
        static uint8_t S0_Read_Resolution;
        static unsigned long Tim_info = 0;
        if ((millis() - Tim_info) >= 1000) {
            S0_Read_Resolution = GBS::STATUS_00::read();
            Tim_info = millis();
        }

        if (S0_Read_Resolution & 0x80) {
            if (S0_Read_Resolution & 0x40) {
                OSD_writeStringAtRow(2, 0xFF, "   576p", OSD_TEXT_NORMAL);
            } else if (S0_Read_Resolution & 0x20) {
                if (abs(GBS::STATUS_SYNC_PROC_VTOTAL::read() - 312) <= 10)
                    OSD_writeStringAtRow(2, 0xFF, "   288p", OSD_TEXT_NORMAL);
                else
                    OSD_writeStringAtRow(2, 0xFF, "   576i", OSD_TEXT_NORMAL);
            } else if (S0_Read_Resolution & 0x10) {
                OSD_writeStringAtRow(2, 0xFF, "   480p", OSD_TEXT_NORMAL);
            } else if (S0_Read_Resolution & 0x08) {
                if (abs(GBS::STATUS_SYNC_PROC_VTOTAL::read() - 262) <= 10)
                    OSD_writeStringAtRow(2, 0xFF, "   240p", OSD_TEXT_NORMAL);
                else
                    OSD_writeStringAtRow(2, 0xFF, "   480i", OSD_TEXT_NORMAL);
            } else {
                OSD_writeStringAtRow(2, 0xFF, "   Err", OSD_TEXT_NORMAL);
            }
        } else {
            OSD_writeStringAtRow(2, 0xFF, "   Err", OSD_TEXT_NORMAL);
        }
    }

    if (irDecode()) {
        switch (results.value) {
            case IR_KEY_MENU:
                exitMenu();
                break;
            case IR_KEY_EXIT:
                exitMenu();
                break;
        }
        irResume();
    }

    return true;
}
