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
#include "patch_fast_blur.hpp"
#ifdef PATCH_SWITCH_FAST_BLUR

#include <numbers>

#include "global.hpp"
#include "offset_address.hpp"
#include "util_int.hpp"
#include "debug_log.hpp"
#include <immintrin.h>


namespace patch::fast {



    struct PixelYC_fbb {
        float y;
        int8_t cb;
        int8_t cr;
    };
    struct PixelYCA_fbbs {
        float y;
        int8_t cb;
        int8_t cr;
        int16_t a;
    };

    struct fastBlurYCfb256 {
        __m256i range;
        __m256 invrange;
        __m256i halfrange;
        __m256 y;
        __m256i cb, cr;
        __m256i offset;
    };

    void __fastcall ycfb256_new_range(fastBlurYCfb256* fb256, int range) {
        fb256->range = _mm256_set1_epi32(range);
        fb256->invrange = _mm256_set1_ps(1.0f / (float)range);
        fb256->halfrange = _mm256_set1_epi32(range >> 1);
    }

    void __declspec(noinline) __fastcall ycfb256_add(fastBlurYCfb256* fb256, PixelYC_fbb* src) {
        __m256 y256 = _mm256_i32gather_ps((float*)src, fb256->offset, 1);
        fb256->y = _mm256_add_ps(fb256->y, y256);
        __m256i cbcr256 = _mm256_i32gather_epi32((int*)((int)src + 2), fb256->offset, 1);
        __m256i cb256 = _mm256_srai_epi32(_mm256_slli_epi32(cbcr256, 8), 24);
        fb256->cb = _mm256_add_epi32(fb256->cb, cb256);
        __m256i cr256 = _mm256_srai_epi32(cbcr256, 24);
        fb256->cr = _mm256_add_epi32(fb256->cr, cr256);
    }
    void __declspec(noinline) __fastcall ycfb256_sub(fastBlurYCfb256* fb256, PixelYC_fbb* src) {
        __m256 y256 = _mm256_i32gather_ps((float*)src, fb256->offset, 1);
        fb256->y = _mm256_sub_ps(fb256->y, y256);
        __m256i cbcr256 = _mm256_i32gather_epi32((int*)((int)src + 2), fb256->offset, 1);
        __m256i cb256 = _mm256_srai_epi32(_mm256_slli_epi32(cbcr256, 8), 24);
        fb256->cb = _mm256_sub_epi32(fb256->cb, cb256);
        __m256i cr256 = _mm256_srai_epi32(cbcr256, 24);
        fb256->cr = _mm256_sub_epi32(fb256->cr, cr256);
    }
    void __declspec(noinline) __fastcall ycfb256_put_average(fastBlurYCfb256* fb256, PixelYC_fbb* dst, int buf_step2) {
        __m256 ave_y256 = _mm256_mul_ps(fb256->y, fb256->invrange);

        __m256i flag256 = _mm256_srai_epi32(fb256->cb, 31);
        __m256i round256 = _mm256_xor_si256(fb256->halfrange, flag256);
        round256 = _mm256_add_epi32(fb256->cb, _mm256_sub_epi32(round256, flag256));
        __m256i ave_cb256 = _mm256_div_epi32(round256, fb256->range);

        flag256 = _mm256_srai_epi32(fb256->cr, 31);
        round256 = _mm256_xor_si256(fb256->halfrange, flag256);
        round256 = _mm256_add_epi32(fb256->cr, _mm256_sub_epi32(round256, flag256));
        __m256i ave_cr256 = _mm256_div_epi32(round256, fb256->range);

        for (int i = 0; i < 8; i++) {
            dst->y = ave_y256.m256_f32[i];
            dst->cb = ave_cb256.m256i_i8[i << 2];
            dst->cr = ave_cr256.m256i_i8[i << 2];
            dst = (PixelYC_fbb*)((int)dst + buf_step2);
        }
    }


    void blur_yc_fb_mt(int n_begin, int n_end, void* buf_dst, void* buf_src, int buf_step1, int buf_step2, int obj_size, int blur_size) {
        int blur_range = blur_size * 2 + 1;
        int loop3 = obj_size - blur_range;
        int offset = n_begin * buf_step2;

        fastBlurYCfb256 fb256;
        fb256.offset = _mm256_mullo_epi32(_mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0), _mm256_set1_epi32(buf_step2));
        int n;
        for (n = n_end - n_begin; 8 <= n; n -= 8) {
            auto dst = (PixelYC_fbb*)((int)buf_dst + offset);
            auto src1 = (PixelYC_fbb*)((int)buf_src + offset);
            auto src2 = src1;
            offset += buf_step2 * 8;

            fb256.y = _mm256_setzero_ps();
            fb256.cb = fb256.cr = _mm256_setzero_si256();
            for (int i = blur_size; 0 < i; i--) {
                ycfb256_add(&fb256, src1);
                src1 = (PixelYC_fbb*)((int)src1 + buf_step1);
            }

            int range = blur_size;
            for (int i = blur_size; 0 <= i; i--) {
                ycfb256_add(&fb256, src1);
                src1 = (PixelYC_fbb*)((int)src1 + buf_step1);

                range++;
                ycfb256_new_range(&fb256, range);
                ycfb256_put_average(&fb256, dst, buf_step2);
                dst = (PixelYC_fbb*)((int)dst + buf_step1);
            }

            for (int i = loop3; 0 < i; i--) {
                ycfb256_sub(&fb256, src2);
                src2 = (PixelYC_fbb*)((int)src2 + buf_step1);
                ycfb256_add(&fb256, src1);
                src1 = (PixelYC_fbb*)((int)src1 + buf_step1);

                ycfb256_put_average(&fb256, dst, buf_step2);
                dst = (PixelYC_fbb*)((int)dst + buf_step1);
            }

            for (int i = blur_size; 0 < i; i--) {
                ycfb256_sub(&fb256, src2);
                src2 = (PixelYC_fbb*)((int)src2 + buf_step1);

                range--;
                ycfb256_new_range(&fb256, range);
                ycfb256_put_average(&fb256, dst, buf_step2);
                dst = (PixelYC_fbb*)((int)dst + buf_step1);
            }
        }


        float f_inv_range = 1.0f / (float)blur_range;
        int half_range = blur_range >> 1;
        for (; 0 < n; n--) {
            auto dst = (PixelYC_fbb*)((int)buf_dst + offset);
            auto src1 = (PixelYC_fbb*)((int)buf_src + offset);
            auto src2 = src1;
            offset += buf_step2;

            float sum_y = 0.0f;
            int sum_cb = 0;
            int sum_cr = 0;
            for (int i = blur_size; 0 < i; i--) {
                sum_y += src1->y;
                sum_cb += src1->cb;
                sum_cr += src1->cr;
                src1 = (PixelYC_fbb*)((int)src1 + buf_step1);
            }

            int range = blur_size;
            for (int i = blur_size; 0 <= i; i--) {
                sum_y += src1->y;
                sum_cb += src1->cb;
                sum_cr += src1->cr;
                src1 = (PixelYC_fbb*)((int)src1 + buf_step1);

                range++;
                dst->y = sum_y / (float)range;
                int round_c0 = range >> 1;
                int round_c1 = round_c0;
                if (sum_cb < 0)round_c1 = -round_c1;
                dst->cb = (int8_t)((sum_cb + round_c1) / range);
                if (sum_cr < 0)round_c0 = -round_c0;
                dst->cr = (int8_t)((sum_cr + round_c0) / range);
                dst = (PixelYC_fbb*)((int)dst + buf_step1);
            }

            for (int i = loop3; 0 < i; i--) {
                sum_y += src1->y - src2->y;
                sum_cb += src1->cb - src2->cb;
                sum_cr += src1->cr - src2->cr;
                src1 = (PixelYC_fbb*)((int)src1 + buf_step1);
                src2 = (PixelYC_fbb*)((int)src2 + buf_step1);

                dst->y = sum_y * f_inv_range;
                int round_c0 = half_range;
                int round_c1 = round_c0;
                if (sum_cb < 0)round_c1 = -round_c1;
                dst->cb = (int8_t)((sum_cb + round_c1) / range);
                if (sum_cr < 0)round_c0 = -round_c0;
                dst->cr = (int8_t)((sum_cr + round_c0) / range);
                dst = (PixelYC_fbb*)((int)dst + buf_step1);
            }

            for (int i = blur_size; 0 < i; i--) {
                sum_y -= src2->y;
                sum_cb -= src2->cb;
                sum_cr -= src2->cr;
                src2 = (PixelYC_fbb*)((int)src2 + buf_step1);

                range--;
                dst->y = sum_y / (float)range;
                int round_c0 = range >> 1;
                int round_c1 = round_c0;
                dst->cb = (int8_t)((sum_cb + round_c1) / range);
                if (sum_cr < 0)round_c0 = -round_c0;
                dst->cr = (int8_t)((sum_cr + round_c0) / range);
                dst = (PixelYC_fbb*)((int)dst + buf_step1);
            }
        }
    }
    void __cdecl Blur_t::vertical_yc_fb_mt_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) { // 11400
        auto blur = (efBlur_var*)(GLOBAL::exedit_base + OFS::ExEdit::efBlur_var_ptr);
        blur_yc_fb_mt(thread_id * efpip->scene_w / thread_num, (thread_id + 1) * efpip->scene_w / thread_num, efpip->frame_temp, efpip->frame_edit,
            efpip->scene_line * sizeof(ExEdit::PixelYC), sizeof(ExEdit::PixelYC), efpip->scene_h, blur->h);
    }
    void __cdecl Blur_t::horizontal_yc_fb_mt_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) { // 116d0
        auto blur = (efBlur_var*)(GLOBAL::exedit_base + OFS::ExEdit::efBlur_var_ptr);
        blur_yc_fb_mt(thread_id * efpip->scene_h / thread_num, (thread_id + 1) * efpip->scene_h / thread_num, efpip->frame_temp, efpip->frame_edit,
            sizeof(ExEdit::PixelYC), efpip->scene_line * sizeof(ExEdit::PixelYC), efpip->scene_w, blur->w);
    }


    struct fastBlurYCAfbs256 {
        __m256i range;
        __m256 invrange;
        __m256i halfrange;
        __m256 y;
        __m256i cb, cr, a;
        __m256i offset;
    };
    void __declspec(noinline) __fastcall ycafbs256_add(fastBlurYCAfbs256* fb256, PixelYCA_fbbs* src) {
        __m256 y256 = _mm256_i32gather_ps((float*)src, fb256->offset, 1);
        fb256->y = _mm256_add_ps(fb256->y, y256);
        __m256i cbcra256 = _mm256_i32gather_epi32((int*)&src->cb, fb256->offset, 1);
        __m256i cb256 = _mm256_srai_epi32(_mm256_slli_epi32(cbcra256, 24), 24);
        fb256->cb = _mm256_add_epi32(fb256->cb, cb256);
        __m256i cr256 = _mm256_srai_epi32(_mm256_slli_epi32(cbcra256, 16), 24);
        fb256->cr = _mm256_add_epi32(fb256->cr, cr256);
        __m256i a256 = _mm256_srai_epi32(cbcra256, 16);
        fb256->a = _mm256_add_epi32(fb256->a, a256);
    }
    void __declspec(noinline) __fastcall ycafbs256_sub(fastBlurYCAfbs256* fb256, PixelYCA_fbbs* src) {
        __m256 y256 = _mm256_i32gather_ps((float*)src, fb256->offset, 1);
        fb256->y = _mm256_sub_ps(fb256->y, y256);
        __m256i cbcra256 = _mm256_i32gather_epi32((int*)&src->cb, fb256->offset, 1);
        __m256i cb256 = _mm256_srai_epi32(_mm256_slli_epi32(cbcra256, 24), 24);
        fb256->cb = _mm256_sub_epi32(fb256->cb, cb256);
        __m256i cr256 = _mm256_srai_epi32(_mm256_slli_epi32(cbcra256, 16), 24);
        fb256->cr = _mm256_sub_epi32(fb256->cr, cr256);
        __m256i a256 = _mm256_srai_epi32(cbcra256, 16);
        fb256->a = _mm256_sub_epi32(fb256->a, a256);
    }
    void __declspec(noinline) __fastcall ycafbs256_put_average(fastBlurYCAfbs256* fb256, PixelYCA_fbbs* dst, int buf_step2) {
        __m256 ave_y256 = _mm256_mul_ps(fb256->y, fb256->invrange);
        __m256i flag256 = _mm256_srai_epi32(fb256->cb, 31);
        __m256i round256 = _mm256_xor_si256(fb256->halfrange, flag256);
        round256 = _mm256_add_epi32(fb256->cb, _mm256_sub_epi32(round256, flag256));
        __m256i ave_cb256 = _mm256_div_epi32(round256, fb256->range);

        flag256 = _mm256_srai_epi32(fb256->cr, 31);
        round256 = _mm256_xor_si256(fb256->halfrange, flag256);
        round256 = _mm256_add_epi32(fb256->cr, _mm256_sub_epi32(round256, flag256));
        __m256i ave_cr256 = _mm256_div_epi32(round256, fb256->range);

        __m256i ave_a256 = _mm256_div_epi32(fb256->a, fb256->range);

        for (int i = 0; i < 8; i++) {
            dst->y = ave_y256.m256_f32[i];
            dst->cb = ave_cb256.m256i_i8[i << 2];
            dst->cr = ave_cr256.m256i_i8[i << 2];
            dst->a = ave_a256.m256i_i16[i << 1];
            dst = (PixelYCA_fbbs*)((int)dst + buf_step2);
        }
    }

    void blur_yca_fb_mt(int n_begin, int n_end, void* buf_dst, void* buf_src, int buf_step1, int buf_step2, int obj_size, int blur_range) {
        int loop2 = obj_size - blur_range;
        int offset = n_begin * buf_step2;

        fastBlurYCAfbs256 fb256;
        fb256.range = _mm256_set1_epi32(blur_range);
        float f_inv_range = 1.0f / (float)blur_range;
        fb256.invrange = _mm256_set1_ps(f_inv_range);
        int half_range = blur_range >> 1;
        fb256.halfrange = _mm256_set1_epi32(half_range);
        fb256.offset = _mm256_mullo_epi32(_mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0), _mm256_set1_epi32(buf_step2));
        int n;
        for (n = n_end - n_begin; 8 <= n; n -= 8) {
            auto dst = (PixelYCA_fbbs*)((int)buf_dst + offset);
            auto src1 = (PixelYCA_fbbs*)((int)buf_src + offset);
            auto src2 = src1;
            offset += buf_step2 * 8;

            fb256.y = _mm256_setzero_ps();
            fb256.cb = fb256.cr = fb256.a = _mm256_setzero_si256();
            for (int i = blur_range; 0 < i; i--) {
                ycafbs256_add(&fb256, src1);
                src1 = (PixelYCA_fbbs*)((int)src1 + buf_step1);

                ycafbs256_put_average(&fb256, dst, buf_step2);
                dst = (PixelYCA_fbbs*)((int)dst + buf_step1);
            }
            for (int i = loop2; 0 < i; i--) {
                ycafbs256_sub(&fb256, src2);
                src2 = (PixelYCA_fbbs*)((int)src2 + buf_step1);
                ycafbs256_add(&fb256, src1);
                src1 = (PixelYCA_fbbs*)((int)src1 + buf_step1);

                ycafbs256_put_average(&fb256, dst, buf_step2);
                dst = (PixelYCA_fbbs*)((int)dst + buf_step1);
            }
            for (int i = blur_range - 1; 0 < i; i--) {
                ycafbs256_sub(&fb256, src2);
                src2 = (PixelYCA_fbbs*)((int)src2 + buf_step1);

                ycafbs256_put_average(&fb256, dst, buf_step2);
                dst = (PixelYCA_fbbs*)((int)dst + buf_step1);
            }
        }
        for (; 0 < n; n--) {
            auto dst = (PixelYCA_fbbs*)((int)buf_dst + offset);
            auto src1 = (PixelYCA_fbbs*)((int)buf_src + offset);
            auto src2 = src1;
            offset += buf_step2;

            float sum_y = 0.0f;
            int sum_cb = 0;
            int sum_cr = 0;
            int sum_a = 0;
            for (int i = blur_range; 0 < i; i--) {
                sum_y += src1->y;
                sum_cb += src1->cb;
                sum_cr += src1->cr;
                sum_a += src1->a;
                src1 = (PixelYCA_fbbs*)((int)src1 + buf_step1);

                dst->y = sum_y * f_inv_range;
                int round_c0 = half_range;
                int round_c1 = round_c0;
                if (sum_cb < 0)round_c1 = -round_c1;
                dst->cb = (int8_t)((sum_cb + round_c1) / blur_range);
                if (sum_cr < 0)round_c0 = -round_c0;
                dst->cr = (int8_t)((sum_cr + round_c0) / blur_range);
                dst->a = (int16_t)(sum_a / blur_range);
                dst = (PixelYCA_fbbs*)((int)dst + buf_step1);
            }
            for (int i = loop2; 0 < i; i--) {
                sum_y += src1->y - src2->y;
                sum_cb += src1->cb - src2->cb;
                sum_cr += src1->cr - src2->cr;
                sum_a += src1->a - src2->a;
                src1 = (PixelYCA_fbbs*)((int)src1 + buf_step1);
                src2 = (PixelYCA_fbbs*)((int)src2 + buf_step1);

                dst->y = sum_y * f_inv_range;
                int round_c0 = half_range;
                int round_c1 = round_c0;
                if (sum_cb < 0)round_c1 = -round_c1;
                dst->cb = (int8_t)((sum_cb + round_c1) / blur_range);
                if (sum_cr < 0)round_c0 = -round_c0;
                dst->cr = (int8_t)((sum_cr + round_c0) / blur_range);
                dst->a = (int16_t)(sum_a / blur_range);
                dst = (PixelYCA_fbbs*)((int)dst + buf_step1);
            }
            for (int i = blur_range - 1; 0 < i; i--) {
                sum_y -= src2->y;
                sum_cb -= src2->cb;
                sum_cr -= src2->cr;
                sum_a -= src2->a;
                src2 = (PixelYCA_fbbs*)((int)src2 + buf_step1);

                dst->y = sum_y * f_inv_range;
                int round_c0 = half_range;
                int round_c1 = round_c0;
                if (sum_cb < 0)round_c1 = -round_c1;
                dst->cb = (int8_t)((sum_cb + round_c1) / blur_range);
                if (sum_cr < 0)round_c0 = -round_c0;
                dst->cr = (int8_t)((sum_cr + round_c0) / blur_range);
                dst->a = (int16_t)(sum_a / blur_range);
                dst = (PixelYCA_fbbs*)((int)dst + buf_step1);
            }
        }
    }

    void __cdecl vertical_yca_fb_mt(int thread_id, int thread_num, int blur_range, ExEdit::FilterProcInfo* efpip) { // 10190
        blur_yca_fb_mt(thread_id * efpip->obj_w / thread_num, (thread_id + 1) * efpip->obj_w / thread_num, efpip->obj_temp, efpip->obj_edit,
            efpip->obj_line * sizeof(ExEdit::PixelYCA), sizeof(ExEdit::PixelYCA), efpip->obj_h, blur_range);
    }
    void __cdecl horizontal_yca_fb_mt(int thread_id, int thread_num, int blur_range, ExEdit::FilterProcInfo* efpip) { // 105e0
        blur_yca_fb_mt(thread_id * efpip->obj_h / thread_num, (thread_id + 1) * efpip->obj_h / thread_num, efpip->obj_temp, efpip->obj_edit,
            sizeof(ExEdit::PixelYCA), efpip->obj_line * sizeof(ExEdit::PixelYCA), efpip->obj_w, blur_range);
    }


    void blur_yca_fb_cs_mt(int n_begin, int n_end, void* buf_dst, void* buf_src, int buf_step1, int buf_step2, int obj_size, int blur_size) {
        int blur_range = blur_size * 2 + 1;
        int loop3 = obj_size - blur_range;
        int offset = n_begin * buf_step2;

        fastBlurYCAfbs256 fb256;
        fb256.offset = _mm256_mullo_epi32(_mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0), _mm256_set1_epi32(buf_step2));
        int n;
        for (n = n_end - n_begin; 8 <= n; n -= 8) {
            auto dst = (PixelYCA_fbbs*)((int)buf_dst + offset);
            auto src1 = (PixelYCA_fbbs*)((int)buf_src + offset);
            auto src2 = src1;
            offset += buf_step2 * 8;

            fb256.y = _mm256_setzero_ps();
            fb256.cb = fb256.cr = fb256.a = _mm256_setzero_si256();
            for (int i = blur_size; 0 < i; i--) {
                ycafbs256_add(&fb256, src1);
                src1 = (PixelYCA_fbbs*)((int)src1 + buf_step1);
            }
            int range = blur_size;
            for (int i = blur_size; 0 <= i; i--) {
                ycafbs256_add(&fb256, src1);
                src1 = (PixelYCA_fbbs*)((int)src1 + buf_step1);

                range++;
                ycfb256_new_range((fastBlurYCfb256*)&fb256, range);
                ycafbs256_put_average(&fb256, dst, buf_step2);
                dst = (PixelYCA_fbbs*)((int)dst + buf_step1);
            }
            for (int i = loop3; 0 < i; i--) {
                ycafbs256_sub(&fb256, src2);
                src2 = (PixelYCA_fbbs*)((int)src2 + buf_step1);
                ycafbs256_add(&fb256, src1);
                src1 = (PixelYCA_fbbs*)((int)src1 + buf_step1);

                ycafbs256_put_average(&fb256, dst, buf_step2);
                dst = (PixelYCA_fbbs*)((int)dst + buf_step1);
            }

            for (int i = blur_size; 0 < i; i--) {
                ycafbs256_sub(&fb256, src2);
                src2 = (PixelYCA_fbbs*)((int)src2 + buf_step1);

                range--;
                ycfb256_new_range((fastBlurYCfb256*)&fb256, range);
                ycafbs256_put_average(&fb256, dst, buf_step2);
                dst = (PixelYCA_fbbs*)((int)dst + buf_step1);
            }
        }
        float f_inv_range = 1.0f / (float)blur_range;
        int half_range = blur_range >> 1;
        for (; 0 < n; n--) {
            auto dst = (PixelYCA_fbbs*)((int)buf_dst + offset);
            auto src1 = (PixelYCA_fbbs*)((int)buf_src + offset);
            auto src2 = src1;
            offset += buf_step2;

            float sum_y = 0.0f;
            int sum_cb = 0;
            int sum_cr = 0;
            int sum_a = 0;
            for (int i = blur_size; 0 < i; i--) {
                sum_y += src1->y;
                sum_cb += src1->cb;
                sum_cr += src1->cr;
                sum_a += src1->a;
                src1 = (PixelYCA_fbbs*)((int)src1 + buf_step1);
            }
            int range = blur_size;
            for (int i = blur_size; 0 <= i; i--) {
                sum_y += src1->y;
                sum_cb += src1->cb;
                sum_cr += src1->cr;
                sum_a += src1->a;
                src1 = (PixelYCA_fbbs*)((int)src1 + buf_step1);

                range++;
                dst->y = sum_y / (float)range;
                int round_c0 = range >> 1;
                int round_c1 = round_c0;
                if (sum_cb < 0)round_c1 = -round_c1;
                dst->cb = (int8_t)((sum_cb + round_c1) / range);
                if (sum_cr < 0)round_c0 = -round_c0;
                dst->cr = (int8_t)((sum_cr + round_c0) / range);
                dst->a = (int16_t)(sum_a / range);
                dst = (PixelYCA_fbbs*)((int)dst + buf_step1);
            }
            for (int i = loop3; 0 < i; i--) {
                sum_y += src1->y - src2->y;
                sum_cb += src1->cb - src2->cb;
                sum_cr += src1->cr - src2->cr;
                sum_a += src1->a - src2->a;
                src1 = (PixelYCA_fbbs*)((int)src1 + buf_step1);
                src2 = (PixelYCA_fbbs*)((int)src2 + buf_step1);

                dst->y = sum_y * f_inv_range;
                int round_c0 = half_range;
                int round_c1 = round_c0;
                if (sum_cb < 0)round_c1 = -round_c1;
                dst->cb = (int8_t)((sum_cb + round_c1) / range);
                if (sum_cr < 0)round_c0 = -round_c0;
                dst->cr = (int8_t)((sum_cr + round_c0) / range);
                dst->a = (int16_t)(sum_a / range);
                dst = (PixelYCA_fbbs*)((int)dst + buf_step1);
            }

            for (int i = blur_size; 0 < i; i--) {
                sum_y -= src2->y;
                sum_cb -= src2->cb;
                sum_cr -= src2->cr;
                sum_a -= src2->a;
                src2 = (PixelYCA_fbbs*)((int)src2 + buf_step1);

                range--;
                dst->y = sum_y / (float)range;
                int round_c0 = range >> 1;
                int round_c1 = round_c0;
                if (sum_cb < 0)round_c1 = -round_c1;
                dst->cb = (int8_t)((sum_cb + round_c1) / range);
                if (sum_cr < 0)round_c0 = -round_c0;
                dst->cr = (int8_t)((sum_cr + round_c0) / range);
                dst->a = (int16_t)(sum_a / range);
                dst = (PixelYCA_fbbs*)((int)dst + buf_step1);
            }
        }
    }
    void __cdecl vertical_yca_fb_cs_mt(int thread_id, int thread_num, int blur_size, ExEdit::FilterProcInfo* efpip) { // 10a30
        blur_yca_fb_cs_mt(thread_id * efpip->obj_w / thread_num, (thread_id + 1) * efpip->obj_w / thread_num, efpip->obj_temp, efpip->obj_edit,
            efpip->obj_line * sizeof(ExEdit::PixelYCA), sizeof(ExEdit::PixelYCA), efpip->obj_h, blur_size);
    }
    void __cdecl horizontal_yca_fb_cs_mt(int thread_id, int thread_num, int blur_size, ExEdit::FilterProcInfo* efpip) { // 10f20
        blur_yca_fb_cs_mt(thread_id * efpip->obj_h / thread_num, (thread_id + 1) * efpip->obj_h / thread_num, efpip->obj_temp, efpip->obj_edit,
            sizeof(ExEdit::PixelYCA), efpip->obj_line * sizeof(ExEdit::PixelYCA), efpip->obj_w, blur_size);
    }


    void blur_yca_fb_cs_mt1(int n_begin, int n_end, void* buf_dst, void* buf_src, int buf_step1, int buf_step2, int obj_size, int blur_size) {
        int loop2 = obj_size - blur_size;
        int loop3 = blur_size * 2 - obj_size;
        int offset = n_begin * buf_step2;

        fastBlurYCAfbs256 fb256;
        fb256.offset = _mm256_mullo_epi32(_mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0), _mm256_set1_epi32(buf_step2));
        int n;
        for (n = n_end - n_begin; 8 <= n; n -= 8) {
            auto dst = (PixelYCA_fbbs*)((int)buf_dst + offset);
            auto src1 = (PixelYCA_fbbs*)((int)buf_src + offset);
            auto src2 = src1;
            offset += buf_step2 * 8;

            fb256.y = _mm256_setzero_ps();
            fb256.cb = fb256.cr = fb256.a = _mm256_setzero_si256();
            for (int i = blur_size; 0 < i; i--) {
                ycafbs256_add(&fb256, src1);
                src1 = (PixelYCA_fbbs*)((int)src1 + buf_step1);
            }
            int range = blur_size;
            for (int i = loop2; 0 < i; i--) {
                ycafbs256_add(&fb256, src1);
                src1 = (PixelYCA_fbbs*)((int)src1 + buf_step1);

                range++;
                ycfb256_new_range((fastBlurYCfb256*)&fb256, range);
                ycafbs256_put_average(&fb256, dst, buf_step2);
                dst = (PixelYCA_fbbs*)((int)dst + buf_step1);
            }
            auto dst1 = dst;
            for (int j = 7; 0 <= j; j--) {
                dst = (PixelYCA_fbbs*)((int)dst1 + j * buf_step2);
                auto yca = *(PixelYCA_fbbs*)((int)dst - buf_step1);
                for (int i = loop3; 0 < i; i--) {
                    *dst = yca;
                    dst = (PixelYCA_fbbs*)((int)dst + buf_step1);
                }
            }

            for (int i = loop2; 0 < i; i--) {
                ycafbs256_sub(&fb256, src2);
                src2 = (PixelYCA_fbbs*)((int)src2 + buf_step1);

                range--;
                ycfb256_new_range((fastBlurYCfb256*)&fb256, range);
                ycafbs256_put_average(&fb256, dst, buf_step2);
                dst = (PixelYCA_fbbs*)((int)dst + buf_step1);
            }
        }
        for (; 0 < n; n--) {
            auto dst = (PixelYCA_fbbs*)((int)buf_dst + offset);
            auto src1 = (PixelYCA_fbbs*)((int)buf_src + offset);
            auto src2 = src1;
            offset += buf_step2;

            float sum_y = 0.0f;
            int sum_cb = 0;
            int sum_cr = 0;
            int sum_a = 0;
            for (int i = blur_size; 0 < i; i--) {
                sum_y += src1->y;
                sum_cb += src1->cb;
                sum_cr += src1->cr;
                sum_a += src1->a;
                src1 = (PixelYCA_fbbs*)((int)src1 + buf_step1);
            }
            int range = blur_size;
            for (int i = loop2; 0 < i; i--) {
                sum_y += src1->y;
                sum_cb += src1->cb;
                sum_cr += src1->cr;
                sum_a += src1->a;
                src1 = (PixelYCA_fbbs*)((int)src1 + buf_step1);

                range++;
                dst->y = sum_y / (float)range;
                int round_c0 = range >> 1;
                int round_c1 = round_c0;
                if (sum_cb < 0)round_c1 = -round_c1;
                dst->cb = (int8_t)((sum_cb + round_c1) / range);
                if (sum_cr < 0)round_c0 = -round_c0;
                dst->cr = (int8_t)((sum_cr + round_c0) / range);
                dst->a = (int16_t)(sum_a / range);
                dst = (PixelYCA_fbbs*)((int)dst + buf_step1);
            }

            auto yca = *(PixelYCA_fbbs*)((int)dst - buf_step1);
            for (int i = loop3; 0 < i; i--) {
                *dst = yca;
                dst = (PixelYCA_fbbs*)((int)dst + buf_step1);
            }

            for (int i = loop2; 0 < i; i--) {
                sum_y -= src2->y;
                sum_cb -= src2->cb;
                sum_cr -= src2->cr;
                sum_a -= src2->a;
                src2 = (PixelYCA_fbbs*)((int)src2 + buf_step1);

                range--;
                dst->y = sum_y / (float)range;
                int round_c0 = range >> 1;
                int round_c1 = round_c0;
                if (sum_cb < 0)round_c1 = -round_c1;
                dst->cb = (int8_t)((sum_cb + round_c1) / range);
                if (sum_cr < 0)round_c0 = -round_c0;
                dst->cr = (int8_t)((sum_cr + round_c0) / range);
                dst->a = (int16_t)(sum_a / range);
                dst = (PixelYCA_fbbs*)((int)dst + buf_step1);
            }
        }
    }
    void __cdecl vertical_yca_fb_cs_mt1(int thread_id, int thread_num, int blur_size, ExEdit::FilterProcInfo* efpip) { // 10a30
        blur_yca_fb_cs_mt1(thread_id * efpip->obj_w / thread_num, (thread_id + 1) * efpip->obj_w / thread_num, efpip->obj_temp, efpip->obj_edit,
            efpip->obj_line * sizeof(ExEdit::PixelYCA), sizeof(ExEdit::PixelYCA), efpip->obj_h, blur_size);
    }
    void __cdecl horizontal_yca_fb_cs_mt1(int thread_id, int thread_num, int blur_size, ExEdit::FilterProcInfo* efpip) { // 10f20
        blur_yca_fb_cs_mt1(thread_id * efpip->obj_h / thread_num, (thread_id + 1) * efpip->obj_h / thread_num, efpip->obj_temp, efpip->obj_edit,
            sizeof(ExEdit::PixelYCA), efpip->obj_line * sizeof(ExEdit::PixelYCA), efpip->obj_w, blur_size);
    }

    void blur_yca_fb_cs_mt2(int n_begin, int n_end, void* buf_dst, void* buf_src, int buf_step1, int buf_step2, int obj_size) {
        int offset = n_begin * buf_step2;

        fastBlurYCAfbs256 fb256;
        ycfb256_new_range((fastBlurYCfb256*)&fb256, obj_size);
        fb256.offset = _mm256_mullo_epi32(_mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0), _mm256_set1_epi32(buf_step2));
        __m256i i2b256 = _mm256_set1_epi32(0xff);
        int n;
        for (n = n_end - n_begin; 8 <= n; n -= 8) {
            auto src = (PixelYCA_fbbs*)((int)buf_src + offset);

            fb256.y = _mm256_setzero_ps();
            fb256.cb = fb256.cr = fb256.a = _mm256_setzero_si256();
            for (int i = obj_size; 0 < i; i--) {
                ycafbs256_add(&fb256, src);
                src = (PixelYCA_fbbs*)((int)src + buf_step1);
            }

            __m256 ave_y256 = _mm256_mul_ps(fb256.y, fb256.invrange);

            __m256i flag256 = _mm256_srai_epi32(fb256.cb, 31);
            __m256i round256 = _mm256_xor_si256(fb256.halfrange, flag256);
            round256 = _mm256_add_epi32(fb256.cb, _mm256_sub_epi32(round256, flag256));
            __m256i ave_cb256 = _mm256_and_si256(_mm256_div_epi32(round256, fb256.range), i2b256);

            flag256 = _mm256_srai_epi32(fb256.cr, 31);
            round256 = _mm256_xor_si256(fb256.halfrange, flag256);
            round256 = _mm256_add_epi32(fb256.cr, _mm256_sub_epi32(round256, flag256));
            __m256i ave_cr256 = _mm256_and_si256(_mm256_div_epi32(round256, fb256.range), i2b256);
            __m256i cbcra256 = _mm256_or_si256(ave_cb256, _mm256_slli_epi32(ave_cr256, 8));
            __m256i ave_a256 = _mm256_div_epi32(fb256.a, fb256.range);
            cbcra256 = _mm256_or_si256(cbcra256, _mm256_slli_epi32(ave_a256, 16));

            for (int nn = 0; nn < 8; nn++) {
                auto dst = (PixelYCA_fbbs*)((int)buf_dst + offset);
                float y = ave_y256.m256_f32[nn];
                int cbcra = cbcra256.m256i_i32[nn];
                for (int i = obj_size; 0 < i; i--) {
                    dst->y = y;
                    *(int*)&dst->cb = cbcra;
                    dst = (PixelYCA_fbbs*)((int)dst + buf_step1);
                }
                offset += buf_step2;
            }
        }
        float f_inv_range = 1.0f / (float)obj_size;
        int half_range = obj_size >> 1;
        for (; 0 < n; n--) {
            auto src = (PixelYCA_fbbs*)((int)buf_src + offset);

            float sum_y = 0.0f;
            int sum_cb = 0;
            int sum_cr = 0;
            int sum_a = 0;
            for (int i = obj_size; 0 < i; i--) {
                sum_y += src->y;
                sum_cb += src->cb;
                sum_cr += src->cr;
                sum_a += src->a;
                src = (PixelYCA_fbbs*)((int)src + buf_step1);
            }
            auto dst = (PixelYCA_fbbs*)((int)buf_dst + offset);
            int round_c0 = half_range;
            int round_c1 = round_c0;
            if (sum_cb < 0)round_c1 = -round_c1;
            if (sum_cr < 0)round_c0 = -round_c0;
            PixelYCA_fbbs yca = {
                sum_y * f_inv_range,
                (int8_t)((sum_cb + round_c1) / obj_size),
                (int8_t)((sum_cr + round_c0) / obj_size),
                (int16_t)(sum_a / obj_size)
            };
            for (int i = obj_size; 0 < i; i--) {
                *dst = yca;
                dst = (PixelYCA_fbbs*)((int)dst + buf_step1);
            }
            offset += buf_step2;
        }
    }
    void __cdecl vertical_yca_fb_cs_mt2(int thread_id, int thread_num, int n0, ExEdit::FilterProcInfo* efpip) { // 10a30
        blur_yca_fb_cs_mt2(thread_id * efpip->obj_w / thread_num, (thread_id + 1) * efpip->obj_w / thread_num, efpip->obj_temp, efpip->obj_edit,
            efpip->obj_line * sizeof(ExEdit::PixelYCA), sizeof(ExEdit::PixelYCA), efpip->obj_h);
    }
    void __cdecl horizontal_yca_fb_cs_mt2(int thread_id, int thread_num, int n0, ExEdit::FilterProcInfo* efpip) { // 10f20
        blur_yca_fb_cs_mt2(thread_id * efpip->obj_h / thread_num, (thread_id + 1) * efpip->obj_h / thread_num, efpip->obj_temp, efpip->obj_edit,
            sizeof(ExEdit::PixelYCA), efpip->obj_line * sizeof(ExEdit::PixelYCA), efpip->obj_w);
    }


    void mt_calc_yc_fbbs(int thread_id, int thread_num, int n1, ExEdit::FilterProcInfo* efpip) {
        for (int y = thread_id; y < efpip->obj_h; y += thread_num) {
            auto src = (PixelYCA_fbbs*)efpip->obj_edit + y * efpip->obj_line;
            for (int x = efpip->obj_w; 0 < x; x--) {
                int src_a = src->a;
                if (src_a <= 0) {
                    *(int32_t*)&src->cb = *(int32_t*)&src->y = 0;
                } else if (src_a < 0x1000) {
                    src->y *= (float)src_a * 0.000244140625f; // 1/4096
                    src->cb = (int8_t)(src->cb * src_a >> 12);
                    src->cr = (int8_t)(src->cr * src_a >> 12);
                }
                src++;
            }
        }
    }
    void mt_calc_yca_fbbs(int thread_id, int thread_num, int n1, ExEdit::FilterProcInfo* efpip) {
        for (int y = thread_id; y < efpip->obj_h; y += thread_num) {
            auto src = (PixelYCA_fbbs*)efpip->obj_edit + y * efpip->obj_line;
            for (int x = efpip->obj_w; 0 < x; x--) {
                int src_a = src->a;
                if (0 < src_a && src_a < 0x1000) {
                    src->y *= 4096.0f / (float)src_a;
                    int round_c0 = src_a >> 1;
                    int round_c1 = round_c0;
                    if (src->cb < 0)round_c0 = -round_c0;
                    src->cb = (int8_t)((((int)src->cb << 12) + round_c0) / src_a);
                    if (src->cr < 0)round_c1 = -round_c1;
                    src->cr = (int8_t)((((int)src->cr << 12) + round_c1) / src_a);
                }
                src++;
            }
        }
    }




    void __declspec(noinline) __fastcall Blur_t::yc256_add(fastBlurYC256* fb256, ExEdit::PixelYC* src) {
        __m256i ycb256 = _mm256_i32gather_epi32((int*)src, fb256->offset, 1);
        __m256i y256 = _mm256_srai_epi32(_mm256_slli_epi32(ycb256, 16), 16);
        fb256->y = _mm256_add_epi32(fb256->y, y256);
        __m256i cb256 = _mm256_srai_epi32(ycb256, 16);
        fb256->cb = _mm256_add_epi32(fb256->cb, cb256);
        __m256i cbcr256 = _mm256_i32gather_epi32((int*)&src->cb, fb256->offset, 1);
        __m256i cr256 = _mm256_srai_epi32(cbcr256, 16);
        fb256->cr = _mm256_add_epi32(fb256->cr, cr256);
    }
    void __declspec(noinline) __fastcall Blur_t::yc256_sub(fastBlurYC256* fb256, ExEdit::PixelYC* src) {
        __m256i ycb256 = _mm256_i32gather_epi32((int*)src, fb256->offset, 1);
        __m256i y256 = _mm256_srai_epi32(_mm256_slli_epi32(ycb256, 16), 16);
        fb256->y = _mm256_sub_epi32(fb256->y, y256);
        __m256i cb256 = _mm256_srai_epi32(ycb256, 16);
        fb256->cb = _mm256_sub_epi32(fb256->cb, cb256);
        __m256i cbcr256 = _mm256_i32gather_epi32((int*)&src->cb, fb256->offset, 1);
        __m256i cr256 = _mm256_srai_epi32(cbcr256, 16);
        fb256->cr = _mm256_sub_epi32(fb256->cr, cr256);
    }
    void __declspec(noinline) __fastcall Blur_t::yc256_put_average(fastBlurYC256* fb256, ExEdit::PixelYC* dst, int buf_step2) {
        __m256i ave_y256 = _mm256_div_epi32(fb256->y, fb256->range);
        __m256i ave_cb256 = _mm256_div_epi32(fb256->cb, fb256->range);
        __m256i ave_cr256 = _mm256_div_epi32(fb256->cr, fb256->range);
        for (int i = 0; i < 16; i += 2) {
            dst->y = ave_y256.m256i_i16[i];
            dst->cb = ave_cb256.m256i_i16[i];
            dst->cr = ave_cr256.m256i_i16[i];
            dst = (ExEdit::PixelYC*)((int)dst + buf_step2);
        }
    }

    void blur_yc_mt(int n_begin, int n_end, void* buf_dst, void* buf_src, int buf_step1, int buf_step2, int obj_size, int blur_size) {
        int loop3 = obj_size - (blur_size * 2 + 1);
        int offset = n_begin * buf_step2;

        Blur_t::fastBlurYC256 fb256;
        fb256.offset = _mm256_mullo_epi32(_mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0), _mm256_set1_epi32(buf_step2));
        int n;
        for (n = n_end - n_begin; 8 <= n; n -= 8) {
            auto dst = (ExEdit::PixelYC*)((int)buf_dst + offset);
            auto src1 = (ExEdit::PixelYC*)((int)buf_src + offset);
            auto src2 = src1;
            offset += buf_step2 * 8;

            fb256.y = fb256.cb = fb256.cr = _mm256_setzero_si256();
            for (int i = blur_size; 0 < i; i--) {
                Blur_t::yc256_add(&fb256, src1);
                src1 = (ExEdit::PixelYC*)((int)src1 + buf_step1);
            }
            int range = blur_size;
            for (int i = blur_size; 0 <= i; i--) {
                Blur_t::yc256_add(&fb256, src1);
                src1 = (ExEdit::PixelYC*)((int)src1 + buf_step1);

                range++;
                fb256.range = _mm256_set1_epi32(range);
                Blur_t::yc256_put_average(&fb256, dst, buf_step2);
                dst = (ExEdit::PixelYC*)((int)dst + buf_step1);
            }
            for (int i = loop3; 0 < i; i--) {
                Blur_t::yc256_sub(&fb256, src2);
                src2 = (ExEdit::PixelYC*)((int)src2 + buf_step1);
                Blur_t::yc256_add(&fb256, src1);
                src1 = (ExEdit::PixelYC*)((int)src1 + buf_step1);

                Blur_t::yc256_put_average(&fb256, dst, buf_step2);
                dst = (ExEdit::PixelYC*)((int)dst + buf_step1);
            }
            for (int i = blur_size; 0 < i; i--) {
                Blur_t::yc256_sub(&fb256, src2);
                src2 = (ExEdit::PixelYC*)((int)src2 + buf_step1);

                range--;
                fb256.range = _mm256_set1_epi32(range);
                Blur_t::yc256_put_average(&fb256, dst, buf_step2);
                dst = (ExEdit::PixelYC*)((int)dst + buf_step1);
            }
        }
        for (; 0 < n; n--) {
            auto dst = (ExEdit::PixelYC*)((int)buf_dst + offset);
            auto src1 = (ExEdit::PixelYC*)((int)buf_src + offset);
            auto src2 = src1;
            offset += buf_step2;

            int sum_y = 0;
            int sum_cb = 0;
            int sum_cr = 0;
            for (int i = blur_size; 0 < i; i--) {
                sum_y += src1->y;
                sum_cb += src1->cb;
                sum_cr += src1->cr;
                src1 = (ExEdit::PixelYC*)((int)src1 + buf_step1);
            }
            int range = blur_size;
            for (int i = blur_size; 0 <= i; i--) {
                sum_y += src1->y;
                sum_cb += src1->cb;
                sum_cr += src1->cr;
                src1 = (ExEdit::PixelYC*)((int)src1 + buf_step1);

                range++;
                dst->y = (short)(sum_y / range);
                dst->cb = (short)(sum_cb / range);
                dst->cr = (short)(sum_cr / range);
                dst = (ExEdit::PixelYC*)((int)dst + buf_step1);
            }
            for (int i = loop3; 0 < i; i--) {
                sum_y += src1->y - src2->y;
                sum_cb += src1->cb - src2->cb;
                sum_cr += src1->cr - src2->cr;
                src1 = (ExEdit::PixelYC*)((int)src1 + buf_step1);
                src2 = (ExEdit::PixelYC*)((int)src2 + buf_step1);

                dst->y = (short)(sum_y / range);
                dst->cb = (short)(sum_cb / range);
                dst->cr = (short)(sum_cr / range);
                dst = (ExEdit::PixelYC*)((int)dst + buf_step1);
            }
            for (int i = blur_size; 0 < i; i--) {
                sum_y -= src2->y;
                sum_cb -= src2->cb;
                sum_cr -= src2->cr;
                src2 = (ExEdit::PixelYC*)((int)src2 + buf_step1);

                range--;
                dst->y = (short)(sum_y / range);
                dst->cb = (short)(sum_cb / range);
                dst->cr = (short)(sum_cr / range);
                dst = (ExEdit::PixelYC*)((int)dst + buf_step1);
            }
        }
    }
    void __cdecl Blur_t::vertical_yc_mt_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) { // fcb0
        auto blur = (efBlur_var*)(GLOBAL::exedit_base + OFS::ExEdit::efBlur_var_ptr);
        blur_yc_mt(thread_id * efpip->scene_w / thread_num, (thread_id + 1) * efpip->scene_w / thread_num, efpip->frame_temp, efpip->frame_edit,
            efpip->scene_line * sizeof(ExEdit::PixelYC), sizeof(ExEdit::PixelYC), efpip->scene_h, blur->h);
    }
    void __cdecl Blur_t::horizontal_yc_mt_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) { // ff40
        auto blur = (efBlur_var*)(GLOBAL::exedit_base + OFS::ExEdit::efBlur_var_ptr);
        blur_yc_mt(thread_id * efpip->scene_h / thread_num, (thread_id + 1) * efpip->scene_h / thread_num, efpip->frame_temp, efpip->frame_edit,
            sizeof(ExEdit::PixelYC), efpip->scene_line * sizeof(ExEdit::PixelYC), efpip->scene_w, blur->w);
    }


    struct fastBlurYCA256 {
        __m256i range;
        __m256i y, cb, cr, a;
        __m256i offset;
    };

    void __declspec(noinline) __fastcall yca256_add(fastBlurYCA256* fb256, ExEdit::PixelYCA* src) {
        __m256i ycb256 = _mm256_i32gather_epi32((int*)src, fb256->offset, 1);
        __m256i y256 = _mm256_srai_epi32(_mm256_slli_epi32(ycb256, 16), 16);
        fb256->y = _mm256_add_epi32(fb256->y, y256);
        __m256i cb256 = _mm256_srai_epi32(ycb256, 16);
        fb256->cb = _mm256_add_epi32(fb256->cb, cb256);
        __m256i cra256 = _mm256_i32gather_epi32((int*)&src->cr, fb256->offset, 1);
        __m256i cr256 = _mm256_srai_epi32(_mm256_slli_epi32(cra256, 16), 16);
        fb256->cr = _mm256_add_epi32(fb256->cr, cr256);
        __m256i a256 = _mm256_srai_epi32(cra256, 16);
        fb256->a = _mm256_add_epi32(fb256->a, a256);
    }
    void __declspec(noinline) __fastcall yca256_sub(fastBlurYCA256* fb256, ExEdit::PixelYCA* src) {
        __m256i ycb256 = _mm256_i32gather_epi32((int*)src, fb256->offset, 1);
        __m256i y256 = _mm256_srai_epi32(_mm256_slli_epi32(ycb256, 16), 16);
        fb256->y = _mm256_sub_epi32(fb256->y, y256);
        __m256i cb256 = _mm256_srai_epi32(ycb256, 16);
        fb256->cb = _mm256_sub_epi32(fb256->cb, cb256);
        __m256i cra256 = _mm256_i32gather_epi32((int*)&src->cr, fb256->offset, 1);
        __m256i cr256 = _mm256_srai_epi32(_mm256_slli_epi32(cra256, 16), 16);
        fb256->cr = _mm256_sub_epi32(fb256->cr, cr256);
        __m256i a256 = _mm256_srai_epi32(cra256, 16);
        fb256->a = _mm256_sub_epi32(fb256->a, a256);
    }
    void __declspec(noinline) __fastcall yca256_put_average(fastBlurYCA256* fb256, ExEdit::PixelYCA* dst, int buf_step2) {
        __m256i ave_y256 = _mm256_div_epi32(fb256->y, fb256->range);
        __m256i ave_cb256 = _mm256_div_epi32(fb256->cb, fb256->range);
        __m256i ave_cr256 = _mm256_div_epi32(fb256->cr, fb256->range);
        __m256i ave_a256 = _mm256_div_epi32(fb256->a, fb256->range);
        for (int i = 0; i < 16; i += 2) {
            dst->y = ave_y256.m256i_i16[i];
            dst->cb = ave_cb256.m256i_i16[i];
            dst->cr = ave_cr256.m256i_i16[i];
            dst->a = ave_a256.m256i_i16[i];
            dst = (ExEdit::PixelYCA*)((int)dst + buf_step2);
        }
    }



    void blur_yca_mt(int n_begin, int n_end, void* buf_dst, void* buf_src, int buf_step1, int buf_step2, int obj_size, int blur_range) {
        int loop2 = obj_size - blur_range;
        int offset = n_begin * buf_step2;
        fastBlurYCA256 fb256;
        fb256.range = _mm256_set1_epi32(blur_range);
        fb256.offset = _mm256_mullo_epi32(_mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0), _mm256_set1_epi32(buf_step2));
        int n;
        for (n = n_end - n_begin; 8 <= n; n -= 8) {
            auto dst = (ExEdit::PixelYCA*)((int)buf_dst + offset);
            auto src1 = (ExEdit::PixelYCA*)((int)buf_src + offset);
            auto src2 = src1;
            offset += buf_step2 * 8;

            fb256.y = fb256.cb = fb256.cr = fb256.a = _mm256_setzero_si256();
            for (int i = blur_range; 0 < i; i--) {
                yca256_add(&fb256, src1);
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);

                yca256_put_average(&fb256, dst, buf_step2);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
            for (int i = loop2; 0 < i; i--) {
                yca256_sub(&fb256, src2);
                src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);
                yca256_add(&fb256, src1);
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);

                yca256_put_average(&fb256, dst, buf_step2);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
            for (int i = blur_range - 1; 0 < i; i--) {
                yca256_sub(&fb256, src2);
                src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);

                yca256_put_average(&fb256, dst, buf_step2);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
        }
        for (; 0 < n; n--) {
            auto dst = (ExEdit::PixelYCA*)((int)buf_dst + offset);
            auto src1 = (ExEdit::PixelYCA*)((int)buf_src + offset);
            auto src2 = src1;
            offset += buf_step2;

            int sum_y = 0;
            int sum_cb = 0;
            int sum_cr = 0;
            int sum_a = 0;
            for (int i = blur_range; 0 < i; i--) {
                sum_y += src1->y;
                sum_cb += src1->cb;
                sum_cr += src1->cr;
                sum_a += src1->a;
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);

                dst->y = (short)(sum_y / blur_range);
                dst->cb = (short)(sum_cb / blur_range);
                dst->cr = (short)(sum_cr / blur_range);
                dst->a = (short)(sum_a / blur_range);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
            for (int i = loop2; 0 < i; i--) {
                sum_y += src1->y - src2->y;
                sum_cb += src1->cb - src2->cb;
                sum_cr += src1->cr - src2->cr;
                sum_a += src1->a - src2->a;
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);
                src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);

                dst->y = (short)(sum_y / blur_range);
                dst->cb = (short)(sum_cb / blur_range);
                dst->cr = (short)(sum_cr / blur_range);
                dst->a = (short)(sum_a / blur_range);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
            for (int i = blur_range - 1; 0 < i; i--) {
                sum_y -= src2->y;
                sum_cb -= src2->cb;
                sum_cr -= src2->cr;
                sum_a -= src2->a;
                src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);

                dst->y = (short)(sum_y / blur_range);
                dst->cb = (short)(sum_cb / blur_range);
                dst->cr = (short)(sum_cr / blur_range);
                dst->a = (short)(sum_y / blur_range);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
        }
    }
    void __cdecl vertical_yca_mt(int thread_id, int thread_num, int blur_range, ExEdit::FilterProcInfo* efpip) { // eae0
        blur_yca_mt(thread_id * efpip->obj_w / thread_num, (thread_id + 1) * efpip->obj_w / thread_num, efpip->obj_temp, efpip->obj_edit,
            efpip->obj_line * sizeof(ExEdit::PixelYCA), sizeof(ExEdit::PixelYCA), efpip->obj_h, blur_range);
    }
    void __cdecl horizontal_yca_mt(int thread_id, int thread_num, int blur_range, ExEdit::FilterProcInfo* efpip) { // eef0
        blur_yca_mt(thread_id * efpip->obj_h / thread_num, (thread_id + 1) * efpip->obj_h / thread_num, efpip->obj_temp, efpip->obj_edit,
            sizeof(ExEdit::PixelYCA), efpip->obj_line * sizeof(ExEdit::PixelYCA), efpip->obj_w, blur_range);
    }



    void blur_yca_cs_mt(int n_begin, int n_end, void* buf_dst, void* buf_src, int buf_step1, int buf_step2, int obj_size, int blur_size) {
        int loop3 = obj_size - (blur_size * 2 + 1);
        int offset = n_begin * buf_step2;

        fastBlurYCA256 fb256;
        fb256.offset = _mm256_mullo_epi32(_mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0), _mm256_set1_epi32(buf_step2));
        int n;
        for (n = n_end - n_begin; 8 <= n; n -= 8) {
            auto dst = (ExEdit::PixelYCA*)((int)buf_dst + offset);
            auto src1 = (ExEdit::PixelYCA*)((int)buf_src + offset);
            auto src2 = src1;
            offset += buf_step2 * 8;

            fb256.y = fb256.cb = fb256.cr = fb256.a = _mm256_setzero_si256();
            for (int i = blur_size; 0 < i; i--) {
                yca256_add(&fb256, src1);
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);
            }
            int range = blur_size;
            for (int i = blur_size; 0 <= i; i--) {
                yca256_add(&fb256, src1);
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);

                range++;
                fb256.range = _mm256_set1_epi32(range);
                yca256_put_average(&fb256, dst, buf_step2);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
            for (int i = loop3; 0 < i; i--) {
                yca256_sub(&fb256, src2);
                src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);
                yca256_add(&fb256, src1);
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);

                yca256_put_average(&fb256, dst, buf_step2);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
            for (int i = blur_size; 0 < i; i--) {
                yca256_sub(&fb256, src2);
                src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);

                range--;
                fb256.range = _mm256_set1_epi32(range);
                yca256_put_average(&fb256, dst, buf_step2);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
        }
        for (; 0 < n; n--) {
            auto dst = (ExEdit::PixelYCA*)((int)buf_dst + offset);
            auto src1 = (ExEdit::PixelYCA*)((int)buf_src + offset);
            auto src2 = src1;
            offset += buf_step2;

            int sum_y = 0;
            int sum_cb = 0;
            int sum_cr = 0;
            int sum_a = 0;
            for (int i = blur_size; 0 < i; i--) {
                sum_y += src1->y;
                sum_cb += src1->cb;
                sum_cr += src1->cr;
                sum_a += src1->a;
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);
            }
            int range = blur_size;
            for (int i = blur_size; 0 <= i; i--) {
                sum_y += src1->y;
                sum_cb += src1->cb;
                sum_cr += src1->cr;
                sum_a += src1->a;
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);

                range++;
                dst->y = (short)(sum_y / range);
                dst->cb = (short)(sum_cb / range);
                dst->cr = (short)(sum_cr / range);
                dst->a = (short)(sum_a / range);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
            for (int i = loop3; 0 < i; i--) {
                sum_y += src1->y - src2->y;
                sum_cb += src1->cb - src2->cb;
                sum_cr += src1->cr - src2->cr;
                sum_a += src1->a - src2->a;
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);
                src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);

                dst->y = (short)(sum_y / range);
                dst->cb = (short)(sum_cb / range);
                dst->cr = (short)(sum_cr / range);
                dst->a = (short)(sum_a / range);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
            for (int i = blur_size; 0 < i; i--) {
                sum_y -= src2->y;
                sum_cb -= src2->cb;
                sum_cr -= src2->cr;
                sum_a -= src2->a;
                src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);

                range--;
                dst->y = (short)(sum_y / range);
                dst->cb = (short)(sum_cb / range);
                dst->cr = (short)(sum_cr / range);
                dst->a = (short)(sum_a / range);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
        }
    }
    void __cdecl vertical_yca_cs_mt(int thread_id, int thread_num, int blur_size, ExEdit::FilterProcInfo* efpip) { // f310
        blur_yca_cs_mt(thread_id * efpip->obj_w / thread_num, (thread_id + 1) * efpip->obj_w / thread_num, efpip->obj_temp, efpip->obj_edit,
            efpip->obj_line * sizeof(ExEdit::PixelYCA), sizeof(ExEdit::PixelYCA), efpip->obj_h, blur_size);
    }
    void __cdecl horizontal_yca_cs_mt(int thread_id, int thread_num, int blur_size, ExEdit::FilterProcInfo* efpip) { // f7e0
        blur_yca_cs_mt(thread_id * efpip->obj_h / thread_num, (thread_id + 1) * efpip->obj_h / thread_num, efpip->obj_temp, efpip->obj_edit,
            sizeof(ExEdit::PixelYCA), efpip->obj_line * sizeof(ExEdit::PixelYCA), efpip->obj_w, blur_size);
    }


    void blur_yca_cs_mt1(int n_begin, int n_end, void* buf_dst, void* buf_src, int buf_step1, int buf_step2, int obj_size, int blur_size) {
        int loop2 = obj_size - blur_size;
        int loop3 = blur_size * 2 - obj_size;
        int offset = n_begin * buf_step2;

        fastBlurYCA256 fb256;
        fb256.offset = _mm256_mullo_epi32(_mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0), _mm256_set1_epi32(buf_step2));
        int n;
        for (n = n_end - n_begin; 8 <= n; n -= 8) {
            auto dst = (ExEdit::PixelYCA*)((int)buf_dst + offset);
            auto src1 = (ExEdit::PixelYCA*)((int)buf_src + offset);
            auto src2 = src1;
            offset += buf_step2 * 8;

            fb256.y = fb256.cb = fb256.cr = fb256.a = _mm256_setzero_si256();
            for (int i = blur_size; 0 < i; i--) {
                yca256_add(&fb256, src1);
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);
            }
            int range = blur_size;
            for (int i = loop2; 0 < i; i--) {
                yca256_add(&fb256, src1);
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);

                range++;
                fb256.range = _mm256_set1_epi32(range);
                yca256_put_average(&fb256, dst, buf_step2);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
            auto dst1 = dst;
            for (int j = 7; 0 <= j; j--) {
                dst = (ExEdit::PixelYCA*)((int)dst1 + j * buf_step2);
                auto yca = *(ExEdit::PixelYCA*)((int)dst - buf_step1);
                for (int i = loop3; 0 < i; i--) {
                    *dst = yca;
                    dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
                }
            }
            for (int i = loop2; 0 < i; i--) {
                yca256_sub(&fb256, src2);
                src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);

                range--;
                fb256.range = _mm256_set1_epi32(range);
                yca256_put_average(&fb256, dst, buf_step2);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
        }
        for (; 0 < n; n--) {
            auto dst = (ExEdit::PixelYCA*)((int)buf_dst + offset);
            auto src1 = (ExEdit::PixelYCA*)((int)buf_src + offset);
            auto src2 = src1;
            offset += buf_step2;

            int sum_y = 0;
            int sum_cb = 0;
            int sum_cr = 0;
            int sum_a = 0;
            for (int i = blur_size; 0 < i; i--) {
                sum_y += src1->y;
                sum_cb += src1->cb;
                sum_cr += src1->cr;
                sum_a += src1->a;
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);
            }
            int range = blur_size;
            for (int i = loop2; 0 < i; i--) {
                sum_y += src1->y;
                sum_cb += src1->cb;
                sum_cr += src1->cr;
                sum_a += src1->a;
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);

                range++;
                dst->y = (short)(sum_y / range);
                dst->cb = (short)(sum_cb / range);
                dst->cr = (short)(sum_cr / range);
                dst->a = (short)(sum_a / range);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
            auto yca = *(ExEdit::PixelYCA*)((int)dst - buf_step1);
            for (int i = loop3; 0 < i; i--) {
                *dst = yca;
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
            for (int i = loop2; 0 < i; i--) {
                sum_y -= src2->y;
                sum_cb -= src2->cb;
                sum_cr -= src2->cr;
                sum_a -= src2->a;
                src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);

                range--;
                dst->y = (short)(sum_y / range);
                dst->cb = (short)(sum_cb / range);
                dst->cr = (short)(sum_cr / range);
                dst->a = (short)(sum_a / range);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
        }
    }

    void __cdecl vertical_yca_cs_mt1(int thread_id, int thread_num, int blur_size, ExEdit::FilterProcInfo* efpip) {
        blur_yca_cs_mt1(thread_id * efpip->obj_w / thread_num, (thread_id + 1) * efpip->obj_w / thread_num, efpip->obj_temp, efpip->obj_edit,
            efpip->obj_line * sizeof(ExEdit::PixelYCA), sizeof(ExEdit::PixelYCA), efpip->obj_h, blur_size);
    }
    void __cdecl horizontal_yca_cs_mt1(int thread_id, int thread_num, int blur_size, ExEdit::FilterProcInfo* efpip) {
        blur_yca_cs_mt1(thread_id * efpip->obj_h / thread_num, (thread_id + 1) * efpip->obj_h / thread_num, efpip->obj_temp, efpip->obj_edit,
            sizeof(ExEdit::PixelYCA), efpip->obj_line * sizeof(ExEdit::PixelYCA), efpip->obj_w, blur_size);
    }

    void blur_yca_cs_mt2(int n_begin, int n_end, void* buf_dst, void* buf_src, int buf_step1, int buf_step2, int obj_size) {
        int offset = n_begin * buf_step2;

        fastBlurYCA256 fb256;
        fb256.range = _mm256_set1_epi32(obj_size);
        fb256.offset = _mm256_mullo_epi32(_mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0), _mm256_set1_epi32(buf_step2));
        __m256i i2s256 = _mm256_set1_epi32(0xffff);
        int n;
        for (n = n_end - n_begin; 8 <= n; n -= 8) {
            auto src = (ExEdit::PixelYCA*)((int)buf_src + offset);

            fb256.y = fb256.cb = fb256.cr = fb256.a = _mm256_setzero_si256();
            for (int i = obj_size; 0 < i; i--) {
                yca256_add(&fb256, src);
                src = (ExEdit::PixelYCA*)((int)src + buf_step1);
            }
            __m256i ave_y256 = _mm256_div_epi32(fb256.y, fb256.range);
            __m256i ave_cb256 = _mm256_div_epi32(fb256.cb, fb256.range);
            __m256i ycb256 = _mm256_or_si256(_mm256_and_si256(ave_y256, i2s256), _mm256_slli_epi32(ave_cb256, 16));

            __m256i ave_cr256 = _mm256_div_epi32(fb256.cr, fb256.range);
            __m256i ave_a256 = _mm256_div_epi32(fb256.a, fb256.range);
            __m256i cra256 = _mm256_or_si256(_mm256_and_si256(ave_cr256, i2s256), _mm256_slli_epi32(ave_a256, 16));

            for (int nn = 0; nn < 8; nn++) {
                auto dst = (int*)((int)buf_dst + offset);
                int ycb = ycb256.m256i_i32[nn];
                int cra = cra256.m256i_i32[nn];
                for (int i = obj_size; 0 < i; i--) {
                    dst[0] = ycb;
                    dst[1] = cra;
                    dst = (int*)((int)dst + buf_step1);
                }
                offset += buf_step2;
            }
        }
        for (; 0 < n; n--) {
            auto src = (ExEdit::PixelYCA*)((int)buf_src + offset);

            int sum_y = 0;
            int sum_cb = 0;
            int sum_cr = 0;
            int sum_a = 0;
            for (int i = obj_size; 0 < i; i--) {
                sum_y += src->y;
                sum_cb += src->cb;
                sum_cr += src->cr;
                sum_a += src->a;
                src = (ExEdit::PixelYCA*)((int)src + buf_step1);
            }
            auto dst = (ExEdit::PixelYCA*)((int)buf_dst + offset);
            ExEdit::PixelYCA yca = {
                (int16_t)(sum_y / obj_size),
                (int16_t)(sum_cb / obj_size),
                (int16_t)(sum_cr / obj_size),
                (int16_t)(sum_a / obj_size)
            };
            for (int i = obj_size; 0 < i; i--) {
                *dst = yca;
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
            offset += buf_step2;
        }
    }
    void __cdecl vertical_yca_cs_mt2(int thread_id, int thread_num, int n0, ExEdit::FilterProcInfo* efpip) {
        blur_yca_cs_mt2(thread_id * efpip->obj_w / thread_num, (thread_id + 1) * efpip->obj_w / thread_num, efpip->obj_temp, efpip->obj_edit,
            efpip->obj_line * sizeof(ExEdit::PixelYCA), sizeof(ExEdit::PixelYCA), efpip->obj_h);
    }
    void __cdecl horizontal_yca_cs_mt2(int thread_id, int thread_num, int n0, ExEdit::FilterProcInfo* efpip) {
        blur_yca_cs_mt2(thread_id * efpip->obj_h / thread_num, (thread_id + 1) * efpip->obj_h / thread_num, efpip->obj_temp, efpip->obj_edit,
            sizeof(ExEdit::PixelYCA), efpip->obj_line * sizeof(ExEdit::PixelYCA), efpip->obj_w);
    }




    void mt_calc_yc_ssss(int thread_id, int thread_num, void* n1, ExEdit::FilterProcInfo* efpip) {
        for (int y = thread_id; y < efpip->obj_h; y += thread_num) {
            auto src = (ExEdit::PixelYCA*)efpip->obj_edit + y * efpip->obj_line;
            for (int x = efpip->obj_w; 0 < x; x--) {
                int src_a = src->a;
                if (src_a <= 0) {
                    *(int32_t*)&src->cr = *(int32_t*)&src->y = 0;
                } else if (src_a < 0x1000) {
                    src->y = src->y * src_a >> 12;
                    src->cb = src->cb * src_a >> 12;
                    src->cr = src->cr * src_a >> 12;
                }
                src++;
            }
        }
    }
    void mt_calc_yca_ssss(int thread_id, int thread_num, void* n1, ExEdit::FilterProcInfo* efpip) {
        for (int y = thread_id; y < efpip->obj_h; y += thread_num) {
            auto src = (ExEdit::PixelYCA*)efpip->obj_edit + y * efpip->obj_line;
            for (int x = efpip->obj_w; 0 < x; x--) {
                int src_a = src->a;
                if (0 < src_a && src_a < 0x1000) {
                    src->y = ((int)src->y << 12) / src_a;
                    src->cb = ((int)src->cb << 12) / src_a;
                    src->cr = ((int)src->cr << 12) / src_a;
                }
                src++;
            }
        }
    }






    BOOL __cdecl Blur_t::efBlur_effect_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        int blur_size = efp->track[0];
        if (blur_size <= 0) {
            return TRUE;
        }
        int blur_w = blur_size;
        int blur_h = blur_size;
        int aspect = efp->track[1];
        if (aspect) {
            if (aspect < 0) {
                blur_w = blur_w * (1000 + aspect) / 1000;
            } else {
                blur_h = blur_h * (1000 - aspect) / 1000;
            }
        }
        int check0 = efp->check[0];
        if (check0 == 0) {
            blur_w = min(blur_w, (*(int*)(GLOBAL::exedit_base + OFS::ExEdit::exedit_max_w) - efpip->obj_w) / 2);
            blur_h = min(blur_h, (efpip->obj_max_h - efpip->obj_h) / 2);
        }

        int conv2_w = blur_w / 2;
        int conv2_h = blur_h / 2;
        int conv1_w = blur_w - conv2_w;
        int conv1_h = blur_h - conv2_h;
        if (check0 == 0 && (efpip->obj_w <= conv1_w * 2 || efpip->obj_w <= conv2_w * 2 || efpip->obj_h <= conv1_h * 2 || efpip->obj_h <= conv2_h * 2)) {
            int new_w = efpip->obj_w + (conv1_w + conv2_w) * 2;
            int new_h = efpip->obj_h + (conv1_h + conv2_h) * 2;
            check0 = 1;
            efp->exfunc->fill(efpip->obj_temp, 0, 0, new_w, new_h, 0, 0, 0, 0, 2);
            efp->exfunc->bufcpy(efpip->obj_temp, conv2_w + conv1_w, conv2_h + conv1_h, efpip->obj_edit, 0, 0, efpip->obj_w, efpip->obj_h, 0, 0x13000003);
            std::swap(efpip->obj_edit, efpip->obj_temp);
            efpip->obj_w = new_w;
            efpip->obj_h = new_h;
        }
        int intensity = efp->track[2];
        if (0 < intensity) {
            reinterpret_cast<void (__cdecl*)(void*, int, int, int)>(GLOBAL::exedit_base + OFS::ExEdit::PixelYCA_ssss2fbbs)(efpip->obj_edit, efpip->obj_w, efpip->obj_h, intensity);
            efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)mt_calc_yc_fbbs, NULL, efpip);
            if (conv1_h) {
                if (check0 == 0) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_yca_fb_mt, (void*)(conv1_h * 2 + 1), efpip);
                    efpip->obj_h += conv1_h * 2;
                } else if (conv1_h * 2 + 1 < efpip->obj_h) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_yca_fb_cs_mt, (void*)conv1_h, efpip);
                } else if (conv1_h < efpip->obj_h) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_yca_fb_cs_mt1, (void*)conv1_h, efpip);
                } else {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_yca_fb_cs_mt2, NULL, efpip);
                }
                std::swap(efpip->obj_edit, efpip->obj_temp);
            }
            if (conv1_w) {
                if (check0 == 0) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_yca_fb_mt, (void*)(conv1_w * 2 + 1), efpip);
                    efpip->obj_w += conv1_w * 2;
                } else if (conv1_w * 2 + 1 < efpip->obj_w) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_yca_fb_cs_mt, (void*)conv1_w, efpip);
                } else if (conv1_w < efpip->obj_w) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_yca_fb_cs_mt1, (void*)conv1_w, efpip);
                } else {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_yca_fb_cs_mt2, NULL, efpip);
                }
                std::swap(efpip->obj_edit, efpip->obj_temp);
            }
            if (conv2_h) {
                if (check0 == 0) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_yca_fb_mt, (void*)(conv2_h * 2 + 1), efpip);
                    efpip->obj_h += conv2_h * 2;
                } else if (conv2_h * 2 + 1 < efpip->obj_h) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_yca_fb_cs_mt, (void*)conv2_h, efpip);
                } else if (conv2_h < efpip->obj_h) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_yca_fb_cs_mt1, (void*)conv2_h, efpip);
                } else {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_yca_fb_cs_mt2, NULL, efpip);
                }
                std::swap(efpip->obj_edit, efpip->obj_temp);
            }
            if (conv2_w) {
                if (check0 == 0) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_yca_fb_mt, (void*)(conv2_w * 2 + 1), efpip);
                    efpip->obj_w += conv2_w * 2;
                } else if (conv2_w * 2 + 1 < efpip->obj_w) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_yca_fb_cs_mt, (void*)conv2_w, efpip);
                } else if (conv2_w < efpip->obj_w) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_yca_fb_cs_mt1, (void*)conv2_w, efpip);
                } else {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_yca_fb_cs_mt2, NULL, efpip);
                }
                std::swap(efpip->obj_edit, efpip->obj_temp);

            }
            efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)mt_calc_yca_fbbs, NULL, efpip);
            reinterpret_cast<void(__cdecl*)(void*, int, int, int)>(GLOBAL::exedit_base + OFS::ExEdit::PixelYCA_fbbs2ssss)(efpip->obj_edit, efpip->obj_w, efpip->obj_h, intensity);
        } else {
            efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)mt_calc_yc_ssss, NULL, efpip);
            if (conv1_h) {
                if (check0 == 0) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_yca_mt, (void*)(conv1_h * 2 + 1), efpip);
                    efpip->obj_h += conv1_h * 2;
                } else if (conv1_h * 2 + 1 < efpip->obj_h) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_yca_cs_mt, (void*)conv1_h, efpip);
                } else if (conv1_h < efpip->obj_h) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_yca_cs_mt1, (void*)conv1_h, efpip);
                } else {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_yca_cs_mt2, NULL, efpip);
                }
                std::swap(efpip->obj_edit, efpip->obj_temp);

            }
            if (conv1_w) {
                if (check0 == 0) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_yca_mt, (void*)(conv1_w * 2 + 1), efpip);
                    efpip->obj_w += conv1_w * 2;
                } else if (conv1_w * 2 + 1 < efpip->obj_w) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_yca_cs_mt, (void*)conv1_w, efpip);
                } else if (conv1_w < efpip->obj_w) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_yca_cs_mt1, (void*)conv1_w, efpip);
                } else {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_yca_cs_mt2, NULL, efpip);
                }
                std::swap(efpip->obj_edit, efpip->obj_temp);

            }
            if (conv2_h) {
                if (check0 == 0) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_yca_mt, (void*)(conv2_h * 2 + 1), efpip);
                    efpip->obj_h += conv2_h * 2;
                } else if (conv2_h * 2 + 1 < efpip->obj_h) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_yca_cs_mt, (void*)conv2_h, efpip);
                } else if (conv2_h < efpip->obj_h) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_yca_cs_mt1, (void*)conv2_h, efpip);
                } else {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_yca_cs_mt2, NULL, efpip);
                }
                std::swap(efpip->obj_edit, efpip->obj_temp);

            }
            if (conv2_w) {
                if (check0 == 0) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_yca_mt, (void*)(conv2_w * 2 + 1), efpip);
                    efpip->obj_w += conv2_w * 2;
                } else if (conv2_w * 2 + 1 < efpip->obj_w) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_yca_cs_mt, (void*)conv2_w, efpip);
                } else if (conv2_w < efpip->obj_w) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_yca_cs_mt1, (void*)conv2_w, efpip);
                } else {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_yca_cs_mt2, NULL, efpip);
                }
                std::swap(efpip->obj_edit, efpip->obj_temp);
            }
            efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)mt_calc_yca_ssss, NULL, efpip);
        }
        return TRUE;
    }


}
#endif // ifdef PATCH_SWITCH_FAST_BLUR
