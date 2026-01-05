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
// Uses VPERIOD_IF to distinguish progressive (240p/288p) from interlaced (480i/576i)
// Progressive signals have odd line counts, interlaced have even line counts
static const char* getInputResolutionName(uint8_t status00)
{
    if (!(status00 & 0x80)) return "Err";

    if (status00 & 0x40) return "576p";
    if (status00 & 0x20) {
        // PAL-like: use VPERIOD_IF to distinguish 288p from 576i
        uint16_t vperiod = GBS::VPERIOD_IF::read();
        // 623, 625, 627 = progressive (288p), 622, 624, 626 = interlaced (576i)
        if (vperiod == 623 || vperiod == 625 || vperiod == 627)
            return "288p";
        else
            return "576i";
    }
    if (status00 & 0x10) return "480p";
    if (status00 & 0x08) {
        // NTSC-like: use VPERIOD_IF to distinguish 240p from 480i
        uint16_t vperiod = GBS::VPERIOD_IF::read();
        // 521, 523, 525 = progressive (240p), 522, 524, 526 = interlaced (480i)
        if (vperiod == 521 || vperiod == 523 || vperiod == 525)
            return "240p";
        else
            return "480i";
    }
    return "Err";
}

// Helper to write padded output resolution (max 10 chars: "1920x1080 ")
static void writeOutputResolution(uint8_t presetID)
{
    const char* res = getOutputResolutionName(presetID);
    char buf[11];
    snprintf(buf, sizeof(buf), "%-10s", res);  // Left-align, pad with spaces
    OSD_writeStringAtRow(1, 6, buf, OSD_TEXT_NORMAL);
}

// Helper to write padded input type (max 5 chars: "YPbPr")
static void writeInputType(uint8_t inputType)
{
    const char* name = getInputTypeName(inputType);
    char buf[6];
    snprintf(buf, sizeof(buf), "%-5s", name);
    OSD_writeStringAtRow(1, 17, buf, OSD_TEXT_NORMAL);
}

// Render info display on TV OSD
// Called every frame while info display is active - uses padded writes to avoid flicker
void OSD_renderInfoDisplay()
{
    float ofr = getOutputFrameRate();
    uint8_t currentInput = GBS::ADC_INPUT_SEL::read();
    rto->presetID = GBS::GBS_PRESET_ID::read();

    // Row 1: "Info: <resolution> <input> <fps>Hz"
    OSD_writeStringAtRow(1, 0, "Info:", OSD_CURSOR_ACTIVE);
    writeOutputResolution(rto->presetID);
    writeInputType(uopt->activeInputType);

    uint8_t frameRate = (uint8_t)ofr;
    OSD_writeCharAtRow(1, 24, '0' + (frameRate / 10), OSD_TEXT_NORMAL);
    OSD_writeCharAtRow(1, 25, '0' + (frameRate % 10), OSD_TEXT_NORMAL);
    OSD_writeStringAtRow(1, 26, "Hz", OSD_TEXT_NORMAL);

    // Row 2: "Current: <signal type> <input resolution>"
    OSD_writeStringAtRow(2, 0, "Current:", OSD_CURSOR_ACTIVE);

    if (rto->sourceDisconnected || !rto->boardHasPower) {
        OSD_writeStringAtRow(2, 9, "No Input    ", OSD_TEXT_NORMAL);
        return;
    }

    // Determine signal type and input resolution
    uint8_t activeInput = uopt->activeInputType;
    const char* signalType = nullptr;

    if (currentInput == 1 || activeInput == InputTypeRGBs || activeInput == InputTypeRGsB || activeInput == InputTypeVGA) {
        boolean vsyncActive = GBS::STATUS_SYNC_PROC_VSACT::read();
        boolean hsyncActive = GBS::STATUS_SYNC_PROC_HSACT::read();
        if (activeInput == InputTypeVGA && !(vsyncActive && hsyncActive)) {
            OSD_writeStringAtRow(2, 9, "No Input    ", OSD_TEXT_NORMAL);
            return;
        }
        signalType = (vsyncActive && hsyncActive) ? "RGBHV" : "RGB  ";
    } else if (activeInput == InputTypeYUV) {
        signalType = "YPbPr";
    } else if (activeInput == InputTypeSV) {
        signalType = "SV   ";
    } else if (activeInput == InputTypeAV) {
        signalType = "AV   ";
    } else {
        OSD_writeStringAtRow(2, 9, "No Input    ", OSD_TEXT_NORMAL);
        return;
    }

    // Write signal type (5 chars) + space + resolution (4 chars) = positions 9-19
    uint8_t status00 = GBS::STATUS_00::read();
    char buf[13];
    snprintf(buf, sizeof(buf), "%s %-4s", signalType, getInputResolutionName(status00));
    OSD_writeStringAtRow(2, 9, buf, OSD_TEXT_NORMAL);
}
