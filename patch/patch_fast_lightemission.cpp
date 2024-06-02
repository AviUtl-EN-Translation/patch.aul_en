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

#pragma once
#include "patch_fast_lightemission.hpp"
#include "patch_fast_blur.hpp"
#ifdef PATCH_SWITCH_FAST_LIGHTEMISSION
#ifdef PATCH_SWITCH_FAST_BLUR

#include <numbers>

#include "global.hpp"
#include "offset_address.hpp"
#include "util_int.hpp"
#include "debug_log.hpp"
#include <immintrin.h>


namespace patch::fast {
    
    void __cdecl LightEmission_t::vertical_yc_fb_cs_mt_wrap(int thread_id, int thread_num, void* mem_ptr, ExEdit::FilterProcInfo* efpip) {
        void* buf_ptr = *reinterpret_cast<void**>(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);
        auto le = (efLightEmission_var*)(GLOBAL::exedit_base + OFS::ExEdit::efLightEmission_var_ptr);
        Blur_t::vertical_yc_fb_cs_mt(thread_id, thread_num, le->w, le->h, efpip->scene_line, le->size_h, buf_ptr, efpip->frame_temp, mem_ptr, 1);
    }

    static AviUtl::SharedMemoryInfo* smi;
    void __cdecl LightEmission_t::vertical_yc_fb_cs_mt_func_wrap(AviUtl::MultiThreadFunc mt, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        void* mem_ptr = efp->aviutl_exfunc->get_shared_mem((int)&smi, (int)&smi, smi);
        if (mem_ptr == nullptr) {
            int size = max(efpip->obj_line * 24, 0x20000);
            mem_ptr = efp->aviutl_exfunc->create_shared_mem((int)&smi, (int)&smi, size, &smi);
            if (mem_ptr == nullptr) {
                efp->aviutl_exfunc->exec_multi_thread_func(mt, efp, efpip);
                return;
            }
        }
        efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&vertical_yc_fb_cs_mt_wrap, mem_ptr, efpip);
    }
    
    void __cdecl LightEmission_t::vertical_yc_cs_mt_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto le = (efLightEmission_var*)(GLOBAL::exedit_base + OFS::ExEdit::efLightEmission_var_ptr);
        void* buf_ptr = *reinterpret_cast<void**>(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);
        Blur_t::blur_yc_cs_mt(thread_id * le->w / thread_num, (thread_id + 1) * le->w / thread_num, buf_ptr, efpip->frame_temp,
            efpip->scene_line * sizeof(ExEdit::PixelYC), sizeof(ExEdit::PixelYC), le->h, le->size_h);
    }

    /*
    struct fastLE {
        int y, cb, cr;
        int n;
        ExEdit::PixelYC* dst1;
        ExEdit::PixelYC* dst2;
        __m256i y256, cb256, cr256;
        __m256i range256;
    };


    void __declspec(noinline) __fastcall lefunc(fastLE* fle) {
        fle->y256 = _mm256_div_epi32(fle->y256, fle->range256);
        fle->cb256 = _mm256_div_epi32(fle->cb256, fle->range256);
        fle->cr256 = _mm256_div_epi32(fle->cr256, fle->range256);
        for (int i = 0; i < fle->n; i++) {
            fle->dst1->y = fle->y256.m256i_i16[i * 2];
            fle->dst1->cb = fle->cb256.m256i_i16[i * 2];
            fle->dst1->cr = fle->cr256.m256i_i16[i * 2];
            int sum_y = fle->dst1->y + fle->dst2->y;
            if (sum_y <= 0x2000) {
                fle->dst2->y = sum_y;
                fle->dst2->cb += fle->dst1->cb;
                fle->dst2->cr += fle->dst1->cr;
            } else {
                fle->dst2->y = 0x2000;
                sum_y = 0x4000 - sum_y;
                if (0 < sum_y) {
                    fle->dst2->cb = (fle->dst1->cb + fle->dst2->cb) * sum_y >> 13;
                    fle->dst2->cr = (fle->dst1->cr + fle->dst2->cr) * sum_y >> 13;
                } else {
                    fle->dst2->cb = fle->dst2->cr = 0;
                }
            }
            fle->dst2++;
            fle->dst1++;
        }
        fle->n = 0;
    }

    void __cdecl LightEmission_t::horizontal_yc_cs_mt_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto le = (efLightEmission_var*)(GLOBAL::exedit_base + OFS::ExEdit::efLightEmission_var_ptr);
        int y = thread_id * le->h / thread_num;
        int ofs = y * efpip->scene_line * 6;
        fastLE fle;
        fle.range256 = _mm256_set1_epi32(le->range_w);
        auto mem_ptr = *reinterpret_cast<void**>(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);
        for (y = (thread_id + 1) * le->h / thread_num - y; 0 < y; y--) {
            auto src1 = (ExEdit::PixelYC*)((int)mem_ptr + ofs);
            auto src2 = src1;
            fle.dst1 = (ExEdit::PixelYC*)((int)efpip->frame_temp + ofs);
            fle.dst2 = (ExEdit::PixelYC*)((int)efpip->obj_temp + ofs);
            fle.y = fle.cb = fle.cr = fle.n = 0;
            for (int x = le->size_w; 0 < x; x--) {
                fle.y += src1->y;
                fle.cb += src1->cb;
                fle.cr += src1->cr;
                src1++;
            }
            for (int x = le->size_w + 1; 0 < x; x--) {
                fle.y += src1->y;
                fle.y256.m256i_i32[fle.n] = fle.y;
                fle.cb += src1->cb;
                fle.cb256.m256i_i32[fle.n] = fle.cb;
                fle.cr += src1->cr;
                fle.cr256.m256i_i32[fle.n] = fle.cr;
                src1++;
                fle.n++;
                if (fle.n == 8) {
                    lefunc(&fle);
                }
            }
            for (int x = le->w - le->range_w; 0 < x; x--) {
                fle.y += src1->y - src2->y;
                fle.y256.m256i_i32[fle.n] = fle.y;
                fle.cb += src1->cb - src2->cb;
                fle.cb256.m256i_i32[fle.n] = fle.cb;
                fle.cr += src1->cr - src2->cr;
                fle.cr256.m256i_i32[fle.n] = fle.cr;
                src1++;
                src2++;
                fle.n++;
                if (fle.n == 8) {
                    lefunc(&fle);
                }
            }
            for (int x = le->size_w; 0 < x; x--) {
                fle.y -= src2->y;
                fle.y256.m256i_i32[fle.n] = fle.y;
                fle.cb -= src2->cb;
                fle.cb256.m256i_i32[fle.n] = fle.cb;
                fle.cr -= src2->cr;
                fle.cr256.m256i_i32[fle.n] = fle.cr;
                src2++;
                fle.n++;
                if (fle.n == 8) {
                    lefunc(&fle);
                }
            }
            if (0 < fle.n) {
                lefunc(&fle);
            }
            ofs += efpip->scene_line * 6;
        }
    }
    */
}
#endif // ifdef PATCH_SWITCH_FAST_BLUR
#endif // ifdef PATCH_SWITCH_FAST_LIGHTEMISSION
