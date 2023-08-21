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

#include "patch_fast_borderblur.hpp"
#ifdef PATCH_SWITCH_FAST_BORDERBLUR
//#include <immintrin.h>


//#define PATCH_STOPWATCH

namespace patch::fast {

    void __cdecl BorderBlur_t::mt1(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto bblur = reinterpret_cast<efBorderBlur_var*>(GLOBAL::exedit_base + 0x11ec48);
        auto lut_minus_cos_half = reinterpret_cast<short*>(GLOBAL::exedit_base + OFS::ExEdit::lut_minus_cos_half);
        auto a = (short*)efpip->obj_temp + thread_id;
        for (int y = bblur->size_h + thread_id + 1; y < bblur->range_h; y += thread_num) {
            *a = lut_minus_cos_half[0x1000 - (short)((y << 12) / bblur->range_h)] * 2;
            a += thread_num;
        }
        if (bblur->size_h != bblur->size_w) {
            a = (short*)efpip->obj_temp + bblur->size_h + thread_num - thread_id - 1;
            for (int x = bblur->size_w + thread_num - 1 - thread_id + 1; x < bblur->range_w; x += thread_num) {
                *a = lut_minus_cos_half[0x1000 - (short)((x << 12) / bblur->range_w)] * 2;
                a += thread_num;
            }
        }
    }

    void __cdecl BorderBlur_t::mt2(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto bblur = reinterpret_cast<efBorderBlur_var*>(GLOBAL::exedit_base + 0x11ec48);
        int half_w = efpip->obj_w >> 1;
        int half_h = efpip->obj_h >> 1;
        short* ya = (short*)efpip->obj_temp;
        short* xa = ya;
        if (bblur->size_h != bblur->size_w) {
            xa += bblur->size_h;
        }
        int y;
        for (y = thread_id; y < bblur->size_h; y += thread_num) {
            auto pix_lt = &((ExEdit::PixelYCA*)efpip->obj_edit + y * efpip->obj_line)->a;
            auto pix_rt = &((ExEdit::PixelYCA*)efpip->obj_edit + y * efpip->obj_line + (efpip->obj_w - 1))->a;
            auto pix_lb = &((ExEdit::PixelYCA*)efpip->obj_edit + (efpip->obj_h - 1 - y) * efpip->obj_line)->a;
            auto pix_rb = &((ExEdit::PixelYCA*)efpip->obj_edit + (efpip->obj_h - 1 - y) * efpip->obj_line + (efpip->obj_w - 1))->a;
            int ya2 = ya[y] * ya[y];
            int x;
            /*__m256i ya256 = _mm256_set1_epi32(ya2);
            for (x = 0; x < bblur->size_w - 7; x+=8) {
                __m256i xa256 = _mm256_cvtepi16_epi32(_mm_loadu_epi16(&xa[x]));
                xa256 = _mm256_add_epi32(_mm256_mullo_epi32(xa256, xa256), ya256);
                xa256 = _mm256_cvtps_epi32(_mm256_sqrt_ps(_mm256_cvtepi32_ps(xa256)));

                for (int i = 0; i < 8; i++) {
                    int sa = xa256.m256i_i32[i];
                    *pix_lt = max(0, *pix_lt - sa);
                    pix_lt += 4;
                    *pix_rt = max(0, *pix_rt - sa);
                    pix_rt -= 4;
                    *pix_lb = max(0, *pix_lb - sa);
                    pix_lb += 4;
                    *pix_rb = max(0, *pix_rb - sa);
                    pix_rb -= 4;
                }
            }*/
            for (x = 0; x < bblur->size_w; x++) {
                int sa = (int)sqrt(xa[x] * xa[x] + ya2);
                *pix_lt = max(0, *pix_lt - sa);
                pix_lt += 4;
                *pix_rt = max(0, *pix_rt - sa);
                pix_rt -= 4;
                *pix_lb = max(0, *pix_lb - sa);
                pix_lb += 4;
                *pix_rb = max(0, *pix_rb - sa);
                pix_rb -= 4;
            }
            ya2 = ya[y];
            for (; x < half_w; x++) {
                *pix_lt = max(0, *pix_lt - ya2);
                pix_lt += 4;
                *pix_rt = max(0, *pix_rt - ya2);
                pix_rt -= 4;
                *pix_lb = max(0, *pix_lb - ya2);
                pix_lb += 4;
                *pix_rb = max(0, *pix_rb - ya2);
                pix_rb -= 4;
            }

        }
        for (; y < half_h; y += thread_num) {
            auto pix_lt = &((ExEdit::PixelYCA*)efpip->obj_edit + y * efpip->obj_line)->a;
            auto pix_rt = &((ExEdit::PixelYCA*)efpip->obj_edit + y * efpip->obj_line + (efpip->obj_w - 1))->a;
            auto pix_lb = &((ExEdit::PixelYCA*)efpip->obj_edit + (efpip->obj_h - 1 - y) * efpip->obj_line)->a;
            auto pix_rb = &((ExEdit::PixelYCA*)efpip->obj_edit + (efpip->obj_h - 1 - y) * efpip->obj_line + (efpip->obj_w - 1))->a;
            int x;
            for (x = 0; x < bblur->size_w; x++) {
                int xa2 = xa[x];
                *pix_lt = max(0, *pix_lt - xa2);
                pix_lt += 4;
                *pix_rt = max(0, *pix_rt - xa2);
                pix_rt -= 4;
                *pix_lb = max(0, *pix_lb - xa2);
                pix_lb += 4;
                *pix_rb = max(0, *pix_rb - xa2);
                pix_rb -= 4;
            }
        }

        if (efpip->obj_w & 1) {
            int n = thread_num * efpip->obj_line * 8;
            auto pix_t = &((ExEdit::PixelYCA*)efpip->obj_edit + thread_id * efpip->obj_line + half_w)->a;
            auto pix_b = &((ExEdit::PixelYCA*)efpip->obj_edit + (efpip->obj_h - 1 - thread_id) * efpip->obj_line + half_w)->a;
            for (y = thread_id; y < bblur->size_h; y += thread_num) {
                int ya2 = ya[y];
                *pix_t = max(0, *pix_t - ya2);
                pix_t = (short*)((int)pix_t + n);
                *pix_b = max(0, *pix_b - ya2);
                pix_b = (short*)((int)pix_b - n);
            }
        }
        if (efpip->obj_h & 1) {
            int n = thread_num * 8;
            int x = thread_num - 1 - thread_id;
            auto pix_l = &((ExEdit::PixelYCA*)efpip->obj_edit + half_h * efpip->obj_line + x)->a;
            auto pix_r = &((ExEdit::PixelYCA*)efpip->obj_edit + half_h * efpip->obj_line + efpip->obj_w - 1 - x)->a;
            for (; x < bblur->size_w; x += thread_num) {
                int xa2 = xa[x];
                *pix_l = max(0, *pix_l - xa2);
                pix_l = (short*)((int)pix_l + n);
                *pix_r = max(0, *pix_r - xa2);
                pix_r = (short*)((int)pix_r - n);
            }
        }
    }
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_BORDERBLUR
