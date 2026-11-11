// jpge.h - C++ class for JPEG compression.
// Public domain, Rich Geldreich <richgel99@gmail.com>
// Alex Evans: Added RGBA support, linear memory allocator.
#ifndef JPEG_ENCODER_H
#define JPEG_ENCODER_H

namespace rvv_jpge {
  typedef unsigned char  uint8;
  typedef signed short   int16;
  typedef signed int     int32_t;
  typedef unsigned short uint16;
  typedef unsigned int   uint32;
  typedef unsigned int   uint;
  
  class jpeg_encoder {
    private:
      int32_t quant_table;
    public:
      void compute_quant_table(int32_t *pDst, int32_t *pSrc, int32_t quality);
  };
}

#endif