// ====================================================================================
// menu-unused.cpp
// Disabled/Commented IR Menu Handlers for Future Implementation
//
// This file contains menu handlers that are currently disabled but preserved
// for future implementation. They are kept as commented code blocks.
// ====================================================================================

#include "menu-common.h"

// ====================================================================================
// Unused External References (uncomment when enabling handlers)
// ====================================================================================

// extern char serialCommand;
// extern char userCommand;

// ====================================================================================
// Output Resolution - Downscale (TODO)
// ====================================================================================

// TODO: Re-enable when Downscale feature is implemented
// if (oled_menuItem == OLED_OutputResolution_Downscale) {
//     showMenu("Menu->Output", "Downscale");
//
//     if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
//         OSD_highlightIcon(3);
//         OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
//     }
//
//     if (irDecode()) {
//         switch (results.value) {
//             case IR_KEY_MENU:
//                 OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
//                 OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
//                 oled_menuItem = OLED_OutputResolution;
//                 break;
//             case IR_KEY_UP:
//                 selectedMenuLine = 2;
//                 OSD_handleCommand(OSD_CMD_OUTPUT_720_480);
//                 oled_menuItem = OLED_OutputResolution_480;
//                 break;
//             case IR_KEY_DOWN:
//                 oled_menuItem = OLED_OutputResolution_PassThrough;
//                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
//                 OSD_handleCommand(OSD_CMD_OUTPUT_PASSTHROUGH);
//                 break;
//             case IR_KEY_OK:
//                 userCommand = 'L';
//                 break;
//             case IR_KEY_EXIT:
//                 exitMenu();
//                 break;
//         }
//         irResume();
//     }
//     return true;
// }

// ====================================================================================
// System Settings - Component/VGA (disabled)
// ====================================================================================

// OLED_SystemSettings_ComponentVGA (disabled)
// else if (oled_menuItem == OLED_SystemSettings_ComponentVGA) {
//     showMenu("Menu->System", "Component/VGA");
//
//     if (results.value == IR_KEY_UP) {
//         OSD_highlightIcon(1);
//         OSD_handleCommand(OSD_CMD_SYS_PAGE2);
//     }
//     OSD_handleCommand(OSD_CMD_SYS_PAGE2_VALUES);
//
//     if (irDecode()) {
//         switch (results.value) {
//             case IR_KEY_MENU:
//                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
//                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
//                 oled_menuItem = OLED_SystemSettings;
//                 break;
//             case IR_KEY_UP:
//                 oled_menuItem = OLED_SystemSettings_UseUpscaling;
//                 OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
//                 OSD_handleCommand(OSD_CMD_SYS_PAGE1);
//                 break;
//             case IR_KEY_DOWN:
//                 selectedMenuLine = 2;
//                 OSD_handleCommand(OSD_CMD_SYS_PAGE2);
//                 oled_menuItem = OLED_SystemSettings_Force5060Hz;
//                 break;
//             case IR_KEY_RIGHT:
//                 serialCommand = 'L';
//                 break;
//             case IR_KEY_LEFT:
//                 serialCommand = 'L';
//                 break;
//             case IR_KEY_EXIT:
//                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
//                 OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
//                 oled_menuItem = OLED_Input;
//                 break;
//         }
//         irResume();
//     }
//     return true;
// }

// ====================================================================================
// Developer Menu (disabled)
// ====================================================================================

// OLED_Developer (disabled)
// if (oled_menuItem == OLED_Developer) {
//     showMenu("Menu->>>", "Developer");
//
//     if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
//         OSD_highlightIcon(2);
//     }
//
//     if (irDecode()) {
//         switch (results.value) {
//             case IR_KEY_UP:
//                 selectedMenuLine = 1;
//                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
//                 oled_menuItem = OLED_SystemSettings;
//                 break;
//             case IR_KEY_DOWN:
//                 selectedMenuLine = 3;
//                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
//                 oled_menuItem = OLED_ResetSettings;
//                 break;
//             case IR_KEY_OK:
//                 oled_menuItem = OLED_Developer_MemoryAdjust;
//                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
//                 OSD_handleCommand(OSD_CMD_DEV_MEMORY);
//                 break;
//             case IR_KEY_EXIT:
//                 exitMenu();
//                 break;
//         }
//         irResume();
//     }
//     return true;
// }

// OLED_Developer_MemoryAdjust (disabled)
// else if (oled_menuItem == OLED_Developer_MemoryAdjust) {
//     showMenu("Menu->Dev", "MEM left/right");
//
//     if (results.value == IR_KEY_UP) {
//         OSD_highlightIcon(1);
//         OSD_handleCommand(OSD_CMD_DEV_MEMORY);
//     }
//     OSD_handleCommand(OSD_CMD_DEV_MEMORY_VALUES);
//
//     if (irDecode()) {
//         switch (results.value) {
//             case IR_KEY_MENU:
//                 OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
//                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
//                 oled_menuItem = OLED_Developer;
//                 break;
//             case IR_KEY_DOWN:
//                 selectedMenuLine = 2;
//                 OSD_handleCommand(OSD_CMD_DEV_MEMORY);
//                 oled_menuItem = OLED_Developer_HSyncAdjust;
//                 break;
//             case IR_KEY_RIGHT:
//                 serialCommand = '+';
//                 break;
//             case IR_KEY_LEFT:
//                 serialCommand = '-';
//                 break;
//             case IR_KEY_EXIT:
//                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
//                 OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
//                 oled_menuItem = OLED_Input;
//                 break;
//         }
//         irResume();
//     }
//     return true;
// }

// OLED_Developer_HSyncAdjust (disabled)
// else if (oled_menuItem == OLED_Developer_HSyncAdjust) {
//     showMenu("Menu->Dev", "HS left/right");
//
//     if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
//         OSD_highlightIcon(2);
//     }
//     OSD_handleCommand(OSD_CMD_DEV_MEMORY_VALUES);
//
//     if (irDecode()) {
//         switch (results.value) {
//             case IR_KEY_MENU:
//                 OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
//                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
//                 oled_menuItem = OLED_Developer;
//                 break;
//             case IR_KEY_UP:
//                 selectedMenuLine = 1;
//                 OSD_handleCommand(OSD_CMD_DEV_MEMORY);
//                 oled_menuItem = OLED_Developer_MemoryAdjust;
//                 break;
//             case IR_KEY_DOWN:
//                 selectedMenuLine = 3;
//                 OSD_handleCommand(OSD_CMD_DEV_MEMORY);
//                 oled_menuItem = OLED_Developer_HTotalAdjust;
//                 break;
//             case IR_KEY_RIGHT:
//                 serialCommand = '0';
//                 break;
//             case IR_KEY_LEFT:
//                 serialCommand = '1';
//                 break;
//             case IR_KEY_EXIT:
//                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
//                 OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
//                 oled_menuItem = OLED_Input;
//                 break;
//         }
//         irResume();
//     }
//     return true;
// }

// OLED_Developer_HTotalAdjust (disabled)
// else if (oled_menuItem == OLED_Developer_HTotalAdjust) {
//     showMenu("Menu->Dev", "HTotal -/+");
//
//     if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
//         OSD_highlightIcon(3);
//         OSD_handleCommand(OSD_CMD_DEV_MEMORY);
//     }
//     OSD_handleCommand(OSD_CMD_DEV_MEMORY_VALUES);
//
//     if (irDecode()) {
//         switch (results.value) {
//             case IR_KEY_MENU:
//                 OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
//                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
//                 oled_menuItem = OLED_Developer;
//                 break;
//             case IR_KEY_UP:
//                 selectedMenuLine = 2;
//                 OSD_handleCommand(OSD_CMD_DEV_MEMORY);
//                 oled_menuItem = OLED_Developer_HSyncAdjust;
//                 break;
//             case IR_KEY_DOWN:
//                 oled_menuItem = OLED_Developer_DebugView;
//                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
//                 OSD_handleCommand(OSD_CMD_DEV_DEBUG);
//                 break;
//             case IR_KEY_RIGHT:
//                 serialCommand = 'a';
//                 break;
//             case IR_KEY_LEFT:
//                 serialCommand = 'A';
//                 break;
//             case IR_KEY_EXIT:
//                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
//                 OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
//                 oled_menuItem = OLED_Input;
//                 break;
//         }
//         irResume();
//     }
//     return true;
// }

// OLED_Developer_DebugView (disabled)
// else if (oled_menuItem == OLED_Developer_DebugView) {
//     showMenu("Menu->Dev", "Debug view");
//
//     if (results.value == IR_KEY_UP) {
//         OSD_highlightIcon(1);
//         OSD_handleCommand(OSD_CMD_DEV_DEBUG);
//     }
//     OSD_handleCommand(OSD_CMD_DEV_DEBUG_VALUES);
//
//     if (irDecode()) {
//         switch (results.value) {
//             case IR_KEY_MENU:
//                 OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
//                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
//                 oled_menuItem = OLED_Developer;
//                 break;
//             case IR_KEY_UP:
//                 oled_menuItem = OLED_Developer_HTotalAdjust;
//                 OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
//                 OSD_handleCommand(OSD_CMD_DEV_MEMORY);
//                 break;
//             case IR_KEY_DOWN:
//                 selectedMenuLine = 2;
//                 OSD_handleCommand(OSD_CMD_DEV_DEBUG);
//                 oled_menuItem = OLED_Developer_ADCFilter;
//                 break;
//             case IR_KEY_RIGHT:
//                 serialCommand = 'D';
//                 break;
//             case IR_KEY_LEFT:
//                 serialCommand = 'D';
//                 break;
//             case IR_KEY_EXIT:
//                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
//                 OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
//                 oled_menuItem = OLED_Input;
//                 break;
//         }
//         irResume();
//     }
//     return true;
// }

// OLED_Developer_ADCFilter (disabled)
// else if (oled_menuItem == OLED_Developer_ADCFilter) {
//     showMenu("Menu->Dev", "ADC filter");
//
//     if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
//         OSD_highlightIcon(2);
//     }
//     OSD_handleCommand(OSD_CMD_DEV_DEBUG_VALUES);
//
//     if (irDecode()) {
//         switch (results.value) {
//             case IR_KEY_MENU:
//                 OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
//                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
//                 oled_menuItem = OLED_Developer;
//                 break;
//             case IR_KEY_UP:
//                 selectedMenuLine = 1;
//                 OSD_handleCommand(OSD_CMD_DEV_DEBUG);
//                 oled_menuItem = OLED_Developer_DebugView;
//                 break;
//             case IR_KEY_DOWN:
//                 selectedMenuLine = 3;
//                 OSD_handleCommand(OSD_CMD_DEV_DEBUG);
//                 oled_menuItem = OLED_Developer_FreezeCapture;
//                 break;
//             case IR_KEY_RIGHT:
//                 serialCommand = 'F';
//                 break;
//             case IR_KEY_LEFT:
//                 serialCommand = 'F';
//                 break;
//             case IR_KEY_EXIT:
//                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
//                 OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
//                 oled_menuItem = OLED_Input;
//                 break;
//         }
//         irResume();
//     }
//     return true;
// }

// OLED_Developer_FreezeCapture (disabled)
// else if (oled_menuItem == OLED_Developer_FreezeCapture) {
//     showMenu("Menu->Dev", "Freeze capture");
//
//     if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
//         OSD_highlightIcon(3);
//         OSD_handleCommand(OSD_CMD_DEV_DEBUG);
//     }
//     OSD_handleCommand(OSD_CMD_DEV_DEBUG_VALUES);
//
//     if (irDecode()) {
//         switch (results.value) {
//             case IR_KEY_MENU:
//                 OSD_handleCommand(OSD_CMD_CURSOR_ROW2);
//                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
//                 oled_menuItem = OLED_Developer;
//                 break;
//             case IR_KEY_UP:
//                 selectedMenuLine = 2;
//                 OSD_handleCommand(OSD_CMD_DEV_DEBUG);
//                 oled_menuItem = OLED_Developer_ADCFilter;
//                 break;
//             case IR_KEY_DOWN:
//                 oled_menuItem = OLED_Developer_MemoryAdjust;
//                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
//                 OSD_handleCommand(OSD_CMD_DEV_MEMORY);
//                 break;
//             case IR_KEY_RIGHT:
//                 userCommand = 'F';
//                 break;
//             case IR_KEY_LEFT:
//                 userCommand = 'F';
//                 break;
//             case IR_KEY_EXIT:
//                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
//                 OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
//                 oled_menuItem = OLED_Input;
//                 break;
//         }
//         irResume();
//     }
//     return true;
// }

// ====================================================================================
// Reset/Misc Settings (disabled)
// ====================================================================================

// OLED_EnableOTA (disabled)
// else if (oled_menuItem == OLED_EnableOTA) {
//     showMenu("Menu->Reset", "Enable OTA");
//
//     if (results.value == IR_KEY_UP) {
//         OSD_highlightIcon(1);
//         OSD_handleCommand(OSD_CMD_SYS_PAGE5);
//     }
//     OSD_handleCommand(OSD_CMD_SYS_PAGE5_VALUES);
//
//     if (irDecode()) {
//         switch (results.value) {
//             case IR_KEY_MENU:
//                 OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
//                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
//                 oled_menuItem = OLED_ResetSettings;
//                 break;
//             case IR_KEY_DOWN:
//                 selectedMenuLine = 2;
//                 OSD_handleCommand(OSD_CMD_SYS_PAGE5);
//                 oled_menuItem = OLED_Restart;
//                 break;
//             case IR_KEY_RIGHT:
//                 serialCommand = 'c';
//                 break;
//             case IR_KEY_LEFT:
//                 serialCommand = 'c';
//                 break;
//             case IR_KEY_EXIT:
//                 OSD_handleCommand(OSD_CMD_CURSOR_ROW1);
//                 OSD_handleCommand(OSD_CMD_MAIN_PAGE1);
//                 oled_menuItem = OLED_Input;
//                 break;
//         }
//         irResume();
//     }
//     return true;
// }

// OLED_ResetDefaults (disabled)
// else if (oled_menuItem == OLED_ResetDefaults) {
//     showMenu("Menu->Reset", "Reset defaults");
//
//     if (results.value == IR_KEY_DOWN || results.value == IR_KEY_UP) {
//         OSD_highlightIcon(3);
//     }
//     OSD_handleCommand(OSD_CMD_SYS_PAGE5_VALUES);
//
//     if (irDecode()) {
//         switch (results.value) {
//             case IR_KEY_MENU:
//                 OSD_handleCommand(OSD_CMD_CURSOR_ROW3);
//                 OSD_handleCommand(OSD_CMD_MAIN_PAGE2);
//                 oled_menuItem = OLED_ResetSettings;
//                 break;
//             case IR_KEY_UP:
//                 selectedMenuLine = 2;
//                 OSD_handleCommand(OSD_CMD_SYS_PAGE5);
//                 oled_menuItem = OLED_Restart;
//                 break;
//             case IR_KEY_OK:
//                 userCommand = '1';
//                 break;
//             case IR_KEY_EXIT:
//                 exitMenu();
//                 break;
//         }
//         irResume();
//     }
//     return true;
// }
