#include "jpgd.h"

#include "jpgd_scripts.h"


namespace jpgd {

#define CONST_BITS  13
#define PASS1_BITS  2
#define SCALEDONE ((int32)1)

#define FIX_0_298631336  ((int32)2446)        /* FIX(0.298631336) */
#define FIX_0_390180644  ((int32)3196)        /* FIX(0.390180644) */
#define FIX_0_541196100  ((int32)4433)        /* FIX(0.541196100) */
#define FIX_0_765366865  ((int32)6270)        /* FIX(0.765366865) */
#define FIX_0_899976223  ((int32)7373)        /* FIX(0.899976223) */
#define FIX_1_175875602  ((int32)9633)        /* FIX(1.175875602) */
#define FIX_1_501321110  ((int32)12299)       /* FIX(1.501321110) */
#define FIX_1_847759065  ((int32)15137)       /* FIX(1.847759065) */
#define FIX_1_961570560  ((int32)16069)       /* FIX(1.961570560) */
#define FIX_2_053119869  ((int32)16819)       /* FIX(2.053119869) */
#define FIX_2_562915447  ((int32)20995)       /* FIX(2.562915447) */
#define FIX_3_072711026  ((int32)25172)       /* FIX(3.072711026) */

#define DESCALE(x,n)  (((x) + (SCALEDONE << ((n)-1))) >> (n))
#define DESCALE_ZEROSHIFT(x,n)  (((x) + (128 << (n)) + (SCALEDONE << ((n)-1))) >> (n))

#define MULTIPLY(var, cnst)  ((var) * (cnst))        
#define CLAMP(i) ((static_cast<uint>(i) > 255) ? (((~i) >> 31) & 0xFF) : (i))

// Compiler creates a fast path 1D IDCT for X non-zero columns
template <int NONZERO_COLS>
struct Row
{
  static void idct(int* pTemp, const jpgd_block_t* pSrc)
  {
    // ACCESS_COL() will be optimized at compile time to either an array access, or 0.
    #define ACCESS_COL(x) (((x) < NONZERO_COLS) ? (int)pSrc[x] : 0)

    const int z2 = ACCESS_COL(2), z3 = ACCESS_COL(6);

    const int z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
    const int tmp2 = z1 + MULTIPLY(z3, - FIX_1_847759065);
    const int tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

    const int tmp0 = (ACCESS_COL(0) + ACCESS_COL(4)) << CONST_BITS;
    const int tmp1 = (ACCESS_COL(0) - ACCESS_COL(4)) << CONST_BITS;

    const int tmp10 = tmp0 + tmp3, tmp13 = tmp0 - tmp3, tmp11 = tmp1 + tmp2, tmp12 = tmp1 - tmp2;

    const int atmp0 = ACCESS_COL(7), atmp1 = ACCESS_COL(5), atmp2 = ACCESS_COL(3), atmp3 = ACCESS_COL(1);

    const int bz1 = atmp0 + atmp3, bz2 = atmp1 + atmp2, bz3 = atmp0 + atmp2, bz4 = atmp1 + atmp3;
    const int bz5 = MULTIPLY(bz3 + bz4, FIX_1_175875602);

    const int az1 = MULTIPLY(bz1, - FIX_0_899976223);
    const int az2 = MULTIPLY(bz2, - FIX_2_562915447);
    const int az3 = MULTIPLY(bz3, - FIX_1_961570560) + bz5;
    const int az4 = MULTIPLY(bz4, - FIX_0_390180644) + bz5;

    const int btmp0 = MULTIPLY(atmp0, FIX_0_298631336) + az1 + az3;
    const int btmp1 = MULTIPLY(atmp1, FIX_2_053119869) + az2 + az4;
    const int btmp2 = MULTIPLY(atmp2, FIX_3_072711026) + az2 + az3;
    const int btmp3 = MULTIPLY(atmp3, FIX_1_501321110) + az1 + az4;

    pTemp[0] = DESCALE(tmp10 + btmp3, CONST_BITS-PASS1_BITS);
    pTemp[7] = DESCALE(tmp10 - btmp3, CONST_BITS-PASS1_BITS);
    pTemp[1] = DESCALE(tmp11 + btmp2, CONST_BITS-PASS1_BITS);
    pTemp[6] = DESCALE(tmp11 - btmp2, CONST_BITS-PASS1_BITS);
    pTemp[2] = DESCALE(tmp12 + btmp1, CONST_BITS-PASS1_BITS);
    pTemp[5] = DESCALE(tmp12 - btmp1, CONST_BITS-PASS1_BITS);
    pTemp[3] = DESCALE(tmp13 + btmp0, CONST_BITS-PASS1_BITS);
    pTemp[4] = DESCALE(tmp13 - btmp0, CONST_BITS-PASS1_BITS);
  }
};

template <>
struct Row<0>
{
  static void idct(int* pTemp, const jpgd_block_t* pSrc)
  {
#ifdef _MSC_VER
    pTemp; pSrc;
#endif
  }
};

template <>
struct Row<1>
{
  static void idct(int* pTemp, const jpgd_block_t* pSrc)
  {
    const int dcval = (pSrc[0] << PASS1_BITS);

    pTemp[0] = dcval;
    pTemp[1] = dcval;
    pTemp[2] = dcval;
    pTemp[3] = dcval;
    pTemp[4] = dcval;
    pTemp[5] = dcval;
    pTemp[6] = dcval;
    pTemp[7] = dcval;
  }
};

// Compiler creates a fast path 1D IDCT for X non-zero rows
template <int NONZERO_ROWS>
struct Col
{
  static void idct(uint8* pDst_ptr, const int* pTemp)
  {
    // ACCESS_ROW() will be optimized at compile time to either an array access, or 0.
    #define ACCESS_ROW(x) (((x) < NONZERO_ROWS) ? pTemp[x * 8] : 0)

    const int z2 = ACCESS_ROW(2);
    const int z3 = ACCESS_ROW(6);

    const int z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
    const int tmp2 = z1 + MULTIPLY(z3, - FIX_1_847759065);
    const int tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

    const int tmp0 = (ACCESS_ROW(0) + ACCESS_ROW(4)) << CONST_BITS;
    const int tmp1 = (ACCESS_ROW(0) - ACCESS_ROW(4)) << CONST_BITS;

    const int tmp10 = tmp0 + tmp3, tmp13 = tmp0 - tmp3, tmp11 = tmp1 + tmp2, tmp12 = tmp1 - tmp2;

    const int atmp0 = ACCESS_ROW(7), atmp1 = ACCESS_ROW(5), atmp2 = ACCESS_ROW(3), atmp3 = ACCESS_ROW(1);

    const int bz1 = atmp0 + atmp3, bz2 = atmp1 + atmp2, bz3 = atmp0 + atmp2, bz4 = atmp1 + atmp3;
    const int bz5 = MULTIPLY(bz3 + bz4, FIX_1_175875602);

    const int az1 = MULTIPLY(bz1, - FIX_0_899976223);
    const int az2 = MULTIPLY(bz2, - FIX_2_562915447);
    const int az3 = MULTIPLY(bz3, - FIX_1_961570560) + bz5;
    const int az4 = MULTIPLY(bz4, - FIX_0_390180644) + bz5;

    const int btmp0 = MULTIPLY(atmp0, FIX_0_298631336) + az1 + az3;
    const int btmp1 = MULTIPLY(atmp1, FIX_2_053119869) + az2 + az4;
    const int btmp2 = MULTIPLY(atmp2, FIX_3_072711026) + az2 + az3;
    const int btmp3 = MULTIPLY(atmp3, FIX_1_501321110) + az1 + az4;

    int i = DESCALE_ZEROSHIFT(tmp10 + btmp3, CONST_BITS+PASS1_BITS+3);
    pDst_ptr[8*0] = (uint8)CLAMP(i);

    i = DESCALE_ZEROSHIFT(tmp10 - btmp3, CONST_BITS+PASS1_BITS+3);
    pDst_ptr[8*7] = (uint8)CLAMP(i);

    i = DESCALE_ZEROSHIFT(tmp11 + btmp2, CONST_BITS+PASS1_BITS+3);
    pDst_ptr[8*1] = (uint8)CLAMP(i);

    i = DESCALE_ZEROSHIFT(tmp11 - btmp2, CONST_BITS+PASS1_BITS+3);
    pDst_ptr[8*6] = (uint8)CLAMP(i);

    i = DESCALE_ZEROSHIFT(tmp12 + btmp1, CONST_BITS+PASS1_BITS+3);
    pDst_ptr[8*2] = (uint8)CLAMP(i);

    i = DESCALE_ZEROSHIFT(tmp12 - btmp1, CONST_BITS+PASS1_BITS+3);
    pDst_ptr[8*5] = (uint8)CLAMP(i);

    i = DESCALE_ZEROSHIFT(tmp13 + btmp0, CONST_BITS+PASS1_BITS+3);
    pDst_ptr[8*3] = (uint8)CLAMP(i);

    i = DESCALE_ZEROSHIFT(tmp13 - btmp0, CONST_BITS+PASS1_BITS+3);
    pDst_ptr[8*4] = (uint8)CLAMP(i);
  }
};

template <>
struct Col<1>
{
  static void idct(uint8* pDst_ptr, const int* pTemp)
  {
    int dcval = DESCALE_ZEROSHIFT(pTemp[0], PASS1_BITS+3);
    const uint8 dcval_clamped = (uint8)CLAMP(dcval);
    pDst_ptr[0*8] = dcval_clamped;
    pDst_ptr[1*8] = dcval_clamped;
    pDst_ptr[2*8] = dcval_clamped;
    pDst_ptr[3*8] = dcval_clamped;
    pDst_ptr[4*8] = dcval_clamped;
    pDst_ptr[5*8] = dcval_clamped;
    pDst_ptr[6*8] = dcval_clamped;
    pDst_ptr[7*8] = dcval_clamped;
  }
};

static const uint8 s_idct_row_table[] =
{
  1,0,0,0,0,0,0,0, 2,0,0,0,0,0,0,0, 2,1,0,0,0,0,0,0, 2,1,1,0,0,0,0,0, 2,2,1,0,0,0,0,0, 3,2,1,0,0,0,0,0, 4,2,1,0,0,0,0,0, 4,3,1,0,0,0,0,0,
  4,3,2,0,0,0,0,0, 4,3,2,1,0,0,0,0, 4,3,2,1,1,0,0,0, 4,3,2,2,1,0,0,0, 4,3,3,2,1,0,0,0, 4,4,3,2,1,0,0,0, 5,4,3,2,1,0,0,0, 6,4,3,2,1,0,0,0,
  6,5,3,2,1,0,0,0, 6,5,4,2,1,0,0,0, 6,5,4,3,1,0,0,0, 6,5,4,3,2,0,0,0, 6,5,4,3,2,1,0,0, 6,5,4,3,2,1,1,0, 6,5,4,3,2,2,1,0, 6,5,4,3,3,2,1,0,
  6,5,4,4,3,2,1,0, 6,5,5,4,3,2,1,0, 6,6,5,4,3,2,1,0, 7,6,5,4,3,2,1,0, 8,6,5,4,3,2,1,0, 8,7,5,4,3,2,1,0, 8,7,6,4,3,2,1,0, 8,7,6,5,3,2,1,0,
  8,7,6,5,4,2,1,0, 8,7,6,5,4,3,1,0, 8,7,6,5,4,3,2,0, 8,7,6,5,4,3,2,1, 8,7,6,5,4,3,2,2, 8,7,6,5,4,3,3,2, 8,7,6,5,4,4,3,2, 8,7,6,5,5,4,3,2,
  8,7,6,6,5,4,3,2, 8,7,7,6,5,4,3,2, 8,8,7,6,5,4,3,2, 8,8,8,6,5,4,3,2, 8,8,8,7,5,4,3,2, 8,8,8,7,6,4,3,2, 8,8,8,7,6,5,3,2, 8,8,8,7,6,5,4,2,
  8,8,8,7,6,5,4,3, 8,8,8,7,6,5,4,4, 8,8,8,7,6,5,5,4, 8,8,8,7,6,6,5,4, 8,8,8,7,7,6,5,4, 8,8,8,8,7,6,5,4, 8,8,8,8,8,6,5,4, 8,8,8,8,8,7,5,4,
  8,8,8,8,8,7,6,4, 8,8,8,8,8,7,6,5, 8,8,8,8,8,7,6,6, 8,8,8,8,8,7,7,6, 8,8,8,8,8,8,7,6, 8,8,8,8,8,8,8,6, 8,8,8,8,8,8,8,7, 8,8,8,8,8,8,8,8,
};

static const uint8 s_idct_col_table[] = { 1, 1, 2, 3, 3, 3, 3, 3, 3, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 };

void idct(const jpgd_block_t* pSrc_ptr, uint8* pDst_ptr, int block_max_zag)
{
  JPGD_ASSERT(block_max_zag >= 1);
  JPGD_ASSERT(block_max_zag <= 64);

  if (block_max_zag <= 1)
  {
    int k = ((pSrc_ptr[0] + 4) >> 3) + 128;
    k = CLAMP(k);
    k = k | (k<<8);
    k = k | (k<<16);

    for (int i = 8; i > 0; i--)
    {
      *(int*)&pDst_ptr[0] = k;
      *(int*)&pDst_ptr[4] = k;
      pDst_ptr += 8;
    }
    return;
  }

  int temp[64];

  const jpgd_block_t* pSrc = pSrc_ptr;
  int* pTemp = temp;

  const uint8* pRow_tab = &s_idct_row_table[(block_max_zag - 1) * 8];
  int i;
  for (i = 8; i > 0; i--, pRow_tab++)
  {
    switch (*pRow_tab)
    {
      case 0: Row<0>::idct(pTemp, pSrc); break;
      case 1: Row<1>::idct(pTemp, pSrc); break;
      case 2: Row<2>::idct(pTemp, pSrc); break;
      case 3: Row<3>::idct(pTemp, pSrc); break;
      case 4: Row<4>::idct(pTemp, pSrc); break;
      case 5: Row<5>::idct(pTemp, pSrc); break;
      case 6: Row<6>::idct(pTemp, pSrc); break;
      case 7: Row<7>::idct(pTemp, pSrc); break;
      case 8: Row<8>::idct(pTemp, pSrc); break;
    }

    pSrc += 8;
    pTemp += 8;
  }

  pTemp = temp;

  const int nonzero_rows = s_idct_col_table[block_max_zag - 1];
  for (i = 8; i > 0; i--)
  {
    switch (nonzero_rows)
    {
      case 1: Col<1>::idct(pDst_ptr, pTemp); break;
      case 2: Col<2>::idct(pDst_ptr, pTemp); break;
      case 3: Col<3>::idct(pDst_ptr, pTemp); break;
      case 4: Col<4>::idct(pDst_ptr, pTemp); break;
      case 5: Col<5>::idct(pDst_ptr, pTemp); break;
      case 6: Col<6>::idct(pDst_ptr, pTemp); break;
      case 7: Col<7>::idct(pDst_ptr, pTemp); break;
      case 8: Col<8>::idct(pDst_ptr, pTemp); break;
    }

    pTemp++;
    pDst_ptr++;
  }
}

void idct_4x4(const jpgd_block_t* pSrc_ptr, uint8* pDst_ptr)
{
  int temp[64];
  int* pTemp = temp;
  const jpgd_block_t* pSrc = pSrc_ptr;

  for (int i = 4; i > 0; i--)
  {
    Row<4>::idct(pTemp, pSrc);
    pSrc += 8;
    pTemp += 8;
  }

  pTemp = temp;
  for (int i = 8; i > 0; i--)
  {
    Col<4>::idct(pDst_ptr, pTemp);
    pTemp++;
    pDst_ptr++;
  }
}

}