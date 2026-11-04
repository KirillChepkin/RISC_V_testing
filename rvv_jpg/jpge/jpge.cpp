#include <riscv_vector.h>
#include "jpge.h"

namespace rvv_jpge {
    void rvv_jpeg_encoder::compute_quant_table(int32_t *pDst, int32_t *pSrc, int32_t quality) {
        int32_t q;
        if (quality < 50) {
            q = 5000 / quality;
        }
        else {
            q = 200 - quality * 2;
        }
        int32_t n = 64;
        while (n > 0) {
            int32_t vl = __riscv_vsetvl_e32m8(n);
            vint32m8_t vector = __riscv_vle32_v_i32m8(pSrc, vl);
            vector = __riscv_vmul_vx_i32m8(vector, q, vl);
            vector = __riscv_vadd_vx_i32m8(vector, 50, vl);
            vector = __riscv_vdiv_vx_i32m8(vector, 100, vl);
            vector = __riscv_vmax_vx_i32m8(vector, 1, vl);
            vector = __riscv_vmin_vx_i32m8(vector, 255, vl);
            __riscv_vse32_v_i32m8(pDst, vector, vl);
            n -= vl;
            pSrc += vl;
            pDst += vl;
        }
    }
}