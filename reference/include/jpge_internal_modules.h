#ifndef JPGE_INTERNAL_MODULES_H
#define JPGE_INTERNAL_MODULES_H

#include "jpge.h"
namespace jpge {
void RGB_to_YCC(uint8*, const uint8*, int);
void RGB_to_Y(uint8*, const uint8*, int);
void RGBA_to_YCC(uint8*, const uint8*, int);
void RGBA_to_Y(uint8*, const uint8*, int);
void Y_to_YCC(uint8*, const uint8*, int);
void DCT2D(int32*);
}

#endif