#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <riscv_vector.h>

#define N 1000

void *memcpy_rvv(void* destination, const void* source, size_t n) {
  unsigned char *dst = (unsigned char*)destination;
  const unsigned char *src = (unsigned char*)source;
  // copy data byte by byte
  for (size_t vl; n > 0; n -= vl, src += vl, dst += vl) {
    vl = __riscv_vsetvl_e8m8(n);
    // Load src[0..vl)
    vuint8m8_t vec_src = __riscv_vle8_v_u8m8(src, vl);
    // Store dst[0..vl)
    __riscv_vse8_v_u8m8(dst, vec_src, vl);
    // src is incremented vl (bytes)
    // dst is incremented vl (bytes)
    // n is decremented vl
  }
  return destination;
}

void memmul_rvv(uint32_t* a, uint32_t* b, size_t n) {
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

int main() {
  uint32_t a[N] = {};
  uint32_t b[N] = {};

  for (unsigned int i = 0; i < N; i++) {
    a[i] = 1;
    b[i] = i;
  }

  // void* ret = memcpy_rvv(b, a, N * sizeof(uint32_t));
  memmul_rvv(a, b, N);
  for (int i = 0; i < N; i++) {
    printf("[%d]", b[i]);
  }
  printf("\n");
}