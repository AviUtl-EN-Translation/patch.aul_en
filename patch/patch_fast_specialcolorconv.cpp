/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "patch_fast_specialcolorconv.hpp"
#include <immintrin.h>
#ifdef PATCH_SWITCH_FAST_SPECIALCOLORCONV


//#define PATCH_STOPWATCH

namespace patch::fast {

    void __cdecl SpecialColorConv_t::mt_border(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) { // 15760
        short* buf_alpha0 = *reinterpret_cast<short**>(GLOBAL::exedit_base + 0x11ed00);
        auto color = (ExEdit::PixelYCA*)efp->exdata_ptr;
        float col_angle1 = atan2((float)color[0].cr, (float)color[0].cb) * -10430.37835047045f;
        int sat_col1 = max(1, max(abs(color[0].cb), abs(color[0].cr)));
        int hue_range = efp->track[0] << 7;
        int sat_range = efp->track[1] * sat_col1 >> 8;

        __m256 col_angle256 = _mm256_set1_ps(col_angle1);
        __m256 divPI256 = _mm256_set1_ps(10430.37835047045f);
        __m256i sat_col256 = _mm256_set1_epi32(sat_col1);
        __m256i hue_range256 = _mm256_set1_epi32(hue_range);
        __m256i sat_range256 = _mm256_set1_epi32(sat_range);
        __m256i offset256 = _mm256_set_epi32(56, 48, 40, 32, 24, 16, 8, 0);
        __m256i zero256 = _mm256_setzero_si256();
        __m256i max256 = _mm256_set1_epi32(0x1000);

        int y = efpip->obj_h * thread_id / thread_num;
        int offset = efpip->obj_line * y;
        for (y = efpip->obj_h * (thread_id + 1) / thread_num - y; 0 < y; y--) {
            short* cbcr = &((ExEdit::PixelYCA*)efpip->obj_edit + offset)->cb;
            short* buf_alpha = buf_alpha0 + offset;
            offset += efpip->obj_line;

            for (int x = efpip->obj_w >> 3; 0 < x; x--) {
                __m256i cbcr256 = _mm256_i32gather_epi32((int*)cbcr, offset256, 1);
                cbcr += 32;
                __m256i cb256 = _mm256_srai_epi32(_mm256_slli_epi32(cbcr256, 16), 16);
                __m256i cr256 = _mm256_srai_epi32(cbcr256, 16);
                __m256 angle256 = _mm256_atan2_ps(_mm256_cvtepi32_ps(cr256), _mm256_cvtepi32_ps(cb256));
                angle256 = _mm256_fmadd_ps(angle256, divPI256, col_angle256);
                __m256i sub_hue256 = _mm256_abs_epi32(_mm256_srai_epi32(_mm256_slli_epi32(_mm256_cvtps_epi32(angle256), 16), 16));
                sub_hue256 = _mm256_max_epi32(zero256, _mm256_sub_epi32(sub_hue256, hue_range256));
                __m256i sub_col256 = _mm256_sub_epi32(sat_col256, _mm256_max_epi32(_mm256_abs_epi32(cb256), _mm256_abs_epi32(cr256)));
                __m256i sub_sat256 = _mm256_max_epi32(zero256, _mm256_sub_epi32(_mm256_abs_epi32(sub_col256), sat_range256));
                sub_hue256 = _mm256_min_epi32(_mm256_add_epi32(sub_hue256, _mm256_slli_epi32(sub_sat256, 3)), max256);
                for (int i = 0; i < 16; i += 2) {
                    *buf_alpha = sub_hue256.m256i_i16[i];
                    buf_alpha++;
                }
            }
            for (int x = efpip->obj_w & 7; 0 < x; x--) {
                int sub_hue = max(0, abs((short)round(fmaf(atan2((float)cbcr[1], (float)cbcr[0]), 10430.37835047045f, col_angle1))) - hue_range);
                cbcr += 4;
                int sub_col = sat_col1 - max(abs(cbcr[0]), abs(cbcr[1]));
                *buf_alpha = (short)min(sub_hue + (max(0, abs(sub_col) - sat_range) << 3), 0x1000);
                buf_alpha++;
            }
        }
    }

    void __cdecl SpecialColorConv_t::mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) { // 15540
        auto color = (ExEdit::PixelYCA*)efp->exdata_ptr;
        float col_angle1 = atan2((float)color[0].cr, (float)color[0].cb) * -10430.37835047045f;
        int sat_col1 = max(1, max(abs(color[0].cb), abs(color[0].cr)));
        int hue_range = efp->track[0] << 7;
        int sat_range = efp->track[1] * sat_col1 >> 8;

        __m256 col_angle256 = _mm256_set1_ps(col_angle1);
        __m256 divPI256 = _mm256_set1_ps(10430.37835047045f);
        __m256i sat_col256 = _mm256_set1_epi32(sat_col1);
        __m256i hue_range256 = _mm256_set1_epi32(hue_range);
        __m256i sat_range256 = _mm256_set1_epi32(sat_range);
        __m256i offset256 = _mm256_set_epi32(56, 48, 40, 32, 24, 16, 8, 0);
        __m256i zero256 = _mm256_setzero_si256();

        int y = efpip->obj_h * thread_id / thread_num;
        int offset = efpip->obj_line * y;
        for (y = efpip->obj_h * (thread_id + 1) / thread_num - y; 0 < y; y--) {
            auto pix = (ExEdit::PixelYCA*)efpip->obj_edit + offset;
            offset += efpip->obj_line;
            for (int x = efpip->obj_w >> 3; 0 < x; x--) {
                __m256i cbcr256 = _mm256_i32gather_epi32((int*)&pix->cb, offset256, 1);
                __m256i cb256 = _mm256_srai_epi32(_mm256_slli_epi32(cbcr256, 16), 16);
                __m256i cr256 = _mm256_srai_epi32(cbcr256, 16);
                __m256 angle256 = _mm256_atan2_ps(_mm256_cvtepi32_ps(cr256), _mm256_cvtepi32_ps(cb256));
                angle256 = _mm256_fmadd_ps(angle256, divPI256, col_angle256);
                __m256i sub_hue256 = _mm256_abs_epi32(_mm256_srai_epi32(_mm256_slli_epi32(_mm256_cvtps_epi32(angle256), 16), 16));
                sub_hue256 = _mm256_max_epi32(zero256, _mm256_sub_epi32(sub_hue256, hue_range256));
                __m256i sub_col256 = _mm256_sub_epi32(sat_col256, _mm256_max_epi32(_mm256_abs_epi32(cb256), _mm256_abs_epi32(cr256)));
                __m256i sub_sat256 = _mm256_max_epi32(zero256, _mm256_sub_epi32(_mm256_abs_epi32(sub_col256), sat_range256));
                sub_hue256 = _mm256_add_epi32(sub_hue256, _mm256_slli_epi32(sub_sat256, 3));

                for (int i = 0; i < 8; i++) {
                    int sub_hue = sub_hue256.m256i_i32[i];
                    if (sub_hue < 0x1000) {
                        pix->cb = (short)((color[1].cb * (0x1000 - sub_hue) + pix->cb * sub_hue) >> 12);
                        pix->cr = (short)((color[1].cr * (0x1000 - sub_hue) + pix->cr * sub_hue) >> 12);
                        sub_hue = max(sub_hue, sub_col256.m256i_i32[i] * 0x1000 / sat_col1);
                        int col_y = color[1].y;
                        int pix_y = max(0, pix->y); // patch_obj_specialcolornv
                        if (pix_y < color[0].y) {
                            col_y = col_y * pix_y / color[0].y;
                        }
                        pix->y = (short)(((0x1000 - sub_hue) * col_y + pix_y * sub_hue) >> 12);
                    }
                    pix++;
                }
            }
            for (int x = efpip->obj_w & 7; 0 < x; x--) {
                int sub_hue = max(0, abs((short)round(fmaf(atan2((float)pix->cr, (float)pix->cb), 10430.37835047045f, col_angle1))) - hue_range);
                int sub_col = sat_col1 - max(abs(pix->cb), abs(pix->cr));
                sub_hue += max(0, abs(sub_col) - sat_range) << 3;
                if (sub_hue < 0x1000) {
                    pix->cb = (short)((color[1].cb * (0x1000 - sub_hue) + pix->cb * sub_hue) >> 12);
                    pix->cr = (short)((color[1].cr * (0x1000 - sub_hue) + pix->cr * sub_hue) >> 12);
                    sub_hue = max(sub_hue, sub_col * 0x1000 / sat_col1);
                    int col_y = color[1].y;
                    int pix_y = max(0, pix->y); // patch_obj_specialcolornv
                    if (pix_y < color[0].y) {
                        col_y = col_y * pix_y / color[0].y;
                    }
                    pix->y = (short)(((0x1000 - sub_hue) * col_y + pix_y * sub_hue) >> 12);
                }
                pix++;
            }
        }
    }

} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_SPECIALCOLORCONV
