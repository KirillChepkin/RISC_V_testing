#include "jpge_color_transform.h"

// In ideal mathematical representation

// Y = 0.299R + 0.587G + 0.114B
// Cb = 128 - 0.168736R - 0.331264G + 0.5B
// Cr = 128 + 0.5R - 0.418688G - 0.081312B

#define HALF 32768 // half of 65536 (2^16)
#define YR 19595 // 19595/65536 approximately equals 0.587
#define YG 38470 // 38470/65536 approximately equals 0.299
#define YB 7471 // 7471/65536 approximately equals 0.114

#define CB_R -11059 // 11059/65536 approximately equals 0.168736
#define CB_G -21709 // 21709/65536 approximately equals 0.331264
#define CB_B HALF // 32768/65536 equals 0.5

#define CR_R HALF // 32768/65536 equals 0.5
#define CR_G -27439 // 27439/65536 approximately equals 0.418688
#define CR_B -5329 // 5329/65536 approximately equals 0.081312

namespace jpge {
static inline uint8 clamp(int i) {
  if (i < 0) {
    i = 0;
  }
  else if (i > 255) {
    i = 255;
  }

  return static_cast<uint8>(i);
}

void RGB_to_YCC(uint8* pDst, const uint8 *pSrc, int num_pixels) {
  while (num_pixels > 0) {
    const int r = pSrc[0],
              g = pSrc[1],
              b = pSrc[2];

    pDst[0] = static_cast<uint8>((r * YR + g * YG + b * YB + HALF) >> 16);
    pDst[1] = clamp(128 + ((r * CB_R + g * CB_G + b * CB_B + HALF) >> 16));
    pDst[2] = clamp(128 + ((r * CR_R + g * CR_G + b * CR_B + HALF) >> 16));

    pDst += 3;
    pSrc += 3;
    num_pixels--;
  }
}

void RGB_to_Y(uint8* pDst, const uint8 *pSrc, int num_pixels) { 
  while (num_pixels > 0) {
    pDst[0] = static_cast<uint8>((pSrc[0] * YR + pSrc[1] * YG + pSrc[2] * YB + HALF) >> 16);

    pDst++;
    pSrc += 3;
    num_pixels--;
  }
}

void RGBA_to_YCC(uint8* pDst, const uint8 *pSrc, int num_pixels) {
  while (num_pixels > 0) {
    const int r = pSrc[0],
              g = pSrc[1],
              b = pSrc[2];

    pDst[0] = static_cast<uint8>((r * YR + g * YG + b * YB + HALF) >> 16);
    pDst[1] = clamp(128 + ((r * CB_R + g * CB_G + b * CB_B + HALF) >> 16));
    pDst[2] = clamp(128 + ((r * CR_R + g * CR_G + b * CR_B + HALF) >> 16));

    pDst += 3;
    pSrc += 4;
    num_pixels--;
  }
}

void RGBA_to_Y(uint8* pDst, const uint8 *pSrc, int num_pixels)
{
  while (num_pixels > 0) {
    pDst[0] = static_cast<uint8>((pSrc[0] * YR + pSrc[1] * YG + pSrc[2] * YB + HALF) >> 16);

    pDst++;
    pSrc += 4;
    num_pixels--;
  }
}

void Y_to_YCC(uint8* pDst, const uint8* pSrc, int num_pixels)
{
  while (num_pixels > 0) {
    pDst[0] = pSrc[0];
    pDst[1] = 128;
    pDst[2] = 128;
    
    pDst += 3;
    pSrc++;
    num_pixels--;
  }
}
} // namespace jpge
