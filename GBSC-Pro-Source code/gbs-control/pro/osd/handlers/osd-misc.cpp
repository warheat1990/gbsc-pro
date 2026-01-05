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

// Get video standard (PAL/NTSC) from STATUS_00 register
static const char* getVideoStandard(uint8_t status00)
{
    if (!(status00 & 0x80)) return "";      // No valid signal
    if (status00 & 0x40) return "PAL";      // 576p = PAL
    if (status00 & 0x20) return "PAL";      // 288p/576i = PAL
    if (status00 & 0x10) return "NTSC";     // 480p = NTSC
    if (status00 & 0x08) return "NTSC";     // 240p/480i = NTSC
    return "";
}

// Render info display on TV OSD
// Row 1: Input info (signal type, PAL/NTSC, resolution)
// Row 2: Output info (resolution, Hz)
void OSD_renderInfoDisplay()
{
    uint8_t currentInput = GBS::ADC_INPUT_SEL::read();
    uint8_t status00 = GBS::STATUS_00::read();
    rto->presetID = GBS::GBS_PRESET_ID::read();

    // === ROW 1: INPUT ===
    OSD_writeStringAtRow(1, 0, "Input:", OSD_CURSOR_ACTIVE);

    char buf1[22];
    const char* inputText = nullptr;

    if (rto->sourceDisconnected || !rto->boardHasPower) {
        inputText = "No Signal";
    } else {
        // Determine input signal type
        uint8_t activeInput = uopt->activeInputType;
        const char* signalType = nullptr;

        if (currentInput == 1 || activeInput == InputTypeRGBs || activeInput == InputTypeRGsB || activeInput == InputTypeVGA) {
            boolean vsyncActive = GBS::STATUS_SYNC_PROC_VSACT::read();
            boolean hsyncActive = GBS::STATUS_SYNC_PROC_HSACT::read();
            if (activeInput == InputTypeVGA && !(vsyncActive && hsyncActive)) {
                inputText = "No Signal";
            } else {
                signalType = (vsyncActive && hsyncActive) ? "RGBHV" : "RGB";
            }
        } else if (activeInput == InputTypeYUV) {
            signalType = "YPbPr";
        } else if (activeInput == InputTypeSV) {
            signalType = "SV";
        } else if (activeInput == InputTypeAV) {
            signalType = "AV";
        } else {
            inputText = "No Signal";
        }

        if (!inputText && signalType) {
            // Format: "YPbPr NTSC 240p"
            const char* standard = getVideoStandard(status00);
            const char* resolution = getInputResolutionName(status00);
            snprintf(buf1, sizeof(buf1), "%-5s %-4s %-4s", signalType, standard, resolution);
            inputText = buf1;
        }
    }

    // Write with padding to clear previous text
    snprintf(buf1, sizeof(buf1), "%-19s", inputText ? inputText : "No Signal");
    OSD_writeStringAtRow(1, 7, buf1, OSD_TEXT_NORMAL);

    // === ROW 2: OUTPUT ===
    OSD_writeStringAtRow(2, 0, "Output:", OSD_CURSOR_ACTIVE);
    const char* outRes = getOutputResolutionName(rto->presetID);
    float ofr = getOutputFrameRate();
    uint8_t outputHz = (uint8_t)ofr;
    char buf2[20];
    snprintf(buf2, sizeof(buf2), "%-10s %2dHz", outRes, outputHz);
    OSD_writeStringAtRow(2, 8, buf2, OSD_TEXT_NORMAL);
}
