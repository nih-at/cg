#include <stdio.h>
#include "decode.h"

#define IL DEC_ILL
#define EQ DEC_EQUAL
#define CO DEC_COLON



int decode_table_base64[256] = {
    /* 0x00 */
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, 
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL,
    /* 0x20 */
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, 62, IL, IL, IL, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, IL, IL, IL, EQ, IL, IL,
    /* 0x40 */
    IL,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, IL, IL, IL, IL, IL,
    /* 0x60 */
    IL, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, IL, IL, IL, IL, IL,
    /* 0x80 */
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, 
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL,
    /* 0xa0 */
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, 
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL,
    /* 0xc0 */
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, 
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL,
    /* 0xe0 */
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, 
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL
};



int decode_table_binhex[256] = {
    /* 0x00 */
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, 
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL,
    /* 0x20 */
    IL,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, IL, IL,
    13, 14, 15, 16, 17, 18, 19, IL, 20, 21, CO, IL, IL, IL, IL, IL,
    /* 0x40 */
    22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, IL,
    37, 38, 39, 40, 41, 42, 43, IL, 44, 45, 46, 47, IL, IL, IL, IL,
    /* 0x60 */
    48, 49, 50, 51, 52, 53, 54, IL, 55, 56, 57, 58, 59, 60, IL, IL,
    61, 62, 63, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, 
    /* 0x80 */
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, 
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL,
    /* 0xa0 */
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, 
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL,
    /* 0xc0 */
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, 
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL,
    /* 0xe0 */
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, 
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL
};



int decode_table_uuencode[256] = {
    /* 0x00 */
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, 
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL,
    /* 0x20 */
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
    /* 0x40 */
    32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
    /* 0x60 */
     0, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, 
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, 
    /* 0x80 */
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, 
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL,
    /* 0xa0 */
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, 
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL,
    /* 0xc0 */
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, 
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL,
    /* 0xe0 */
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, 
    IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL, IL
};
