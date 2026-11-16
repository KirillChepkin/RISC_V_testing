#include <riscv_vector.h>
#include "jpge_forward_dct.h"

namespace rvv_jpge {

// Forward DCT - DCT derived from jfdctint.
enum {
  CONST_BITS = 13,
  ROW_BITS = 2,

  ROW_SHIFT = CONST_BITS - ROW_BITS,
  COLUMN_SHIFT = ROW_BITS + 3,
  COLUMN_SHIFT_AC = CONST_BITS + ROW_BITS + 3
};

inline vint32m8_t DCT_DESCALE_VECTOR(vint32m8_t x, size_t n, size_t vl) {
    int32_t rounding = 1 << (n - 1);
    x = __riscv_vadd_vx_i32m8(x, rounding, vl);
    return __riscv_vsra_vx_i32m8(x, n, vl);
}

inline vint32m8_t narrow_to_i16(vint32m8_t x, size_t vl) {
    x = __riscv_vsll_vx_i32m8(x, 16, vl);
    x = __riscv_vsra_vx_i32m8(x, 16, vl);
    return x;
}

inline vint32m8_t DCT_MUL_VECTOR(vint32m8_t x, int32 c, size_t vl) {
    x = narrow_to_i16(x, vl);
    return __riscv_vmul_vx_i32m8(x, c, vl);
}

inline vint32m8_t sum_3_vectors(vint32m8_t a, vint32m8_t b, vint32m8_t c, size_t vl) {
  vint32m8_t a_plus_b = __riscv_vadd_vv_i32m8(a, b, vl);
  return __riscv_vadd_vv_i32m8(a_plus_b, c, vl);
}

inline void DCT1D(
    vint32m8_t& s0, vint32m8_t& s1,
    vint32m8_t& s2, vint32m8_t& s3,
    vint32m8_t& s4, vint32m8_t& s5,
    vint32m8_t& s6, vint32m8_t& s7,
    size_t vl
) {
    vint32m8_t t0 = __riscv_vadd_vv_i32m8(s0, s7, vl);
    vint32m8_t t7 = __riscv_vsub_vv_i32m8(s0, s7, vl);

    vint32m8_t t1 = __riscv_vadd_vv_i32m8(s1, s6, vl);
    vint32m8_t t6 = __riscv_vsub_vv_i32m8(s1, s6, vl);

    vint32m8_t t2 = __riscv_vadd_vv_i32m8(s2, s5, vl);
    vint32m8_t t5 = __riscv_vsub_vv_i32m8(s2, s5, vl);

    vint32m8_t t3 = __riscv_vadd_vv_i32m8(s3, s4, vl);
    vint32m8_t t4 = __riscv_vsub_vv_i32m8(s3, s4, vl);

    vint32m8_t t10 = __riscv_vadd_vv_i32m8(t0, t3, vl);
    vint32m8_t t13 = __riscv_vsub_vv_i32m8(t0, t3, vl);

    vint32m8_t t11 = __riscv_vadd_vv_i32m8(t1, t2, vl);
    vint32m8_t t12 = __riscv_vsub_vv_i32m8(t1, t2, vl);

    vint32m8_t t14 = __riscv_vadd_vv_i32m8(t12, t13, vl);
    vint32m8_t u1 = DCT_MUL_VECTOR(t14, 4433, vl);
    
    vint32m8_t t15 = DCT_MUL_VECTOR(t13, 6270, vl);
    s2 = __riscv_vadd_vv_i32m8(u1, t15, vl);

    vint32m8_t t16 = DCT_MUL_VECTOR(t12, -15137, vl);
    s6 = __riscv_vadd_vv_i32m8(u1, t16, vl);

    u1 = __riscv_vadd_vv_i32m8(t4, t7, vl);
    vint32m8_t u2 = __riscv_vadd_vv_i32m8(t5, t6, vl);
    vint32m8_t u3 = __riscv_vadd_vv_i32m8(t4, t6, vl);
    vint32m8_t u4 = __riscv_vadd_vv_i32m8(t5, t7, vl);

    vint32m8_t u5 = __riscv_vadd_vv_i32m8(u3, u4, vl);

    vint32m8_t z5 = DCT_MUL_VECTOR(u5, 9633, vl);

    t4 = DCT_MUL_VECTOR(t4, 2446, vl);
    t5 = DCT_MUL_VECTOR(t5, 16819, vl);
    t6 = DCT_MUL_VECTOR(t6, 25172, vl);
    t7 = DCT_MUL_VECTOR(t7, 12299, vl);

    u1 = DCT_MUL_VECTOR(u1, -7373, vl);
    u2 = DCT_MUL_VECTOR(u2, -20995, vl);
    u3 = DCT_MUL_VECTOR(u3, -16069, vl);
    u4 = DCT_MUL_VECTOR(u4, -3196, vl);

    u3 = __riscv_vadd_vv_i32m8(u3, z5, vl);
    u4 = __riscv_vadd_vv_i32m8(u4, z5, vl);

    s0 = __riscv_vadd_vv_i32m8(t10, t11, vl);

    s1 = sum_3_vectors(t7, u1, u4, vl);
    s3 = sum_3_vectors(t6, u2, u3, vl);

    s4 = __riscv_vsub_vv_i32m8(t10, t11, vl);

    s5 = sum_3_vectors(t5, u2, u4, vl);
    s7 = sum_3_vectors(t4, u1, u3, vl);
}

void DCT2D(int32 *p) {
  int32 s0_s[] = {p[0], p[8], p[16], p[24], p[32], p[40], p[48], p[56]};
  int32 s1_s[] = {p[1], p[9], p[17], p[25], p[33], p[41], p[49], p[57]};
  int32 s2_s[] = {p[2], p[10], p[18], p[26], p[34], p[42], p[50], p[58]};
  int32 s3_s[] = {p[3], p[11], p[19], p[27], p[35], p[43], p[51], p[59]};
  int32 s4_s[] = {p[4], p[12], p[20], p[28], p[36], p[44], p[52], p[60]};
  int32 s5_s[] = {p[5], p[13], p[21], p[29], p[37], p[45], p[53], p[61]};
  int32 s6_s[] = {p[6], p[14], p[22], p[30], p[38], p[46], p[54], p[62]};
  int32 s7_s[] = {p[7], p[15], p[23], p[31], p[39], p[47], p[55], p[63]};
  size_t n = 8, offs = 0;
  while (n > 0) {
    size_t vl =  __riscv_vsetvl_e32m8(n);
    vint32m8_t s0 = __riscv_vle32_v_i32m8(s0_s + offs, vl);
    vint32m8_t s1 = __riscv_vle32_v_i32m8(s1_s + offs, vl);
    vint32m8_t s2 = __riscv_vle32_v_i32m8(s2_s + offs, vl);
    vint32m8_t s3 = __riscv_vle32_v_i32m8(s3_s + offs, vl);
    vint32m8_t s4 = __riscv_vle32_v_i32m8(s4_s + offs, vl);
    vint32m8_t s5 = __riscv_vle32_v_i32m8(s5_s + offs, vl);
    vint32m8_t s6 = __riscv_vle32_v_i32m8(s6_s + offs, vl);
    vint32m8_t s7 = __riscv_vle32_v_i32m8(s7_s + offs, vl);
    DCT1D(s0, s1, s2, s3, s4, s5, s6, s7, vl);
    s0 = __riscv_vsll_vx_i32m8(s0, ROW_BITS, vl);
    s1 = DCT_DESCALE_VECTOR(s1, ROW_SHIFT, vl);
    s2 = DCT_DESCALE_VECTOR(s2, ROW_SHIFT, vl);
    s3 = DCT_DESCALE_VECTOR(s3, ROW_SHIFT, vl);
    s4 = __riscv_vsll_vx_i32m8(s4, ROW_BITS, vl);
    s5 = DCT_DESCALE_VECTOR(s5, ROW_SHIFT, vl);
    s6 = DCT_DESCALE_VECTOR(s6, ROW_SHIFT, vl);
    s7 = DCT_DESCALE_VECTOR(s7, ROW_SHIFT, vl);
    __riscv_vse32_v_i32m8(s0_s + offs, s0, vl);
    __riscv_vse32_v_i32m8(s1_s + offs, s1, vl);
    __riscv_vse32_v_i32m8(s2_s + offs, s2, vl);
    __riscv_vse32_v_i32m8(s3_s + offs, s3, vl);
    __riscv_vse32_v_i32m8(s4_s + offs, s4, vl);
    __riscv_vse32_v_i32m8(s5_s + offs, s5, vl);
    __riscv_vse32_v_i32m8(s6_s + offs, s6, vl);
    __riscv_vse32_v_i32m8(s7_s + offs, s7, vl);
    n -= vl;
    offs += vl;
  }
  for (int row = 0; row < 8; ++row) {
    p[row * 8 + 0] = s0_s[row];
    p[row * 8 + 1] = s1_s[row];
    p[row * 8 + 2] = s2_s[row];
    p[row * 8 + 3] = s3_s[row];
    p[row * 8 + 4] = s4_s[row];
    p[row * 8 + 5] = s5_s[row];
    p[row * 8 + 6] = s6_s[row];
    p[row * 8 + 7] = s7_s[row];
  }
  int32* s0_p = p;
  int32* s1_p = p + 8;
  int32* s2_p = p + 16;
  int32* s3_p = p + 24;
  int32* s4_p = p + 32;
  int32* s5_p = p + 40;
  int32* s6_p = p + 48;
  int32* s7_p = p + 56;

  n = 8; offs = 0;
  while (n > 0) {
    size_t vl =  __riscv_vsetvl_e32m8(n);
    vint32m8_t s0 = __riscv_vle32_v_i32m8(s0_p + offs, vl);
    vint32m8_t s1 = __riscv_vle32_v_i32m8(s1_p + offs, vl);
    vint32m8_t s2 = __riscv_vle32_v_i32m8(s2_p + offs, vl);
    vint32m8_t s3 = __riscv_vle32_v_i32m8(s3_p + offs, vl);
    vint32m8_t s4 = __riscv_vle32_v_i32m8(s4_p + offs, vl);
    vint32m8_t s5 = __riscv_vle32_v_i32m8(s5_p + offs, vl);
    vint32m8_t s6 = __riscv_vle32_v_i32m8(s6_p + offs, vl);
    vint32m8_t s7 = __riscv_vle32_v_i32m8(s7_p + offs, vl);
    DCT1D(s0, s1, s2, s3, s4, s5, s6, s7, vl);
    s0 = DCT_DESCALE_VECTOR(s0, COLUMN_SHIFT, vl);
    s1 = DCT_DESCALE_VECTOR(s1, COLUMN_SHIFT_AC, vl);
    s2 = DCT_DESCALE_VECTOR(s2, COLUMN_SHIFT_AC, vl);
    s3 = DCT_DESCALE_VECTOR(s3, COLUMN_SHIFT_AC, vl);
    s4 = DCT_DESCALE_VECTOR(s4, COLUMN_SHIFT, vl);
    s5 = DCT_DESCALE_VECTOR(s5, COLUMN_SHIFT_AC, vl);
    s6 = DCT_DESCALE_VECTOR(s6, COLUMN_SHIFT_AC, vl);
    s7 = DCT_DESCALE_VECTOR(s7, COLUMN_SHIFT_AC, vl);
    __riscv_vse32_v_i32m8(s0_p + offs, s0, vl);
    __riscv_vse32_v_i32m8(s1_p + offs, s1, vl);
    __riscv_vse32_v_i32m8(s2_p + offs, s2, vl);
    __riscv_vse32_v_i32m8(s3_p + offs, s3, vl);
    __riscv_vse32_v_i32m8(s4_p + offs, s4, vl);
    __riscv_vse32_v_i32m8(s5_p + offs, s5, vl);
    __riscv_vse32_v_i32m8(s6_p + offs, s6, vl);
    __riscv_vse32_v_i32m8(s7_p + offs, s7, vl);
    n -= vl;
    offs += vl;
  }
}
} // namespace rvv_jpge