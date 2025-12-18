// ====================================================================================
// ir-menu-pro.h
// IR Remote and Menu Navigation
//
// This file contains declarations for:
// - IR_handleMenuSelection(): Main menu state machine
// - IR_handleInput(): IR remote input handler
// ====================================================================================

#pragma once

#include <Arduino.h>

// ====================================================================================
// Function Declarations
// ====================================================================================

void IR_handleMenuSelection(void);
void IR_handleInput(void);
