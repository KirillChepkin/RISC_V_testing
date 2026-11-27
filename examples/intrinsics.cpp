#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1000

#ifdef VECTOR
#include <riscv_vector.h>

void memmul(uint32_t* a, uint32_t* b, size_t n) {
  // multiplies all elements from arrays a and b of equal length n using riscv intrinsics.
  while (n > 0) {
    size_t vl = __riscv_vsetvl_e32m8(n);
    vuint32m8_t a_vector = __riscv_vle32_v_u32m8(a, vl);
    vuint32m8_t b_vector = __riscv_vle32_v_u32m8(b, vl);
    b_vector = __riscv_vmul_vv_u32m8(a_vector, b_vector, vl);
    __riscv_vse32_v_u32m8(b, b_vector, vl);
    a += vl;
    b += vl;
    n -= vl;
  }
}

#else

void memmul(uint32_t* a, uint32_t* b, size_t n) {
  // multiplies all elements from arrays a and b of equal length n.
  for (size_t i = 0; i < n; i++) {
    b[i] = a[i] * b[i];
  }
}

#endif

int main() {
  static uint32_t s_std_croma_quant[64] = {17,18,18,24,21,24,47,26,26,47,99,66,56,66,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99 };
  uint32_t table[64] = {0};

  #ifdef VECTOR
    printf("Compiling for riscv vector.\n");
    printf("number of 32-bit elements in one register: %lu\n", __riscv_vsetvl_e32m1(1024));
    uint8_t src[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint32_t dst[8] = {};

    size_t vl = __riscv_vsetvl_e8m1(8);

    vuint8m1_t v8 = __riscv_vle8_v_u8m1(src, vl);
    // vuint16m2_t v16 = __riscv_vwaddu_vx_u16m2(v8, 0, vl);
    // vuint32m4_t v32 = __riscv_vzext_vf4_u32m4(v8, vl);

    volatile unsigned int zero = 0;
    vuint16m2_t v16 = __riscv_vwaddu_vx_u16m2(v8, zero, vl);
    vuint32m4_t v32 = __riscv_vwaddu_vx_u32m4(v16, zero, vl);


    // vuint16m2_t v16 =  __riscv_vzext_vf2_u16m2(v8, vl);
    // vuint32m4_t v32 =  __riscv_vzext_vf2_u32m2(v16, vl);
    // vuint32m4_t v32 =  __riscv_vzext_vf2(v16, vl);

    __riscv_vse32_v_u32m4(dst, v32, vl);

    for (size_t i = 0; i < vl; ++i) {
        printf("%u ", dst[i]);
    }

    printf("\n");
  #else 
    printf("Compiling for general riscv.\n");
  #endif
  for (size_t i = 0; i < 64; i++) {
    table[i] = 2U;
  }
  memmul(s_std_croma_quant, table, 64);
  for (int i = 0; i < 64; i++) {
    printf("%d ", table[i]);
  }
  printf("\n");
}