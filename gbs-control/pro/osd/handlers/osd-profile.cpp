// ====================================================================================
// osd-profile.cpp
// TV OSD Handlers for Profile Management
// ====================================================================================

#include "../osd-core.h"
#include "../../../slot.h"
#include <LittleFS.h>

// ====================================================================================
// External References
// ====================================================================================

extern String slotIndexMap;
extern bool loadSlotSettings();

// ====================================================================================
// Slot Names (updated on left/right navigation only)
// ====================================================================================

static char loadSlotName[19] = "";   // Name for Load row
static char saveSlotName[19] = "";   // Name for Save row
static int8_t loadSlotIdx = -1;      // Current Load slot index
static int8_t saveSlotIdx = -1;      // Current Save slot index

// ====================================================================================
// Helper Functions
// ====================================================================================

// Read slot name from slots.bin file (0-based index)
// Returns true if slot has a custom name, false if empty/default
static bool getSlotName(uint8_t slotIndex, char* nameBuffer, uint8_t maxLen)
{
    if (slotIndex >= SLOTS_TOTAL) return false;

    File f = LittleFS.open(SLOTS_FILE, "r");
    if (!f) return false;

    SlotMeta slotData;
    f.seek(slotIndex * sizeof(SlotMeta));
    size_t bytesRead = f.read((byte*)&slotData, sizeof(SlotMeta));
    f.close();

    if (bytesRead != sizeof(SlotMeta)) return false;

    // Check if slot is empty or has default name
    if (strncmp(slotData.name, EMPTY_SLOT_NAME, 5) == 0 || slotData.name[0] == '\0') {
        return false;  // Empty slot
    }

    // Copy name, truncate if needed
    strncpy(nameBuffer, slotData.name, maxLen - 1);
    nameBuffer[maxLen - 1] = '\0';
    return true;
}

// Format slot name into buffer: custom name or "Preset X"
static void formatSlotName(uint8_t slotIndex, char* outBuf, uint8_t bufLen)
{
    char name[19];
    if (getSlotName(slotIndex, name, sizeof(name))) {
        snprintf(outBuf, bufLen, "%-18s", name);
    } else {
        snprintf(outBuf, bufLen, "Preset %-11d", slotIndex + 1);
    }
}

// Update Load slot name (call on left/right navigation)
void updateLoadSlotName(uint8_t slotIndex)
{
    loadSlotIdx = slotIndex;
    formatSlotName(slotIndex, loadSlotName, sizeof(loadSlotName));
}

// Update Save slot name (call on left/right navigation)
void updateSaveSlotName(uint8_t slotIndex)
{
    saveSlotIdx = slotIndex;
    formatSlotName(slotIndex, saveSlotName, sizeof(saveSlotName));
}

// Get current Load slot index (returns 0 if not set)
int8_t getLoadSlotIdx(void)
{
    return loadSlotIdx >= 0 ? loadSlotIdx : 0;
}

// Get current Save slot index (returns 0 if not set)
int8_t getSaveSlotIdx(void)
{
    return saveSlotIdx >= 0 ? saveSlotIdx : 0;
}

// Display slot name using cache for Load/Save rows
static void displaySlotName(uint8_t row, uint8_t slotIndex, uint8_t startCol, uint8_t color, bool useCache)
{
    char buf[20];

    if (useCache) {
        // Use stored name if index matches
        if (row == 1 && loadSlotIdx == slotIndex && loadSlotName[0] != '\0') {
            OSD_writeStringAtRow(row, startCol, loadSlotName, color);
            return;
        }
        if (row == 2 && saveSlotIdx == slotIndex && saveSlotName[0] != '\0') {
            OSD_writeStringAtRow(row, startCol, saveSlotName, color);
            return;
        }
    }

    // Read from file and store for future use
    formatSlotName(slotIndex, buf, sizeof(buf));
    OSD_writeStringAtRow(row, startCol, buf, color);

    // Store for future use
    if (row == 1) {
        loadSlotIdx = slotIndex;
        strncpy(loadSlotName, buf, sizeof(loadSlotName));
    } else if (row == 2) {
        saveSlotIdx = slotIndex;
        strncpy(saveSlotName, buf, sizeof(saveSlotName));
    }
}

// ====================================================================================
// Profile Handlers
// ====================================================================================

void handle_Profile_SaveLoad(void)
{
    OSD_setMenuLineColors(selectedMenuLine);
    OSD_writeStringAtRow(1, 1, "Load:");
    OSD_writeStringAtRow(2, 1, "Save:");
    OSD_writeStringAtRow(3, 1, "Current:", OSD_TEXT_SELECTED);
}

void handle_Profile_SlotDisplay(void)
{
    // Row 1: Load profile - use stored index
    displaySlotName(1, getLoadSlotIdx(), 7, OSD_getMenuLineColor(1), true);

    // Row 2: Save profile - use stored index
    displaySlotName(2, getSaveSlotIdx(), 7, OSD_getMenuLineColor(2), true);

    // Row 3: Current active slot
    int activeIdx = slotIndexMap.indexOf(uopt->presetSlot);
    if (activeIdx >= 0 && activeIdx < 36) {
        displaySlotName(3, activeIdx, 10, OSD_TEXT_SELECTED, false);
    } else {
        OSD_writeStringAtRow(3, 10, "None", OSD_TEXT_SELECTED);
    }
}
