#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <jpge.h>

#define N 1000

// #ifdef VECTOR
// #include <riscv_vector.h>

// void *memcpy_rvv(void* destination, const void* source, size_t n) {
//   unsigned char *dst = (unsigned char*)destination;
//   const unsigned char *src = (unsigned char*)source;
//   // copy data byte by byte
//   for (size_t vl; n > 0; n -= vl, src += vl, dst += vl) {
//     vl = __riscv_vsetvl_e8m8(n);
//     // Load src[0..vl)
//     vuint8m8_t vec_src = __riscv_vle8_v_u8m8(src, vl);
//     // Store dst[0..vl)
//     __riscv_vse8_v_u8m8(dst, vec_src, vl);
//     // src is incremented vl (bytes)
//     // dst is incremented vl (bytes)
//     // n is decremented vl
//   }
//   return destination;
// }

// void memmul(uint32_t* a, uint32_t* b, size_t n) {
//   // multiplies all elements from arrays a and b of equal length n using riscv intrinsics.
//   while (n > 0) {
//     size_t vl = __riscv_vsetvl_e32m8(n);
//     vuint32m8_t a_vector = __riscv_vle32_v_u32m8(a, vl);
//     vuint32m8_t b_vector = __riscv_vle32_v_u32m8(b, vl);
//     b_vector = __riscv_vmul_vv_u32m8(a_vector, b_vector, vl);
//     __riscv_vse32_v_u32m8(b, b_vector, vl);
//     a += vl;
//     b += vl;
//     n -= vl;
//   }
// }

// #else

// void memmul(uint32_t* a, uint32_t* b, size_t n) {
//   // multiplies all elements from arrays a and b of equal length n.
//   for (size_t i = 0; i < n; i++) {
//     b[i] = a[i] * b[i];
//   }
// }

// #endif

int main() {
  #ifdef VECTOR
    printf("Compiling for riscv vector.\n");
    printf("number of 32-bit elements in one register: %lu\n", __riscv_vsetvl_e32m1(1024));
  #else 
    printf("Compiling for general riscv.\n");
  #endif

  static int32_t s_std_croma_quant[64] = {17,18,18,24,21,24,47,26,26,47,99,66,56,66,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99,99 };
  int32_t table[64];
  rvv_jpge::jpeg_encoder* encoder = new rvv_jpge::jpeg_encoder();
  encoder -> compute_quant_table(table, s_std_croma_quant, 50);
  delete encoder;
  for (int i = 0; i < 64; i++) {
    printf("%d ", table[i]);
  }
  printf("\n");
}