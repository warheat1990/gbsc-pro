/*
 * Created in 21.04.2024
 * By Karabanov Aleksandr
 * https://www.youtube.com/channel/UCkNk5gQMIu8k9xW-uENsBzw
 */

#define pt2257 0x44
// #define MUTE_ON 0x79
// #define MUTE_OFF 0x78

/*
 11010000 DO  0 dB          11100000 E0   0 dB
 11010001 D1 -1 dB          11100001 E1 -10 dB
 11010010 D2 -2 dB          11100010 E2 -20 dB
 11010011 D3 -3 dB          11100011 E3 -30 dB
 11010100 D4 -4 dB          11100100 E4 -40 dB
 11010101 D5 -5 dB          11100101 E5 -50 dB
 11010110 D6 -6 dB          11100110 E6 -60 dB
 11010111 D7 -7 dB          11100111 E7 -70 dB
 11011000 D8 -8 dB
 11011001 D9 -9 dB          11111111 FF OFF
*/

inline void PT_MUTE(char A_data)
{
    Wire.beginTransmission(pt2257);
    Wire.write(A_data);
    Wire.endTransmission();
}

inline void PT_2257(char A_data)
{
    Wire.beginTransmission(pt2257);
    Wire.write(A_data / 10 + 224); // -10 dB
    Wire.write(A_data % 10 + 208); // -1 dB
    Wire.endTransmission();
}

/*
inline void PT_Left (char A_data) {
  Wire.beginTransmission(pt2257);
  Wire.write(A_data / 10 + 176); // -10 dB
  Wire.write(A_data % 10 + 160); // -1 dB
  Wire.endTransmission();
}

inline void PT_Right (char A_data) {
  Wire.beginTransmission(pt2257);
  Wire.write(A_data / 10 + 48); // -10 dB
  Wire.write(A_data % 10 + 32); // -1 dB
  Wire.endTransmission();
}
*/