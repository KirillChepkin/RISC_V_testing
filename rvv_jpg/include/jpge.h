#include <riscv_vector.h>
#include <cstdint>

#ifndef JPEG_ENCODER_H
#define JPEG_ENCODER_H


using uint8  = std::uint8_t;
using int16  = std::int16_t;
using int32  = std::int32_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint   = unsigned int;

namespace rvv_jpge {
  
  class jpeg_encoder {
    private:
    public:
      void compute_quant_table(int32 *pDst, int32 *pSrc, int32 quality);
  };
}

#endif