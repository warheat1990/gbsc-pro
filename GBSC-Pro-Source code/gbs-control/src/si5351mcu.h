/*
 * si5351mcu - Si5351 library for Arduino MCU tuned for size and click-less
 *
 * Copyright (C) 2017 Pavel Milanes <pavelmc@gmail.com>
 *
 * Many chunk of codes are derived-from/copied from other libs
 * all GNU GPL licenced:
 *  - Linux Kernel (www.kernel.org)
 *  - Hans Summers libs and demo code (qrp-labs.com)
 *  - Etherkit (NT7S) Si5351 libs on github
 *  - DK7IH example.
 *  - Jerry Gaffke integer routines for the bitx20 group
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


/****************************************************************************
 *  This lib tricks:
 *
 * CLK0 will use PLLA
 * CLK1 will use PLLB
 * CLK2 will use PLLB
 *
 * Lib defaults
 * - XTAL is 27 Mhz.
 * - Always put the internal 8pF across the xtal legs to GND
 * - lowest power output (2mA)
 * - After the init all outputs are off, you need to enable them in your code.
 *
 * The correction procedure is not for the PPM as other libs, this
 * is just +/- Hz to the XTAL freq, you may get a click noise after
 * applying a correction
 *
 * The init procedure is mandatory as it set the Xtal (the default or a custom
 * one) and prepare the Wire (I2C) library for operation.
 ****************************************************************************/

#ifndef SI5351MCU_H
#define SI5351MCU_H

// rigor includes
#include "Arduino.h"
#include "Wire.h"

// default I2C address of the Si5351A - other variants may differ
#define SIADDR 0x60

// The number of output channels - 3 for Si5351A 10 pin
#define SICHANNELS 3

// register's power modifiers
#define SIOUT_2mA 0
#define SIOUT_4mA 1
#define SIOUT_6mA 2
#define SIOUT_8mA 3

// ============================================================================
// GBSC-Pro Si5351 Stability Enhancements
// ============================================================================

// Si5351 Status Register (0x00)
// Bit 7 (0x80): SYS_INIT - System initialization in progress
// Bit 5 (0x20): LOL_A - Loss of Lock PLL A
// Bit 4 (0x10): LOL_B - Loss of Lock PLL B
#define SI5351_STATUS_REG       0x00
#define SI5351_STATUS_SYS_INIT  0x80
#define SI5351_STATUS_LOL_A     0x20
#define SI5351_STATUS_LOL_B     0x10

// Si5351 Crystal Load Capacitance Register (183)
// Values: 0x52=6pF, 0x92=8pF, 0xD2=10pF
#define SI5351_XTAL_CL_REG      183
#define SI5351_XTAL_CL_6PF      0x52
#define SI5351_XTAL_CL_8PF      0x92
#define SI5351_XTAL_CL_10PF     0xD2

// Lock detection timeout in milliseconds
#define SI5351_LOCK_TIMEOUT_MS  80

// registers base (2mA by default)
#define SICLK0_R   76       // 0b01001100
#define SICLK12_R 108       // 0b01101100

class Si5351mcu {
    private:
        // base xtal freq, over this we apply the correction factor
        // by default 27 MHz
        uint32_t base_xtal = 27000000L;

        // this is the work value, with the correction applied
        // via the correction() procedure
        uint32_t int_xtal = base_xtal;

        // clk# power holders (2ma by default)
        uint8_t clkpower[SICHANNELS] = { 0 };

        // local var to keep track of when to reset the "pll"
        /*********************************************************
         * BAD CONCEPT on the datasheet and AN:
         *
         * The chip has a soft-reset for PLL A & B but in
         * practice the PLL does not need to be reseted.
         *
         * Test shows that if you fix the Msynth output
         * dividers and move any of the VCO from bottom to top
         * the frequency moves smooth and clean, no reset needed
         *
         * The reset is needed when you changes the value of the
         * Msynth output divider, even so it's not always needed
         * so we use this var to keep track of all three and only
         * reset the "PLL" when this value changes to be sure
         *
         * It's a word (16 bit) because the final max value is 900
         *********************************************************/
        uint16_t  omsynth[SICHANNELS] = { 0 };
        uint8_t   o_Rdiv[SICHANNELS] = { 0 };

    public:
        // var to check the clock state
        bool clkOn[SICHANNELS] = { 0 };     // This should not really be public - use isEnabled()

    public:
        // default init procedure
        void init(void);

        // custom init procedure (XTAL in Hz);
        void init(uint32_t);

        // reset all PLLs
        static void reset(void);

        // set CLKx(0..2) to freq (Hz)
        void setFreq(uint8_t, uint32_t);

        // pass a correction factor
        void correction(int32_t);

        // enable some CLKx output
        void enable(uint8_t);

        // disable some CLKx output
        void disable(uint8_t);

        // disable all outputs
        void off(void);

        // set power output to a specific clk
        void setPower(uint8_t, uint8_t);

        // used to talk with the chip, via Arduino Wire lib
        //
        // declared as static, since they do not reference any this-> class attributes
        //
        static void     i2cWrite( uint8_t reg, uint8_t val );
        static uint8_t  i2cWriteBurst( uint8_t start_register, const uint8_t *data, uint8_t numbytes );
        static int16_t  i2cRead( uint8_t reg );

        inline bool isEnabled( const uint8_t channel ) {
          return channel < SICHANNELS && clkOn[ channel ] != 0;
        };

        inline uint8_t getPower( const uint8_t channel ) {
          return channel < SICHANNELS ? clkpower[ channel ] : 0;
        };

        inline uint32_t getXtalBase( void ) {
          return base_xtal;
        };

        inline uint32_t getXtalCurrent( void ) {
          return int_xtal;
        };

        // ====================================================================
        // GBSC-Pro Si5351 Stability Enhancements
        // ====================================================================

        // Wait for PLL lock (SYS_INIT=0 and LOL_A=0)
        // Returns true if locked within timeout, false otherwise
        bool waitForLock(uint16_t timeout_ms = SI5351_LOCK_TIMEOUT_MS);

        // Read status register (0x00)
        uint8_t readStatus(void);

        // Check if PLL is locked (SYS_INIT=0 and LOL_A=0)
        bool isLocked(void);

        // Set XTAL load capacitance (6pF, 8pF, or 10pF)
        void setXtalCL(uint8_t cl_value);

        // Get current XTAL load capacitance setting
        uint8_t getXtalCL(void);

        // Auto-discover best XTAL load capacitance
        // Tests 10pF, 8pF, 6pF and returns the one that locks fastest
        uint8_t autoDiscoverXtalCL(uint32_t test_freq = 108000000L);

        // Store the discovered best XTAL CL value
        uint8_t bestXtalCL = SI5351_XTAL_CL_10PF;

};


#endif //SI5351MCU_H
