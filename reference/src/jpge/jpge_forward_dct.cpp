#include "jpge_forward_dct.h"

namespace jpge {

// Forward DCT - DCT derived from jfdctint.
enum { CONST_BITS = 13, ROW_BITS = 2 };

#define DCT_DESCALE(x, n) (((x) + (((int32)1) << ((n) - 1))) >> (n))
#define DCT_MUL(var, c) (static_cast<int16>(var) * static_cast<int32>(c))
#define DCT1D(s0, s1, s2, s3, s4, s5, s6, s7) \
  int32 t0 = s0 + s7, t7 = s0 - s7, t1 = s1 + s6, t6 = s1 - s6, t2 = s2 + s5, t5 = s2 - s5, t3 = s3 + s4, t4 = s3 - s4; \
  int32 t10 = t0 + t3, t13 = t0 - t3, t11 = t1 + t2, t12 = t1 - t2; \
  int32 u1 = DCT_MUL(t12 + t13, 4433); \
  s2 = u1 + DCT_MUL(t13, 6270); \
  s6 = u1 + DCT_MUL(t12, -15137); \
  u1 = t4 + t7; \
  int32 u2 = t5 + t6, u3 = t4 + t6, u4 = t5 + t7; \
  int32 z5 = DCT_MUL(u3 + u4, 9633); \
  t4 = DCT_MUL(t4, 2446); t5 = DCT_MUL(t5, 16819); \
  t6 = DCT_MUL(t6, 25172); t7 = DCT_MUL(t7, 12299); \
  u1 = DCT_MUL(u1, -7373); u2 = DCT_MUL(u2, -20995); \
  u3 = DCT_MUL(u3, -16069); u4 = DCT_MUL(u4, -3196); \
  u3 += z5; u4 += z5; \
  s0 = t10 + t11; s1 = t7 + u1 + u4; s3 = t6 + u2 + u3; s4 = t10 - t11; s5 = t5 + u2 + u4; s7 = t4 + u1 + u3;

void DCT2D(int32 *p)
{
  int32 c, *q = p;
  for (c = 7; c >= 0; c--, q += 8)
  {
    int32 s0 = q[0], s1 = q[1], s2 = q[2], s3 = q[3], s4 = q[4], s5 = q[5], s6 = q[6], s7 = q[7];
    DCT1D(s0, s1, s2, s3, s4, s5, s6, s7);
    q[0] = s0 << ROW_BITS; q[1] = DCT_DESCALE(s1, CONST_BITS-ROW_BITS); q[2] = DCT_DESCALE(s2, CONST_BITS-ROW_BITS); q[3] = DCT_DESCALE(s3, CONST_BITS-ROW_BITS);
    q[4] = s4 << ROW_BITS; q[5] = DCT_DESCALE(s5, CONST_BITS-ROW_BITS); q[6] = DCT_DESCALE(s6, CONST_BITS-ROW_BITS); q[7] = DCT_DESCALE(s7, CONST_BITS-ROW_BITS);
  }
  
  for (q = p, c = 7; c >= 0; c--, q++)
  {
    int32 s0 = q[0*8], s1 = q[1*8], s2 = q[2*8], s3 = q[3*8], s4 = q[4*8], s5 = q[5*8], s6 = q[6*8], s7 = q[7*8];
    DCT1D(s0, s1, s2, s3, s4, s5, s6, s7);
    q[0*8] = DCT_DESCALE(s0, ROW_BITS+3); q[1*8] = DCT_DESCALE(s1, CONST_BITS+ROW_BITS+3); q[2*8] = DCT_DESCALE(s2, CONST_BITS+ROW_BITS+3); q[3*8] = DCT_DESCALE(s3, CONST_BITS+ROW_BITS+3);
    q[4*8] = DCT_DESCALE(s4, ROW_BITS+3); q[5*8] = DCT_DESCALE(s5, CONST_BITS+ROW_BITS+3); q[6*8] = DCT_DESCALE(s6, CONST_BITS+ROW_BITS+3); q[7*8] = DCT_DESCALE(s7, CONST_BITS+ROW_BITS+3);
  }
}
} // namespace jpge
