#include <riscv_vector.h>
#include <cstdint>

#ifndef JPEG_ENCODER_H
#define JPEG_ENCODER_H

namespace rvv_jpge {
  typedef unsigned char  uint8;
  typedef signed short   int16;
  typedef signed int     int32;
  class jpeg_encoder {
    private:
      //vint32m8_t quant_table;
    public:
      void compute_quant_table(int32_t *pDst, int32_t *pSrc, int32_t quality);
  };
}

#endif