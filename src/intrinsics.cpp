#include <stdio.h>
#include <riscv_vector.h>

#define N 10

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

void memmul_rvv(const uint32_t* a, uint32_t* b, size_t n) {
  // adds a to b.
  size_t vl = __riscv_vsetvl_e32m8(avl);
  uint32_t i = 0;
  while (n >= vl) {
    vuint32m1_t a_vector = __riscv_vle32_v_u32m8(a + i * vl, vl);
    vuint32m1_t b_vector = __riscv_vle32_v_u32m8(b + i * vl, vl);
    b_vector = __riscv_vmul_vv_u32m1(a_vector, b_vector, vl);
    __riscv_vse8_v_u8m8(b + i * vl, b_vector, vl);
    n -= vl;
    i++;
  }
  vuint32m1_t a_vector = __riscv_vle32_v_u32m8(a + i * vl, n);
  vuint32m1_t b_vector = __riscv_vle32_v_u32m8(b + i * vl, n);
  b_vector = __riscv_vmul_vv_u32m1(a_vector, b_vector, n);
  __riscv_vse8_v_u8m8(b + i * vl, b_vector, n);
}

int main() {
    uint32_t a[N] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    uint32_t b[N] = {};
    memset(b, 2, N);
    // void* ret = memcpy_rvv(b, a, N * sizeof(uint32_t));
    memmul_rvv(a, b, N);
    for (int i = 0; i < N; i++) {
        printf("[%d]", b[i]);
    }
    printf("\n");
}