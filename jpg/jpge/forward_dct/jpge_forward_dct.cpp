#include "jpge_forward_dct.h"

namespace jpge {

// Forward DCT - DCT derived from jfdctint.
enum {
  CONST_BITS = 13,
  ROW_BITS = 2,

  ROW_SHIFT = CONST_BITS - ROW_BITS,
  COLUMN_SHIFT = ROW_BITS + 3,
  COLUMN_SHIFT_AC = CONST_BITS + ROW_BITS + 3
};

#define DCT_DESCALE(x, n) (((x) + (((int32)1) << ((n) - 1))) >> (n))
#define DCT_MUL(var, c) (static_cast<int16>(var) * static_cast<int32>(c))

inline void DCT1D(
    int32& s0, int32& s1, int32& s2, int32& s3,
    int32& s4, int32& s5, int32& s6, int32& s7
) {
    int32 t0 = s0 + s7;
    int32 t7 = s0 - s7;
    int32 t1 = s1 + s6;
    int32 t6 = s1 - s6;
    int32 t2 = s2 + s5;
    int32 t5 = s2 - s5;
    int32 t3 = s3 + s4;
    int32 t4 = s3 - s4;

    int32 t10 = t0 + t3;
    int32 t13 = t0 - t3;
    int32 t11 = t1 + t2;
    int32 t12 = t1 - t2;

    int32 u1 = DCT_MUL(t12 + t13, 4433);
    s2 = u1 + DCT_MUL(t13, 6270);
    s6 = u1 + DCT_MUL(t12, -15137);

    u1 = t4 + t7;
    int32 u2 = t5 + t6;
    int32 u3 = t4 + t6;
    int32 u4 = t5 + t7;

    int32 z5 = DCT_MUL(u3 + u4, 9633);

    t4 = DCT_MUL(t4, 2446);
    t5 = DCT_MUL(t5, 16819);
    t6 = DCT_MUL(t6, 25172);
    t7 = DCT_MUL(t7, 12299);

    u1 = DCT_MUL(u1, -7373);
    u2 = DCT_MUL(u2, -20995);
    u3 = DCT_MUL(u3, -16069);
    u4 = DCT_MUL(u4, -3196);

    u3 += z5;
    u4 += z5;

    s0 = t10 + t11;
    s1 = t7 + u1 + u4;
    s3 = t6 + u2 + u3;
    s4 = t10 - t11;
    s5 = t5 + u2 + u4;
    s7 = t4 + u1 + u3;
}

void DCT2D(int32 *p) {
  for (int row = 0; row < 8; row++) {
    int32 s0 = p[row * 8 + 0];
    int32 s1 = p[row * 8 + 1];
    int32 s2 = p[row * 8 + 2];
    int32 s3 = p[row * 8 + 3];
    int32 s4 = p[row * 8 + 4];
    int32 s5 = p[row * 8 + 5];
    int32 s6 = p[row * 8 + 6];
    int32 s7 = p[row * 8 + 7];
    DCT1D(s0, s1, s2, s3, s4, s5, s6, s7);
    p[row * 8 + 0] = s0 << ROW_BITS;
    p[row * 8 + 1] = DCT_DESCALE(s1, ROW_SHIFT);
    p[row * 8 + 2] = DCT_DESCALE(s2, ROW_SHIFT);
    p[row * 8 + 3] = DCT_DESCALE(s3, ROW_SHIFT);
    p[row * 8 + 4] = s4 << ROW_BITS;
    p[row * 8 + 5] = DCT_DESCALE(s5, ROW_SHIFT);
    p[row * 8 + 6] = DCT_DESCALE(s6, ROW_SHIFT);
    p[row * 8 + 7] = DCT_DESCALE(s7, ROW_SHIFT);
  }
  for (int column = 0; column < 8; column++) {
    int32 s0 = p[0 * 8 + column];
    int32 s1 = p[1 * 8 + column];
    int32 s2 = p[2 * 8 + column];
    int32 s3 = p[3 * 8 + column];
    int32 s4 = p[4 * 8 + column];
    int32 s5 = p[5 * 8 + column];
    int32 s6 = p[6 * 8 + column];
    int32 s7 = p[7 * 8 + column];
    DCT1D(s0, s1, s2, s3, s4, s5, s6, s7);
    p[0 * 8 + column] = DCT_DESCALE(s0, COLUMN_SHIFT);
    p[1 * 8 + column] = DCT_DESCALE(s1, COLUMN_SHIFT_AC);
    p[2 * 8 + column] = DCT_DESCALE(s2, COLUMN_SHIFT_AC);
    p[3 * 8 + column] = DCT_DESCALE(s3, COLUMN_SHIFT_AC);
    p[4 * 8 + column] = DCT_DESCALE(s4, COLUMN_SHIFT);
    p[5 * 8 + column] = DCT_DESCALE(s5, COLUMN_SHIFT_AC);
    p[6 * 8 + column] = DCT_DESCALE(s6, COLUMN_SHIFT_AC);
    p[7 * 8 + column] = DCT_DESCALE(s7, COLUMN_SHIFT_AC);
  }
}
} // namespace jpge
