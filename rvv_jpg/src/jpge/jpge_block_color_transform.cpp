#include <cstddef>
#include <riscv_vector.h>

#include "jpge_color_transform.h"

// In ideal mathematical representation
// Y = 0.299R + 0.587G + 0.114B
// Cb = 128 - 0.168736R - 0.331264G + 0.5B
// Cr = 128 + 0.5R - 0.418688G - 0.081312B

#define HALF 32768 // half of 65536 (2^16)
#define YR 19595 // 19595/65536 approximately equals 0.299
#define YG 38470 // 38470/65536 approximately equals 0.587
#define YB 7471 // 7471/65536 approximately equals 0.114

#define CB_R -11059 // 11059/65536 approximately equals 0.168736
#define CB_G -21709 // 21709/65536 approximately equals 0.331264
#define CB_B HALF // 32768/65536 equals 0.5

#define CR_R HALF // 32768/65536 equals 0.5
#define CR_G -27439 // 27439/65536 approximately equals 0.418688
#define CR_B -5329 // 5329/65536 approximately equals 0.081312

namespace rvv_jpge {
    volatile unsigned int zero = 0;

    void jpeg_encoder :: convert_RGB_block_to_YCC(
        vuint8m1_t r_group, vuint8m1_t g_group, vuint8m1_t b_group,
        vuint8m1_t *y_group, vuint8m1_t *cb_group, vuint8m1_t *cr_group) {
        
        const int pixels = 32;
        const size_t vl = 8;

        vuint8m1_t y_result = __riscv_vundefined_u8m1();
        vuint8m1_t cb_result = __riscv_vundefined_u8m1();
        vuint8m1_t cr_result = __riscv_vundefined_u8m1();
        
        for (size_t part = 0; part < 4; part++) {
            const size_t offset = part * vl;

            // problem - extra registers are allocated to store sliding result
            // problem reported
            // solution: __riscv_vslideup_vx_i32m1_tu
            vuint8m1_t r_shifted = __riscv_vslidedown_vx_u8m1(r_group, offset, pixels);
            vuint8m1_t g_shifted = __riscv_vslidedown_vx_u8m1(g_group, offset, pixels);
            vuint8m1_t b_shifted = __riscv_vslidedown_vx_u8m1(b_group, offset, pixels);

            // Truncate lmul from 1 to 1/4
            // no problem
            vuint8mf4_t r8mf4 = __riscv_vlmul_trunc_v_u8m1_u8mf4(r_shifted);
            vuint8mf4_t g8mf4 = __riscv_vlmul_trunc_v_u8m1_u8mf4(g_shifted);
            vuint8mf4_t b8mf4 = __riscv_vlmul_trunc_v_u8m1_u8mf4(b_shifted);
            
            // Widen 8-bit values to 16-bit values.
            // problem - widening-with-addition operation forbids reusing the register
            // problem unreported because there is no obvious solution
            vuint16mf2_t r16 = __riscv_vwaddu_vx_u16mf2(r8mf4, zero, vl);
            vuint16mf2_t g16 = __riscv_vwaddu_vx_u16mf2(g8mf4, zero, vl);
            vuint16mf2_t b16 = __riscv_vwaddu_vx_u16mf2(b8mf4, zero, vl);

            // Widen 16-bit values to 32-bit values.
            vuint32m1_t ru = __riscv_vwaddu_vx_u32m1(r16, zero, vl);
            vuint32m1_t gu = __riscv_vwaddu_vx_u32m1(g16, zero, vl);
            vuint32m1_t bu = __riscv_vwaddu_vx_u32m1(b16, zero, vl);

            vint32m1_t r = __riscv_vreinterpret_i32m1(ru);
            vint32m1_t g = __riscv_vreinterpret_i32m1(gu);
            vint32m1_t b = __riscv_vreinterpret_i32m1(bu);

            // calculate Y Cb and Cr
            // Y = 0.299R + 0.587G + 0.114B
            vint32m1_t y = __riscv_vmul_vx_i32m1(r, YR, vl);
            y = __riscv_vmacc_vx_i32m1(y, YG, g, vl);
            y = __riscv_vmacc_vx_i32m1(y, YB, b, vl);

            y = __riscv_vadd_vx_i32m1(y, HALF, vl);
            y = __riscv_vsra_vx_i32m1(y, 16, vl);

            // Cb = 128 - 0.168736R - 0.331264G + 0.5B
            vint32m1_t cb = __riscv_vmul_vx_i32m1(r, CB_R, vl);
            cb = __riscv_vmacc_vx_i32m1(cb, CB_G, g, vl);
            cb = __riscv_vmacc_vx_i32m1(cb, CB_B, b, vl);

            cb = __riscv_vadd_vx_i32m1(cb, HALF, vl);
            cb = __riscv_vsra_vx_i32m1(cb, 16, vl);
            cb = __riscv_vadd_vx_i32m1(cb, 128, vl);

            cb = __riscv_vmax_vx_i32m1(cb, 0, vl);
            cb = __riscv_vmin_vx_i32m1(cb, 255, vl);

            // Cr = 128 + 0.5R - 0.418688G - 0.081312B
            vint32m1_t cr = __riscv_vmul_vx_i32m1(r, CR_R, vl);
            cr = __riscv_vmacc_vx_i32m1(cr, CR_G, g, vl);
            cr = __riscv_vmacc_vx_i32m1(cr, CR_B, b, vl);

            cr = __riscv_vadd_vx_i32m1(cr, HALF, vl);
            cr = __riscv_vsra_vx_i32m1(cr, 16, vl);
            cr = __riscv_vadd_vx_i32m1(cr, 128, vl);

            cr = __riscv_vmax_vx_i32m1(cr, 0, vl);
            cr = __riscv_vmin_vx_i32m1(cr, 255, vl);

            // convert to unsised 8bit int
            // problem - shortening operations take extra registers
            // problem unreported because this registers will likely be the reused registers from widening.
            // registers used for widening are also a problem but as long as it is impossible to solve, this
            // problem is not a problem.
            vuint32m1_t yu = __riscv_vreinterpret_u32m1(y);
            vuint16mf2_t y16 = __riscv_vnclipu_wx_u16mf2(yu, 0, vl);
            vuint8mf4_t y8 = __riscv_vnclipu_wx_u8mf4(y16, 0, vl);

            vuint32m1_t cbu = __riscv_vreinterpret_u32m1(cb);
            vuint16mf2_t cb16 = __riscv_vnclipu_wx_u16mf2(cbu, 0, vl);
            vuint8mf4_t cb8 = __riscv_vnclipu_wx_u8mf4(cb16, 0, vl);

            vuint32m1_t cru = __riscv_vreinterpret_u32m1(cr);
            vuint16mf2_t cr16 = __riscv_vnclipu_wx_u16mf2(cru, 0, vl);
            vuint8mf4_t cr8 = __riscv_vnclipu_wx_u8mf4(cr16, 0, vl);

            // Extend lmul
            vuint8m1_t y8m1 = __riscv_vlmul_ext_v_u8mf4_u8m1(y8);
            vuint8m1_t cb8m1 = __riscv_vlmul_ext_v_u8mf4_u8m1(cb8);
            vuint8m1_t cr8m1 = __riscv_vlmul_ext_v_u8mf4_u8m1(cr8);


            y_result = __riscv_vslideup_vx_u8m1(y_result, y8m1, offset, offset + 8);
            cb_result = __riscv_vslideup_vx_u8m1(cb_result, cb8m1, offset, offset + 8);
            cr_result = __riscv_vslideup_vx_u8m1(cr_result, cr8m1, offset, offset + 8);
        }

        *y_group = y_result;
        *cb_group = cb_result;
        *cr_group = cr_result;
    }

    void jpeg_encoder::convert_RGB_block_to_Y(vuint8m1_t r_group, vuint8m1_t g_group, vuint8m1_t b_group, vuint8m1_t *y_group) {
        int pixels = 32;
        size_t vl = 8;

        vuint8m1_t y_result = __riscv_vundefined_u8m1();
        
        for (size_t part = 0; part < 4; part++) {
            const size_t offset = part * vl;

            vuint8m1_t r_shifted = __riscv_vslidedown_vx_u8m1(r_group, offset, pixels);
            vuint8m1_t g_shifted = __riscv_vslidedown_vx_u8m1(g_group, offset, pixels);
            vuint8m1_t b_shifted = __riscv_vslidedown_vx_u8m1(b_group, offset, pixels);

            // Truncate lmul from 1 to 1/4
            vuint8mf4_t r8mf4 = __riscv_vlmul_trunc_v_u8m1_u8mf4(r_shifted);
            vuint8mf4_t g8mf4 = __riscv_vlmul_trunc_v_u8m1_u8mf4(g_shifted);
            vuint8mf4_t b8mf4 = __riscv_vlmul_trunc_v_u8m1_u8mf4(b_shifted);
            
            // Widen 8-bit values to 16-bit values.
            vuint16mf2_t r16 = __riscv_vwaddu_vx_u16mf2(r8mf4, zero, vl);
            vuint16mf2_t g16 = __riscv_vwaddu_vx_u16mf2(g8mf4, zero, vl);
            vuint16mf2_t b16 = __riscv_vwaddu_vx_u16mf2(b8mf4, zero, vl);

            // Widen 16-bit values to 32-bit values.
            vuint32m1_t ru = __riscv_vwaddu_vx_u32m1(r16, zero, vl);
            vuint32m1_t gu = __riscv_vwaddu_vx_u32m1(g16, zero, vl);
            vuint32m1_t bu = __riscv_vwaddu_vx_u32m1(b16, zero, vl);

            vint32m1_t r = __riscv_vreinterpret_i32m1(ru);
            vint32m1_t g = __riscv_vreinterpret_i32m1(gu);
            vint32m1_t b = __riscv_vreinterpret_i32m1(bu);

            // calculate Y Cb and Cr
            // Y = 0.299R + 0.587G + 0.114B
            vint32m1_t y = __riscv_vmul_vx_i32m1(r, YR, vl);
            y = __riscv_vmacc_vx_i32m1(y, YG, g, vl);
            y = __riscv_vmacc_vx_i32m1(y, YB, b, vl);

            y = __riscv_vadd_vx_i32m1(y, HALF, vl);
            y = __riscv_vsra_vx_i32m1(y, 16, vl);

            // convert to unsised 8bit int
            vuint32m1_t yu = __riscv_vreinterpret_u32m1(y);
            vuint16mf2_t y16 = __riscv_vnclipu_wx_u16mf2(yu, 0, vl);
            vuint8mf4_t y8 = __riscv_vnclipu_wx_u8mf4(y16, 0, vl);

            // Extand lmul
            vuint8m1_t y8m1 = __riscv_vlmul_ext_v_u8mf4_u8m1(y8);
            
            y_result = __riscv_vslideup_vx_u8m1(y_result, y8m1, offset, offset + 8);
        }

        *y_group = y_result;
    }

    void jpeg_encoder::convert_Y_block_to_YCC(vuint8m1_t y_in_group, vuint8m1_t *y_out_group, vuint8m1_t *cb_group, vuint8m1_t *cr_group) {
        size_t vl = 32;
        
        vuint8m1_t v = __riscv_vmv_v_x_u8m1(128, vl);

        *y_out_group = y_in_group;
        *cb_group =v;
        *cr_group = v;
    }
}
