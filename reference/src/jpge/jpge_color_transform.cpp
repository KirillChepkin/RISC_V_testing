#include "jpge_color_transform.h"

namespace jpge {
static const int YR=19595, YG=38470, YB=7471, CB_R=-11059, CB_G=-21709, CB_B=32768, CR_R=32768, CR_G=-27439, CR_B=-5329;
static inline uint8 clamp(int i) {
  if (static_cast<uint>(i) > 255U) {
    if (i < 0) {
      i = 0;
    }
    else if (i > 255) {
      i = 255;
    }
  }
  return static_cast<uint8>(i);
}

void RGB_to_YCC(uint8* pDst, const uint8 *pSrc, int num_pixels) {
  for ( ; num_pixels; pDst += 3, pSrc += 3, num_pixels--)
  {
    const int r = pSrc[0], g = pSrc[1], b = pSrc[2];
    pDst[0] = static_cast<uint8>((r * YR + g * YG + b * YB + 32768) >> 16);
    pDst[1] = clamp(128 + ((r * CB_R + g * CB_G + b * CB_B + 32768) >> 16));
    pDst[2] = clamp(128 + ((r * CR_R + g * CR_G + b * CR_B + 32768) >> 16));
  }
}

void RGB_to_Y(uint8* pDst, const uint8 *pSrc, int num_pixels)
{
  for ( ; num_pixels; pDst++, pSrc += 3, num_pixels--)
    pDst[0] = static_cast<uint8>((pSrc[0] * YR + pSrc[1] * YG + pSrc[2] * YB + 32768) >> 16);
}

void RGBA_to_YCC(uint8* pDst, const uint8 *pSrc, int num_pixels)
{
  for ( ; num_pixels; pDst += 3, pSrc += 4, num_pixels--)
  {
    const int r = pSrc[0], g = pSrc[1], b = pSrc[2];
    pDst[0] = static_cast<uint8>((r * YR + g * YG + b * YB + 32768) >> 16);
    pDst[1] = clamp(128 + ((r * CB_R + g * CB_G + b * CB_B + 32768) >> 16));
    pDst[2] = clamp(128 + ((r * CR_R + g * CR_G + b * CR_B + 32768) >> 16));
  }
}

void RGBA_to_Y(uint8* pDst, const uint8 *pSrc, int num_pixels)
{
  for ( ; num_pixels; pDst++, pSrc += 4, num_pixels--)
    pDst[0] = static_cast<uint8>((pSrc[0] * YR + pSrc[1] * YG + pSrc[2] * YB + 32768) >> 16);
}

void Y_to_YCC(uint8* pDst, const uint8* pSrc, int num_pixels)
{
  for( ; num_pixels; pDst += 3, pSrc++, num_pixels--) { pDst[0] = pSrc[0]; pDst[1] = 128; pDst[2] = 128; }
}
} // namespace jpge
