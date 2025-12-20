/*
 * OSD Profile Name Configuration
 * Custom profile names for TV OSD display (9 characters max)
 *
 * Original: Karabanov Aleksandr (2024-02-16)
 * https://www.youtube.com/channel/UCkNk5gQMIu8k9xW-uENsBzw
 *
 * Refactored: December 2024 - Using profileChars[] array and setProfileName()
 *
 * Character codes:
 *   =       -       .       /       :       '
 * 0x3D    0x3E    0x2E    0x2F    0x3A    0x27
 *
 * Available characters:
 * A-Z (uppercase), a-z (lowercase), _0_ to _9_ (digits)
 *
 * Profile name array: profileChars[9] (9 character positions, 0-indexed)
 * Max profiles: 20
 *
 * Example - "Sega.2...":
 * setProfileName('S', 'e', 'g', 'a', 0x2E, _2_, 0x2E, 0x2E, 0x2E);
 */

// ====================================================================================
// Profile Name Helper
// ====================================================================================

// Set all 9 profile name characters at once
inline void setProfileName(char c0, char c1, char c2, char c3, char c4,
                           char c5, char c6, char c7, char c8)
{
    profileChars[0] = c0;
    profileChars[1] = c1;
    profileChars[2] = c2;
    profileChars[3] = c3;
    profileChars[4] = c4;
    profileChars[5] = c5;
    profileChars[6] = c6;
    profileChars[7] = c7;
    profileChars[8] = c8;
}

// ====================================================================================
// Profile Name Functions (name_1 to name_20)
// All display "profile-N" format
// ====================================================================================

inline void name_1()  { setProfileName(p, r, o, f, i, l, e, 0x3E, _1_); }
inline void name_2()  { setProfileName(p, r, o, f, i, l, e, 0x3E, _2_); }
inline void name_3()  { setProfileName(p, r, o, f, i, l, e, 0x3E, _3_); }
inline void name_4()  { setProfileName(p, r, o, f, i, l, e, 0x3E, _4_); }
inline void name_5()  { setProfileName(p, r, o, f, i, l, e, 0x3E, _5_); }
inline void name_6()  { setProfileName(p, r, o, f, i, l, e, 0x3E, _6_); }
inline void name_7()  { setProfileName(p, r, o, f, i, l, e, 0x3E, _7_); }
inline void name_8()  { setProfileName(p, r, o, f, i, l, e, 0x3E, _8_); }
inline void name_9()  { setProfileName(p, r, o, f, i, l, e, 0x3E, _9_); }
inline void name_10() { setProfileName(p, r, o, f, i, l, e, _1_, _0_); }
inline void name_11() { setProfileName(p, r, o, f, i, l, e, _1_, _1_); }
inline void name_12() { setProfileName(p, r, o, f, i, l, e, _1_, _2_); }
inline void name_13() { setProfileName(p, r, o, f, i, l, e, _1_, _3_); }
inline void name_14() { setProfileName(p, r, o, f, i, l, e, _1_, _4_); }
inline void name_15() { setProfileName(p, r, o, f, i, l, e, _1_, _5_); }
inline void name_16() { setProfileName(p, r, o, f, i, l, e, _1_, _6_); }
inline void name_17() { setProfileName(p, r, o, f, i, l, e, _1_, _7_); }
inline void name_18() { setProfileName(p, r, o, f, i, l, e, _1_, _8_); }
inline void name_19() { setProfileName(p, r, o, f, i, l, e, _1_, _9_); }
inline void name_20() { setProfileName(p, r, o, f, i, l, e, _2_, _0_); }
