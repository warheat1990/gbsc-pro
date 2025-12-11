/*
   STV9426 library for ESP8266 for arduino ide operation

   Created in 0:50 16.02.2024
   By Karabanov Aleksandr
   https://www.youtube.com/channel/UCkNk5gQMIu8k9xW-uENsBzw

*/

#ifndef OSD_STV9426_H_
#define OSD_STV9426_H_

#define ADDR_STV 0x5D

// Линии сканирования N1
#define A1_lines 0x43
#define A2_lines 0x00
#define A3_lines 0xD0

// Линии сканирования N2
#define B1_lines 0x45
#define B2_lines 0x00
#define B3_lines 0xFE

// Линии сканирования N3
#define C1_lines 0x47
#define C2_lines 0x00
#define C3_lines 0xF3

// Линии сканирования N0
#define D1_lines 0x40
#define D2_lines 0x00
#define D3_lines 0x3A

// ДЛИТЕЛЬНОСТЬ СТРОКИ (Движение по горизонтали)
#define A1_parameters 0xF0
#define A2_parameters 0x3F
#define A3_parameters 0x3F

// ГОРИЗОНТАЛЬНАЯ ЗАДЕРЖКА (Движение по горизонтали)
#define B1_parameters 0xF1
#define B2_parameters 0x3F
#define B3_parameters 0x26

// ВЫСОТА СИМВОЛОВ
#define C1_parameters 0xF2
#define C2_parameters 0x3F
#define C3_parameters 0x15

// УМНОЖИТЕЛЬ ЧАСТОТЫ (Помогает стабилизировать картинку)
// #define Z1_parameters 0xF7
// #define Z2_parameters 0x3F
// #define Z3_parameters 0x09

// НАЧАЛЬНЫЙ ПЕРИОД ПИКСЕЛЯ
#define D1_parameters 0xF6
#define D2_parameters 0x3F
#define D3_parameters 0x02

// LOCKING CONDITION TIME CONSTANT (Убирает желе)
#define E1_parameters 0xF4
#define E2_parameters 0x3F
#define E3_parameters 0x02

// CAPTURE PROCESS TIME CONSTANT (Захват)
#define G1_parameters 0xF5
#define G2_parameters 0x3F
#define G3_parameters 0x03

// DISPLAYCONTROL (Управление дисплеем )
#define H1_parameters 0xF3
#define H2_parameters 0x3F
#define H3_parameters 0x81

extern char colour1;
extern char number_stroca;
extern char sequence_number1;
extern char sequence_number2;
extern char sequence_number3;

//--------------------------------------OSD font-----------------------------------------------------------------------------------------------------------------//
static const uint8_t n0 = 0x30, n1 = 0x31, n2 = 0x32, n3 = 0x33, n4 = 0x34, n5 = 0x35, n6 = 0x36, n7 = 0x37, n8 = 0x38, n9 = 0x39;

static const uint8_t _0_ = 0x30, _1_ = 0x31, _2_ = 0x32, _3_ = 0x33, _4_ = 0x34, _5_ = 0x35, _6_ = 0x36, _7_ = 0x37, _8_ = 0x38, _9_ = 0x39;

static const uint8_t a = 0x61, b = 0x62, c = 0x63, d = 0x64, e = 0x65, f = 0x66, g = 0x67, h = 0x68, i = 0x69, j = 0x6A, k = 0x6B, l = 0x6C,
                     m = 0x6D, n = 0x6E, o = 0x6F, p = 0x70, q = 0x71, r = 0x72, s = 0x73, t = 0x74, u = 0x75, v = 0x76, w = 0x77, x = 0x78, y = 0x79, z = 0x7A,
                     A = 0x41, B = 0x42, C = 0x43, D = 0x44, E = 0x45, F = 0x46, G = 0x47, H = 0x48, I = 0x49, J = 0x4A, K = 0x4B, L = 0x4C,
                     M = 0x4D, N = 0x4E, O = 0x4F, P = 0x50, Q = 0x51, R = 0x52, S = 0x53, T = 0x54, U = 0x55, V = 0x56, W = 0x57, X = 0x58, Y = 0x59, Z = 0x5A,
                     icon1 = 0x09, icon2 = 0x19, icon3 = 0x05, icon4 = 0x15, icon5 = 0x06, icon6 = 0x16;
// = 0x3D, - 0x3E, . 0x2E, / 0x2F, : 0x3A, ' 0x27

static const uint8_t P0 = 0x01, P1 = 0x03, P2 = 0x05, P3 = 0x07, P4 = 0x09, P5 = 0x0B, P6 = 0x0D, P7 = 0x0F, P8 = 0x11, P9 = 0x13, P10 = 0x15,
                     P11 = 0x17, P12 = 0x19, P13 = 0x1B, P14 = 0x1D, P15 = 0x1F, P16 = 0x21, P17 = 0x23, P18 = 0x25, P19 = 0x27, P20 = 0x29, P21 = 0x2B, P22 = 0x2D,
                     P23 = 0x2F, P24 = 0x31, P25 = 0x33, P26 = 0x35, P27 = 0x37; // P28 = 0x39, P29 = 0x3B, P30 = 0x3D

static const uint8_t _0 = 0x01, _1 = 0x03, _2 = 0x05, _3 = 0x07, _4 = 0x09, _5 = 0x0B, _6 = 0x0D, _7 = 0x0F, _8 = 0x11, _9 = 0x13, _10 = 0x15,
                     _11 = 0x17, _12 = 0x19, _13 = 0x1B, _14 = 0x1D, _15 = 0x1F, _16 = 0x21, _17 = 0x23, _18 = 0x25, _19 = 0x27, _20 = 0x29, _21 = 0x2B, _22 = 0x2D,
                     _23 = 0x2F, _24 = 0x31, _25 = 0x33, _26 = 0x35, _27 = 0x37;

static const uint8_t blue = 0x12, yellow = 0x60, yellowT = 0x16, main0 = 0x17, blue_fill = 0x11, clearP = 0xC0, blue_dark = 0x13, red = 0x14, pink = 0x15;

static const uint8_t stroca1 = 0x00, stroca2 = 0x02, stroca3 = 0x03;

extern char x1, x2, x3, x4, x5, x6, x7, x8, x9;

//------------------------------delivery of settings for STV9426-------------------------------//
inline void OSD_parameters(char A_data, char B_data, char C_data)
{
    Wire.beginTransmission(ADDR_STV);
    Wire.write(A_data); // LSB
    Wire.write(B_data); // MSB
    Wire.write(C_data); //
    Wire.endTransmission();
}

//-----------------------------------------Settings-----------------------------------------//
inline void OSD()
{
    for (byte SPACING = 0x40; SPACING < 0x48; ++SPACING) { // 48
        OSD_parameters(SPACING, 0x00, 0x00);
    }
    OSD_parameters(A1_lines, A2_lines, A3_lines);                // Линии сканирования N1
    OSD_parameters(B1_lines, B2_lines, B3_lines);                // Линии сканирования N2
    OSD_parameters(C1_lines, C2_lines, C3_lines);                // Линии сканирования N3
    OSD_parameters(D1_lines, D2_lines, D3_lines);                // Линии сканирования N0
    OSD_parameters(A1_parameters, A2_parameters, A3_parameters); // 行长
    OSD_parameters(B1_parameters, B2_parameters, B3_parameters); // 水平延迟
    OSD_parameters(C1_parameters, C2_parameters, C3_parameters); // 字符高度
    OSD_parameters(D1_parameters, D2_parameters, D3_parameters); // 像素的初始周期
    OSD_parameters(E1_parameters, E2_parameters, E3_parameters); // 锁定条件时间常数
    OSD_parameters(G1_parameters, G2_parameters, G3_parameters); // 捕捉过程时间常数
    OSD_parameters(H1_parameters, H2_parameters, H3_parameters); // 显示控制
}

//*************************************FBLK clear**********************************************//
inline void OSD_symbols_1()
{
    for (byte addressC1 = 0x00; addressC1 < 0x3F; ++addressC1) { // 0xFF
        OSD_parameters(addressC1, 0x00, 0x00);
    }
}
inline void OSD_Cut_0x01()
{
    for (byte addressC2 = 0x00; addressC2 < 0xFF; ++addressC2) { // 0xFF
        OSD_parameters(addressC2, 0x00, 0xC0);
    }
}
inline void OSD_symbols_2()
{
    for (byte addressP = 0x00; addressP < 0xFF; ++addressP) { // 0xFF
        OSD_parameters(addressP, 0x02, 0x00);
    }
}
inline void OSD_Cut_0x02()
{
    for (byte addressP = 0x00; addressP < 0xFF; ++addressP) { // 0xFF
        OSD_parameters(addressP, 0x02, 0xC0);
    }
}
inline void OSD_symbols_3()
{
    for (byte addressC = 0x00; addressC < 0xFF; ++addressC) { // 0xFF
        OSD_parameters(addressC, 0x03, 0x00);
    }
}
inline void OSD_Cut_0x03()
{
    for (byte addressC = 0x00; addressC < 0xFF; ++addressC) { // 0xFF
        OSD_parameters(addressC, 0x03, 0xC0);
    }
}

//*************************************过时的方法 ( 颜色+符号 )*********************************************//
inline void OSD_c1(volatile int test, volatile int test1, volatile int colour)
{
    OSD_parameters(test1, 0x00, test);
    OSD_parameters(test1 - 1, 0x00, colour);
}
inline void OSD_c2(volatile int test, volatile int test1, volatile int colour)
{
    OSD_parameters(test1, 0x02, test);
    OSD_parameters(test1 - 1, 0x02, colour);
}
inline void OSD_c3(volatile int test, volatile int test1, volatile int colour)
{
    OSD_parameters(test1, 0x03, test);
    OSD_parameters(test1 - 1, 0x03, colour);
}

//***************************************颜色+符号+轮廓*********************************************//
inline void __(volatile int symbol, volatile int number)
{
    OSD_parameters(number, number_stroca, symbol);
    OSD_parameters(number - 1, number_stroca, colour1);
}

//***************************************资料名称*********************************************//
inline void nameP()
{
    __(x1, _15);
    __(x2, _16);
    __(x3, _17);
    __(x4, _18);
    __(x5, _19);
    __(x6, _20);
    __(x7, _21);
    __(x8, _22);
    __(x9, _23);
}


//***************************************OSD clear*********************************************//
inline void OSD_clear()
{
    OSD_symbols_1();
    OSD_symbols_2();
    OSD_symbols_3();
    OSD_Cut_0x01();
    OSD_Cut_0x02();
    OSD_Cut_0x03();
}

inline void clean_up(char stroca, char str, char str1)
{                           // очистка строки (+2)
    number_stroca = stroca; // номер строки  OSD
    for (byte addressT1 = str1; addressT1 < str; ++addressT1) {
        switch (addressT1) {
            case 1:
                colour1 = blue_fill;
                __(o, _0);
                break;
            case 2:
                colour1 = blue_fill;
                __(o, _1);
                break;
            case 3:
                colour1 = blue_fill;
                __(o, _2);
                break;
            case 4:
                colour1 = blue_fill;
                __(o, _3);
                break;
            case 5:
                colour1 = blue_fill;
                __(o, _4);
                break;
            case 6:
                colour1 = blue_fill;
                __(o, _5);
                break;
            case 7:
                colour1 = blue_fill;
                __(o, _6);
                break;
            case 8:
                colour1 = blue_fill;
                __(o, _7);
                break;
            case 9:
                colour1 = blue_fill;
                __(o, _8);
                break;
            case 10:
                colour1 = blue_fill;
                __(o, _9);
                break;
            case 11:
                colour1 = blue_fill;
                __(o, _10);
                break;
            case 12:
                colour1 = blue_fill;
                __(o, _11);
                break;
            case 13:
                colour1 = blue_fill;
                __(o, _12);
                break;
            case 14:
                colour1 = blue_fill;
                __(o, _13);
                break;
            case 15:
                colour1 = blue_fill;
                __(o, _14);
                break;
            case 16:
                colour1 = blue_fill;
                __(o, _15);
                break;
            case 17:
                colour1 = blue_fill;
                __(o, _16);
                break;
            case 18:
                colour1 = blue_fill;
                __(o, _17);
                break;
            case 19:
                colour1 = blue_fill;
                __(o, _18);
                break;
            case 20:
                colour1 = blue_fill;
                __(o, _19);
                break;
            case 21:
                colour1 = blue_fill;
                __(o, _20);
                break;
            case 22:
                colour1 = blue_fill;
                __(o, _21);
                break;
            case 23:
                colour1 = blue_fill;
                __(o, _22);
                break;
            case 24:
                colour1 = blue_fill;
                __(o, _23);
                break;
            case 25:
                colour1 = blue_fill;
                __(o, _24);
                break;
            case 26:
                colour1 = blue_fill;
                __(o, _25);
                break;
            case 27:
                colour1 = blue_fill;
                __(o, _26);
                break;
            case 28:
                colour1 = blue_fill;
                __(o, _27);
                break;
        }
    }
}

//**************************************Background********************************************//
inline void background_up(char stroca, char str, char colour_)
{
    number_stroca = stroca;
    for (byte addressT1 = 0x00; addressT1 < str; ++addressT1) {
        OSD_parameters(addressT1, number_stroca, colour_);
    }
}

//**************************************Background 1,2,3***************************************//
inline void OSD_background()
{
    background_up(stroca1, _27, blue_fill);
    background_up(stroca2, _27, blue_fill);
    background_up(stroca3, _27, blue_fill);
}

//***************************The meaning of the expression (0-255)****************************//
inline void Typ(byte addr)
{                   // 000
    switch (addr) { // 000
        case 0x00:
            __(n0, sequence_number1);
            __(n0, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x01:
            __(n1, sequence_number1);
            __(n0, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x02:
            __(n2, sequence_number1);
            __(n0, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x03:
            __(n3, sequence_number1);
            __(n0, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x04:
            __(n4, sequence_number1);
            __(n0, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x05:
            __(n5, sequence_number1);
            __(n0, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x06:
            __(n6, sequence_number1);
            __(n0, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x07:
            __(n7, sequence_number1);
            __(n0, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x08:
            __(n8, sequence_number1);
            __(n0, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x09:
            __(n9, sequence_number1);
            __(n0, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x0A:
            __(n0, sequence_number1);
            __(n1, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x0B:
            __(n1, sequence_number1);
            __(n1, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x0C:
            __(n2, sequence_number1);
            __(n1, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x0D:
            __(n3, sequence_number1);
            __(n1, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x0E:
            __(n4, sequence_number1);
            __(n1, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x0F:
            __(n5, sequence_number1);
            __(n1, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x10:
            __(n6, sequence_number1);
            __(n1, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x11:
            __(n7, sequence_number1);
            __(n1, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x12:
            __(n8, sequence_number1);
            __(n1, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x13:
            __(n9, sequence_number1);
            __(n1, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x14:
            __(n0, sequence_number1);
            __(n2, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x15:
            __(n1, sequence_number1);
            __(n2, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x16:
            __(n2, sequence_number1);
            __(n2, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x17:
            __(n3, sequence_number1);
            __(n2, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x18:
            __(n4, sequence_number1);
            __(n2, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x19:
            __(n5, sequence_number1);
            __(n2, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x1A:
            __(n6, sequence_number1);
            __(n2, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x1B:
            __(n7, sequence_number1);
            __(n2, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x1C:
            __(n8, sequence_number1);
            __(n2, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x1D:
            __(n9, sequence_number1);
            __(n2, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x1E:
            __(n0, sequence_number1);
            __(n3, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x1F:
            __(n1, sequence_number1);
            __(n3, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x20:
            __(n2, sequence_number1);
            __(n3, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x21:
            __(n3, sequence_number1);
            __(n3, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x22:
            __(n4, sequence_number1);
            __(n3, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x23:
            __(n5, sequence_number1);
            __(n3, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x24:
            __(n6, sequence_number1);
            __(n3, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x25:
            __(n7, sequence_number1);
            __(n3, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x26:
            __(n8, sequence_number1);
            __(n3, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x27:
            __(n9, sequence_number1);
            __(n3, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x28:
            __(n0, sequence_number1);
            __(n4, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x29:
            __(n1, sequence_number1);
            __(n4, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x2A:
            __(n2, sequence_number1);
            __(n4, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x2B:
            __(n3, sequence_number1);
            __(n4, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x2C:
            __(n4, sequence_number1);
            __(n4, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x2D:
            __(n5, sequence_number1);
            __(n4, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x2E:
            __(n6, sequence_number1);
            __(n4, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x2F:
            __(n7, sequence_number1);
            __(n4, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x30:
            __(n8, sequence_number1);
            __(n4, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x31:
            __(n9, sequence_number1);
            __(n4, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x32:
            __(n0, sequence_number1);
            __(n5, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x33:
            __(n1, sequence_number1);
            __(n5, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x34:
            __(n2, sequence_number1);
            __(n5, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x35:
            __(n3, sequence_number1);
            __(n5, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x36:
            __(n4, sequence_number1);
            __(n5, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x37:
            __(n5, sequence_number1);
            __(n5, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x38:
            __(n6, sequence_number1);
            __(n5, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x39:
            __(n7, sequence_number1);
            __(n5, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x3A:
            __(n8, sequence_number1);
            __(n5, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x3B:
            __(n9, sequence_number1);
            __(n5, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x3C:
            __(n0, sequence_number1);
            __(n6, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x3D:
            __(n1, sequence_number1);
            __(n6, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x3E:
            __(n2, sequence_number1);
            __(n6, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x3F:
            __(n3, sequence_number1);
            __(n6, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x40:
            __(n4, sequence_number1);
            __(n6, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x41:
            __(n5, sequence_number1);
            __(n6, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x42:
            __(n6, sequence_number1);
            __(n6, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x43:
            __(n7, sequence_number1);
            __(n6, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x44:
            __(n8, sequence_number1);
            __(n6, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x45:
            __(n9, sequence_number1);
            __(n6, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x46:
            __(n0, sequence_number1);
            __(n7, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x47:
            __(n1, sequence_number1);
            __(n7, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x48:
            __(n2, sequence_number1);
            __(n7, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x49:
            __(n3, sequence_number1);
            __(n7, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x4A:
            __(n4, sequence_number1);
            __(n7, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x4B:
            __(n5, sequence_number1);
            __(n7, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x4C:
            __(n6, sequence_number1);
            __(n7, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x4D:
            __(n7, sequence_number1);
            __(n7, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x4E:
            __(n8, sequence_number1);
            __(n7, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x4F:
            __(n9, sequence_number1);
            __(n7, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x50:
            __(n0, sequence_number1);
            __(n8, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x51:
            __(n1, sequence_number1);
            __(n8, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x52:
            __(n2, sequence_number1);
            __(n8, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x53:
            __(n3, sequence_number1);
            __(n8, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x54:
            __(n4, sequence_number1);
            __(n8, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x55:
            __(n5, sequence_number1);
            __(n8, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x56:
            __(n6, sequence_number1);
            __(n8, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x57:
            __(n7, sequence_number1);
            __(n8, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x58:
            __(n8, sequence_number1);
            __(n8, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x59:
            __(n9, sequence_number1);
            __(n8, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x5A:
            __(n0, sequence_number1);
            __(n9, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x5B:
            __(n1, sequence_number1);
            __(n9, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x5C:
            __(n2, sequence_number1);
            __(n9, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x5D:
            __(n3, sequence_number1);
            __(n9, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x5E:
            __(n4, sequence_number1);
            __(n9, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x5F:
            __(n5, sequence_number1);
            __(n9, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x60:
            __(n6, sequence_number1);
            __(n9, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x61:
            __(n7, sequence_number1);
            __(n9, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x62:
            __(n8, sequence_number1);
            __(n9, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x63:
            __(n9, sequence_number1);
            __(n9, sequence_number2);
            __(n0, sequence_number3);
            break;
        case 0x64:
            __(n0, sequence_number1);
            __(n0, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x65:
            __(n1, sequence_number1);
            __(n0, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x66:
            __(n2, sequence_number1);
            __(n0, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x67:
            __(n3, sequence_number1);
            __(n0, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x68:
            __(n4, sequence_number1);
            __(n0, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x69:
            __(n5, sequence_number1);
            __(n0, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x6A:
            __(n6, sequence_number1);
            __(n0, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x6B:
            __(n7, sequence_number1);
            __(n0, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x6C:
            __(n8, sequence_number1);
            __(n0, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x6D:
            __(n9, sequence_number1);
            __(n0, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x6E:
            __(n0, sequence_number1);
            __(n1, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x6F:
            __(n1, sequence_number1);
            __(n1, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x70:
            __(n2, sequence_number1);
            __(n1, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x71:
            __(n3, sequence_number1);
            __(n1, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x72:
            __(n4, sequence_number1);
            __(n1, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x73:
            __(n5, sequence_number1);
            __(n1, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x74:
            __(n6, sequence_number1);
            __(n1, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x75:
            __(n7, sequence_number1);
            __(n1, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x76:
            __(n8, sequence_number1);
            __(n1, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x77:
            __(n9, sequence_number1);
            __(n1, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x78:
            __(n0, sequence_number1);
            __(n2, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x79:
            __(n1, sequence_number1);
            __(n2, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x7A:
            __(n2, sequence_number1);
            __(n2, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x7B:
            __(n3, sequence_number1);
            __(n2, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x7C:
            __(n4, sequence_number1);
            __(n2, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x7D:
            __(n5, sequence_number1);
            __(n2, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x7E:
            __(n6, sequence_number1);
            __(n2, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x7F:
            __(n7, sequence_number1);
            __(n2, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x80:
            __(n8, sequence_number1);
            __(n2, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x81:
            __(n9, sequence_number1);
            __(n2, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x82:
            __(n0, sequence_number1);
            __(n3, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x83:
            __(n1, sequence_number1);
            __(n3, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x84:
            __(n2, sequence_number1);
            __(n3, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x85:
            __(n3, sequence_number1);
            __(n3, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x86:
            __(n4, sequence_number1);
            __(n3, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x87:
            __(n5, sequence_number1);
            __(n3, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x88:
            __(n6, sequence_number1);
            __(n3, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x89:
            __(n7, sequence_number1);
            __(n3, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x8A:
            __(n8, sequence_number1);
            __(n3, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x8B:
            __(n9, sequence_number1);
            __(n3, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x8C:
            __(n0, sequence_number1);
            __(n4, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x8D:
            __(n1, sequence_number1);
            __(n4, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x8E:
            __(n2, sequence_number1);
            __(n4, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x8F:
            __(n3, sequence_number1);
            __(n4, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x90:
            __(n4, sequence_number1);
            __(n4, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x91:
            __(n5, sequence_number1);
            __(n4, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x92:
            __(n6, sequence_number1);
            __(n4, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x93:
            __(n7, sequence_number1);
            __(n4, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x94:
            __(n8, sequence_number1);
            __(n4, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x95:
            __(n9, sequence_number1);
            __(n4, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x96:
            __(n0, sequence_number1);
            __(n5, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x97:
            __(n1, sequence_number1);
            __(n5, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x98:
            __(n2, sequence_number1);
            __(n5, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x99:
            __(n3, sequence_number1);
            __(n5, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x9A:
            __(n4, sequence_number1);
            __(n5, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x9B:
            __(n5, sequence_number1);
            __(n5, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x9C:
            __(n6, sequence_number1);
            __(n5, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x9D:
            __(n7, sequence_number1);
            __(n5, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x9E:
            __(n8, sequence_number1);
            __(n5, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0x9F:
            __(n9, sequence_number1);
            __(n5, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xA0:
            __(n0, sequence_number1);
            __(n6, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xA1:
            __(n1, sequence_number1);
            __(n6, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xA2:
            __(n2, sequence_number1);
            __(n6, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xA3:
            __(n3, sequence_number1);
            __(n6, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xA4:
            __(n4, sequence_number1);
            __(n6, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xA5:
            __(n5, sequence_number1);
            __(n6, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xA6:
            __(n6, sequence_number1);
            __(n6, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xA7:
            __(n7, sequence_number1);
            __(n6, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xA8:
            __(n8, sequence_number1);
            __(n6, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xA9:
            __(n9, sequence_number1);
            __(n6, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xAA:
            __(n0, sequence_number1);
            __(n7, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xAB:
            __(n1, sequence_number1);
            __(n7, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xAC:
            __(n2, sequence_number1);
            __(n7, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xAD:
            __(n3, sequence_number1);
            __(n7, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xAE:
            __(n4, sequence_number1);
            __(n7, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xAF:
            __(n5, sequence_number1);
            __(n7, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xB0:
            __(n6, sequence_number1);
            __(n7, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xB1:
            __(n7, sequence_number1);
            __(n7, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xB2:
            __(n8, sequence_number1);
            __(n7, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xB3:
            __(n9, sequence_number1);
            __(n7, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xB4:
            __(n0, sequence_number1);
            __(n8, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xB5:
            __(n1, sequence_number1);
            __(n8, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xB6:
            __(n2, sequence_number1);
            __(n8, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xB7:
            __(n3, sequence_number1);
            __(n8, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xB8:
            __(n4, sequence_number1);
            __(n8, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xB9:
            __(n5, sequence_number1);
            __(n8, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xBA:
            __(n6, sequence_number1);
            __(n8, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xBB:
            __(n7, sequence_number1);
            __(n8, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xBC:
            __(n8, sequence_number1);
            __(n8, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xBD:
            __(n9, sequence_number1);
            __(n8, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xBE:
            __(n0, sequence_number1);
            __(n9, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xBF:
            __(n1, sequence_number1);
            __(n9, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xC0:
            __(n2, sequence_number1);
            __(n9, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xC1:
            __(n3, sequence_number1);
            __(n9, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xC2:
            __(n4, sequence_number1);
            __(n9, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xC3:
            __(n5, sequence_number1);
            __(n9, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xC4:
            __(n6, sequence_number1);
            __(n9, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xC5:
            __(n7, sequence_number1);
            __(n9, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xC6:
            __(n8, sequence_number1);
            __(n9, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xC7:
            __(n9, sequence_number1);
            __(n9, sequence_number2);
            __(n1, sequence_number3);
            break;
        case 0xC8:
            __(n0, sequence_number1);
            __(n0, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xC9:
            __(n1, sequence_number1);
            __(n0, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xCA:
            __(n2, sequence_number1);
            __(n0, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xCB:
            __(n3, sequence_number1);
            __(n0, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xCC:
            __(n4, sequence_number1);
            __(n0, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xCD:
            __(n5, sequence_number1);
            __(n0, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xCE:
            __(n6, sequence_number1);
            __(n0, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xCF:
            __(n7, sequence_number1);
            __(n0, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xD0:
            __(n8, sequence_number1);
            __(n0, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xD1:
            __(n9, sequence_number1);
            __(n0, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xD2:
            __(n0, sequence_number1);
            __(n1, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xD3:
            __(n1, sequence_number1);
            __(n1, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xD4:
            __(n2, sequence_number1);
            __(n1, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xD5:
            __(n3, sequence_number1);
            __(n1, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xD6:
            __(n4, sequence_number1);
            __(n1, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xD7:
            __(n5, sequence_number1);
            __(n1, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xD8:
            __(n6, sequence_number1);
            __(n1, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xD9:
            __(n7, sequence_number1);
            __(n1, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xDA:
            __(n8, sequence_number1);
            __(n1, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xDB:
            __(n9, sequence_number1);
            __(n1, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xDC:
            __(n0, sequence_number1);
            __(n2, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xDD:
            __(n1, sequence_number1);
            __(n2, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xDE:
            __(n2, sequence_number1);
            __(n2, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xDF:
            __(n3, sequence_number1);
            __(n2, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xE0:
            __(n4, sequence_number1);
            __(n2, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xE1:
            __(n5, sequence_number1);
            __(n2, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xE2:
            __(n6, sequence_number1);
            __(n2, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xE3:
            __(n7, sequence_number1);
            __(n2, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xE4:
            __(n8, sequence_number1);
            __(n2, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xE5:
            __(n9, sequence_number1);
            __(n2, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xE6:
            __(n0, sequence_number1);
            __(n3, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xE7:
            __(n1, sequence_number1);
            __(n3, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xE8:
            __(n2, sequence_number1);
            __(n3, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xE9:
            __(n3, sequence_number1);
            __(n3, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xEA:
            __(n4, sequence_number1);
            __(n3, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xEB:
            __(n5, sequence_number1);
            __(n3, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xEC:
            __(n6, sequence_number1);
            __(n3, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xED:
            __(n7, sequence_number1);
            __(n3, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xEE:
            __(n8, sequence_number1);
            __(n3, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xEF:
            __(n9, sequence_number1);
            __(n3, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xF0:
            __(n0, sequence_number1);
            __(n4, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xF1:
            __(n1, sequence_number1);
            __(n4, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xF2:
            __(n2, sequence_number1);
            __(n4, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xF3:
            __(n3, sequence_number1);
            __(n4, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xF4:
            __(n4, sequence_number1);
            __(n4, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xF5:
            __(n5, sequence_number1);
            __(n4, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xF6:
            __(n6, sequence_number1);
            __(n4, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xF7:
            __(n7, sequence_number1);
            __(n4, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xF8:
            __(n8, sequence_number1);
            __(n4, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xF9:
            __(n9, sequence_number1);
            __(n4, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xFA:
            __(n0, sequence_number1);
            __(n5, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xFB:
            __(n1, sequence_number1);
            __(n5, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xFC:
            __(n2, sequence_number1);
            __(n5, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xFD:
            __(n3, sequence_number1);
            __(n5, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xFE:
            __(n4, sequence_number1);
            __(n5, sequence_number2);
            __(n2, sequence_number3);
            break;
        case 0xFF:
            __(n5, sequence_number1);
            __(n5, sequence_number2);
            __(n2, sequence_number3);
            break;
    }
}

//*************************************************************************************//
inline void Type4(byte addr)
{ // inversion 255-0
    switch (addr) {
        case 0x00:
            OSD_c1(n5, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x01:
            OSD_c1(n4, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x02:
            OSD_c1(n3, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x03:
            OSD_c1(n2, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x04:
            OSD_c1(n1, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x05:
            OSD_c1(n0, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x06:
            OSD_c1(n9, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x07:
            OSD_c1(n8, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x08:
            OSD_c1(n7, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x09:
            OSD_c1(n6, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x0A:
            OSD_c1(n5, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x0B:
            OSD_c1(n4, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x0C:
            OSD_c1(n3, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x0D:
            OSD_c1(n2, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x0E:
            OSD_c1(n1, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x0F:
            OSD_c1(n0, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x10:
            OSD_c1(n9, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x11:
            OSD_c1(n8, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x12:
            OSD_c1(n7, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x13:
            OSD_c1(n6, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x14:
            OSD_c1(n5, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x15:
            OSD_c1(n4, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x16:
            OSD_c1(n3, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x17:
            OSD_c1(n2, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x18:
            OSD_c1(n1, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x19:
            OSD_c1(n0, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x1A:
            OSD_c1(n9, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x1B:
            OSD_c1(n8, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x1C:
            OSD_c1(n7, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x1D:
            OSD_c1(n6, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x1E:
            OSD_c1(n5, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x1F:
            OSD_c1(n4, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x20:
            OSD_c1(n3, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x21:
            OSD_c1(n2, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x22:
            OSD_c1(n1, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x23:
            OSD_c1(n0, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x24:
            OSD_c1(n9, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x25:
            OSD_c1(n8, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x26:
            OSD_c1(n7, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x27:
            OSD_c1(n6, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x28:
            OSD_c1(n5, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x29:
            OSD_c1(n4, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x2A:
            OSD_c1(n3, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x2B:
            OSD_c1(n2, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x2C:
            OSD_c1(n1, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x2D:
            OSD_c1(n0, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x2E:
            OSD_c1(n9, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x2F:
            OSD_c1(n8, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x30:
            OSD_c1(n7, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x31:
            OSD_c1(n6, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x32:
            OSD_c1(n5, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x33:
            OSD_c1(n4, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x34:
            OSD_c1(n3, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x35:
            OSD_c1(n2, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x36:
            OSD_c1(n1, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x37:
            OSD_c1(n0, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n2, P19, main0);
            break;
        case 0x38:
            OSD_c1(n9, P21, main0);
            OSD_c1(n9, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x39:
            OSD_c1(n8, P21, main0);
            OSD_c1(n9, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x3A:
            OSD_c1(n7, P21, main0);
            OSD_c1(n9, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x3B:
            OSD_c1(n6, P21, main0);
            OSD_c1(n9, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x3C:
            OSD_c1(n5, P21, main0);
            OSD_c1(n9, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x3D:
            OSD_c1(n4, P21, main0);
            OSD_c1(n9, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x3E:
            OSD_c1(n3, P21, main0);
            OSD_c1(n9, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x3F:
            OSD_c1(n2, P21, main0);
            OSD_c1(n9, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x40:
            OSD_c1(n1, P21, main0);
            OSD_c1(n9, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x41:
            OSD_c1(n0, P21, main0);
            OSD_c1(n9, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x42:
            OSD_c1(n9, P21, main0);
            OSD_c1(n8, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x43:
            OSD_c1(n8, P21, main0);
            OSD_c1(n8, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x44:
            OSD_c1(n7, P21, main0);
            OSD_c1(n8, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x45:
            OSD_c1(n6, P21, main0);
            OSD_c1(n8, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x46:
            OSD_c1(n5, P21, main0);
            OSD_c1(n8, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x47:
            OSD_c1(n4, P21, main0);
            OSD_c1(n8, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x48:
            OSD_c1(n3, P21, main0);
            OSD_c1(n8, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x49:
            OSD_c1(n2, P21, main0);
            OSD_c1(n8, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x4A:
            OSD_c1(n1, P21, main0);
            OSD_c1(n8, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x4B:
            OSD_c1(n0, P21, main0);
            OSD_c1(n8, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x4C:
            OSD_c1(n9, P21, main0);
            OSD_c1(n7, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x4D:
            OSD_c1(n8, P21, main0);
            OSD_c1(n7, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x4E:
            OSD_c1(n7, P21, main0);
            OSD_c1(n7, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x4F:
            OSD_c1(n6, P21, main0);
            OSD_c1(n7, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x50:
            OSD_c1(n5, P21, main0);
            OSD_c1(n7, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x51:
            OSD_c1(n4, P21, main0);
            OSD_c1(n7, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x52:
            OSD_c1(n3, P21, main0);
            OSD_c1(n7, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x53:
            OSD_c1(n2, P21, main0);
            OSD_c1(n7, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x54:
            OSD_c1(n1, P21, main0);
            OSD_c1(n7, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x55:
            OSD_c1(n0, P21, main0);
            OSD_c1(n7, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x56:
            OSD_c1(n9, P21, main0);
            OSD_c1(n6, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x57:
            OSD_c1(n8, P21, main0);
            OSD_c1(n6, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x58:
            OSD_c1(n7, P21, main0);
            OSD_c1(n6, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x59:
            OSD_c1(n6, P21, main0);
            OSD_c1(n6, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x5A:
            OSD_c1(n5, P21, main0);
            OSD_c1(n6, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x5B:
            OSD_c1(n4, P21, main0);
            OSD_c1(n6, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x5C:
            OSD_c1(n3, P21, main0);
            OSD_c1(n6, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x5D:
            OSD_c1(n2, P21, main0);
            OSD_c1(n6, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x5E:
            OSD_c1(n1, P21, main0);
            OSD_c1(n6, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x5F:
            OSD_c1(n0, P21, main0);
            OSD_c1(n6, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x60:
            OSD_c1(n9, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x61:
            OSD_c1(n8, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x62:
            OSD_c1(n7, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x63:
            OSD_c1(n6, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x64:
            OSD_c1(n5, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x65:
            OSD_c1(n4, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x66:
            OSD_c1(n3, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x67:
            OSD_c1(n2, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x68:
            OSD_c1(n1, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x69:
            OSD_c1(n0, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x6A:
            OSD_c1(n9, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x6B:
            OSD_c1(n8, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x6C:
            OSD_c1(n7, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x6D:
            OSD_c1(n6, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x6E:
            OSD_c1(n5, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x6F:
            OSD_c1(n4, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x70:
            OSD_c1(n3, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x71:
            OSD_c1(n2, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x72:
            OSD_c1(n1, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x73:
            OSD_c1(n0, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x74:
            OSD_c1(n9, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x75:
            OSD_c1(n8, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x76:
            OSD_c1(n7, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x77:
            OSD_c1(n6, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x78:
            OSD_c1(n5, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x79:
            OSD_c1(n4, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x7A:
            OSD_c1(n3, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x7B:
            OSD_c1(n2, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x7C:
            OSD_c1(n1, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x7D:
            OSD_c1(n0, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x7E:
            OSD_c1(n9, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x7F:
            OSD_c1(n8, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x80:
            OSD_c1(n7, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x81:
            OSD_c1(n6, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x82:
            OSD_c1(n5, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x83:
            OSD_c1(n4, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x84:
            OSD_c1(n3, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x85:
            OSD_c1(n2, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x86:
            OSD_c1(n1, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x87:
            OSD_c1(n0, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x88:
            OSD_c1(n9, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x89:
            OSD_c1(n8, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x8A:
            OSD_c1(n7, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x8B:
            OSD_c1(n6, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x8C:
            OSD_c1(n5, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x8D:
            OSD_c1(n4, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x8E:
            OSD_c1(n3, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x8F:
            OSD_c1(n2, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x90:
            OSD_c1(n1, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x91:
            OSD_c1(n0, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x92:
            OSD_c1(n9, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x93:
            OSD_c1(n8, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x94:
            OSD_c1(n7, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x95:
            OSD_c1(n6, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x96:
            OSD_c1(n5, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x97:
            OSD_c1(n4, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x98:
            OSD_c1(n3, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x99:
            OSD_c1(n2, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x9A:
            OSD_c1(n1, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x9B:
            OSD_c1(n0, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n1, P19, main0);
            break;
        case 0x9C:
            OSD_c1(n9, P21, main0);
            OSD_c1(n9, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0x9D:
            OSD_c1(n8, P21, main0);
            OSD_c1(n9, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0x9E:
            OSD_c1(n7, P21, main0);
            OSD_c1(n9, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0x9F:
            OSD_c1(n6, P21, main0);
            OSD_c1(n9, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xA0:
            OSD_c1(n5, P21, main0);
            OSD_c1(n9, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xA1:
            OSD_c1(n4, P21, main0);
            OSD_c1(n9, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xA2:
            OSD_c1(n3, P21, main0);
            OSD_c1(n9, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xA3:
            OSD_c1(n2, P21, main0);
            OSD_c1(n9, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xA4:
            OSD_c1(n1, P21, main0);
            OSD_c1(n9, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xA5:
            OSD_c1(n0, P21, main0);
            OSD_c1(n9, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xA6:
            OSD_c1(n9, P21, main0);
            OSD_c1(n8, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xA7:
            OSD_c1(n8, P21, main0);
            OSD_c1(n8, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xA8:
            OSD_c1(n7, P21, main0);
            OSD_c1(n8, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xA9:
            OSD_c1(n6, P21, main0);
            OSD_c1(n8, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xAA:
            OSD_c1(n5, P21, main0);
            OSD_c1(n8, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xAB:
            OSD_c1(n4, P21, main0);
            OSD_c1(n8, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xAC:
            OSD_c1(n3, P21, main0);
            OSD_c1(n8, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xAD:
            OSD_c1(n2, P21, main0);
            OSD_c1(n8, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xAE:
            OSD_c1(n1, P21, main0);
            OSD_c1(n8, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xAF:
            OSD_c1(n0, P21, main0);
            OSD_c1(n8, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xB0:
            OSD_c1(n9, P21, main0);
            OSD_c1(n7, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xB1:
            OSD_c1(n8, P21, main0);
            OSD_c1(n7, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xB2:
            OSD_c1(n7, P21, main0);
            OSD_c1(n7, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xB3:
            OSD_c1(n6, P21, main0);
            OSD_c1(n7, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xB4:
            OSD_c1(n5, P21, main0);
            OSD_c1(n7, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xB5:
            OSD_c1(n4, P21, main0);
            OSD_c1(n7, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xB6:
            OSD_c1(n3, P21, main0);
            OSD_c1(n7, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xB7:
            OSD_c1(n2, P21, main0);
            OSD_c1(n7, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xB8:
            OSD_c1(n1, P21, main0);
            OSD_c1(n7, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xB9:
            OSD_c1(n0, P21, main0);
            OSD_c1(n7, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xBA:
            OSD_c1(n9, P21, main0);
            OSD_c1(n6, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xBB:
            OSD_c1(n8, P21, main0);
            OSD_c1(n6, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xBC:
            OSD_c1(n7, P21, main0);
            OSD_c1(n6, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xBD:
            OSD_c1(n6, P21, main0);
            OSD_c1(n6, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xBE:
            OSD_c1(n5, P21, main0);
            OSD_c1(n6, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xBF:
            OSD_c1(n4, P21, main0);
            OSD_c1(n6, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xC0:
            OSD_c1(n3, P21, main0);
            OSD_c1(n6, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xC1:
            OSD_c1(n2, P21, main0);
            OSD_c1(n6, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xC2:
            OSD_c1(n1, P21, main0);
            OSD_c1(n6, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xC3:
            OSD_c1(n0, P21, main0);
            OSD_c1(n6, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xC4:
            OSD_c1(n9, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xC5:
            OSD_c1(n8, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xC6:
            OSD_c1(n7, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xC7:
            OSD_c1(n6, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xC8:
            OSD_c1(n5, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xC9:
            OSD_c1(n4, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xCA:
            OSD_c1(n3, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xCB:
            OSD_c1(n2, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xCC:
            OSD_c1(n1, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xCD:
            OSD_c1(n0, P21, main0);
            OSD_c1(n5, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xCE:
            OSD_c1(n9, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xCF:
            OSD_c1(n8, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xD0:
            OSD_c1(n7, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xD1:
            OSD_c1(n6, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xD2:
            OSD_c1(n5, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xD3:
            OSD_c1(n4, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xD4:
            OSD_c1(n3, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xD5:
            OSD_c1(n2, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xD6:
            OSD_c1(n1, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xD7:
            OSD_c1(n0, P21, main0);
            OSD_c1(n4, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xD8:
            OSD_c1(n9, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xD9:
            OSD_c1(n8, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xDA:
            OSD_c1(n7, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xDB:
            OSD_c1(n6, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xDC:
            OSD_c1(n5, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xDD:
            OSD_c1(n4, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xDE:
            OSD_c1(n3, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xDF:
            OSD_c1(n2, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xE0:
            OSD_c1(n1, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xE1:
            OSD_c1(n0, P21, main0);
            OSD_c1(n3, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xE2:
            OSD_c1(n9, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xE3:
            OSD_c1(n8, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xE4:
            OSD_c1(n7, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xE5:
            OSD_c1(n6, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xE6:
            OSD_c1(n5, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xE7:
            OSD_c1(n4, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xE8:
            OSD_c1(n3, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xE9:
            OSD_c1(n2, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xEA:
            OSD_c1(n1, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xEB:
            OSD_c1(n0, P21, main0);
            OSD_c1(n2, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xEC:
            OSD_c1(n9, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xED:
            OSD_c1(n8, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xEE:
            OSD_c1(n7, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xEF:
            OSD_c1(n6, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xF0:
            OSD_c1(n5, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xF1:
            OSD_c1(n4, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xF2:
            OSD_c1(n3, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xF3:
            OSD_c1(n2, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xF4:
            OSD_c1(n1, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xF5:
            OSD_c1(n0, P21, main0);
            OSD_c1(n1, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xF6:
            OSD_c1(n9, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xF7:
            OSD_c1(n8, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xF8:
            OSD_c1(n7, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xF9:
            OSD_c1(n6, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xFA:
            OSD_c1(n5, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xFB:
            OSD_c1(n4, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xFC:
            OSD_c1(n3, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xFD:
            OSD_c1(n2, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xFE:
            OSD_c1(n1, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
        case 0xFF:
            OSD_c1(n0, P21, main0);
            OSD_c1(n0, P20, main0);
            OSD_c1(n0, P19, main0);
            break;
    }
}
#endif
