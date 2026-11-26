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
      void convert_RGB_block_to_YCC(
        vuint8m1_t r_group, vuint8m1_t g_group, vuint8m1_t b_group,
        vuint8m1_t *y_group, vuint8m1_t *cr_group, vuint8m1_t *cb_group);
      void convert_RGB_block_to_Y(vuint8m1_t r_group, vuint8m1_t g_group, vuint8m1_t b_group, vuint8m1_t *y_group);
      void convert_Y_block_to_YCC(vuint8m1_t y_in_group, vuint8m1_t *y_out_group, vuint8m1_t *cr_group, vuint8m1_t *cb_group);
      };
}

#endif