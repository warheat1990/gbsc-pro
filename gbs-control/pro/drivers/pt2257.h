/*
 * PT2257 Electronic Volume Controller Driver
 * 2-channel I2C audio attenuator (0 to -79 dB)
 *
 * Datasheet: https://www.princeton.com.tw/Portals/0/Product/PT2257.pdf
 * Original: Karabanov Aleksandr (2024-04-21)
 */

#ifndef PT2257_H_
#define PT2257_H_

#include <stdint.h>

// I2C address (7-bit)
#define PT2257_ADDR 0x44

// Attenuation command bases
#define PT2257_ATT_1DB   0xD0  // 0xD0-0xD9 = -0 to -9 dB (1 dB steps)
#define PT2257_ATT_10DB  0xE0  // 0xE0-0xE7 = -0 to -70 dB (10 dB steps)

// Mute control
#define PT2257_MUTE_ON   0x79
#define PT2257_MUTE_OFF  0x78

// Set attenuation level (0-79 dB, both channels)
// Attenuation 0 = 0 dB (max), Attenuation 79 = -79 dB (min)
inline void PT2257_setAttenuation(uint8_t dB) {
    if (dB > 79) dB = 79;
    Wire.beginTransmission(PT2257_ADDR);
    Wire.write(PT2257_ATT_10DB + (dB / 10));  // Tens: -0, -10, ... -70 dB
    Wire.write(PT2257_ATT_1DB + (dB % 10));   // Units: -0, -1, ... -9 dB
    Wire.endTransmission();
}

// Set volume level (0-50, where 50=max, 0=mute)
// Converts to attenuation internally: volume 50 = 0dB, volume 0 = -50dB
inline void PT2257_setVolume(uint8_t level) {
    uint8_t dB = (level > 50) ? 0 : (50 - level);
    PT2257_setAttenuation(dB);
}

// Mute/unmute audio
inline void PT2257_mute(bool mute) {
    Wire.beginTransmission(PT2257_ADDR);
    Wire.write(mute ? PT2257_MUTE_ON : PT2257_MUTE_OFF);
    Wire.endTransmission();
}

#endif // PT2257_H_
