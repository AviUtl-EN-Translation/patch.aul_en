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

    void __cdecl SpecialColorConv_t::mt_avx2(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) { // 15540
        /*
        if (rand() & 1) {
            reinterpret_cast<void(__cdecl*)(int, int, ExEdit::Filter*, ExEdit::FilterProcInfo*)>(GLOBAL::exedit_base + 0x15540)(thread_id, thread_num, efp, efpip);
            return;
        }
        */
        
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
                __m256i ma_col256 = _mm256_max_epi32(_mm256_abs_epi32(cb256), _mm256_abs_epi32(cr256));
                __m256i sub_sat256 = _mm256_max_epi32(zero256, _mm256_sub_epi32(_mm256_abs_epi32(_mm256_sub_epi32(sat_col256, ma_col256)), sat_range256));
                sub_hue256 = _mm256_sub_epi32(max256, _mm256_add_epi32(sub_hue256, _mm256_slli_epi32(sub_sat256, 3)));
                ma_col256 = _mm256_min_epi32(sub_hue256, _mm256_div_epi32(_mm256_slli_epi32(ma_col256, 12), sat_col256));

                for (int i = 0; i < 8; i++) {
                    int sub_hue = sub_hue256.m256i_i32[i];
                    if (0 < sub_hue) {
                        pix->cb += (color[1].cb - pix->cb) * sub_hue >> 12;
                        pix->cr += (color[1].cr - pix->cr) * sub_hue >> 12;
                        int col_y = color[1].y;
                        int pix_y = max(0, pix->y); // patch_obj_specialcolornv
                        if (pix_y < color[0].y) {
                            col_y = col_y * pix_y / color[0].y;
                        }
                        pix->y = pix_y + ((col_y - pix_y) * ma_col256.m256i_i32[i] >> 12);
                    }
                    pix++;
                }
            }
            for (int x = efpip->obj_w & 7; 0 < x; x--) {
                int sub_hue = max(0, abs((short)round(fmaf(atan2((float)pix->cr, (float)pix->cb), 10430.37835047045f, col_angle1))) - hue_range);
                int ma_col = max(abs(pix->cb), abs(pix->cr));
                sub_hue += max(0, abs(sat_col1 - ma_col) - sat_range) << 3;
                sub_hue = 0x1000 - sub_hue;
                if (0 < sub_hue) {
                    pix->cb += (color[1].cb - pix->cb) * sub_hue >> 12;
                    pix->cr += (color[1].cr - pix->cr) * sub_hue >> 12;
                    int col_y = color[1].y;
                    int pix_y = max(0, pix->y); // patch_obj_specialcolornv
                    if (pix_y < color[0].y) {
                        col_y = col_y * pix_y / color[0].y;
                    }
                    sub_hue = min(sub_hue, (ma_col << 12) / sat_col1);
                    pix->y = pix_y + ((col_y - pix_y) * sub_hue >> 12);
                }
                pix++;
            }

        }
    }

    void __cdecl SpecialColorConv_t::mt_border(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) { // 15760
        auto scc = reinterpret_cast<efSpecialColorConv_var*>(GLOBAL::exedit_base + OFS::ExEdit::efSpecialColorConv_var_ptr);
        auto color = (ExEdit::PixelYCA*)efp->exdata_ptr;
        float col_angle1 = atan2((float)color[0].cr, (float)color[0].cb) * -10430.37835047045f;
        int sat_col1 = max(1, max(abs(color[0].cb), abs(color[0].cr)));
        int hue_range = efp->track[0] << 7;
        int sat_range = efp->track[1] * sat_col1 >> 8;

        int y = efpip->obj_h * thread_id / thread_num;
        short* cbcr = &((ExEdit::PixelYCA*)efpip->obj_edit + efpip->obj_line * y)->cb;
        int next = (efpip->obj_line - efpip->obj_w) * 4;
        auto temp = (int*)scc->temp2 + efpip->obj_w * y;
        if (has_flag(get_CPUCmdSet(), CPUCmdSet::F_AVX2)) {
            __m256 col_angle256 = _mm256_set1_ps(col_angle1);
            __m256 divPI256 = _mm256_set1_ps(10430.37835047045f);
            __m256i sat_col256 = _mm256_set1_epi32(sat_col1);
            __m256i hue_range256 = _mm256_set1_epi32(hue_range);
            __m256i sat_range256 = _mm256_set1_epi32(sat_range);
            __m256i offset256 = _mm256_set_epi32(56, 48, 40, 32, 24, 16, 8, 0);
            __m256i zero256 = _mm256_setzero_si256();
            __m256i max256 = _mm256_set1_epi32(0x1000);
            for (y = efpip->obj_h * (thread_id + 1) / thread_num - y; 0 < y; y--) {
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
                    _mm256_storeu_si256((__m256i*)temp, sub_hue256);
                    temp += 8;
                }
                for (int x = efpip->obj_w & 7; 0 < x; x--) {
                    int sub_hue = max(0, abs((short)round(fmaf(atan2((float)cbcr[1], (float)cbcr[0]), 10430.37835047045f, col_angle1))) - hue_range);
                    cbcr += 4;
                    int sub_col = sat_col1 - max(abs(cbcr[0]), abs(cbcr[1]));
                    *temp = min(sub_hue + (max(0, abs(sub_col) - sat_range) << 3), 0x1000);
                    temp++;
                }
                cbcr += next;
            }
        } else {
            for (y = efpip->obj_h * (thread_id + 1) / thread_num - y; 0 < y; y--) {
                for (int x = efpip->obj_w; 0 < x; x--) {
                    int sub_hue = max(0, abs((short)round(fmaf(atan2((float)cbcr[1], (float)cbcr[0]), 10430.37835047045f, col_angle1))) - hue_range);
                    cbcr += 4;
                    int sub_col = sat_col1 - max(abs(cbcr[0]), abs(cbcr[1]));
                    *temp = min(sub_hue + (max(0, abs(sub_col) - sat_range) << 3), 0x1000);
                    temp++;
                }
                cbcr += next;
            }
        }
    }

    void __cdecl SpecialColorConv_t::mt_blur1(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) { // 15930
        auto scc = reinterpret_cast<efSpecialColorConv_var*>(GLOBAL::exedit_base + OFS::ExEdit::efSpecialColorConv_var_ptr);
        int x = thread_id * efpip->obj_w / thread_num;
        auto src1 = (int*)scc->temp2 + x;
        auto src2 = src1;
        auto dst = (int*)scc->temp1 + x;
        auto cnv = dst - efpip->obj_w;
        int cnv_w = (thread_id + 1) * efpip->obj_w / thread_num - x;
        int line = (efpip->obj_w - cnv_w) * sizeof(*cnv);
        memset(cnv, 0, cnv_w * sizeof(*cnv));
        for (int y = scc->size; 0 < y; y--) {
            for (int x = cnv_w; 0 < x; x--) {
                *cnv += *src1;
                cnv++; src1++;
            }
            cnv -= cnv_w;
            src1 = (int*)((int)src1 + line);
        }
        for (int y = scc->range - scc->size; 0 < y; y--) {
            for (int x = cnv_w; 0 < x; x--) {
                *dst = *cnv + *src1;
                dst++; cnv++; src1++;
            }
            dst = (int*)((int)dst + line);
            cnv = (int*)((int)cnv + line);
            src1 = (int*)((int)src1 + line);
        }
        for (int y = efpip->obj_h - scc->range; 0 < y; y--) {
            for (int x = cnv_w; 0 < x; x--) {
                *dst = *cnv - *src2 + *src1;
                dst++; cnv++; src1++; src2++;
            }
            dst = (int*)((int)dst + line);
            cnv = (int*)((int)cnv + line);
            src1 = (int*)((int)src1 + line);
            src2 = (int*)((int)src2 + line);
        }
        for (int y = scc->size; 0 < y; y--) {
            for (int x = cnv_w; 0 < x; x--) {
                *dst = *cnv - *src2;
                dst++; cnv++; src2++;
            }
            dst = (int*)((int)dst + line);
            cnv = (int*)((int)cnv + line);
            src2 = (int*)((int)src2 + line);
        }
    }

    void SpecialColorConv_t::mt_blur2(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) { // 15ad0
        auto scc = reinterpret_cast<efSpecialColorConv_var*>(GLOBAL::exedit_base + OFS::ExEdit::efSpecialColorConv_var_ptr);

        int oa = (1 - scc->size) << 12;
        int range = scc->range * scc->range;
        int thres = (range << 12) - (range << 12) / scc->size;

        auto dst = (ExEdit::PixelYCA*)efpip->obj_edit + thread_id * efpip->obj_line;
        auto src1 = (int*)scc->temp2 + thread_id * efpip->obj_w;
        auto src2 = (int*)scc->temp1 + thread_id * efpip->obj_w;

        int line = (efpip->obj_line * thread_num - efpip->obj_w) * sizeof(*dst);
        int addw = efpip->obj_w * (thread_num - 1) * sizeof(*src1);

        auto color = (ExEdit::PixelYCA*)efp->exdata_ptr;
        int sat_col1 = max(1, max(abs(color[0].cb), abs(color[0].cr)));
        for (int y = (efpip->obj_h - 1 - thread_id) / thread_num + 1; 0 < y; y--) {
            auto src3 = src2;
            int cnv = 0;
            for (int x = scc->size; 0 < x; x--) {
                cnv += *src2;
                src2++;
            }
            for (int x = scc->range - scc->size; 0 < x; x--) {
                cnv += *src2;
                int a = 0x1000;
                if (thres < cnv) {
                    int b = (cnv / range * *src1 >> 12) * scc->size + oa;
                    if (0 < b) {
                        a -= dst->a * b >> 12;
                    }
                }
                if (0 < a) {
                    int b = min(a, (max(abs(dst->cb), abs(dst->cr)) << 12) / sat_col1);
                    dst->cb += (color[1].cb - dst->cb) * a >> 12;
                    dst->cr += (color[1].cr - dst->cr) * a >> 12;
                    int cvt_y = color[1].y;
                    int dst_y = max(0, dst->y); // patch_obj_specialcolornv
                    if (dst_y < color[0].y) {
                        cvt_y = cvt_y * dst_y / color[0].y;
                    }
                    dst->y = dst_y + ((cvt_y - dst_y) * b >> 12);
                }
                dst++; src1++; src2++;
            }
            for (int x = efpip->obj_w - scc->range; 0 < x; x--) {
                cnv += *src2 - *src3;
                int a = 0x1000;
                if (thres < cnv) {
                    int b = (cnv / range * *src1 >> 12) * scc->size + oa;
                    if (0 < b) {
                        a -= dst->a * b >> 12;
                    }
                }
                if (0 < a) {
                    int b = min(a, (max(abs(dst->cb), abs(dst->cr)) << 12) / sat_col1);
                    dst->cb += (color[1].cb - dst->cb) * a >> 12;
                    dst->cr += (color[1].cr - dst->cr) * a >> 12;
                    int cvt_y = color[1].y;
                    int dst_y = max(0, dst->y); // patch_obj_specialcolornv
                    if (dst_y < color[0].y) {
                        cvt_y = cvt_y * dst_y / color[0].y;
                    }
                    dst->y = dst_y + ((cvt_y - dst_y) * b >> 12);
                }
                dst++; src1++; src2++; src3++;
            }
            for (int x = scc->size; 0 < x; x--) {
                cnv -= *src3;
                int a = 0x1000;
                if (thres < cnv) {
                    int b = (cnv / range * *src1 >> 12) * scc->size + oa;
                    if (0 < b) {
                        a -= dst->a * b >> 12;
                    }
                }
                if (0 < a) {
                    int b = min(a, (max(abs(dst->cb), abs(dst->cr)) << 12) / sat_col1);
                    dst->cb += (color[1].cb - dst->cb) * a >> 12;
                    dst->cr += (color[1].cr - dst->cr) * a >> 12;
                    int cvt_y = color[1].y;
                    int dst_y = max(0, dst->y); // patch_obj_specialcolornv
                    if (dst_y < color[0].y) {
                        cvt_y = cvt_y * dst_y / color[0].y;
                    }
                    dst->y = dst_y + ((cvt_y - dst_y) * b >> 12);
                }
                dst++; src1++; src3++;
            }
            dst = (ExEdit::PixelYCA*)((int)dst + line);
            src1 = (int*)((int)src1 + addw);
            src2 = (int*)((int)src2 + addw);
        }
    }

} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_SPECIALCOLORCONV
