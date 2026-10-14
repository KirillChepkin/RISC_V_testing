#include <stdio.h>
#include <riscv_vector.h>

void *memcpy_rvv(void *restrict destination, const void *restrict source,
    size_t n) {
  unsigned char *dst = destination;
  const unsigned char *src = source;
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

int main() {
    uint32_t a[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    uint32_t b[20] = {};
    void* ret = memcpy_rvv(b, a, 10 * sizeof(uint32_t));
    printf("return value: %x\n", d);
    for (int i = 0; i < 10; i++) {
        printf("[%d]", b[i]);
    }
    printf("\n");
}