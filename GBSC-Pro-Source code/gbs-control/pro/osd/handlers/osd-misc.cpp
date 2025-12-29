// ====================================================================================
// osd-misc.cpp
// TV OSD Handlers for Mute, Volume, and Info Display
// ====================================================================================

#include "../osd-core.h"
#include "../../../tv5725.h"

// ====================================================================================
// External References
// ====================================================================================

typedef TV5725<GBS_ADDR> GBS;
extern struct runTimeOptions *rto;
extern struct userOptions *uopt;
extern float getOutputFrameRate();

// ====================================================================================
// Mute Status Display
// ====================================================================================

// Render mute status on TV OSD (non-blocking, called each frame)
void OSD_renderMuteDisplay(bool muted)
{
    OSD_writeStringAtRow(1, 1, "MUTE", OSD_TEXT_NORMAL);
    if (muted) {
        OSD_writeStringAtRow(1, 6, "ON ", OSD_TEXT_NORMAL);  // Extra space to clear "OFF"
    } else {
        OSD_writeStringAtRow(1, 6, "OFF", OSD_TEXT_NORMAL);
    }
}

// ====================================================================================
// Resolution Countdown Display
// ====================================================================================

// Display resolution confirmation countdown timer on TV OSD
void OSD_renderResolutionCountdown(uint8_t secondsRemaining)
{
    if (secondsRemaining >= 10) {
        OSD_writeCharAtRow(2, 11, (secondsRemaining / 10) + '0');
        OSD_writeCharAtRow(2, 12, (secondsRemaining % 10) + '0');
        OSD_writeStringAtRow(2, 14, " s ");
    } else {
        OSD_writeCharAtRow(2, 12, ' ', OSD_BACKGROUND);  // Clear tens digit
        OSD_writeCharAtRow(2, 11, secondsRemaining + '0');
        OSD_writeStringAtRow(2, 13, " s ");
    }
}

// ====================================================================================
// Volume Display
// ====================================================================================

// Display volume value on TV OSD (called during volume adjustment)
void OSD_updateVolumeDisplay(uint8_t volumeValue)
{
    OSD_writeStringAtRow(1, 1, "Line input volume", OSD_TEXT_NORMAL);
    // Display 2-digit volume value at positions 20-21 (0-50 range)
    OSD_writeCharAtRow(1, 20, '0' + (volumeValue / 10), OSD_TEXT_NORMAL);  // tens
    OSD_writeCharAtRow(1, 21, '0' + (volumeValue % 10), OSD_TEXT_NORMAL);  // units
}

// ====================================================================================
// Info Display - Main Rendering
// ====================================================================================

// Get input resolution string from STATUS_00 register
static const char* getInputResolutionName(uint8_t status00)
{
    if (!(status00 & 0x80)) return "Err";

    if (status00 & 0x40) return "576p";
    if (status00 & 0x20) {
        if (abs(GBS::STATUS_SYNC_PROC_VTOTAL::read() - 312) <= 10)
            return "288p";
        else
            return "576i";
    }
    if (status00 & 0x10) return "480p";
    if (status00 & 0x08) {
        if (abs(GBS::STATUS_SYNC_PROC_VTOTAL::read() - 262) <= 10)
            return "240p";
        else
            return "480i";
    }
    return "Err";
}

// Render info display on TV OSD
void OSD_renderInfoDisplay(uint8_t isInfoDisplayActive)
{
    float ofr = getOutputFrameRate();
    uint8_t currentInput = GBS::ADC_INPUT_SEL::read();
    rto->presetID = GBS::GBS_PRESET_ID::read();

    // Row 1: "Info: <resolution> <input> <fps>Hz"
    OSD_writeStringAtRow(1, 0, "Info:", OSD_CURSOR_ACTIVE);

    // Output resolution (positions 6-15)
    OSD_clearRowContent(ROW_1, 15, 6);
    OSD_writeStringAtRow(1, 6, getOutputResolutionName(rto->presetID), OSD_TEXT_NORMAL);

    // Input type (positions 17-22)
    OSD_clearRowContent(ROW_1, 22, 17);
    OSD_writeStringAtRow(1, 17, getInputTypeName(uopt->activeInputType), OSD_TEXT_NORMAL);

    // Frame rate (positions 24-27)
    uint8_t frameRate = (uint8_t)ofr;
    OSD_writeCharAtRow(1, 24, '0' + (frameRate / 10), OSD_TEXT_NORMAL);
    OSD_writeCharAtRow(1, 25, '0' + (frameRate % 10), OSD_TEXT_NORMAL);
    OSD_writeStringAtRow(1, 26, "Hz", OSD_TEXT_NORMAL);

    // Row 2: "Current: <signal type> <input resolution>"
    OSD_writeStringAtRow(2, 0, "Current:", OSD_CURSOR_ACTIVE);

    // Clear area after "Current:" (positions 9-27)
    OSD_clearRowContent(ROW_2, 27, 9);

    if (rto->sourceDisconnected || !rto->boardHasPower) {
        OSD_writeStringAtRow(2, 9, "No Input", OSD_TEXT_NORMAL);
        return;
    }

    // Show signal type at position 9
    uint8_t activeInput = uopt->activeInputType;
    if (currentInput == 1 || activeInput == InputTypeRGBs || activeInput == InputTypeRGsB || activeInput == InputTypeVGA) {
        OSD_writeStringAtRow(2, 9, "RGB", OSD_TEXT_NORMAL);
        boolean vsyncActive = GBS::STATUS_SYNC_PROC_VSACT::read();
        boolean hsyncActive = GBS::STATUS_SYNC_PROC_HSACT::read();
        if (vsyncActive && hsyncActive) {
            OSD_writeStringAtRow(2, 13, "HV", OSD_TEXT_NORMAL);
        } else if (activeInput == InputTypeVGA) {
            OSD_writeStringAtRow(2, 9, "No Input", OSD_TEXT_NORMAL);
            return;
        }
    } else if (activeInput == InputTypeYUV) {
        OSD_writeStringAtRow(2, 9, "YPbPr", OSD_TEXT_NORMAL);
    } else if (activeInput == InputTypeSV) {
        OSD_writeStringAtRow(2, 9, "SV", OSD_TEXT_NORMAL);
    } else if (activeInput == InputTypeAV) {
        OSD_writeStringAtRow(2, 9, "AV", OSD_TEXT_NORMAL);
    } else {
        OSD_writeStringAtRow(2, 9, "No Input", OSD_TEXT_NORMAL);
        return;
    }

    // Show input resolution at position 16 (read every frame, no caching)
    uint8_t status00 = GBS::STATUS_00::read();
    OSD_writeStringAtRow(2, 16, getInputResolutionName(status00), OSD_TEXT_NORMAL);
}
