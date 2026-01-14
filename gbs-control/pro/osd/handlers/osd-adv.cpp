// ====================================================================================
// osd-adv.cpp
// TV OSD Handlers for ADV7280 Settings (SV/AV Input)
// ====================================================================================

#include "../osd-core.h"

// ====================================================================================
// SV/AV Input Settings - Page 1 (I2P Settings, Video Filters, ACE Settings)
// ====================================================================================

void handle_SVAVInput_Page1(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(false, '1', true);
    OSD_writeStringAtRow(1, 1, "I2P Settings");
    OSD_writeCharAtRow(1, 0xFF, arrow_right_icon, (selectedMenuLine == 1) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
    OSD_writeStringAtRow(2, 1, "Video Filters");
    OSD_writeCharAtRow(2, 0xFF, arrow_right_icon, (selectedMenuLine == 2) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
    OSD_writeStringAtRow(3, 1, "ACE Settings");
    OSD_writeCharAtRow(3, 0xFF, arrow_right_icon, (selectedMenuLine == 3) ? OSD_TEXT_SELECTED : OSD_CURSOR_INACTIVE);
}

void handle_SVAVInput_Page1_Values(void)
{
    // All rows are submenu links, no values to display
}

// ====================================================================================
// SV/AV Input Settings - Page 2 (Brightness, Contrast, Saturation)
// ====================================================================================

void handle_SVAVInput_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '2', true);
    OSD_writeStringAtRow(1, 1, "Brightness");
    OSD_drawDashRange(1, 11, 22);
    OSD_writeStringAtRow(2, 1, "Contrast");
    OSD_drawDashRange(2, 9, 22);
    OSD_writeStringAtRow(3, 1, "Saturation");
    OSD_drawDashRange(3, 11, 22);
}

void handle_SVAVInput_Page2_Values(void)
{
    OSD_displayNumber3DigitAtRow(1, uopt->advBrightness, 25, 24, 23);
    OSD_displayNumber3DigitAtRow(2, uopt->advContrast, 25, 24, 23);
    OSD_displayNumber3DigitAtRow(3, uopt->advSaturation, 25, 24, 23);
}

// ====================================================================================
// SV/AV Input Settings - Page 3 (Hue, Default)
// ====================================================================================

void handle_SVAVInput_Page3(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '3', false);
    OSD_writeStringAtRow(1, 1, "Hue");
    OSD_drawDashRange(1, 4, 22);
    OSD_writeStringAtRow(2, 1, "Default");
}

void handle_SVAVInput_Page3_Values(void)
{
    OSD_displayNumber3DigitAtRow(1, uopt->advHue, 25, 24, 23);
}

// ====================================================================================
// ACE Settings - Page 1 (Enable, Luma Gain, Chroma Gain)
// ====================================================================================

void handle_ACE_Page1(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(false, '1', true);
    OSD_writeStringAtRow(1, 1, "Enable");
    OSD_drawDashRange(1, 7, 22);
    OSD_writeStringAtRow(2, 1, "Luma Gain", uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_drawDashRange(2, 10, 23, uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_writeStringAtRow(3, 1, "Chroma Gain", uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_drawDashRange(3, 12, 23, uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
}

void handle_ACE_Page1_Values(void)
{
    OSD_writeOnOff(1, uopt->advACE);
    OSD_displayNumber2DigitAtRow(2, uopt->advACELumaGain, 25, 24, uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_displayNumber2DigitAtRow(3, uopt->advACEChromaGain, 25, 24, uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
}

// ====================================================================================
// ACE Settings - Page 2 (Chroma Max, Gamma Gain, Response Speed)
// ====================================================================================

void handle_ACE_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '2', true);
    OSD_writeStringAtRow(1, 1, "Chroma Max", uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_drawDashRange(1, 11, 23, uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_writeStringAtRow(2, 1, "Gamma Gain", uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_drawDashRange(2, 11, 23, uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_writeStringAtRow(3, 1, "Response Spd", uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_drawDashRange(3, 13, 23, uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
}

void handle_ACE_Page2_Values(void)
{
    OSD_displayNumber2DigitAtRow(1, uopt->advACEChromaMax, 25, 24, uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_displayNumber2DigitAtRow(2, uopt->advACEGammaGain, 25, 24, uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_displayNumber2DigitAtRow(3, uopt->advACEResponseSpeed, 25, 24, uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
}

// ====================================================================================
// ACE Settings - Page 3 (Default)
// ====================================================================================

void handle_ACE_Page3(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '3', false);
    OSD_writeStringAtRow(1, 1, "Default", uopt->advACE ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
}

void handle_ACE_Page3_Values(void)
{
    // No values to display on page 3
}

// ====================================================================================
// Video Filters Settings (3 pages)
// Page 1: Y Filter, C Filter/Override, Bandwidth
// Page 2: Luma Mode, Chroma Mode, Chroma Taps
// Page 3: Default
// ====================================================================================

// Helper to display Y filter name for CVBS (register 0x17 YSFM - datasheet names)
// Valid range: 0-30 (AutoWide-NTSC WN3).
static void OSD_displayYFilterName(uint8_t row, uint8_t filter) {
    const char* name = "AutoNarrow";  // Default fallback

    switch (filter) {
        case 0:  name = "--AutoWide"; break;
        case 1:  name = "AutoNarrow"; break;
        case 2:  name = "----SVHS-1"; break;
        case 3:  name = "----SVHS-2"; break;
        case 4:  name = "----SVHS-3"; break;
        case 5:  name = "----SVHS-4"; break;
        case 6:  name = "----SVHS-5"; break;
        case 7:  name = "----SVHS-6"; break;
        case 8:  name = "----SVHS-7"; break;
        case 9:  name = "----SVHS-8"; break;
        case 10: name = "----SVHS-9"; break;
        case 11: name = "---SVHS-10"; break;
        case 12: name = "---SVHS-11"; break;
        case 13: name = "---SVHS-12"; break;
        case 14: name = "---SVHS-13"; break;
        case 15: name = "---SVHS-14"; break;
        case 16: name = "---SVHS-15"; break;
        case 17: name = "---SVHS-16"; break;
        case 18: name = "---SVHS-17"; break;
        case 19: name = "---SVHS-18"; break;
        case 20: name = "---PAL-NN1"; break;
        case 21: name = "---PAL-NN2"; break;
        case 22: name = "---PAL-NN3"; break;
        case 23: name = "---PAL-WN1"; break;
        case 24: name = "---PAL-WN2"; break;
        case 25: name = "--NTSC-NN1"; break;
        case 26: name = "--NTSC-NN2"; break;
        case 27: name = "--NTSC-NN3"; break;
        case 28: name = "--NTSC-WN1"; break;
        case 29: name = "--NTSC-WN2"; break;
        case 30: name = "--NTSC-WN3"; break;
    }

    OSD_writeStringAtRow(row, 16, name);
}

// Helper to display WY filter name for S-Video (register 0x18 WYSFM - datasheet names)
// Valid range: 2-19 (SVHS 1-18).
static void OSD_displayWYFilterName(uint8_t row, uint8_t filter) {
    const char* name = "--SVHS-8";  // Default fallback (default value is 19)

    switch (filter) {
        case 2:  name = "--SVHS-1"; break;
        case 3:  name = "--SVHS-2"; break;
        case 4:  name = "--SVHS-3"; break;
        case 5:  name = "--SVHS-4"; break;
        case 6:  name = "--SVHS-5"; break;
        case 7:  name = "--SVHS-6"; break;
        case 8:  name = "--SVHS-7"; break;
        case 9:  name = "--SVHS-8"; break;
        case 10: name = "--SVHS-9"; break;
        case 11: name = "-SVHS-10"; break;
        case 12: name = "-SVHS-11"; break;
        case 13: name = "-SVHS-12"; break;
        case 14: name = "-SVHS-13"; break;
        case 15: name = "-SVHS-14"; break;
        case 16: name = "-SVHS-15"; break;
        case 17: name = "-SVHS-16"; break;
        case 18: name = "-SVHS-17"; break;
        case 19: name = "-SVHS-18"; break;
    }

    OSD_writeStringAtRow(row, 18, name);
}

// Helper to display C filter name (datasheet names)
// Valid range: 0-7 (Auto1.5M-Wideband)
static void OSD_displayCFilterName(uint8_t row, uint8_t filter) {
    const char* name = "Auto1.5M";  // Default fallback
    switch (filter) {
        case 0: name = "Auto1.5M"; break;  // Autoselection 1.5 MHz
        case 1: name = "Auto2.2M"; break;  // Autoselection 2.17 MHz
        case 2: name = "-----SH1"; break;
        case 3: name = "-----SH2"; break;
        case 4: name = "-----SH3"; break;
        case 5: name = "-----SH4"; break;
        case 6: name = "-----SH5"; break;
        case 7: name = "Wideband"; break;
    }
    OSD_writeStringAtRow(row, 18, name);
}

// Helper to display comb filter bandwidth name (uses PAL names: Narrow/Medium/Wide/Widest)
static void OSD_displayCombBW(uint8_t row, uint8_t bw) {
    const char* name = "Narrow";
    switch (bw) {
        case 0: name = "Narrow"; break;
        case 1: name = "Medium"; break;
        case 2: name = "--Wide"; break;
        case 3: name = "Widest"; break;
    }
    OSD_writeStringAtRow(row, 20, name);
}

// Helper: Check if current signal uses NTSC comb filter (3.58 MHz subcarrier)
// Returns true for NTSC-family, false for PAL-family
// Format indices from adv_controller.h ADV_VideoFormats[]:
//   0=Auto, 1=PAL, 2=NTSC-M, 3=PAL-60, 4=NTSC-443, 5=NTSC-J,
//   6=PAL-N, 7=PAL-M(wop), 8=PAL-M(wp), 9=PAL-Cn, 10=PAL-Cn(wp), 11=SECAM
static bool isNTSCSignal(void) {
    uint8_t format = (uopt->activeInputType == InputTypeSV) ? uopt->svVideoFormat : uopt->avVideoFormat;
    // Auto mode (0) - check actual detected signal from GBS STATUS_00 register
    if (format == 0) {
        // Use same logic as getVideoStandard() in osd-misc.cpp
        uint8_t status00 = GBS::STATUS_00::read();
        if (!(status00 & 0x80)) return true;  // No valid signal, default to NTSC
        if (status00 & 0x60) return false;    // 0x40=576p or 0x20=288p/576i = PAL
        if (status00 & 0x18) return true;     // 0x10=480p or 0x08=240p/480i = NTSC
        return true;  // Default to NTSC
    }
    // NTSC comb filter: formats with 3.58 MHz subcarrier
    // 2=NTSC-M, 3=PAL-60, 4=NTSC-443, 5=NTSC-J, 7=PAL-M(wop), 8=PAL-M(wp)
    return (format == 2 || format == 3 || format == 4 || format == 5 || format == 7 || format == 8);
}

// Helper to display Luma mode name
// Values: 0=Adaptive, 4=Notch, 5=Fixed Top, 6=Fixed All, 7=Fixed Bottom
static void OSD_displayLumaMode(uint8_t row, uint8_t mode) {
    const char* name = "Adaptive";
    switch (mode) {
        case 0: name = "Adaptive"; break;
        case 4: name = "---Notch"; break;
        case 5: name = "FixedTop"; break;
        case 6: name = "FixedAll"; break;
        case 7: name = "FixedBot"; break;
    }
    OSD_writeStringAtRow(row, 18, name);
}

// Helper to display Chroma mode name
// Values: 0=Adaptive, 4=Off, 5=Fixed Top, 6=Fixed All, 7=Fixed Bottom
static void OSD_displayChromaMode(uint8_t row, uint8_t mode) {
    const char* name = "Adaptive";
    switch (mode) {
        case 0: name = "Adaptive"; break;
        case 4: name = "-----OFF"; break;
        case 5: name = "FixedTop"; break;
        case 6: name = "FixedAll"; break;
        case 7: name = "FixedBot"; break;
    }
    OSD_writeStringAtRow(row, 18, name);
}

// Helper to display Chroma taps
// NTSC: 0=None, 1=3→2, 2=5→3, 3=5→4
// PAL:  0=None, 1=5→2, 2=5→3, 3=5→4
static void OSD_displayChromaTaps(uint8_t row, uint8_t taps, bool isNTSC) {
    const char* name = "5->3";
    if (isNTSC) {
        switch (taps) {
            case 0: name = "----None"; break;
            case 1: name = "----3to2"; break;
            case 2: name = "----5to3"; break;
            case 3: name = "----5to4"; break;
        }
    } else {
        switch (taps) {
            case 0: name = "----None"; break;
            case 1: name = "----5to2"; break;
            case 2: name = "----5to3"; break;
            case 3: name = "----5to4"; break;
        }
    }
    OSD_writeStringAtRow(row, 18, name);
}

// Page 1: Y Filter, C Filter/Override, Bandwidth
void handle_VideoFilters_Page1(void)
{
    bool isSV = (uopt->activeInputType == InputTypeSV);
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(false, '1', true);

    if (isSV) {
        // S-Video mode
        OSD_writeStringAtRow(1, 1, "Y Filter");
        OSD_drawDashRange(1, 9, 17);
        OSD_writeStringAtRow(2, 1, "Override");
        OSD_drawDashRange(2, 9, 17);
    } else {
        // AV (Composite) mode
        OSD_writeStringAtRow(1, 1, "Y Filter");
        OSD_drawDashRange(1, 9, 15);
        OSD_writeStringAtRow(2, 1, "C Filter");
        OSD_drawDashRange(2, 9, 17);
    }
    // Row 3: Bandwidth (same for both modes)
    OSD_writeStringAtRow(3, 1, "Bandwidth");
    OSD_drawDashRange(3, 10, 19);
}

void handle_VideoFilters_Page1_Values(void)
{
    bool isSV = (uopt->activeInputType == InputTypeSV);
    bool isNTSC = isNTSCSignal();

    if (isSV) {
        // S-Video: Y Filter uses WY register, Override
        if (uopt->advFilterWYOverride) {
            OSD_displayWYFilterName(1, uopt->advFilterWYShaping);
        } else {
            OSD_writeStringAtRow(1, 18, "----Auto");
        }
        OSD_writeStringAtRow(2, 18, uopt->advFilterWYOverride ? "--Manual" : "----Auto");
    } else {
        // AV: Y Filter, C Filter
        OSD_displayYFilterName(1, uopt->advFilterYShaping);
        OSD_displayCFilterName(2, uopt->advFilterCShaping);
    }
    // Row 3: Bandwidth
    OSD_displayCombBW(3, isNTSC ? uopt->advFilterCombNTSC : uopt->advFilterCombPAL);
}

// Page 2: Luma Mode, Chroma Mode, Chroma Taps
void handle_VideoFilters_Page2(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '2', true);
    OSD_writeStringAtRow(1, 1, "Luma Mode");
    OSD_drawDashRange(1, 10, 17);
    OSD_writeStringAtRow(2, 1, "Chroma Mode");
    OSD_drawDashRange(2, 12, 17);
    OSD_writeStringAtRow(3, 1, "Chroma Taps");
    OSD_drawDashRange(3, 12, 17);
}

void handle_VideoFilters_Page2_Values(void)
{
    bool isNTSC = isNTSCSignal();
    if (isNTSC) {
        OSD_displayLumaMode(1, uopt->advCombLumaModeNTSC);
        OSD_displayChromaMode(2, uopt->advCombChromaModeNTSC);
        OSD_displayChromaTaps(3, uopt->advCombChromaTapsNTSC, true);
    } else {
        OSD_displayLumaMode(1, uopt->advCombLumaModePAL);
        OSD_displayChromaMode(2, uopt->advCombChromaModePAL);
        OSD_displayChromaTaps(3, uopt->advCombChromaTapsPAL, false);
    }
}

// Page 3: Default only
void handle_VideoFilters_Page3(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(true, '3', false);
    OSD_writeStringAtRow(1, 1, "Default");
}

void handle_VideoFilters_Page3_Values(void)
{
    // No values to display
}

// ====================================================================================
// I2P Settings - Single Page (Enable I2P/2X, Smooth)
// ====================================================================================

void handle_I2P_Page1(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writePageIcons(false, '1', false);
    OSD_writeStringAtRow(1, 1, "Enable I2P/2X");
    OSD_drawDashRange(1, 14, 22);
    OSD_writeStringAtRow(2, 1, "Smooth", uopt->advI2P ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
    OSD_drawDashRange(2, 7, 22, uopt->advI2P ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
}

void handle_I2P_Page1_Values(void)
{
    OSD_writeOnOff(1, uopt->advI2P);
    OSD_writeOnOff(2, uopt->advSmooth, uopt->advI2P ? OSD_COLOR_AUTO : OSD_TEXT_DISABLED);
}

