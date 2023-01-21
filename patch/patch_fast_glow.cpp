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
#include "patch_fast_glow.hpp"
#include "patch_fast_blur.hpp"
#ifdef PATCH_SWITCH_FAST_GLOW

#include <numbers>

#include "global.hpp"
#include "offset_address.hpp"
#include "util_int.hpp"
#include "debug_log.hpp"
#include <immintrin.h>


namespace patch::fast {


    void __cdecl Glow_t::lower_right_convolution1_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        thread_num *= 2;
        int func_ptr = GLOBAL::exedit_base + 0x570d0;
        reinterpret_cast<void(__cdecl*)(int, int, ExEdit::Filter*, ExEdit::FilterProcInfo*)>(func_ptr)(thread_id, thread_num, efp, efpip);
        reinterpret_cast<void(__cdecl*)(int, int, ExEdit::Filter*, ExEdit::FilterProcInfo*)>(func_ptr)(thread_num - thread_id - 1, thread_num, efp, efpip);
    }
    void __cdecl Glow_t::lower_right_convolution2_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        thread_num *= 2;
        int func_ptr = GLOBAL::exedit_base + 0x57730;
        reinterpret_cast<void(__cdecl*)(int, int, ExEdit::Filter*, ExEdit::FilterProcInfo*)>(func_ptr)(thread_id, thread_num, efp, efpip);
        reinterpret_cast<void(__cdecl*)(int, int, ExEdit::Filter*, ExEdit::FilterProcInfo*)>(func_ptr)(thread_num - thread_id - 1, thread_num, efp, efpip);
    }
    void __cdecl Glow_t::lower_left_convolution1_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        thread_num *= 2;
        int func_ptr = GLOBAL::exedit_base + 0x57d90;
        reinterpret_cast<void(__cdecl*)(int, int, ExEdit::Filter*, ExEdit::FilterProcInfo*)>(func_ptr)(thread_id, thread_num, efp, efpip);
        reinterpret_cast<void(__cdecl*)(int, int, ExEdit::Filter*, ExEdit::FilterProcInfo*)>(func_ptr)(thread_num - thread_id - 1, thread_num, efp, efpip);
    }
    void __cdecl Glow_t::lower_left_convolution2_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        thread_num *= 2;
        int func_ptr = GLOBAL::exedit_base + 0x58430;
        reinterpret_cast<void(__cdecl*)(int, int, ExEdit::Filter*, ExEdit::FilterProcInfo*)>(func_ptr)(thread_id, thread_num, efp, efpip);
        reinterpret_cast<void(__cdecl*)(int, int, ExEdit::Filter*, ExEdit::FilterProcInfo*)>(func_ptr)(thread_num - thread_id - 1, thread_num, efp, efpip);
    }




    struct fastGlow256 {
        __m256i data;
        __m256i y, cb, cr;
        __m256i offset;
    };


    void __declspec(noinline) __fastcall fg_put256_weight(fastGlow256* fg256, ExEdit::PixelYCA* dst, int n) {
        fastGlow256 fg256s;

        __m256i y256 = _mm256_srai_epi32(fg256->y, 4);
        y256 = _mm256_mullo_epi32(y256, fg256->data);
        fg256s.y = _mm256_srai_epi32(y256, 10);

        __m256i cb256 = _mm256_srai_epi32(fg256->cb, 4);
        cb256 = _mm256_mullo_epi32(cb256, fg256->data);
        fg256s.cb = _mm256_srai_epi32(cb256, 10);

        __m256i cr256 = _mm256_srai_epi32(fg256->cr, 4);
        cr256 = _mm256_mullo_epi32(cr256, fg256->data);
        fg256s.cr = _mm256_srai_epi32(cr256, 10);

        fg256s.offset = fg256->offset;
        Blur_t::yc256_add((Blur_t::fastBlurYC256*)&fg256s, (ExEdit::PixelYC*)dst);

        for (int i = 0; i < 8; i++) {
            int y = fg256s.y.m256i_i32[i];
            if (0x2000 < y) {
                dst->y = 0x2000;
                dst->cb = (short)((fg256s.cb.m256i_i32[i] << 13) / y);
                dst->cr = (short)((fg256s.cr.m256i_i32[i] << 13) / y);
            } else {
                dst->y = (short)y;
                dst->cb = fg256s.cb.m256i_i16[i * 2];
                dst->cr = fg256s.cr.m256i_i16[i * 2];
            }
            dst = (ExEdit::PixelYCA*)((int)dst + n);
        }
    }


    struct fastGlow128 {
        __m128i data;
        int y, cb, cr, a;
    };
    void __declspec(noinline) __fastcall fg_put128_weight(fastGlow128* fg128, ExEdit::PixelYCA* dst) {
        __m128i yc128 = _mm_srai_epi32(_mm_loadu_si128((__m128i*)&fg128->y), 4);
        yc128 = _mm_mullo_epi32(yc128, fg128->data);
        yc128 = _mm_srai_epi32(yc128, 10);
        yc128 = _mm_add_epi32(yc128, _mm_cvtepi16_epi32(_mm_loadu_epi64(dst)));

        int y = yc128.m128i_i32[0];
        if (0x2000 < y) {
            dst->y = 0x2000;
            dst->cb = (yc128.m128i_i32[1] << 13) / y;
            dst->cr = (yc128.m128i_i32[2] << 13) / y;
        } else {
            dst->y = (short)y;
            dst->cb = yc128.m128i_i16[2];
            dst->cr = yc128.m128i_i16[4];
        }
    }

    

    void convolution(int n_begin, int n_end, void* buf_dst, void* buf_src, int buf_step1, int buf_step2, int obj_size, int blur_size) {
        blur_size = min(blur_size, (obj_size - 1) / 2);
        int range = blur_size * 2 + 1;
        int loop3 = obj_size - range;

        fastGlow256 fg256;
        fg256.data = _mm256_set1_epi32(range);
        fg256.offset = _mm256_mullo_epi32(_mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0), _mm256_set1_epi32(buf_step2));

        int n = n_begin;
        int offset = n * buf_step2;
        int n_end256 = n_end - 7;
        for (; n < n_end256; n += 8) {
            auto src1 = (ExEdit::PixelYCA*)((int)buf_src + offset);
            auto src2 = src1;
            auto dst = (ExEdit::PixelYCA*)((int)buf_dst + offset);
            offset += buf_step2 * 8;

            fg256.y = fg256.cb = fg256.cr = _mm256_setzero_si256();
            for (int i = 0; i < blur_size; i++) {
                Blur_t::yc256_add((Blur_t::fastBlurYC256*)&fg256, (ExEdit::PixelYC*)src1);
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);
            }
            for (int i = 0; i <= blur_size; i++) {
                Blur_t::yc256_add((Blur_t::fastBlurYC256*)&fg256, (ExEdit::PixelYC*)src1);
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);

                Blur_t::yc256_put_average((Blur_t::fastBlurYC256*)&fg256, (ExEdit::PixelYC*)dst, buf_step2);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
            for (int i = 0; i < loop3; i++) {
                Blur_t::yc256_sub((Blur_t::fastBlurYC256*)&fg256, (ExEdit::PixelYC*)src2);
                src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);

                Blur_t::yc256_add((Blur_t::fastBlurYC256*)&fg256, (ExEdit::PixelYC*)src1);
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);

                Blur_t::yc256_put_average((Blur_t::fastBlurYC256*)&fg256, (ExEdit::PixelYC*)dst, buf_step2);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
            for (int i = 0; i < blur_size; i++) {
                Blur_t::yc256_sub((Blur_t::fastBlurYC256*)&fg256, (ExEdit::PixelYC*)src2);
                src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);

                Blur_t::yc256_put_average((Blur_t::fastBlurYC256*)&fg256, (ExEdit::PixelYC*)dst, buf_step2);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
        }

        for (; n < n_end; n++) {
            auto src1 = (ExEdit::PixelYCA*)((int)buf_src + offset);
            auto src2 = src1;
            auto dst = (ExEdit::PixelYCA*)((int)buf_dst + offset);
            offset += buf_step2;

            int sum_y = 0;
            int sum_cb = 0;
            int sum_cr = 0;
            for (int i = 0; i < blur_size; i++) {
                sum_y += src1->y;
                sum_cb += src1->cb;
                sum_cr += src1->cr;
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);
            }
            for (int i = 0; i <= blur_size; i++) {
                sum_y += src1->y;
                sum_cb += src1->cb;
                sum_cr += src1->cr;
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);
                dst->y = (short)(sum_y / range);
                dst->cb = (short)(sum_cb / range);
                dst->cr = (short)(sum_cr / range);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
            for (int i = 0; i < loop3; i++) {
                sum_y += src1->y - src2->y;
                sum_cb += src1->cb - src2->cb;
                sum_cr += src1->cr - src2->cr;
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);
                src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);
                dst->y = (short)(sum_y / range);
                dst->cb = (short)(sum_cb / range);
                dst->cr = (short)(sum_cr / range);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
            for (int i = 0; i < blur_size; i++) {
                sum_y -= src2->y;
                sum_cb -= src2->cb;
                sum_cr -= src2->cr;
                src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);
                dst->y = (short)(sum_y / range);
                dst->cb = (short)(sum_cb / range);
                dst->cr = (short)(sum_cr / range);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
        }
    }
    void __cdecl Glow_t::vertical_convolution(int thi, int thn, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto glow = (efGlow_var*)(GLOBAL::exedit_base + OFS::ExEdit::efGlow_var_ptr);
        int w = glow->src_w + glow->diffusion_w * 2;
        int h = glow->src_h + glow->diffusion_h * 2;
        convolution(thi * w / thn, (thi + 1) * w / thn, glow->buf_temp2, glow->buf_temp,
            efpip->obj_line * sizeof(struct ExEdit::PixelYCA), sizeof(struct ExEdit::PixelYCA), h, glow->blur);
    }
    void __cdecl Glow_t::horizontal_convolution(int thi, int thn, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto glow = (efGlow_var*)(GLOBAL::exedit_base + OFS::ExEdit::efGlow_var_ptr);
        int w = glow->src_w + glow->diffusion_w * 2;
        int h = glow->src_h + glow->diffusion_h * 2;
        convolution(thi * h / thn, (thi + 1) * h / thn, glow->buf_temp, glow->buf_temp2,
            sizeof(struct ExEdit::PixelYCA), efpip->obj_line * sizeof(struct ExEdit::PixelYCA), w, glow->blur);
    }


    void __cdecl Glow_t::horizontal_convolution_intensity_blur(int thi, int thn, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto glow = (efGlow_var*)(GLOBAL::exedit_base + OFS::ExEdit::efGlow_var_ptr);

        int h = glow->src_h + glow->diffusion_h * 2;
        int blur = glow->blur;
        int range_w = glow->src_w + (glow->diffusion_w - blur) * 2 - 1;
        int intensity = glow->intensity;
        int linesize = efpip->obj_line * sizeof(struct ExEdit::PixelYCA);
        int y_begin = thi * h / thn;
        int y_end = (thi + 1) * h / thn;
        int offset = y_begin * linesize;

        fastGlow256 fg256;
        fg256.data = _mm256_set1_epi32(intensity);
        fg256.offset = _mm256_mullo_epi32(_mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0), _mm256_set1_epi32(linesize));

        int y = y_begin;
        int y_end256 = y_end - 7;
        for (; y < y_end256; y += 8) {
            auto src1 = (ExEdit::PixelYCA*)((int)glow->buf_temp2 + offset);
            auto src2 = src1;
            auto dst = (ExEdit::PixelYCA*)((int)glow->buf_temp + offset);
            offset += linesize * 8;

            fg256.y = fg256.cb = fg256.cr = _mm256_setzero_si256();
            for (int x = 0; x < blur; x++) {
                Blur_t::yc256_add((Blur_t::fastBlurYC256*)&fg256, (ExEdit::PixelYC*)src1);
                src1++;
            }
            for (int x = 0; x <= blur; x++) {
                Blur_t::yc256_add((Blur_t::fastBlurYC256*)&fg256, (ExEdit::PixelYC*)src1);
                src1++;

                fg_put256_weight(&fg256, dst, linesize);
                dst++;
            }
            for (int x = 0; x < range_w; x++) {
                Blur_t::yc256_sub((Blur_t::fastBlurYC256*)&fg256, (ExEdit::PixelYC*)src2);
                src2++;

                Blur_t::yc256_add((Blur_t::fastBlurYC256*)&fg256, (ExEdit::PixelYC*)src1);
                src1++;

                fg_put256_weight(&fg256, dst, linesize);
                dst++;
            }
            for (int x = 0; x < blur; x++) {
                Blur_t::yc256_sub((Blur_t::fastBlurYC256*)&fg256, (ExEdit::PixelYC*)src2);
                src2++;

                fg_put256_weight(&fg256, dst, linesize);
                dst++;
            }
        }


        fastGlow128 fg128;
        fg128.data = _mm_set1_epi32(intensity);
        for (; y < y_end; y++) {
            auto src1 = (ExEdit::PixelYCA*)((int)glow->buf_temp2 + offset);
            auto src2 = src1;
            auto dst = (ExEdit::PixelYCA*)((int)glow->buf_temp + offset);
            offset += linesize;

            fg128.y = fg128.cb = fg128.cr = 0;
            for (int x = 0; x < blur; x++) {
                fg128.y += src1->y;
                fg128.cb += src1->cb;
                fg128.cr += src1->cr;
                src1++;
            }
            for (int x = 0; x <= blur; x++) {
                fg128.y += src1->y;
                fg128.cb += src1->cb;
                fg128.cr += src1->cr;
                src1++;

                fg_put128_weight(&fg128, dst);
                dst++;
            }
            for (int x = 0; x < range_w; x++) {
                fg128.y += src1->y - src2->y;
                fg128.cb += src1->cb - src2->cb;
                fg128.cr += src1->cr - src2->cr;
                src1++;
                src2++;

                fg_put128_weight(&fg128, dst);
                dst++;
            }
            for (int x = 0; x < blur; x++) {
                fg128.y -= src2->y;
                fg128.cb -= src2->cb;
                fg128.cr -= src2->cr;
                src2++;

                fg_put128_weight(&fg128, dst);
                dst++;
            }
        }
    }


    void convolution_intensity_main(int n_begin, int n_end, void* buf_dst, void* buf_src, int buf_step1, int buf_step2, int obj_size, int diff_size, int diff1, int diff2, int intensity) {
        diff_size = min(diff_size, (obj_size - 1) / 2);
        int range = diff_size * 2;
        int loop2 = obj_size - range - 1;

        buf_dst = (void*)((int)buf_dst + (diff1 - diff_size) * buf_step1 + diff2 * buf_step2);

        fastGlow256 fg256;
        fg256.data = _mm256_set1_epi32(intensity);
        fg256.offset = _mm256_mullo_epi32(_mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0), _mm256_set1_epi32(buf_step2));
        
        int n = n_begin;
        int offset = n * buf_step2;
        int n_end256 = n_end - 7;
        for (; n < n_end256; n += 8) {
            auto src1 = (ExEdit::PixelYCA*)((int)buf_src + offset);
            auto src2 = src1;
            auto dst = (ExEdit::PixelYCA*)((int)buf_dst + offset);
            offset += buf_step2 * 8;

            fg256.y = fg256.cb = fg256.cr = _mm256_setzero_si256();
            for (int i = 0; i <= range; i++) {
                Blur_t::yc256_add((Blur_t::fastBlurYC256*)&fg256, (ExEdit::PixelYC*)src1);
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);

                fg_put256_weight(&fg256, dst, buf_step2);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
            for (int i = 0; i < loop2; i++) {
                Blur_t::yc256_sub((Blur_t::fastBlurYC256*)&fg256, (ExEdit::PixelYC*)src2);
                src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);
                Blur_t::yc256_add((Blur_t::fastBlurYC256*)&fg256, (ExEdit::PixelYC*)src1);
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);

                fg_put256_weight(&fg256, dst, buf_step2);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
            for (int i = 0; i < range; i++) {
                Blur_t::yc256_sub((Blur_t::fastBlurYC256*)&fg256, (ExEdit::PixelYC*)src2);
                src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);

                fg_put256_weight(&fg256, dst, buf_step2);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
        }

        fastGlow128 fg128;
        fg128.data = _mm_set1_epi32(intensity);
        for (; n < n_end; n++) {
            auto src1 = (ExEdit::PixelYCA*)((int)buf_src + offset);
            auto src2 = src1;
            auto dst = (ExEdit::PixelYCA*)((int)buf_dst + offset);
            offset += buf_step2;

            fg128.y = fg128.cb = fg128.cr = 0;
            for (int i = 0; i <= range; i++) {
                fg128.y += src1->y;
                fg128.cb += src1->cb;
                fg128.cr += src1->cr;
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);

                fg_put128_weight(&fg128, dst);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
            for (int i = 0; i < loop2; i++) {
                fg128.y += src1->y - src2->y;
                fg128.cb += src1->cb - src2->cb;
                fg128.cr += src1->cr - src2->cr;
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);
                src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);

                fg_put128_weight(&fg128, dst);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
            for (int i = 0; i < range; i++) {
                fg128.y -= src2->y;
                fg128.cb -= src2->cb;
                fg128.cr -= src2->cr;
                src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);

                fg_put128_weight(&fg128, dst);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
        }
    }
    void __cdecl Glow_t::vertical_convolution_intensity3(int thi, int thn, ExEdit::Filter* efp, ExEdit::FilterProcInfo * efpip) {
        auto glow = (Glow_t::efGlow_var*)(GLOBAL::exedit_base + OFS::ExEdit::efGlow_var_ptr);

        convolution_intensity_main(thi * glow->src_w / thn, (thi + 1) * glow->src_w / thn, glow->buf_temp, glow->buf_temp2,
            efpip->obj_line * sizeof(struct ExEdit::PixelYCA), sizeof(struct ExEdit::PixelYCA),
            glow->src_h, glow->diffusion_h, glow->diffusion_h, glow->diffusion_w, glow->intensity);
        convolution_intensity_main(thi * glow->src_w / thn, (thi + 1) * glow->src_w / thn, glow->buf_temp, glow->buf_temp2,
            efpip->obj_line * sizeof(struct ExEdit::PixelYCA), sizeof(struct ExEdit::PixelYCA),
            glow->src_h, glow->diffusion_h / 2, glow->diffusion_h, glow->diffusion_w, glow->intensity * 2);
        convolution_intensity_main(thi * glow->src_w / thn, (thi + 1) * glow->src_w / thn, glow->buf_temp, glow->buf_temp2,
            efpip->obj_line * sizeof(struct ExEdit::PixelYCA), sizeof(struct ExEdit::PixelYCA),
            glow->src_h, glow->diffusion_h / 4, glow->diffusion_h, glow->diffusion_w, glow->intensity * 4);
    }
    void __cdecl Glow_t::horizontal_convolution_intensity3(int thi, int thn, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto glow = (Glow_t::efGlow_var*)(GLOBAL::exedit_base + OFS::ExEdit::efGlow_var_ptr);

        convolution_intensity_main(thi * glow->src_h / thn, (thi + 1) * glow->src_h / thn, glow->buf_temp, glow->buf_temp2,
            sizeof(struct ExEdit::PixelYCA), efpip->obj_line * sizeof(struct ExEdit::PixelYCA),
            glow->src_w, glow->diffusion_w, glow->diffusion_w, glow->diffusion_h, glow->intensity);
        convolution_intensity_main(thi * glow->src_h / thn, (thi + 1) * glow->src_h / thn, glow->buf_temp, glow->buf_temp2,
            sizeof(struct ExEdit::PixelYCA), efpip->obj_line * sizeof(struct ExEdit::PixelYCA),
            glow->src_w, glow->diffusion_w / 2, glow->diffusion_w, glow->diffusion_h, glow->intensity * 2);
        convolution_intensity_main(thi * glow->src_h / thn, (thi + 1) * glow->src_h / thn, glow->buf_temp, glow->buf_temp2,
            sizeof(struct ExEdit::PixelYCA), efpip->obj_line * sizeof(struct ExEdit::PixelYCA),
            glow->src_w, glow->diffusion_w / 4, glow->diffusion_w, glow->diffusion_h, glow->intensity * 4);
    }

}
#endif // ifdef PATCH_SWITCH_FAST_GLOW
