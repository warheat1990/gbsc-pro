// ====================================================================================
// menu-profile.cpp
// IR Menu Handler for Profile Management
// ====================================================================================

#include "menu-common.h"
#include "../osd-render-pro.h"
#include "../drivers/ir_remote.h"

// ====================================================================================
// IR_handleProfileManagement - Profile Management Menu
// ====================================================================================

bool IR_handleProfileManagement()
{
    // Row 1: Load profile - loads preset from selected slot
    if (handleProfileRow(true)) return true;

    // Row 2: Save profile - saves current settings to selected slot
    if (handleProfileRow(false)) return true;

    return false;
}
