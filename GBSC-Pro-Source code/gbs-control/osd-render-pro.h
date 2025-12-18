// ====================================================================================
// osd-render-pro.h
// TV OSD Menu Rendering and Handlers
//
// This file contains declarations for:
// - OSD_writeChar(), OSD_writeString(), OSD_writeStringAtLine(): TV OSD display helpers
// - menuTable[]: Command dispatch table
// - OSD_handleCommand(): Menu command dispatcher
// - handle_X(): Individual menu screen rendering functions
//
// Note: MenuEntry type is defined in gbs-control-pro.h
// ====================================================================================

#pragma once

#include <Arduino.h>

// ====================================================================================
// Function Declarations - TV OSD Display Helpers
// ====================================================================================

void OSD_writeChar(const int T, const char C);
void OSD_writeString(uint8_t start, const char str[]);
void OSD_writeStringAtLine(int startPos, int row, const char *str);

// ====================================================================================
// External Variables - Menu Table
// ====================================================================================

extern const MenuEntry menuTable[];
extern const size_t menuTableSize;

// ====================================================================================
// Function Declarations - Menu Dispatcher
// ====================================================================================

void OSD_handleCommand(char incomingByte);

// ====================================================================================
// Function Declarations - Menu Handlers (0-9)
// ====================================================================================

void handle_0(void);
void handle_1(void);
void handle_2(void);
void handle_3(void);
void handle_4(void);
void handle_5(void);
void handle_6(void);
void handle_7(void);
void handle_8(void);
void handle_9(void);

// ====================================================================================
// Function Declarations - Menu Handlers (a-z)
// ====================================================================================

void handle_a(void);
void handle_b(void);
void handle_c(void);
void handle_d(void);
void handle_e(void);
void handle_f(void);
void handle_g(void);
void handle_h(void);
void handle_i(void);
void handle_j(void);
void handle_k(void);
void handle_l(void);
void handle_m(void);
void handle_n(void);
void handle_o(void);
void handle_p(void);
void handle_q(void);
void handle_r(void);
void handle_s(void);
void handle_t(void);
void handle_u(void);
void handle_v(void);
void handle_w(void);
void handle_x(void);
void handle_y(void);
void handle_z(void);

// ====================================================================================
// Function Declarations - Menu Handlers (Special)
// ====================================================================================

void handle_A(void);
void handle_caret(void);      // ^
void handle_at(void);         // @
void handle_exclamation(void); // !
void handle_hash(void);       // #
void handle_dollar(void);     // $
void handle_percent(void);    // %
void handle_ampersand(void);  // &
void handle_asterisk(void);   // *
