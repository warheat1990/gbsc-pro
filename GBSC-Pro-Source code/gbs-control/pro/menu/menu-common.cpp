// ====================================================================================
// menu-common.cpp
// Shared helper functions for IR menu handlers
// ====================================================================================

#include "menu-common.h"
#include "../../OLEDMenuManager.h"
#include "../../OLEDMenuImplementation.h"

#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <SSD1306Wire.h>

#include "../drivers/pt2257.h"

// ====================================================================================
// External References
// ====================================================================================

extern SSD1306Wire display;
extern struct userOptions *uopt;
extern char userCommand;
extern void saveUserPrefs();

extern OLEDMenuManager oledMenu;
extern unsigned long oledMenuFreezeStartTime;
extern unsigned long oledMenuFreezeTimeoutInMS;

// ====================================================================================
// OLED Display Helpers
// ====================================================================================

void oledPrepare() {
    if (oledClearFlag) display.clear();
    oledClearFlag = ~0;
    display.setColor(OLEDDISPLAY_COLOR::WHITE);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_16);
}

void showMenu(const char* title, const char* label) {
    oledPrepare();
    display.drawString(1, 0, title);
    display.drawString(1, 28, label);
    display.display();
}

void showMenuCentered(const char* text, uint8_t x, uint8_t y) {
    oledPrepare();
    display.drawString(x, y, text);
    display.display();
}

void showMenuToggle(const char* title, const char* label, bool isOn) {
    oledPrepare();
    display.drawString(1, 0, title);
    display.drawString(1, 22, label);
    display.drawString(1, 44, isOn ? "ON" : "OFF");
    display.display();
}

void showMenuValue(const char* title, const char* label, const char* value) {
    oledPrepare();
    display.drawString(1, 0, title);
    display.drawString(1, 22, label);
    display.drawString(1, 44, value);
    display.display();
}

void exitMenu() {
    oled_menuItem = OLED_None;
    OSD_displayOff();   // Turn off OSD before clearing (avoids glitch)
    OSD_clearAll();
    OSD_init();
}

// ====================================================================================
// IR Receiver Helpers
// ====================================================================================

bool irDecode() {
    if (irrecv.decode(&results)) {
        irDecodedFlag = 1;
        return true;
    }
    return false;
}

void irResume() {
    irrecv.resume();
}

// ====================================================================================
// OLED Menu Feedback Helpers
// ====================================================================================

void showSelectionFeedback(OLEDMenuManager *manager, const char* label) {
    oledMenuFreezeTimeoutInMS = 1000;
    oledMenuFreezeStartTime = millis();
    OLEDDisplay *oledDisplay = manager->getDisplay();
    oledDisplay->clear();
    oledDisplay->setColor(OLEDDISPLAY_COLOR::WHITE);
    oledDisplay->setFont(ArialMT_Plain_16);
    oledDisplay->setTextAlignment(OLEDDISPLAY_TEXT_ALIGNMENT::TEXT_ALIGN_CENTER);
    oledDisplay->drawString(OLED_MENU_WIDTH / 2, 16, label);
    oledDisplay->drawXbm((OLED_MENU_WIDTH - TEXT_LOADED_WIDTH) / 2, OLED_MENU_HEIGHT / 2, IMAGE_ITEM(TEXT_LOADED));
    oledDisplay->display();
    manager->freeze();
}

bool checkFreezeTimeout(OLEDMenuManager *manager) {
    if (millis() - oledMenuFreezeStartTime >= oledMenuFreezeTimeoutInMS) {
        manager->unfreeze();
        return false;
    }
    return true;
}

// ====================================================================================
// Profile Navigation Helpers
// ====================================================================================

int getLoadSlotIndex(int state) {
    int idx = state - OLED_Profile_Load1;
    return (idx >= 0 && idx < 20) ? idx : -1;
}

int getSaveSlotIndex(int state) {
    int idx = state - OLED_Profile_Save1;
    return (idx >= 0 && idx < 20) ? idx : -1;
}

OLED_MenuState getNextSlot(OLED_MenuState base, int idx) {
    return (OLED_MenuState)(base + ((idx + 1) % 20));
}

OLED_MenuState getPrevSlot(OLED_MenuState base, int idx) {
    return (OLED_MenuState)(base + ((idx + 19) % 20));
}

char getSlotChar(int idx) {
    return 'A' + idx;
}

bool handleProfileRow(bool isLoadRow) {
    int idx = isLoadRow ? getLoadSlotIndex(oled_menuItem) : getSaveSlotIndex(oled_menuItem);
    if (idx < 0) return false;

    OLED_MenuState base = isLoadRow ? OLED_Profile_Load1 : OLED_Profile_Save1;

    // Special case: first Load slot shows menu header
    if (isLoadRow && idx == 0) {
        showMenu("Menu-", "Profile");
    }

    // Visual feedback for up/down navigation
    if (results.value == IR_KEY_UP || results.value == IR_KEY_DOWN) {
        OSD_writeCharAtRow(1, 0, arrow_right_icon, isLoadRow ? OSD_CURSOR_ACTIVE : OSD_BACKGROUND);
        OSD_writeCharAtRow(2, 0, arrow_right_icon, isLoadRow ? OSD_BACKGROUND : OSD_CURSOR_ACTIVE);
        OSD_writeCharAtRow(3, 0, arrow_right_icon, OSD_BACKGROUND);
        if (isLoadRow) OSD_handleCommand(OSD_CMD_PROFILE_SAVELOAD);
    }

    OSD_handleCommand(OSD_CMD_PROFILE_SLOTDISPLAY);

    if (irDecode()) {
        switch (results.value) {
            case IR_KEY_MENU:
                exitMenu();
                break;
            case IR_KEY_UP:
                if (!isLoadRow) {
                    oled_menuItem = OLED_Profile_Load1;
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW1);
                    OSD_handleCommand(OSD_CMD_PROFILE_SAVELOAD);
                }
                break;
            case IR_KEY_DOWN:
                if (isLoadRow) {
                    oled_menuItem = OLED_Profile_Save1;
                    OSD_handleCommand(OSD_CMD_PAGE_CHANGE_ROW2);
                    OSD_handleCommand(OSD_CMD_PROFILE_SAVELOAD);
                }
                break;
            case IR_KEY_RIGHT:
                oled_menuItem = getNextSlot(base, idx);
                break;
            case IR_KEY_LEFT:
                oled_menuItem = getPrevSlot(base, idx);
                break;
            case IR_KEY_OK:
                uopt->presetSlot = getSlotChar(idx);
                if (isLoadRow) {
                    OSD_handleCommand(OSD_CMD_PROFILE_SLOTROW1);
                } else {
                    uopt->presetPreference = OutputCustomized;
                    saveUserPrefs();
                    userCommand = '4';
                    OSD_showOkFeedback(ROW_2);
                }
                break;
            case IR_KEY_EXIT:
                exitMenu();
                break;
        }
        irResume();
    }
    return true;
}
