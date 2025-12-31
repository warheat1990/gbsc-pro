/**
 * Virtual Preset Menu for GBSC-Pro
 */
#include "menu-presets.h"
#include <LittleFS.h>
#include "../../slot.h"
#include "../../options.h"
#include "../../OLEDMenuConfig.h"
#include "../../fonts.h"

extern void applyPresets(uint8_t videoMode);
extern bool loadSlotSettings();
extern void saveUserPrefs();
extern userOptions *uopt;
extern runTimeOptions *rto;
extern unsigned long oledMenuFreezeStartTime;
extern unsigned long oledMenuFreezeTimeoutInMS;

// ====================================================================================
// Compact preset entry structure
// ====================================================================================
struct PresetEntry {
    char name[25];
    uint8_t slotId;
};

// ====================================================================================
// Virtual menu state
// ====================================================================================
static PresetEntry presetList[SLOTS_TOTAL];
static uint8_t presetCount = 0;
static uint8_t presetCursor = 0;
static bool presetMenuActive = false;
static bool presetLoading = false;
static bool statusBarSelected = false;

// ====================================================================================
// Helper: Load preset list from file
// ====================================================================================
static void loadPresetList() {
    presetCount = 0;
    File file = LittleFS.open(SLOTS_FILE, "r");
    if (!file) return;

    SlotMeta slot;
    for (uint8_t i = 0; i < SLOTS_TOTAL; ++i) {
        file.seek(i * sizeof(SlotMeta));
        file.read((byte *)&slot, sizeof(SlotMeta));
        if (strcmp(EMPTY_SLOT_NAME, slot.name) == 0 || !slot.name[0]) continue;
        strncpy(presetList[presetCount].name, slot.name, 24);
        presetList[presetCount].name[24] = '\0';
        presetList[presetCount].slotId = slot.slot;
        presetCount++;
    }
    file.close();
}

// ====================================================================================
// Helper: Draw preset list on display
// ====================================================================================
static void drawPresetList(OLEDDisplay *display) {
    display->clear();
    display->setColor(OLEDDISPLAY_COLOR::WHITE);

    // Draw status bar with back button (highlighted if selected)
    if (statusBarSelected) {
        display->fillRect(0, 0, OLED_MENU_WIDTH, OLED_MENU_STATUS_BAR_HEIGHT);
        display->setColor(OLEDDISPLAY_COLOR::BLACK);
    }
    display->drawXbm(0, 0, IMAGE_ITEM(OM_STATUS_BAR_BACK));
    display->setColor(OLEDDISPLAY_COLOR::WHITE);

    // Draw page indicator (only if not on status bar)
    if (!statusBarSelected) {
        display->setFont(DejaVu_Sans_Mono_10);
        display->setTextAlignment(OLEDDISPLAY_TEXT_ALIGNMENT::TEXT_ALIGN_RIGHT);
        char buf[8];
        snprintf(buf, sizeof(buf), "%d/%d", presetCursor + 1, presetCount);
        display->drawString(OLED_MENU_WIDTH, 1, buf);
    }

    // Draw visible presets
    display->setFont(DejaVu_Sans_Mono_12);
    display->setTextAlignment(OLEDDISPLAY_TEXT_ALIGNMENT::TEXT_ALIGN_LEFT);

    const uint8_t itemHeight = 16;
    const uint8_t startY = OLED_MENU_STATUS_BAR_HEIGHT;
    const uint8_t visibleItems = (OLED_MENU_HEIGHT - startY) / itemHeight;

    // Calculate which items to show (keep cursor centered when possible)
    uint8_t startIdx = 0;
    if (presetCount > visibleItems) {
        if (presetCursor >= visibleItems / 2) {
            startIdx = presetCursor - visibleItems / 2;
        }
        if (startIdx + visibleItems > presetCount) {
            startIdx = presetCount - visibleItems;
        }
    }

    for (uint8_t i = 0; i < visibleItems && (startIdx + i) < presetCount; i++) {
        uint8_t idx = startIdx + i;
        uint8_t y = startY + i * itemHeight;

        if (idx == presetCursor && !statusBarSelected) {
            // Highlight selected item (only if status bar not selected)
            display->setColor(OLEDDISPLAY_COLOR::WHITE);
            display->fillRect(0, y, OLED_MENU_WIDTH, itemHeight);
            display->setColor(OLEDDISPLAY_COLOR::BLACK);
        } else {
            display->setColor(OLEDDISPLAY_COLOR::WHITE);
        }
        display->drawString(2, y, presetList[idx].name);
    }

    display->display();
}

// ====================================================================================
// Helper: Draw "No Presets" message
// ====================================================================================
static void drawNoPresets(OLEDDisplay *display) {
    display->clear();
    display->setColor(OLEDDISPLAY_COLOR::WHITE);
    display->drawXbm(0, 0, IMAGE_ITEM(OM_STATUS_BAR_BACK));
    display->drawXbm(0, OLED_MENU_STATUS_BAR_HEIGHT, IMAGE_ITEM(TEXT_NO_PRESETS));
    display->display();
}

// ====================================================================================
// Helper: Apply selected preset
// ====================================================================================
static void applySelectedPreset(OLEDMenuManager *manager) {
    if (presetCursor >= presetCount) return;

    PresetEntry &entry = presetList[presetCursor];
    uopt->presetSlot = (entry.slotId < 26) ? ('A' + entry.slotId) : ('0' + entry.slotId - 26);
    uopt->presetPreference = PresetPreference::OutputCustomized;

    // Show loading feedback
    OLEDDisplay *display = manager->getDisplay();
    display->clear();
    display->setColor(OLEDDISPLAY_COLOR::WHITE);
    display->setFont(ArialMT_Plain_16);
    display->setTextAlignment(OLEDDISPLAY_TEXT_ALIGNMENT::TEXT_ALIGN_CENTER);
    display->drawString(OLED_MENU_WIDTH / 2, 16, entry.name);
    display->drawXbm((OLED_MENU_WIDTH - TEXT_LOADED_WIDTH) / 2, OLED_MENU_HEIGHT / 2, IMAGE_ITEM(TEXT_LOADED));
    display->display();

    // Load and apply preset
    loadSlotSettings();
    applyPresets(rto->videoStandardInput);
    saveUserPrefs();

    // Set freeze state
    presetLoading = true;
    oledMenuFreezeTimeoutInMS = 2000;
    oledMenuFreezeStartTime = millis();
    manager->freeze();
}

// ====================================================================================
// Main virtual menu handler
// ====================================================================================
bool presetsVirtualMenuHandler(OLEDMenuManager *manager, OLEDMenuItem *, OLEDMenuNav nav, bool isFirstTime) {
    OLEDDisplay *display = manager->getDisplay();

    // Handle freeze state (after preset loading)
    if (presetLoading) {
        if (millis() - oledMenuFreezeStartTime >= oledMenuFreezeTimeoutInMS) {
            presetLoading = false;
            drawPresetList(display);
        }
        return false;
    }

    // First time entering the menu
    if (isFirstTime && !presetMenuActive) {
        loadPresetList();
        presetCursor = 0;
        statusBarSelected = false;
        presetMenuActive = true;

        if (presetCount == 0) {
            drawNoPresets(display);
            manager->freeze();
            oledMenuFreezeTimeoutInMS = 2000;
            oledMenuFreezeStartTime = millis();
            presetLoading = true;
            return false;
        }

        drawPresetList(display);
        manager->freeze();
        return false;
    }

    // Handle navigation while menu is active
    if (presetMenuActive && presetCount > 0) {
        // Reset screen saver timer on any key press
        if (nav != OLEDMenuNav::IDLE) {
            manager->resetScreenSaverTimer();
        }

        switch (nav) {
            case OLEDMenuNav::UP:
                if (statusBarSelected) {
                    // From status bar, go to last preset
                    statusBarSelected = false;
                    presetCursor = presetCount - 1;
                } else if (presetCursor > 0) {
                    presetCursor--;
                } else {
                    // From first preset, go to status bar
                    statusBarSelected = true;
                }
                drawPresetList(display);
                break;

            case OLEDMenuNav::DOWN:
                if (statusBarSelected) {
                    // From status bar, go to first preset
                    statusBarSelected = false;
                    presetCursor = 0;
                } else if (presetCursor < presetCount - 1) {
                    presetCursor++;
                } else {
                    // From last preset, go to status bar
                    statusBarSelected = true;
                }
                drawPresetList(display);
                break;

            case OLEDMenuNav::ENTER:
                if (statusBarSelected) {
                    // Go back to parent menu
                    presetMenuActive = false;
                    statusBarSelected = false;
                    manager->unfreeze();
                    manager->goBack();
                } else {
                    applySelectedPreset(manager);
                }
                break;

            case OLEDMenuNav::IDLE:
                drawPresetList(display);
                break;
        }
    }

    return false;
}
