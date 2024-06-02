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


#pragma pack(push, 1)
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
#pragma pack(pop)


#define BYTE_DIV_BIT 24

    void __cdecl Blur_t::vertical_yc_fb_cs_mt(int thread_id, int thread_num, int w, int h, int line, int blur_size, void* dst_ptr, void* src_ptr, void* mem_ptr, int flag) {
        int loop[7];
        loop[2] = loop[5] = 0;
        if (blur_size * 2 < h) {
            loop[0] = blur_size;
            loop[1] = blur_size + 1;
            loop[3] = h - blur_size * 2 - 1;
            loop[4] = 0;
            loop[6] = blur_size;
        } else {
            int size = min(blur_size, h - 1);
            loop[0] = size;
            loop[1] = h - size;
            loop[3] = 0;
            loop[4] = size * 2 + 1 - h;
            loop[6] = h - size - 1;
        }
        int range_bytediv2mul;
        double range_doublediv2mul;
        if (flag & 1) {
            std::swap(loop[1], loop[2]);
            std::swap(loop[5], loop[6]);
            range_bytediv2mul = (1 << BYTE_DIV_BIT) / (blur_size * 2 + 1);
            range_doublediv2mul = 1.0 / (double)(blur_size * 2 + 1);
        }

        int x = thread_id * w / thread_num;
        auto src1 = reinterpret_cast<PixelYC_fbb*>(src_ptr) + x;
        auto src2 = src1;
        auto dst = reinterpret_cast<PixelYC_fbb*>(dst_ptr) + x;
        struct {
            double y;
            int32_t cb, cr;
        }*cnv0 = reinterpret_cast<decltype(cnv0)>((reinterpret_cast<int>(mem_ptr) + 7) & 0xfffffff8) + x;
        int thw = (thread_id + 1) * w / thread_num - x;
        memset(cnv0, 0, thw * sizeof(*cnv0));
        int step = (line - thw) * sizeof(*dst);

        for (int y = loop[0]; 0 < y; y--) {
            auto cnv = cnv0;
            for (x = thw; 0 < x; x--) {
                cnv->y += src1->y;
                cnv->cb += src1->cb;
                cnv->cr += src1->cr;
                cnv++; src1++;
            }
            src1 = reinterpret_cast<decltype(src1)>(reinterpret_cast<int>(src1) + step);
        }
        int range = loop[0];
        for (int y = loop[1]; 0 < y; y--) { // flag 0
            range++;
            range_doublediv2mul = 1.0 / (double)range;
            range_bytediv2mul = (1 << BYTE_DIV_BIT) / range;
            auto cnv = cnv0;
            for (x = thw; 0 < x; x--) {
                cnv->y += src1->y;
                cnv->cb += src1->cb;
                cnv->cr += src1->cr;

                dst->y = (float)(cnv->y * range_doublediv2mul);
                dst->cb = (int8_t)((cnv->cb * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->cr = (int8_t)((cnv->cr * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);

                dst++; cnv++; src1++;
            }
            dst = reinterpret_cast<decltype(dst)>(reinterpret_cast<int>(dst) + step);
            src1 = reinterpret_cast<decltype(src1)>(reinterpret_cast<int>(src1) + step);
        }
        for (int y = loop[2]; 0 < y; y--) { // flag 1
            auto cnv = cnv0;
            for (x = thw; 0 < x; x--) {
                cnv->y += src1->y;
                cnv->cb += src1->cb;
                cnv->cr += src1->cr;

                dst->y = (float)(cnv->y * range_doublediv2mul);
                dst->cb = (int8_t)((cnv->cb * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->cr = (int8_t)((cnv->cr * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);

                dst++; cnv++; src1++;
            }
            dst = reinterpret_cast<decltype(dst)>(reinterpret_cast<int>(dst) + step);
            src1 = reinterpret_cast<decltype(src1)>(reinterpret_cast<int>(src1) + step);
        }

        for (int y = loop[3]; 0 < y; y--) {
            auto cnv = cnv0;
            for (x = thw; 0 < x; x--) {
                cnv->y += src1->y - src2->y;
                cnv->cb += src1->cb - src2->cb;
                cnv->cr += src1->cr - src2->cr;

                dst->y = (float)(cnv->y * range_doublediv2mul);
                dst->cb = (int8_t)((cnv->cb * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->cr = (int8_t)((cnv->cr * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);

                dst++; cnv++; src1++; src2++;
            }
            dst = reinterpret_cast<decltype(dst)>(reinterpret_cast<int>(dst) + step);
            src1 = reinterpret_cast<decltype(src1)>(reinterpret_cast<int>(src1) + step);
            src2 = reinterpret_cast<decltype(src2)>(reinterpret_cast<int>(src2) + step);
        }
        for (int y = loop[4]; 0 < y; y--) {
            memcpy(dst, dst - line, thw * sizeof(*dst));
            dst += line;
        }
        for (int y = loop[5]; 0 < y; y--) { // flag 1
            auto cnv = cnv0;
            for (x = thw; 0 < x; x--) {
                cnv->y -= src2->y;
                cnv->cb -= src2->cb;
                cnv->cr -= src2->cr;

                dst->y = (float)(cnv->y * range_doublediv2mul);
                dst->cb = (int8_t)((cnv->cb * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->cr = (int8_t)((cnv->cr * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);

                dst++; cnv++; src2++;
            }
            dst = reinterpret_cast<decltype(dst)>(reinterpret_cast<int>(dst) + step);
            src2 = reinterpret_cast<decltype(src2)>(reinterpret_cast<int>(src2) + step);
        }
        for (int y = loop[6]; 0 < y; y--) { // flag 0
            range--;
            range_bytediv2mul = (1 << BYTE_DIV_BIT) / range;
            range_doublediv2mul = 1.0 / (double)range;
            auto cnv = cnv0;
            for (x = thw; 0 < x; x--) {
                cnv->y -= src2->y;
                cnv->cb -= src2->cb;
                cnv->cr -= src2->cr;

                dst->y = (float)(cnv->y * range_doublediv2mul);
                dst->cb = (int8_t)((cnv->cb * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->cr = (int8_t)((cnv->cr * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);

                dst++; cnv++; src2++;
            }
            dst = reinterpret_cast<decltype(dst)>(reinterpret_cast<int>(dst) + step);
            src2 = reinterpret_cast<decltype(src2)>(reinterpret_cast<int>(src2) + step);
        }
    }
    void __cdecl Blur_t::vertical_yc_fb_cs_mt_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) { // 11400
        auto blur = (efBlur_var*)(GLOBAL::exedit_base + OFS::ExEdit::efBlur_var_ptr);
        vertical_yc_fb_cs_mt(thread_id, thread_num, efpip->scene_w, efpip->scene_h, efpip->scene_line, blur->h, efpip->frame_temp, efpip->frame_edit, *reinterpret_cast<void**>(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr), 0);
    }
    void __cdecl Blur_t::horizontal_yc_fb_cs_mt(int thread_id, int thread_num, int w, int h, int line, int blur_size, void* dst_ptr, void* src_ptr, void* mem_ptr, int flag) {
        int loop[7];
        loop[2] = loop[5] = 0;
        if (blur_size * 2 < w) {
            loop[0] = blur_size;
            loop[1] = blur_size + 1;
            loop[3] = w - blur_size * 2 - 1;
            loop[4] = 0;
            loop[6] = blur_size;
        } else {
            int size = min(blur_size, w - 1);
            loop[0] = size;
            loop[1] = w - size;
            loop[3] = 0;
            loop[4] = size * 2 + 1 - w;
            loop[6] = w - size - 1;
        }
        int range_bytediv2mul;
        double range_doublediv2mul;
        int* range_bytediv2mul_arr = NULL;
        double* range_doublediv2mul_arr = NULL;
        if (flag & 1) {
            std::swap(loop[1], loop[2]);
            std::swap(loop[5], loop[6]);
            range_bytediv2mul = (1 << BYTE_DIV_BIT) / (blur_size * 2 + 1);
            range_doublediv2mul = 1.0 / (double)(blur_size * 2 + 1);
        } else {
            int e = loop[0] + 1 + loop[1];
            range_bytediv2mul_arr = reinterpret_cast<int*>((reinterpret_cast<int>(mem_ptr) + 3) & 0xfffffffc);
            range_doublediv2mul_arr = reinterpret_cast<double*>((reinterpret_cast<int>(range_bytediv2mul_arr + e) + 7) & 0xfffffff8);
            for (int range = loop[0] + 1; range < e; range++) {
                range_bytediv2mul_arr[range] = (1 << BYTE_DIV_BIT) / range;
                range_doublediv2mul_arr[range] = 1.0 / (double)range;
            }
        }

        int y = thread_id * h / thread_num;

        auto src0 = reinterpret_cast<PixelYC_fbb*>(src_ptr) + y * line;
        auto dst0 = reinterpret_cast<PixelYC_fbb*>(dst_ptr) + y * line;
        int step = line * sizeof(*dst0);

        for (y = (thread_id + 1) * h / thread_num - y; 0 < y; y--) {
            auto src1 = src0;
            auto src2 = src1;
            src0 = reinterpret_cast<decltype(src0)>(reinterpret_cast<int>(src0) + step);
            auto dst = dst0;
            dst0 = reinterpret_cast<decltype(dst0)>(reinterpret_cast<int>(dst0) + step);
            double cnv_y = 0.0;
            int cnv_cb = 0;
            int cnv_cr = 0;
            for (int i = loop[0]; 0 < i; i--) {
                cnv_y += src1->y;
                cnv_cb += src1->cb;
                cnv_cr += src1->cr;
                src1++;
            }
            int range = loop[0];
            for (int i = loop[1]; 0 < i; i--) { // flag 0
                cnv_y += src1->y;
                cnv_cb += src1->cb;
                cnv_cr += src1->cr;

                range++;
                range_bytediv2mul = range_bytediv2mul_arr[range];
                range_doublediv2mul = range_doublediv2mul_arr[range];

                dst->y = (float)(cnv_y * range_doublediv2mul);
                dst->cb = (int8_t)((cnv_cb * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->cr = (int8_t)((cnv_cr * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);

                dst++; src1++;
            }
            for (int i = loop[2]; 0 < i; i--) { // flag 1
                cnv_y += src1->y;
                cnv_cb += src1->cb;
                cnv_cr += src1->cr;

                dst->y = (float)(cnv_y * range_doublediv2mul);
                dst->cb = (int8_t)((cnv_cb * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->cr = (int8_t)((cnv_cr * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);

                dst++; src1++;
            }

            for (int i = loop[3]; 0 < i; i--) {
                cnv_y += src1->y - src2->y;
                cnv_cb += src1->cb - src2->cb;
                cnv_cr += src1->cr - src2->cr;

                dst->y = (float)(cnv_y * range_doublediv2mul);
                dst->cb = (int8_t)((cnv_cb * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->cr = (int8_t)((cnv_cr * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);

                dst++; src1++; src2++;
            }
            if (0 < loop[4]) {
                short yca[3] = { ((short*)dst)[-3], ((short*)dst)[-2], ((short*)dst)[-1] };
                for (int i = loop[4]; 0 < i; i--) {
                    ((short*)dst)[0] = yca[0];
                    ((short*)dst)[1] = yca[1];
                    ((short*)dst)[2] = yca[2];
                    dst++;
                }
            }
            for (int i = loop[5]; 0 < i; i--) { // flag 1
                cnv_y -= src2->y;
                cnv_cb -= src2->cb;
                cnv_cr -= src2->cr;

                dst->y = (float)(cnv_y * range_doublediv2mul);
                dst->cb = (int8_t)((cnv_cb * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->cr = (int8_t)((cnv_cr * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);

                dst++; src2++;
            }
            for (int i = loop[6]; 0 < i; i--) { // flag 0
                cnv_y -= src2->y;
                cnv_cb -= src2->cb;
                cnv_cr -= src2->cr;

                range--;
                range_bytediv2mul = range_bytediv2mul_arr[range];
                range_doublediv2mul = range_doublediv2mul_arr[range];

                dst->y = (float)(cnv_y * range_doublediv2mul);
                dst->cb = (int8_t)((cnv_cb * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->cr = (int8_t)((cnv_cr * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);

                dst++; src2++;
            }
        }
    }
    void __cdecl Blur_t::horizontal_yc_fb_cs_mt_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) { // 116d0
        auto blur = (efBlur_var*)(GLOBAL::exedit_base + OFS::ExEdit::efBlur_var_ptr);
        horizontal_yc_fb_cs_mt(thread_id, thread_num, efpip->scene_w, efpip->scene_h, efpip->scene_line, blur->w, efpip->frame_temp, efpip->frame_edit, *reinterpret_cast<void**>(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr), 0);
    }


#define SHORT_DIV_BIT 19
    void __cdecl Blur_t::vertical_yca_fb_mt(int thread_id, int thread_num, int w, int h, int line, int blur_size, void* dst_ptr, void* src_ptr, void* mem_ptr, int flag) {
        int range_bytediv2mul;
        int range_shortdiv2mul;
        double range_doublediv2mul;
        int loop[7];
        if (flag & 2) {
            range_bytediv2mul = (1 << BYTE_DIV_BIT) / (blur_size * 2 + 1);
            range_shortdiv2mul = (1 << SHORT_DIV_BIT) / (blur_size * 2 + 1);
            range_doublediv2mul = 1.0 / (double)(blur_size * 2 + 1);
            loop[0] = loop[1] = loop[6] = 0;
            if (blur_size * 2 + 1 < h) {
                loop[2] = blur_size * 2 + 1;
                loop[3] = h - blur_size * 2 - 1;
                loop[4] = 0;
                loop[5] = blur_size * 2 + 1 - 1;
            } else {
                loop[2] = h;
                loop[3] = 0;
                loop[4] = blur_size * 2 + 1 - h;
                loop[5] = h - 1;
            }
        } else {
            loop[2] = loop[5] = 0;
            if (blur_size * 2 < h) {
                loop[0] = blur_size;
                loop[1] = blur_size + 1;
                loop[3] = h - blur_size * 2 - 1;
                loop[4] = 0;
                loop[6] = blur_size;
            } else {
                blur_size = min(blur_size, h - 1);
                loop[0] = blur_size;
                loop[1] = h - blur_size;
                loop[3] = 0;
                loop[4] = blur_size * 2 + 1 - h;
                loop[6] = h - blur_size - 1;
            }
        }

        int x = thread_id * w / thread_num;
        auto src1 = reinterpret_cast<PixelYCA_fbbs*>(src_ptr) + x;
        auto src2 = src1;
        auto dst = reinterpret_cast<PixelYCA_fbbs*>(dst_ptr) + x;
        struct {
            double y;
            int32_t cb, cr;
            uint32_t a;
            int32_t _padding;
        }*cnv0 = reinterpret_cast<decltype(cnv0)>((reinterpret_cast<int>(mem_ptr) + 7) & 0xfffffff8) + x;
        int thw = (thread_id + 1) * w / thread_num - x;
        memset(cnv0, 0, thw * sizeof(*cnv0));
        int step = (line - thw) * sizeof(*dst);

        for (int y = loop[0]; 0 < y; y--) { // flag 0
            auto cnv = cnv0;
            for (x = thw; 0 < x; x--) {
                cnv->y += src1->y;
                cnv->cb += src1->cb;
                cnv->cr += src1->cr;
                cnv->a += src1->a;
                cnv++; src1++;
            }
            src1 = reinterpret_cast<decltype(src1)>(reinterpret_cast<int>(src1) + step);
        }
        int range = loop[0];
        for (int y = loop[1]; 0 < y; y--) { // flag 0
            range++;
            range_doublediv2mul = 1.0 / (double)range;
            range_shortdiv2mul = (1 << SHORT_DIV_BIT) / range;
            range_bytediv2mul = (1 << BYTE_DIV_BIT) / range;
            auto cnv = cnv0;
            for (x = thw; 0 < x; x--) {
                cnv->y += src1->y;
                cnv->cb += src1->cb;
                cnv->cr += src1->cr;
                cnv->a += src1->a;

                dst->y = (float)(cnv->y * range_doublediv2mul);
                dst->cb = (int8_t)((cnv->cb * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->cr = (int8_t)((cnv->cr * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->a = (int16_t)((uint32_t)(cnv->a * range_shortdiv2mul) >> SHORT_DIV_BIT);

                dst++; cnv++; src1++;
            }
            dst = reinterpret_cast<decltype(dst)>(reinterpret_cast<int>(dst) + step);
            src1 = reinterpret_cast<decltype(src1)>(reinterpret_cast<int>(src1) + step);
        }
        for (int y = loop[2]; 0 < y; y--) { // flag 2
            auto cnv = cnv0;
            for (x = thw; 0 < x; x--) {
                cnv->y += src1->y;
                cnv->cb += src1->cb;
                cnv->cr += src1->cr;
                cnv->a += src1->a;

                dst->y = (float)(cnv->y * range_doublediv2mul);
                dst->cb = (int8_t)((cnv->cb * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->cr = (int8_t)((cnv->cr * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->a = (int16_t)((uint32_t)(cnv->a * range_shortdiv2mul) >> SHORT_DIV_BIT);

                dst++; cnv++; src1++;
            }
            dst = reinterpret_cast<decltype(dst)>(reinterpret_cast<int>(dst) + step);
            src1 = reinterpret_cast<decltype(src1)>(reinterpret_cast<int>(src1) + step);
        }

        for (int y = loop[3]; 0 < y; y--) {
            auto cnv = cnv0;
            for (x = thw; 0 < x; x--) {
                cnv->y += src1->y - src2->y;
                cnv->cb += src1->cb - src2->cb;
                cnv->cr += src1->cr - src2->cr;
                cnv->a += src1->a - src2->a;

                dst->y = (float)(cnv->y * range_doublediv2mul);
                dst->cb = (int8_t)((cnv->cb * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->cr = (int8_t)((cnv->cr * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->a = (int16_t)((uint32_t)(cnv->a * range_shortdiv2mul) >> SHORT_DIV_BIT);

                dst++; cnv++; src1++; src2++;
            }
            dst = reinterpret_cast<decltype(dst)>(reinterpret_cast<int>(dst) + step);
            src1 = reinterpret_cast<decltype(src1)>(reinterpret_cast<int>(src1) + step);
            src2 = reinterpret_cast<decltype(src2)>(reinterpret_cast<int>(src2) + step);
        }
        for (int y = loop[4]; 0 < y; y--) {
            memcpy(dst, dst - line, thw * sizeof(*dst));
            dst += line;
        }
        for (int y = loop[5]; 0 < y; y--) { // flag 2
            auto cnv = cnv0;
            for (x = thw; 0 < x; x--) {
                cnv->y -= src2->y;
                cnv->cb -= src2->cb;
                cnv->cr -= src2->cr;
                cnv->a -= src2->a;

                dst->y = (float)(cnv->y * range_doublediv2mul);
                dst->cb = (int8_t)((cnv->cb * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->cr = (int8_t)((cnv->cr * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->a = (int16_t)((uint32_t)(cnv->a * range_shortdiv2mul) >> SHORT_DIV_BIT);

                dst++; cnv++; src2++;
            }
            dst = reinterpret_cast<decltype(dst)>(reinterpret_cast<int>(dst) + step);
            src2 = reinterpret_cast<decltype(src2)>(reinterpret_cast<int>(src2) + step);
        }
        for (int y = loop[6]; 0 < y; y--) { // flag 0
            range--;
            range_bytediv2mul = (1 << BYTE_DIV_BIT) / range;
            range_shortdiv2mul = (1 << SHORT_DIV_BIT) / range;
            range_doublediv2mul = 1.0 / (double)range;
            auto cnv = cnv0;
            for (x = thw; 0 < x; x--) {
                cnv->y -= src2->y;
                cnv->cb -= src2->cb;
                cnv->cr -= src2->cr;
                cnv->a -= src2->a;

                dst->y = (float)(cnv->y * range_doublediv2mul);
                dst->cb = (int8_t)((cnv->cb * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->cr = (int8_t)((cnv->cr * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->a = (int16_t)((uint32_t)(cnv->a * range_shortdiv2mul) >> SHORT_DIV_BIT);

                dst++; cnv++; src2++;
            }
            dst = reinterpret_cast<decltype(dst)>(reinterpret_cast<int>(dst) + step);
            src2 = reinterpret_cast<decltype(src2)>(reinterpret_cast<int>(src2) + step);
        }
    }
    void __cdecl vertical_yca_fb_mt_wrap(int thread_id, int thread_num, int blur_size, ExEdit::FilterProcInfo* efpip) { // 10190
        Blur_t::vertical_yca_fb_mt(thread_id, thread_num, efpip->obj_w, efpip->obj_h, efpip->obj_line, blur_size, efpip->obj_temp, efpip->obj_edit, *reinterpret_cast<void**>(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr), 2);
    }
    void __cdecl vertical_yca_fb_cs_mt_wrap(int thread_id, int thread_num, int blur_size, ExEdit::FilterProcInfo* efpip) { // 10a30
        Blur_t::vertical_yca_fb_mt(thread_id, thread_num, efpip->obj_w, efpip->obj_h, efpip->obj_line, blur_size, efpip->obj_temp, efpip->obj_edit, *reinterpret_cast<void**>(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr), 0);
    }

    void __cdecl Blur_t::horizontal_yca_fb_mt(int thread_id, int thread_num, int w, int h, int line, int blur_size, void* dst_ptr, void* src_ptr, void* mem_ptr, int flag) {
        int loop[7];
        if (flag & 2) {
            loop[0] = loop[1] = loop[6] = 0;
            if (blur_size * 2 + 1 < w) {
                loop[2] = blur_size * 2 + 1;
                loop[3] = w - blur_size * 2 - 1;
                loop[4] = 0;
                loop[5] = blur_size * 2 + 1 - 1;
            } else {
                loop[2] = w;
                loop[3] = 0;
                loop[4] = blur_size * 2 + 1 - w;
                loop[5] = w - 1;
            }
        } else {
            loop[2] = loop[5] = 0;
            if (blur_size * 2 < w) {
                loop[0] = blur_size;
                loop[1] = blur_size + 1;
                loop[3] = w - blur_size * 2 - 1;
                loop[4] = 0;
                loop[6] = blur_size;
            } else {
                blur_size = min(blur_size, w - 1);
                loop[0] = blur_size;
                loop[1] = w - blur_size;
                loop[3] = 0;
                loop[4] = blur_size * 2 + 1 - w;
                loop[6] = w - blur_size - 1;
            }
        }
        int range_bytediv2mul;
        int range_shortdiv2mul;
        double range_doublediv2mul;
        int* range_bytediv2mul_arr = NULL;
        int* range_shortdiv2mul_arr = NULL;
        double* range_doublediv2mul_arr = NULL;
        if (flag & 2) {
            range_bytediv2mul = (1 << BYTE_DIV_BIT) / (blur_size * 2 + 1);
            range_shortdiv2mul = (1 << SHORT_DIV_BIT) / (blur_size * 2 + 1);
            range_doublediv2mul = 1.0 / (double)(blur_size * 2 + 1);
        } else {
            int e = loop[0] + 1 + loop[1];
            range_bytediv2mul_arr = reinterpret_cast<int*>((reinterpret_cast<int>(mem_ptr) + 3) & 0xfffffffc);
            range_shortdiv2mul_arr = reinterpret_cast<int*>(range_bytediv2mul_arr + e);
            range_doublediv2mul_arr = reinterpret_cast<double*>((reinterpret_cast<int>(range_shortdiv2mul_arr + e) + 7) & 0xfffffff8);
            for (int range = loop[0] + 1; range < e; range++) {
                range_bytediv2mul_arr[range] = (1 << BYTE_DIV_BIT) / range;
                range_shortdiv2mul_arr[range] = (1 << SHORT_DIV_BIT) / range;
                range_doublediv2mul_arr[range] = 1.0 / (double)range;
            }
        }

        int y = thread_id * h / thread_num;

        auto src0 = reinterpret_cast<PixelYCA_fbbs*>(src_ptr) + y * line;
        auto dst0 = reinterpret_cast<PixelYCA_fbbs*>(dst_ptr) + y * line;
        int step = line * sizeof(*dst0);

        for (y = (thread_id + 1) * h / thread_num - y; 0 < y; y--) {
            auto src1 = src0;
            auto src2 = src1;
            src0 = reinterpret_cast<decltype(src0)>(reinterpret_cast<int>(src0) + step);
            auto dst = dst0;
            dst0 = reinterpret_cast<decltype(dst0)>(reinterpret_cast<int>(dst0) + step);
            double cnv_y = 0.0;
            int cnv_cb = 0;
            int cnv_cr = 0;
            uint32_t cnv_a = 0;
            for (int i = loop[0]; 0 < i; i--) { // flag 0
                cnv_y += src1->y;
                cnv_cb += src1->cb;
                cnv_cr += src1->cr;
                cnv_a += src1->a;
                src1++;
            }
            int range = loop[0];
            for (int i = loop[1]; 0 < i; i--) { // flag 0
                cnv_y += src1->y;
                cnv_cb += src1->cb;
                cnv_cr += src1->cr;
                cnv_a += src1->a;

                range++;
                range_bytediv2mul = range_bytediv2mul_arr[range];
                range_shortdiv2mul = range_shortdiv2mul_arr[range];
                range_doublediv2mul = range_doublediv2mul_arr[range];

                dst->y = (float)(cnv_y * range_doublediv2mul);
                dst->cb = (int8_t)((cnv_cb * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->cr = (int8_t)((cnv_cr * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->a = (int16_t)((uint32_t)(cnv_a * range_shortdiv2mul) >> SHORT_DIV_BIT);

                dst++; src1++;
            }
            for (int i = loop[2]; 0 < i; i--) { // flag 2
                cnv_y += src1->y;
                cnv_cb += src1->cb;
                cnv_cr += src1->cr;
                cnv_a += src1->a;

                dst->y = (float)(cnv_y * range_doublediv2mul);
                dst->cb = (int8_t)((cnv_cb * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->cr = (int8_t)((cnv_cr * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->a = (int16_t)((uint32_t)(cnv_a * range_shortdiv2mul) >> SHORT_DIV_BIT);

                dst++; src1++;
            }

            for (int i = loop[3]; 0 < i; i--) {
                cnv_y += src1->y - src2->y;
                cnv_cb += src1->cb - src2->cb;
                cnv_cr += src1->cr - src2->cr;
                cnv_a += src1->a - src2->a;

                dst->y = (float)(cnv_y * range_doublediv2mul);
                dst->cb = (int8_t)((cnv_cb * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->cr = (int8_t)((cnv_cr * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->a = (int16_t)((uint32_t)(cnv_a * range_shortdiv2mul) >> SHORT_DIV_BIT);

                dst++; src1++; src2++;
            }
            if (0 < loop[4]) {
                auto yca = dst[-1];
                for (int i = loop[4]; 0 < i; i--) {
                    *dst = yca;
                    dst++;
                }
            }
            for (int i = loop[5]; 0 < i; i--) { // flag 2
                cnv_y -= src2->y;
                cnv_cb -= src2->cb;
                cnv_cr -= src2->cr;
                cnv_a -= src2->a;

                dst->y = (float)(cnv_y * range_doublediv2mul);
                dst->cb = (int8_t)((cnv_cb * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->cr = (int8_t)((cnv_cr * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->a = (int16_t)((uint32_t)(cnv_a * range_shortdiv2mul) >> SHORT_DIV_BIT);

                dst++; src2++;
            }
            for (int i = loop[6]; 0 < i; i--) { // flag 0
                cnv_y -= src2->y;
                cnv_cb -= src2->cb;
                cnv_cr -= src2->cr;
                cnv_a -= src2->a;

                range--;
                range_bytediv2mul = range_bytediv2mul_arr[range];
                range_shortdiv2mul = range_shortdiv2mul_arr[range];
                range_doublediv2mul = range_doublediv2mul_arr[range];

                dst->y = (float)(cnv_y * range_doublediv2mul);
                dst->cb = (int8_t)((cnv_cb * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->cr = (int8_t)((cnv_cr * range_bytediv2mul + (1 << (BYTE_DIV_BIT - 1))) >> BYTE_DIV_BIT);
                dst->a = (int16_t)((uint32_t)(cnv_a * range_shortdiv2mul) >> SHORT_DIV_BIT);

                dst++; src2++;
            }
        }
    }
    void __cdecl horizontal_yca_fb_mt_wrap(int thread_id, int thread_num, int blur_size, ExEdit::FilterProcInfo* efpip) { // 105e0
        Blur_t::horizontal_yca_fb_mt(thread_id, thread_num, efpip->obj_w, efpip->obj_h, efpip->obj_line, blur_size, efpip->obj_temp, efpip->obj_edit, *reinterpret_cast<void**>(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr), 2);
    }
    void __cdecl horizontal_yca_fb_cs_mt_wrap(int thread_id, int thread_num, int blur_size, ExEdit::FilterProcInfo* efpip) { // 10f20
        Blur_t::horizontal_yca_fb_mt(thread_id, thread_num, efpip->obj_w, efpip->obj_h, efpip->obj_line, blur_size, efpip->obj_temp, efpip->obj_edit, *reinterpret_cast<void**>(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr), 0);
    }


    void mt_calc_yc_fbbs(int thread_id, int thread_num, int n1, ExEdit::FilterProcInfo* efpip) {
        for (int y = thread_id; y < efpip->obj_h; y += thread_num) {
            auto src = (PixelYCA_fbbs*)efpip->obj_edit + y * efpip->obj_line;
            for (int x = efpip->obj_w; 0 < x; x--) {
                int src_a = src->a;
                if (src_a == 0) {
                    *(int32_t*)&src->cb = *(int32_t*)&src->y = 0;
                } else if (src_a < 0x1000) {
                    src->y *= (float)src_a * 0.000244140625f; // 1/4096
                    src->cb = (int8_t)((int)src->cb * src_a >> 12);
                    src->cr = (int8_t)((int)src->cr * src_a >> 12);
                } else if (0x1000 < src_a) {
                    src->a = 0x1000;
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
                if (src_a) {
                    src->y *= 4096.0f / (float)src_a;
                    int round_c0 = src_a >> 1;
                    int round_c1 = round_c0;
                    if (src->cb < 0)round_c0 = -round_c0;
                    src->cb = (int8_t)std::clamp((((int)src->cb << 12) + round_c0) / src_a, -128, 127);
                    if (src->cr < 0)round_c1 = -round_c1;
                    src->cr = (int8_t)std::clamp((((int)src->cr << 12) + round_c1) / src_a, -128, 127);
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

    void Blur_t::blur_yc_cs_mt(int n_begin, int n_end, void* buf_dst, void* buf_src, int buf_step1, int buf_step2, int obj_size, int blur_size) {
        blur_size = min(blur_size, obj_size - 1);
        int blur_range = blur_size * 2 + 1;
        int loop[5];
        loop[0] = blur_size;
        if (blur_range < obj_size) {
            loop[1] = blur_size + 1;
            loop[2] = obj_size - blur_range;
            loop[3] = 0;
            loop[4] = blur_size;
        } else {
            loop[1] = obj_size - blur_size;
            loop[2] = 0;
            loop[3] = blur_range - obj_size;
            loop[4] = obj_size - blur_size - 1;
        }
        int offset = n_begin * buf_step2;

        int n = n_end - n_begin;
        if (has_flag(get_CPUCmdSet(), CPUCmdSet::F_AVX2)) {
            Blur_t::fastBlurYC256 fb256;
            fb256.range = _mm256_set1_epi32(blur_range);
            fb256.offset = _mm256_mullo_epi32(_mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0), _mm256_set1_epi32(buf_step2));
            for (; 8 <= n; n -= 8) {
                auto dst = (ExEdit::PixelYC*)((int)buf_dst + offset);
                auto src1 = (ExEdit::PixelYC*)((int)buf_src + offset);
                auto src2 = src1;
                offset += buf_step2 * 8;

                fb256.y = fb256.cb = fb256.cr = _mm256_setzero_si256();
                for (int i = loop[0]; 0 < i; i--) {
                    Blur_t::yc256_add(&fb256, src1);
                    src1 = (ExEdit::PixelYC*)((int)src1 + buf_step1);
                }
                for (int i = loop[1]; 0 < i; i--) {
                    Blur_t::yc256_add(&fb256, src1);
                    src1 = (ExEdit::PixelYC*)((int)src1 + buf_step1);

                    Blur_t::yc256_put_average(&fb256, dst, buf_step2);
                    dst = (ExEdit::PixelYC*)((int)dst + buf_step1);
                }
                for (int i = loop[2]; 0 < i; i--) {
                    Blur_t::yc256_sub(&fb256, src2);
                    src2 = (ExEdit::PixelYC*)((int)src2 + buf_step1);
                    Blur_t::yc256_add(&fb256, src1);
                    src1 = (ExEdit::PixelYC*)((int)src1 + buf_step1);

                    Blur_t::yc256_put_average(&fb256, dst, buf_step2);
                    dst = (ExEdit::PixelYC*)((int)dst + buf_step1);
                }
                if (0 < loop[3]) {
                    auto dst1 = dst;
                    for (int j = 7; 0 <= j; j--) {
                        dst = (ExEdit::PixelYC*)((int)dst1 + j * buf_step2);
                        auto yca = *(ExEdit::PixelYC*)((int)dst - buf_step1);
                        for (int i = loop[3]; 0 < i; i--) {
                            *dst = yca;
                            dst = (ExEdit::PixelYC*)((int)dst + buf_step1);
                        }
                    }
                }
                for (int i = loop[4]; 0 < i; i--) {
                    Blur_t::yc256_sub(&fb256, src2);
                    src2 = (ExEdit::PixelYC*)((int)src2 + buf_step1);

                    Blur_t::yc256_put_average(&fb256, dst, buf_step2);
                    dst = (ExEdit::PixelYC*)((int)dst + buf_step1);
                }
            }
        }

        for (; 0 < n; n--) {
            auto dst = (ExEdit::PixelYC*)((int)buf_dst + offset);
            auto src1 = (ExEdit::PixelYC*)((int)buf_src + offset);
            auto src2 = src1;
            offset += buf_step2;

            int cnv_y = 0;
            int cnv_cb = 0;
            int cnv_cr = 0;
            for (int i = loop[0]; 0 < i; i--) {
                cnv_y += src1->y;
                cnv_cb += src1->cb;
                cnv_cr += src1->cr;
                src1 = (ExEdit::PixelYC*)((int)src1 + buf_step1);
            }
            for (int i = loop[1]; 0 < i; i--) {
                cnv_y += src1->y;
                cnv_cb += src1->cb;
                cnv_cr += src1->cr;
                src1 = (ExEdit::PixelYC*)((int)src1 + buf_step1);

                dst->y = (short)(cnv_y / blur_range);
                dst->cb = (short)(cnv_cb / blur_range);
                dst->cr = (short)(cnv_cr / blur_range);
                dst = (ExEdit::PixelYC*)((int)dst + buf_step1);
            }
            for (int i = loop[2]; 0 < i; i--) {
                cnv_y += src1->y - src2->y;
                cnv_cb += src1->cb - src2->cb;
                cnv_cr += src1->cr - src2->cr;
                src1 = (ExEdit::PixelYC*)((int)src1 + buf_step1);
                src2 = (ExEdit::PixelYC*)((int)src2 + buf_step1);

                dst->y = (short)(cnv_y / blur_range);
                dst->cb = (short)(cnv_cb / blur_range);
                dst->cr = (short)(cnv_cr / blur_range);
                dst = (ExEdit::PixelYC*)((int)dst + buf_step1);
            }
            if (0 < loop[3]) {
                auto yca = *(ExEdit::PixelYC*)((int)dst - buf_step1);
                for (int i = loop[3]; 0 < i; i--) {
                    *dst = yca;
                    dst = (ExEdit::PixelYC*)((int)dst + buf_step1);
                }
            }
            for (int i = loop[4]; 0 < i; i--) {
                cnv_y -= src2->y;
                cnv_cb -= src2->cb;
                cnv_cr -= src2->cr;
                src2 = (ExEdit::PixelYC*)((int)src2 + buf_step1);

                dst->y = (short)(cnv_y / blur_range);
                dst->cb = (short)(cnv_cb / blur_range);
                dst->cr = (short)(cnv_cr / blur_range);
                dst = (ExEdit::PixelYC*)((int)dst + buf_step1);
            }
        }
    }
    void Blur_t::blur_yc_csa_mt(int n_begin, int n_end, void* buf_dst, void* buf_src, int buf_step1, int buf_step2, int obj_size, int blur_size) {
        int loop[5];
        if (blur_size * 2 < obj_size) {
            loop[0] = blur_size;
            loop[1] = blur_size + 1;
            loop[2] = obj_size - blur_size * 2 - 1;
            loop[3] = 0;
            loop[4] = blur_size;
        } else {
            blur_size = min(blur_size, obj_size - 1);
            loop[0] = blur_size;
            loop[1] = obj_size - blur_size;
            loop[2] = 0;
            loop[3] = blur_size * 2 + 1 - obj_size;
            loop[4] = obj_size - blur_size - 1;
        }
        int offset = n_begin * buf_step2;

        int n = n_end - n_begin;
        if (has_flag(get_CPUCmdSet(), CPUCmdSet::F_AVX2)) {
            Blur_t::fastBlurYC256 fb256;
            fb256.offset = _mm256_mullo_epi32(_mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0), _mm256_set1_epi32(buf_step2));
            for (; 8 <= n; n -= 8) {
                auto dst = (ExEdit::PixelYC*)((int)buf_dst + offset);
                auto src1 = (ExEdit::PixelYC*)((int)buf_src + offset);
                auto src2 = src1;
                offset += buf_step2 * 8;

                fb256.y = fb256.cb = fb256.cr = _mm256_setzero_si256();
                for (int i = loop[0]; 0 < i; i--) {
                    Blur_t::yc256_add(&fb256, src1);
                    src1 = (ExEdit::PixelYC*)((int)src1 + buf_step1);
                }
                int range = loop[0];
                for (int i = loop[1]; 0 < i; i--) {
                    Blur_t::yc256_add(&fb256, src1);
                    src1 = (ExEdit::PixelYC*)((int)src1 + buf_step1);

                    range++;
                    fb256.range = _mm256_set1_epi32(range);
                    Blur_t::yc256_put_average(&fb256, dst, buf_step2);
                    dst = (ExEdit::PixelYC*)((int)dst + buf_step1);
                }
                for (int i = loop[2]; 0 < i; i--) {
                    Blur_t::yc256_sub(&fb256, src2);
                    src2 = (ExEdit::PixelYC*)((int)src2 + buf_step1);
                    Blur_t::yc256_add(&fb256, src1);
                    src1 = (ExEdit::PixelYC*)((int)src1 + buf_step1);

                    Blur_t::yc256_put_average(&fb256, dst, buf_step2);
                    dst = (ExEdit::PixelYC*)((int)dst + buf_step1);
                }
                if (0 < loop[3]) {
                    auto dst1 = dst;
                    for (int j = 7; 0 <= j; j--) {
                        dst = (ExEdit::PixelYC*)((int)dst1 + j * buf_step2);
                        auto yca = *(ExEdit::PixelYC*)((int)dst - buf_step1);
                        for (int i = loop[3]; 0 < i; i--) {
                            *dst = yca;
                            dst = (ExEdit::PixelYC*)((int)dst + buf_step1);
                        }
                    }
                }
                for (int i = loop[4]; 0 < i; i--) {
                    Blur_t::yc256_sub(&fb256, src2);
                    src2 = (ExEdit::PixelYC*)((int)src2 + buf_step1);

                    range--;
                    fb256.range = _mm256_set1_epi32(range);
                    Blur_t::yc256_put_average(&fb256, dst, buf_step2);
                    dst = (ExEdit::PixelYC*)((int)dst + buf_step1);
                }
            }
        }

        for (; 0 < n; n--) {
            auto dst = (ExEdit::PixelYC*)((int)buf_dst + offset);
            auto src1 = (ExEdit::PixelYC*)((int)buf_src + offset);
            auto src2 = src1;
            offset += buf_step2;

            int cnv_y = 0;
            int cnv_cb = 0;
            int cnv_cr = 0;
            for (int i = loop[0]; 0 < i; i--) {
                cnv_y += src1->y;
                cnv_cb += src1->cb;
                cnv_cr += src1->cr;
                src1 = (ExEdit::PixelYC*)((int)src1 + buf_step1);
            }
            int range = loop[0];
            for (int i = loop[1]; 0 < i; i--) {
                cnv_y += src1->y;
                cnv_cb += src1->cb;
                cnv_cr += src1->cr;
                src1 = (ExEdit::PixelYC*)((int)src1 + buf_step1);

                range++;
                dst->y = (short)(cnv_y / range);
                dst->cb = (short)(cnv_cb / range);
                dst->cr = (short)(cnv_cr / range);
                dst = (ExEdit::PixelYC*)((int)dst + buf_step1);
            }
            for (int i = loop[2]; 0 < i; i--) {
                cnv_y += src1->y - src2->y;
                cnv_cb += src1->cb - src2->cb;
                cnv_cr += src1->cr - src2->cr;
                src1 = (ExEdit::PixelYC*)((int)src1 + buf_step1);
                src2 = (ExEdit::PixelYC*)((int)src2 + buf_step1);

                dst->y = (short)(cnv_y / range);
                dst->cb = (short)(cnv_cb / range);
                dst->cr = (short)(cnv_cr / range);
                dst = (ExEdit::PixelYC*)((int)dst + buf_step1);
            }
            if (0 < loop[3]) {
                auto yca = *(ExEdit::PixelYC*)((int)dst - buf_step1);
                for (int i = loop[3]; 0 < i; i--) {
                    *dst = yca;
                    dst = (ExEdit::PixelYC*)((int)dst + buf_step1);
                }
            }
            for (int i = loop[4]; 0 < i; i--) {
                cnv_y -= src2->y;
                cnv_cb -= src2->cb;
                cnv_cr -= src2->cr;
                src2 = (ExEdit::PixelYC*)((int)src2 + buf_step1);

                range--;
                dst->y = (short)(cnv_y / range);
                dst->cb = (short)(cnv_cb / range);
                dst->cr = (short)(cnv_cr / range);
                dst = (ExEdit::PixelYC*)((int)dst + buf_step1);
            }
        }
    }
    void __cdecl Blur_t::vertical_yc_csa_mt_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) { // fcb0
        auto blur = (efBlur_var*)(GLOBAL::exedit_base + OFS::ExEdit::efBlur_var_ptr);
        blur_yc_csa_mt(thread_id * efpip->scene_w / thread_num, (thread_id + 1) * efpip->scene_w / thread_num, efpip->frame_temp, efpip->frame_edit,
            efpip->scene_line * sizeof(ExEdit::PixelYC), sizeof(ExEdit::PixelYC), efpip->scene_h, blur->h);
    }
    void __cdecl Blur_t::horizontal_yc_csa_mt_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) { // ff40
        auto blur = (efBlur_var*)(GLOBAL::exedit_base + OFS::ExEdit::efBlur_var_ptr);
        blur_yc_csa_mt(thread_id * efpip->scene_h / thread_num, (thread_id + 1) * efpip->scene_h / thread_num, efpip->frame_temp, efpip->frame_edit,
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



    void Blur_t::blur_yca_mt(int n_begin, int n_end, void* buf_dst, void* buf_src, int buf_step1, int buf_step2, int obj_size, int blur_size) {
        int blur_range = blur_size * 2 + 1;
        int loop[4];
        if (blur_range < obj_size) {
            loop[0] = blur_range;
            loop[1] = obj_size - blur_range;
            loop[2] = 0;
            loop[3] = blur_range - 1;
        } else {
            loop[0] = obj_size;
            loop[1] = 0;
            loop[2] = blur_range - obj_size;
            loop[3] = obj_size - 1;
        }

        int offset = n_begin * buf_step2;
        int n = n_end - n_begin;
        if (has_flag(get_CPUCmdSet(), CPUCmdSet::F_AVX2)) {
            fastBlurYCA256 fb256;
            fb256.range = _mm256_set1_epi32(blur_range);
            fb256.offset = _mm256_mullo_epi32(_mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0), _mm256_set1_epi32(buf_step2));
            for (; 8 <= n; n -= 8) {
                auto dst = (ExEdit::PixelYCA*)((int)buf_dst + offset);
                auto src1 = (ExEdit::PixelYCA*)((int)buf_src + offset);
                auto src2 = src1;
                offset += buf_step2 * 8;

                fb256.y = fb256.cb = fb256.cr = fb256.a = _mm256_setzero_si256();
                for (int i = loop[0]; 0 < i; i--) {
                    yca256_add(&fb256, src1);
                    src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);

                    yca256_put_average(&fb256, dst, buf_step2);
                    dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
                }
                for (int i = loop[1]; 0 < i; i--) {
                    yca256_sub(&fb256, src2);
                    src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);
                    yca256_add(&fb256, src1);
                    src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);

                    yca256_put_average(&fb256, dst, buf_step2);
                    dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
                }
                if (0 < loop[2]) {
                    auto dst1 = dst;
                    for (int j = 7; 0 <= j; j--) {
                        dst = (ExEdit::PixelYCA*)((int)dst1 + j * buf_step2);
                        auto yca = *(ExEdit::PixelYCA*)((int)dst - buf_step1);
                        for (int i = loop[2]; 0 < i; i--) {
                            *dst = yca;
                            dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
                        }
                    }
                }
                for (int i = loop[3]; 0 < i; i--) {
                    yca256_sub(&fb256, src2);
                    src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);

                    yca256_put_average(&fb256, dst, buf_step2);
                    dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
                }
            }
        }

        for (; 0 < n; n--) {
            auto dst = (ExEdit::PixelYCA*)((int)buf_dst + offset);
            auto src1 = (ExEdit::PixelYCA*)((int)buf_src + offset);
            auto src2 = src1;
            offset += buf_step2;

            int cnv_y = 0;
            int cnv_cb = 0;
            int cnv_cr = 0;
            int cnv_a = 0;
            for (int i = loop[0]; 0 < i; i--) {
                cnv_y += src1->y;
                cnv_cb += src1->cb;
                cnv_cr += src1->cr;
                cnv_a += src1->a;
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);

                dst->y = (short)(cnv_y / blur_range);
                dst->cb = (short)(cnv_cb / blur_range);
                dst->cr = (short)(cnv_cr / blur_range);
                dst->a = (short)(cnv_a / blur_range);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
            for (int i = loop[1]; 0 < i; i--) {
                cnv_y += src1->y - src2->y;
                cnv_cb += src1->cb - src2->cb;
                cnv_cr += src1->cr - src2->cr;
                cnv_a += src1->a - src2->a;
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);
                src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);

                dst->y = (short)(cnv_y / blur_range);
                dst->cb = (short)(cnv_cb / blur_range);
                dst->cr = (short)(cnv_cr / blur_range);
                dst->a = (short)(cnv_a / blur_range);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
            if (0 < loop[2]) {
                auto yca = *(ExEdit::PixelYCA*)((int)dst - buf_step1);
                for (int i = loop[2]; 0 < i; i--) {
                    *dst = yca;
                    dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
                }
            }
            for (int i = loop[3]; 0 < i; i--) {
                cnv_y -= src2->y;
                cnv_cb -= src2->cb;
                cnv_cr -= src2->cr;
                cnv_a -= src2->a;
                src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);

                dst->y = (short)(cnv_y / blur_range);
                dst->cb = (short)(cnv_cb / blur_range);
                dst->cr = (short)(cnv_cr / blur_range);
                dst->a = (short)(cnv_a / blur_range);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
        }
    }
    void __cdecl vertical_yca_mt_wrap(int thread_id, int thread_num, int blur_size, ExEdit::FilterProcInfo* efpip) { // eae0
        Blur_t::blur_yca_mt(thread_id * efpip->obj_w / thread_num, (thread_id + 1) * efpip->obj_w / thread_num, efpip->obj_temp, efpip->obj_edit,
            efpip->obj_line * sizeof(ExEdit::PixelYCA), sizeof(ExEdit::PixelYCA), efpip->obj_h, blur_size);
    }
    void __cdecl horizontal_yca_mt_wrap(int thread_id, int thread_num, int blur_size, ExEdit::FilterProcInfo* efpip) { // eef0
        Blur_t::blur_yca_mt(thread_id * efpip->obj_h / thread_num, (thread_id + 1) * efpip->obj_h / thread_num, efpip->obj_temp, efpip->obj_edit,
            sizeof(ExEdit::PixelYCA), efpip->obj_line * sizeof(ExEdit::PixelYCA), efpip->obj_w, blur_size);
    }



    void Blur_t::blur_yca_csa_mt(int n_begin, int n_end, void* buf_dst, void* buf_src, int buf_step1, int buf_step2, int obj_size, int blur_size) {
        int loop[5];
        if (blur_size * 2 < obj_size) {
            loop[0] = blur_size;
            loop[1] = blur_size + 1;
            loop[2] = obj_size - blur_size * 2 - 1;
            loop[3] = 0;
            loop[4] = blur_size;
        } else {
            blur_size = min(blur_size, obj_size - 1);
            loop[0] = blur_size;
            loop[1] = obj_size - blur_size;
            loop[2] = 0;
            loop[3] = blur_size * 2 + 1 - obj_size;
            loop[4] = obj_size - blur_size - 1;
        }
        int offset = n_begin * buf_step2;

        int n = n_end - n_begin;
        if (has_flag(get_CPUCmdSet(), CPUCmdSet::F_AVX2)) {
            fastBlurYCA256 fb256;
            fb256.offset = _mm256_mullo_epi32(_mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0), _mm256_set1_epi32(buf_step2));
            for (; 8 <= n; n -= 8) {
                auto dst = (ExEdit::PixelYCA*)((int)buf_dst + offset);
                auto src1 = (ExEdit::PixelYCA*)((int)buf_src + offset);
                auto src2 = src1;
                offset += buf_step2 * 8;

                fb256.y = fb256.cb = fb256.cr = fb256.a = _mm256_setzero_si256();
                for (int i = loop[0]; 0 < i; i--) {
                    yca256_add(&fb256, src1);
                    src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);
                }
                int range = loop[0];
                for (int i = loop[1]; 0 < i; i--) {
                    yca256_add(&fb256, src1);
                    src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);

                    range++;
                    fb256.range = _mm256_set1_epi32(range);
                    yca256_put_average(&fb256, dst, buf_step2);
                    dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
                }
                for (int i = loop[2]; 0 < i; i--) {
                    yca256_sub(&fb256, src2);
                    src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);
                    yca256_add(&fb256, src1);
                    src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);

                    yca256_put_average(&fb256, dst, buf_step2);
                    dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
                }
                if (0 < loop[3]) {
                    auto dst1 = dst;
                    for (int j = 7; 0 <= j; j--) {
                        dst = (ExEdit::PixelYCA*)((int)dst1 + j * buf_step2);
                        auto yca = *(ExEdit::PixelYCA*)((int)dst - buf_step1);
                        for (int i = loop[3]; 0 < i; i--) {
                            *dst = yca;
                            dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
                        }
                    }
                }
                for (int i = loop[4]; 0 < i; i--) {
                    yca256_sub(&fb256, src2);
                    src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);

                    range--;
                    fb256.range = _mm256_set1_epi32(range);
                    yca256_put_average(&fb256, dst, buf_step2);
                    dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
                }
            }
        }

        for (; 0 < n; n--) {
            auto dst = (ExEdit::PixelYCA*)((int)buf_dst + offset);
            auto src1 = (ExEdit::PixelYCA*)((int)buf_src + offset);
            auto src2 = src1;
            offset += buf_step2;

            int cnv_y = 0;
            int cnv_cb = 0;
            int cnv_cr = 0;
            int cnv_a = 0;
            for (int i = loop[0]; 0 < i; i--) {
                cnv_y += src1->y;
                cnv_cb += src1->cb;
                cnv_cr += src1->cr;
                cnv_a += src1->a;
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);
            }
            int range = loop[0];
            for (int i = loop[1]; 0 < i; i--) {
                cnv_y += src1->y;
                cnv_cb += src1->cb;
                cnv_cr += src1->cr;
                cnv_a += src1->a;
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);

                range++;
                dst->y = (short)(cnv_y / range);
                dst->cb = (short)(cnv_cb / range);
                dst->cr = (short)(cnv_cr / range);
                dst->a = (short)(cnv_a / range);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
            for (int i = loop[2]; 0 < i; i--) {
                cnv_y += src1->y - src2->y;
                cnv_cb += src1->cb - src2->cb;
                cnv_cr += src1->cr - src2->cr;
                cnv_a += src1->a - src2->a;
                src1 = (ExEdit::PixelYCA*)((int)src1 + buf_step1);
                src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);

                dst->y = (short)(cnv_y / range);
                dst->cb = (short)(cnv_cb / range);
                dst->cr = (short)(cnv_cr / range);
                dst->a = (short)(cnv_a / range);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
            if (0 < loop[3]) {
                auto yca = *(ExEdit::PixelYCA*)((int)dst - buf_step1);
                for (int i = loop[3]; 0 < i; i--) {
                    *dst = yca;
                    dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
                }
            }
            for (int i = loop[4]; 0 < i; i--) {
                cnv_y -= src2->y;
                cnv_cb -= src2->cb;
                cnv_cr -= src2->cr;
                cnv_a -= src2->a;
                src2 = (ExEdit::PixelYCA*)((int)src2 + buf_step1);

                range--;
                dst->y = (short)(cnv_y / range);
                dst->cb = (short)(cnv_cb / range);
                dst->cr = (short)(cnv_cr / range);
                dst->a = (short)(cnv_a / range);
                dst = (ExEdit::PixelYCA*)((int)dst + buf_step1);
            }
        }
    }
    void __cdecl vertical_yca_cs_mt_wrap(int thread_id, int thread_num, int blur_size, ExEdit::FilterProcInfo* efpip) { // f310
        Blur_t::blur_yca_csa_mt(thread_id * efpip->obj_w / thread_num, (thread_id + 1) * efpip->obj_w / thread_num, efpip->obj_temp, efpip->obj_edit,
            efpip->obj_line * sizeof(ExEdit::PixelYCA), sizeof(ExEdit::PixelYCA), efpip->obj_h, blur_size);
    }
    void __cdecl horizontal_yca_cs_mt_wrap(int thread_id, int thread_num, int blur_size, ExEdit::FilterProcInfo* efpip) { // f7e0
        Blur_t::blur_yca_csa_mt(thread_id * efpip->obj_h / thread_num, (thread_id + 1) * efpip->obj_h / thread_num, efpip->obj_temp, efpip->obj_edit,
            sizeof(ExEdit::PixelYCA), efpip->obj_line * sizeof(ExEdit::PixelYCA), efpip->obj_w, blur_size);
    }


    void Blur_t::mt_calc_yc_ssss(int thread_id, int thread_num, void* n1, ExEdit::FilterProcInfo* efpip) {
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
                } else if (0x1000 < src_a) {
                    src->a = 0x1000;
                }
                src++;
            }
        }
    }
    void Blur_t::mt_calc_yca_ssss(int thread_id, int thread_num, void* n1, ExEdit::FilterProcInfo* efpip) {
        for (int y = thread_id; y < efpip->obj_h; y += thread_num) {
            auto src = (ExEdit::PixelYCA*)efpip->obj_edit + y * efpip->obj_line;
            for (int x = efpip->obj_w; 0 < x; x--) {
                int src_a = src->a;
                if (0 < src_a && src_a < 0x1000) {
                    src->y = std::clamp(((int)src->y << 12) / src_a, SHRT_MIN, SHRT_MAX);
                    src->cb = std::clamp(((int)src->cb << 12) / src_a, SHRT_MIN, SHRT_MAX);
                    src->cr = std::clamp(((int)src->cr << 12) / src_a, SHRT_MIN, SHRT_MAX);
                }
                src++;
            }
        }
    }

#define _ASM_MULHI_2(a, d, r) \
_asm { \
	__asm mov eax, a \
	__asm mov edx, d \
	__asm shl eax, 1 \
	__asm mul edx \
	__asm mov r, edx \
}

    BOOL __cdecl Blur_t::efBlur_effect_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        int blur_size = efp->track[0];
        if (blur_size <= 0 || efpip->obj_w <= 0 || efpip->obj_h <= 0) return TRUE;
        if (efpip->xf4) {
            auto scene_w = efpip->scene_w;
            auto scene_h = efpip->scene_h;
            auto frame_edit = efpip->frame_edit;
            auto frame_temp = efpip->frame_temp;
            efpip->scene_w = efpip->obj_w;
            efpip->scene_h = efpip->obj_h;
            efpip->frame_edit = reinterpret_cast<decltype(efpip->frame_edit)>(efpip->obj_edit);
            efpip->frame_temp = reinterpret_cast<decltype(efpip->frame_temp)>(efpip->obj_temp);

            efp->flag ^= static_cast<decltype(efp->flag)>(0x20);
            BOOL ret = reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efBlur_Filter_ptr)->func_proc(efp, efpip);
            efp->flag ^= static_cast<decltype(efp->flag)>(0x20);

            efpip->obj_edit = reinterpret_cast<decltype(efpip->obj_edit)>(efpip->frame_edit);
            efpip->obj_temp = reinterpret_cast<decltype(efpip->obj_temp)>(efpip->frame_temp);
            efpip->scene_w = scene_w;
            efpip->scene_h = scene_h;
            efpip->frame_edit = frame_edit;
            efpip->frame_temp = frame_temp;

            return ret;
        }

        int blur_w = blur_size;
        int blur_h = blur_size;
        int aspect = efp->track[1];
        if (aspect) {
            if (aspect < 0) {
                blur_w = blur_w * max(0, 1000 + aspect) / 1000;
            } else {
                blur_h = blur_h * max(0, 1000 - aspect) / 1000;
            }
        }
        int check0 = efp->check[0];
        if (check0 == 0) {
            blur_w = min(blur_w, (*(int*)(GLOBAL::exedit_base + OFS::ExEdit::yca_max_w) - efpip->obj_w) / 2);
            blur_h = min(blur_h, (*(int*)(GLOBAL::exedit_base + OFS::ExEdit::yca_max_h) - efpip->obj_h) / 2);
        }

        int conv1_w = blur_w >> 1;
        int conv1_h = blur_h >> 1;
        int conv2_w = blur_w - conv1_w;
        int conv2_h = blur_h - conv1_h;

        int intensity = efp->track[2];
        if (0 < intensity) {
            reinterpret_cast<void (__cdecl*)(void*, int, int, int)>(GLOBAL::exedit_base + OFS::ExEdit::PixelYCA_ssss2fbbs)(efpip->obj_edit, efpip->obj_w, efpip->obj_h, intensity);
            efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)mt_calc_yc_fbbs, NULL, efpip);
            if (conv1_h) {
                if (check0 == 0) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_yca_fb_mt_wrap, (void*)conv1_h, efpip);
                    efpip->obj_h += conv1_h * 2;
                    std::swap(efpip->obj_edit, efpip->obj_temp);
                } else if (1 < efpip->obj_h) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_yca_fb_cs_mt_wrap, (void*)conv1_h, efpip);
                    std::swap(efpip->obj_edit, efpip->obj_temp);
                }
            }
            if (conv1_w) {
                if (check0 == 0) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_yca_fb_mt_wrap, (void*)conv1_w, efpip);
                    efpip->obj_w += conv1_w * 2;
                    std::swap(efpip->obj_edit, efpip->obj_temp);
                } else if (1 < efpip->obj_w) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_yca_fb_cs_mt_wrap, (void*)conv1_w, efpip);
                    std::swap(efpip->obj_edit, efpip->obj_temp);
                }
            }
            if (conv2_h) {
                if (check0 == 0) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_yca_fb_mt_wrap, (void*)conv2_h, efpip);
                    efpip->obj_h += conv2_h * 2;
                    std::swap(efpip->obj_edit, efpip->obj_temp);
                } else if (1 < efpip->obj_h) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_yca_fb_cs_mt_wrap, (void*)conv2_h, efpip);
                    std::swap(efpip->obj_edit, efpip->obj_temp);
                }
            }
            if (conv2_w) {
                if (check0 == 0) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_yca_fb_mt_wrap, (void*)conv2_w, efpip);
                    efpip->obj_w += conv2_w * 2;
                    std::swap(efpip->obj_edit, efpip->obj_temp);
                } else if (1 < efpip->obj_w) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_yca_fb_cs_mt_wrap, (void*)conv2_w, efpip);
                    std::swap(efpip->obj_edit, efpip->obj_temp);
                }

            }
            efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)mt_calc_yca_fbbs, NULL, efpip);
            reinterpret_cast<void(__cdecl*)(void*, int, int, int)>(GLOBAL::exedit_base + OFS::ExEdit::PixelYCA_fbbs2ssss)(efpip->obj_edit, efpip->obj_w, efpip->obj_h, intensity);
        } else {
            efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)mt_calc_yc_ssss, NULL, efpip);
            if (conv1_h) {
                if (check0 == 0) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_yca_mt_wrap, (void*)conv1_h, efpip);
                    efpip->obj_h += conv1_h * 2;
                    std::swap(efpip->obj_edit, efpip->obj_temp);
                } else if (1 < efpip->obj_h) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_yca_cs_mt_wrap, (void*)conv1_h, efpip);
                    std::swap(efpip->obj_edit, efpip->obj_temp);
                }

            }
            if (conv1_w) {
                if (check0 == 0) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_yca_mt_wrap, (void*)conv1_w, efpip);
                    efpip->obj_w += conv1_w * 2;
                    std::swap(efpip->obj_edit, efpip->obj_temp);
                } else if (1 < efpip->obj_w) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_yca_cs_mt_wrap, (void*)conv1_w, efpip);
                    std::swap(efpip->obj_edit, efpip->obj_temp);
                }

            }
            if (conv2_h) {
                if (check0 == 0) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_yca_mt_wrap, (void*)conv2_h, efpip);
                    efpip->obj_h += conv2_h * 2;
                    std::swap(efpip->obj_edit, efpip->obj_temp);
                } else if (1 < efpip->obj_h) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)vertical_yca_cs_mt_wrap, (void*)conv2_h, efpip);
                    std::swap(efpip->obj_edit, efpip->obj_temp);
                }

            }
            if (conv2_w) {
                if (check0 == 0) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_yca_mt_wrap, (void*)conv2_w, efpip);
                    efpip->obj_w += conv2_w * 2;
                    std::swap(efpip->obj_edit, efpip->obj_temp);
                } else if (1 < efpip->obj_w) {
                    efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)horizontal_yca_cs_mt_wrap, (void*)conv2_w, efpip);
                    std::swap(efpip->obj_edit, efpip->obj_temp);
                }
            }
            efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)mt_calc_yca_ssss, NULL, efpip);
        }
        return TRUE;
    }


}
#endif // ifdef PATCH_SWITCH_FAST_BLUR
