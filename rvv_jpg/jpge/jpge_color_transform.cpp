#if __riscv_v_intrinsic >= 1000000
#include <riscv_vector.h>
#endif /* __riscv_v_intric*/

#include "jpge_color_transform.h"

// In ideal mathematical representation
// Y = 0.299R + 0.587G + 0.114B
// Cb = 128 - 0.168736R - 0.331264G + 0.5B
// Cr = 128 + 0.5R - 0.418688G - 0.081312B

#define HALF 32768 // half of 65536 (2^16)
#define YR 19595 // 19595/65536 approximately equals 0.587
#define YG 38470 // 38470/65536 approximately equals 0.299
#define YB 7471 // 7471/65536 approximately equals 0.114

#define CB_R -11059 // 11059/65536 approximately equals 0.168736
#define CB_G -21709 // 21709/65536 approximately equals 0.331264
#define CB_B HALF // 32768/65536 equals 0.5

#define CR_R HALF // 32768/65536 equals 0.5
#define CR_G -27439 // 27439/65536 approximately equals 0.418688
#define CR_B -5329 // 5329/65536 approximately equals 0.081312

namespace rvv_jpge {

    void RGB_to_YCC(uint8* pDst, const uint8 *pSrc, int num_pixels) {
        while (num_pixels > 0) {
            size_t vl = __riscv_vsetvl_e8m1(num_pixels);

            // put rgb in vector
            vuint8m1_t r8 = __riscv_vlse8_v_u8m1(pSrc, 3, vl);
            vuint8m1_t g8 = __riscv_vlse8_v_u8m1(pSrc + 1, 3, vl);
            vuint8m1_t b8 = __riscv_vlse8_v_u8m1(pSrc + 2, 3, vl);

            vuint32m4_t ru = __riscv_vzext_vf4_u32m4(r8, vl);
            vuint32m4_t gu = __riscv_vzext_vf4_u32m4(g8, vl);
            vuint32m4_t bu = __riscv_vzext_vf4_u32m4(b8, vl);

            vint32m4_t r = __riscv_vreinterpret_i32m4(ru);
            vint32m4_t g = __riscv_vreinterpret_i32m4(gu);
            vint32m4_t b = __riscv_vreinterpret_i32m4(bu);

            // calculate Y Cb and Cr
            // Y = 0.299R + 0.587G + 0.114B
            vint32m4_t y = __riscv_vmul_vx_i32m4(r, YR, vl);
            y = __riscv_vmacc_vx_i32m4(y, YG, g, vl);
            y = __riscv_vmacc_vx_i32m4(y, YB, b, vl);

            y = __riscv_vadd_vx_i32m4(y, HALF, vl);
            y = __riscv_vsra_vx_i32m4(y, 16, vl);

            // Cb = 128 - 0.168736R - 0.331264G + 0.5B
            vint32m4_t cb = __riscv_vmul_vx_i32m4(r, CB_R, vl);
            cb = __riscv_vmacc_vx_i32m4(cb, CB_G, g, vl);
            cb = __riscv_vmacc_vx_i32m4(cb, CB_B, b, vl);

            cb = __riscv_vadd_vx_i32m4(cb, HALF, vl);
            cb = __riscv_vsra_vx_i32m4(cb, 16, vl);
            cb = __riscv_vadd_vx_i32m4(cb, 128, vl);

            cb = __riscv_vmax_vx_i32m4(cb, 0, vl);
            cb = __riscv_vmin_vx_i32m4(cb, 255, vl);

            // Cr = 128 + 0.5R - 0.418688G - 0.081312B
            vint32m4_t cr = __riscv_vmul_vx_i32m4(r, CR_R, vl);
            cr = __riscv_vmacc_vx_i32m4(cr, CR_G, g, vl);
            cr = __riscv_vmacc_vx_i32m4(cr, CR_B, b, vl);

            cr = __riscv_vadd_vx_i32m4(cr, HALF, vl);
            cr = __riscv_vsra_vx_i32m4(cr, 16, vl);
            cr = __riscv_vadd_vx_i32m4(cr, 128, vl);

            cr = __riscv_vmax_vx_i32m4(cr, 0, vl);
            cr = __riscv_vmin_vx_i32m4(cr, 255, vl);

            // convert to unsised 8bit int
            vuint32m4_t yu = __riscv_vreinterpret_u32m4(y);
            vuint16m2_t y16 = __riscv_vnclipu_wx_u16m2(yu, 0, __RISCV_VXRM_RNU, vl);
            vuint8m1_t y8 = __riscv_vnclipu_wx_u8m1(y16, 0, __RISCV_VXRM_RNU, vl);

            vuint32m4_t cbu = __riscv_vreinterpret_u32m4(cb);
            vuint16m2_t cb16 = __riscv_vnclipu_wx_u16m2(cbu, 0, __RISCV_VXRM_RNU, vl);
            vuint8m1_t cb8 = __riscv_vnclipu_wx_u8m1(cb16, 0, __RISCV_VXRM_RNU, vl);

            vuint32m4_t cru = __riscv_vreinterpret_u32m4(cr);
            vuint16m2_t cr16 = __riscv_vnclipu_wx_u16m2(cru, 0, __RISCV_VXRM_RNU, vl);
            vuint8m1_t cr8 = __riscv_vnclipu_wx_u8m1(cr16, 0, __RISCV_VXRM_RNU, vl);

            // store from vector to distination array
            vuint8m1x3_t ycbcr = __riscv_vcreate_v_u8m1x3(y8, cb8, cr8);
            __riscv_vsseg3e8_v_u8m1x3(pDst, ycbcr, vl);

            pSrc += 3 * vl;
            pDst += 3 * vl;
            num_pixels -= vl;
        }
    }

    void RGB_to_Y(uint8* pDst, const uint8 *pSrc, int num_pixels) { 
        while (num_pixels > 0) {
            size_t vl = __riscv_vsetvl_e8m1(num_pixels);

            // put rgb in vector
            vuint8m1_t r8 = __riscv_vlse8_v_u8m1(pSrc, 3, vl);
            vuint8m1_t g8 = __riscv_vlse8_v_u8m1(pSrc + 1, 3, vl);
            vuint8m1_t b8 = __riscv_vlse8_v_u8m1(pSrc + 2, 3, vl);

            vuint32m4_t ru = __riscv_vzext_vf4_u32m4(r8, vl);
            vuint32m4_t gu = __riscv_vzext_vf4_u32m4(g8, vl);
            vuint32m4_t bu = __riscv_vzext_vf4_u32m4(b8, vl);

            vint32m4_t r = __riscv_vreinterpret_i32m4(ru);
            vint32m4_t g = __riscv_vreinterpret_i32m4(gu);
            vint32m4_t b = __riscv_vreinterpret_i32m4(bu);

            // calculate Y
            // Y = 0.299R + 0.587G + 0.114B
            vint32m4_t y = __riscv_vmul_vx_i32m4(r, YR, vl);
            y = __riscv_vmacc_vx_i32m4(y, YG, g, vl);
            y = __riscv_vmacc_vx_i32m4(y, YB, b, vl);

            y = __riscv_vadd_vx_i32m4(y, HALF, vl);
            y = __riscv_vsra_vx_i32m4(y, 16, vl);

            // convert to unsised 8bit int
            vuint32m4_t yu = __riscv_vreinterpret_u32m4(y);
            vuint16m2_t y16 = __riscv_vnclipu_wx_u16m2(yu, 0, __RISCV_VXRM_RNU, vl);
            vuint8m1_t y8 = __riscv_vnclipu_wx_u8m1(y16, 0, __RISCV_VXRM_RNU, vl);
            
            // store from vector to distination array
            __riscv_vsse8_v_u8m1(pDst, 3, y8, vl);

            pDst += vl;
            pSrc += 3 * vl;
            num_pixels -= vl;
        }
    }

    void RGBA_to_YCC(uint8* pDst, const uint8 *pSrc, int num_pixels) {
        while (num_pixels > 0) {
            size_t vl = __riscv_vsetvl_e8m1(num_pixels);

            // put rgb in vector
            vuint8m1_t r8 = __riscv_vlse8_v_u8m1(pSrc, 3, vl);
            vuint8m1_t g8 = __riscv_vlse8_v_u8m1(pSrc + 1, 3, vl);
            vuint8m1_t b8 = __riscv_vlse8_v_u8m1(pSrc + 2, 3, vl);

            vuint32m4_t ru = __riscv_vzext_vf4_u32m4(r8, vl);
            vuint32m4_t gu = __riscv_vzext_vf4_u32m4(g8, vl);
            vuint32m4_t bu = __riscv_vzext_vf4_u32m4(b8, vl);

            vint32m4_t r = __riscv_vreinterpret_i32m4(ru);
            vint32m4_t g = __riscv_vreinterpret_i32m4(gu);
            vint32m4_t b = __riscv_vreinterpret_i32m4(bu);


            // calculate Y Cb and Cr
            // Y = 0.299R + 0.587G + 0.114B
            vint32m4_t y = __riscv_vmul_vx_i32m4(r, YR, vl);
            y = __riscv_vmacc_vx_i32m4(y, YG, g, vl);
            y = __riscv_vmacc_vx_i32m4(y, YB, b, vl);

            y = __riscv_vadd_vx_i32m4(y, HALF, vl);
            y = __riscv_vsra_vx_i32m4(y, 16, vl);

            // Cb = 128 - 0.168736R - 0.331264G + 0.5B
            vint32m4_t cb = __riscv_vmul_vx_i32m4(r, CB_R, vl);
            cb = __riscv_vmacc_vx_i32m4(cb, CB_G, g, vl);
            cb = __riscv_vmacc_vx_i32m4(cb, CB_B, b, vl);

            cb = __riscv_vadd_vx_i32m4(cb, HALF, vl);
            cb = __riscv_vsra_vx_i32m4(cb, 16, vl);
            cb = __riscv_vadd_vx_i32m4(cb, 128, vl);

            cb = __riscv_vmax_vx_i32m4(cb, 0, vl);
            cb = __riscv_vmin_vx_i32m4(cb, 255, vl);

            // Cr = 128 + 0.5R - 0.418688G - 0.081312B
            vint32m4_t cr = __riscv_vmul_vx_i32m4(r, CR_R, vl);
            cr = __riscv_vmacc_vx_i32m4(cr, CR_G, g, vl);
            cr = __riscv_vmacc_vx_i32m4(cr, CR_B, b, vl);

            cr = __riscv_vadd_vx_i32m4(cr, HALF, vl);
            cr = __riscv_vsra_vx_i32m4(cr, 16, vl);
            cr = __riscv_vadd_vx_i32m4(cr, 128, vl);

            cr = __riscv_vmax_vx_i32m4(cr, 0, vl);
            cr = __riscv_vmin_vx_i32m4(cr, 255, vl);

            // convert to unsised 8bit int
            vuint32m4_t yu = __riscv_vreinterpret_u32m4(y);
            vuint16m2_t y16 = __riscv_vnclipu_wx_u16m2(yu, 0, __RISCV_VXRM_RNU, vl);
            vuint8m1_t y8 = __riscv_vnclipu_wx_u8m1(y16, 0, __RISCV_VXRM_RNU, vl);

            vuint32m4_t cbu = __riscv_vreinterpret_u32m4(cb);
            vuint16m2_t cb16 = __riscv_vnclipu_wx_u16m2(cbu, 0, __RISCV_VXRM_RNU, vl);
            vuint8m1_t cb8 = __riscv_vnclipu_wx_u8m1(cb16, 0, __RISCV_VXRM_RNU, vl);

            vuint32m4_t cru = __riscv_vreinterpret_u32m4(cr);
            vuint16m2_t cr16 = __riscv_vnclipu_wx_u16m2(cru, 0, __RISCV_VXRM_RNU, vl);
            vuint8m1_t cr8 = __riscv_vnclipu_wx_u8m1(cr16, 0, __RISCV_VXRM_RNU, vl);

            // store from vector to distination array
            vuint8m1x3_t ycbcr = __riscv_vcreate_v_u8m1x3(y8, cb8, cr8);
            __riscv_vsseg3e8_v_u8m1x3(pDst, ycbcr, vl);

            pSrc += 4 * vl;
            pDst += 3 * vl;
            num_pixels -= vl;
        }
    }

    void RGBA_to_Y(uint8* pDst, const uint8 *pSrc, int num_pixels) {
        while (num_pixels > 0) {
            size_t vl = __riscv_vsetvl_e8m1(num_pixels);

            // put rgb in vector
            vuint8m1_t r8 = __riscv_vlse8_v_u8m1(pSrc, 3, vl);
            vuint8m1_t g8 = __riscv_vlse8_v_u8m1(pSrc + 1, 3, vl);
            vuint8m1_t b8 = __riscv_vlse8_v_u8m1(pSrc + 2, 3, vl);

            vuint32m4_t ru = __riscv_vzext_vf4_u32m4(r8, vl);
            vuint32m4_t gu = __riscv_vzext_vf4_u32m4(g8, vl);
            vuint32m4_t bu = __riscv_vzext_vf4_u32m4(b8, vl);

            vint32m4_t r = __riscv_vreinterpret_i32m4(ru);
            vint32m4_t g = __riscv_vreinterpret_i32m4(gu);
            vint32m4_t b = __riscv_vreinterpret_i32m4(bu);

            // calculate Y
            // Y = 0.299R + 0.587G + 0.114B
            vint32m4_t y = __riscv_vmul_vx_i32m4(r, YR, vl);
            y = __riscv_vmacc_vx_i32m4(y, YG, g, vl);
            y = __riscv_vmacc_vx_i32m4(y, YB, b, vl);

            y = __riscv_vadd_vx_i32m4(y, HALF, vl);
            y = __riscv_vsra_vx_i32m4(y, 16, vl);

            // convert to unsised 8bit int
            vuint32m4_t yu = __riscv_vreinterpret_u32m4(y);
            vuint16m2_t y16 = __riscv_vnclipu_wx_u16m2(yu, 0, __RISCV_VXRM_RNU, vl);
            vuint8m1_t y8 = __riscv_vnclipu_wx_u8m1(y16, 0, __RISCV_VXRM_RNU, vl);
            
            // store from vector to distination array
            __riscv_vsse8_v_u8m1(pDst, 3, y8, vl);

            pDst += vl;
            pSrc += 4 * vl;
            num_pixels -= vl;
        }
    }

    void Y_to_YCC(uint8* pDst, const uint8* pSrc, int num_pixels) {
        while (num_pixels > 0) {
            size_t vl = __riscv_vsetvl_e8m1(num_pixels);

            // put Y Cb and Cr in vector
            vuint8m1_t y = __riscv_vlse8_v_u8m1(pSrc, 1, vl);
            vuint8m1_t v = __riscv_vmv_v_x_u8m1(128, vl);

            // store from vector to distination array
            vuint8m1x3_t ycbcr = __riscv_vcreate_v_u8m1x3(y, v, v);
            __riscv_vsseg3e8_v_u8m1x3(pDst, ycbcr, vl);
            
            pDst += 3 * vl;
            pSrc += vl;
            num_pixels -= vl;
        }
    }
}
