#pragma once

#define WHITE_WIDTH 60
#define BLACK_WIDTH 40
#define WHITE_HEIGHT 250
#define BLACK_HEIGHT 150

#define KEY_TOP 200
#define KEY_LEFT 100

#define INSTRUMENT_TOP 40
#define INSTRUMENT_LEFT 320

#define COLOR_TOP 40
#define COLOR_LEFT 830
#define COLOR_WIDTH 150
#define COLOR_HEIGHT 50

#define ID_BTN_DOWN_OCTAVE 3000
#define ID_BTN_UP_OCTAVE 3001
#define ID_BTN_DOWN_INSTRUMENT 3002
#define ID_BTN_UP_INSTRUMENT 3003
#define ID_BTN_DOWN_COLOR1 3004
#define ID_BTN_UP_COLOR1 3005
#define ID_BTN_DOWN_COLOR2 3006
#define ID_BTN_UP_COLOR2 3007
#define ID_BTN_SIGN 3008
#define ID_BTN_PEDAL 3009

int keySimbols[] = { 90, 83, 88, 68, 67, 86, 71, 66, 72, 78, 74, 77, 188, 76, 190, 186, 191, 81, 50, 87, 51, 69, 82, 53, 84, 54, 89, 55, 85, 73, 57, 79, 48, 80, 219, 187, 221 };
int tranformFromAllKeysToWB[] = { 0, 0, 1, 1, 2, 3, 2, 4, 3, 5, 4, 6, 7, 5, 8, 6, 9, 10, 7, 11, 8, 12, 9, 13, 14, 10, 15, 11, 16, 17, 12, 18 };
int tranformFromWhiteToAll[] = { 0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19, 21, 23, 24, 26, 28, 29, 31 };
int tranformFromBlackToAll[] = { 1, 3, 6, 8, 10, 13, 15, 18, 20, 22, 25, 27, 30 };
const char* instrumentPicArray[] = { "res\\piano.bmp", "res\\guitar.bmp", "res\\flex.bmp", "res\\drum.bmp", "res\\secret.bmp", "res\\ksilophone.bmp" };
int instrumentArray[] = { 0x0000 , 0x1800, 0x6000, 0x7400, 0x3700, 0x0400 };
const char* whiteSimbols[] = {"z", "x", "c", "v", "b", "n", "m", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]"};
const char* blackSimbols[] = {"s", "d", "g", "h", "j", "2", "3", "5", "6", "7", "9", "0", "="};
const char* octaves[] = { "Contra octave", "Greate octave", "Small octave", "First octave", "Second octave", "Third octave" };

