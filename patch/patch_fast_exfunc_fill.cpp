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
#include "patch_fast_exfunc_fill.hpp"
#ifdef PATCH_SWITCH_FAST_EXFUNC_FILL

#include <numbers>

#include "global.hpp"
#include "offset_address.hpp"
#include "util_int.hpp"
#include <emmintrin.h>


namespace patch::fast {

    void __cdecl exfunc_fill_t::fill_simd_stream(void* ycp, int ox, int oy, int width, int height, short y, short cb, short cr, short a, int flag) {
        if (width * height < 0x80000) {
            fill_org(ycp, ox, oy, width, height, y, cb, cr, a, flag);
            return;
        }
        int pixsize, linesize, maxw, maxh;
        if (flag & 2) {
            maxw = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yca_max_w);
            maxh = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yca_max_h);
            pixsize = 8;
            linesize = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yca_vram_line_size);
        } else {
            maxw = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yc_max_w);
            maxh = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yc_max_h);
            pixsize = 6;
            linesize = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yc_vram_line_size);
        }
        if (linesize & 15) { // 通常であればこの条件は満たさないので不要な処理だけど念のため。
            fill_org(ycp, ox, oy, width, height, y, cb, cr, a, flag);
            return;
        }
        if (ox < 0) {
            width += ox;
            ox = 0;
        }
        if (oy < 0) {
            height += oy;
            oy = 0;
        }
        if (maxw - ox < width) {
            width = maxw - ox;
        }
        if (maxh - oy < height) {
            height = maxh - oy;
        }
        if (width <= 0 || height <= 0) {
            return;
        }
        auto dst0 = (byte*)((int)ycp + linesize * oy + pixsize * ox);
        width *= pixsize;
        int size1 = min((((int)dst0 + 15) & 0xfffffff0) - (int)dst0, width);
        int size2 = (width - size1) >> 4;
        int size3 = (width - size1) & 15;
        if (pixsize == 8) {
            short temps[12] = { y,cb,cr,a,y,cb,cr,a,y,cb,cr,a };
            auto temp = (byte*)temps;
            __m128i yca128 = _mm_loadu_si128((__m128i*)&temp[size1 & 7]);
            for (int y = height; 0 < y; y--) {
                auto dst = dst0;
                dst0 += linesize;
                memcpy(dst, temp, size1);
                dst += size1;
                for (int x = size2; 0 < x; x--) {
                    _mm_stream_si128((__m128i*)dst, yca128);
                    dst += sizeof(__m128i);
                }
                memcpy(dst, temp + (size1 & 7), size3);
            }
        } else {
            short temps[11] = { y,cb,cr,y,cb,cr,y,cb,cr,y,cb };
            auto temp = (byte*)temps;
            __m128i yc1281 = _mm_loadu_si128((__m128i*)&temp[size1 % 6]);
            __m128i yc1282 = _mm_loadu_si128((__m128i*)&temp[(size1 + 4) % 6]);
            __m128i yc1283 = _mm_loadu_si128((__m128i*)&temp[(size1 + 2) % 6]);
            int size2_1 = size2 / 3;
            int size2_2 = size2 % 3;
            for (int y = height; 0 < y; y--) {
                auto dst = dst0;
                dst0 += linesize;
                memcpy(dst, temp, size1);
                dst += size1;
                for (int x = size2_1; 0 < x; x--) {
                    _mm_stream_si128((__m128i*)dst, yc1281);
                    dst += sizeof(__m128i);
                    _mm_stream_si128((__m128i*)dst, yc1282);
                    dst += sizeof(__m128i);
                    _mm_stream_si128((__m128i*)dst, yc1283);
                    dst += sizeof(__m128i);
                }
                if (size2_2 == 2) {
                    _mm_stream_si128((__m128i*)dst, yc1281);
                    dst += sizeof(__m128i);
                    _mm_stream_si128((__m128i*)dst, yc1282);
                    dst += sizeof(__m128i);
                } else if (0 < size2_2) { // == 1
                    _mm_stream_si128((__m128i*)dst, yc1281);
                    dst += sizeof(__m128i);
                }
                memcpy(dst, temp + (18 - size3), size3);
            }
        }
        /* AVX2版 SSE2と比べて速度変わらなかった
        if (flag & 2) {
            linesize = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yca_vram_line_size);
            if (linesize & 31) { // 通常であればこの条件は満たさないので不要な処理だけど念のため。
                reinterpret_cast<void(__cdecl*)(void*, int, int, int, int, short, short, short, short, int)>(GLOBAL::exedit_base + 0x81f88)(ycp, ox, oy, width, height, y, cb, cr, a, flag);
                return;
            }
            pixsize = 8;
            maxw = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yca_max_w);
            maxh = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yca_max_h);
        } else {
            linesize = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yc_vram_line_size);
            if (linesize & 1) { // 通常であればこの条件は満たさないので不要な処理だけど念のため。
                reinterpret_cast<void(__cdecl*)(void*, int, int, int, int, short, short, short, short, int)>(GLOBAL::exedit_base + 0x81f88)(ycp, ox, oy, width, height, y, cb, cr, a, flag);
                return;
            }
            pixsize = 6;
            maxw = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yc_max_w);
            maxh = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yc_max_h);
        }
        
        //

        if (flag & 2) {
            short temps[20] = { y,cb,cr,a,y,cb,cr,a,y,cb,cr,a,y,cb,cr,a,y,cb,cr,a };
            auto temp = (byte*)temps;
            int size1 = min((((int)dst0 + 31) & 0xffffffe0) - (int)dst0, width);
            __m256i yca256 = _mm256_loadu_si256((__m256i*)&temp[size1 & 7]);
            int size2 = (width - size1) >> 5;
            int size3 = (width - size1) & 31;
            for (int y = height; 0 < y; y--) {
                auto dst = dst0;
                dst0 += linesize;
                memcpy(dst, temp, size1);
                dst += size1;
                for (int x = size2; 0 < x; x--) {
                    _mm256_stream_si256((__m256i*)dst, yca256);
                    dst += sizeof(__m256i);
                }
                memcpy(dst, temp + (size1 & 7), size3);
            }
        } else {
            short temps[19] = { y,cb,cr,y,cb,cr,y,cb,cr,y,cb,cr,y,cb,cr,y,cb,cr,y };
            auto temp = (byte*)temps;
            if (linesize & 31) {
                __m256i yc256[3] = { _mm256_loadu_si256((__m256i*) & temp[((int)dst0 & 1) + 4]),_mm256_loadu_si256((__m256i*) & temp[((int)dst0 & 1) + 2]),_mm256_loadu_si256((__m256i*) & temp[(int)dst0 & 1]) };
                for (int y = height; 0 < y; y--) {
                    auto dst = dst0;
                    dst0 += linesize;
                    int size1 = min((((int)dst + 31) & 0xffffffe0) - (int)dst, width);
                    int size2 = (width - size1) >> 5;
                    int size3 = (width - size1) & 31;
                    memcpy(dst, temp, size1);
                    dst += size1;
                    int i = 2 - ((size1 % 6) >> 1);
                    for (int x = size2; 0 < x; x--) {
                        _mm256_stream_si256((__m256i*)dst, yc256[i]);
                        dst += sizeof(__m256i);
                        i--;
                        if (i < 0) {
                            i = 2;
                        }
                    }
                    memcpy(dst, temp + (36 - size3), size3);
                }
            } else {
                int size1 = min((((int)dst0 + 31) & 0xffffffe0) - (int)dst0, width);
                __m256i yc256[3] = { _mm256_loadu_si256((__m256i*) & temp[(size1 + 4) % 6]),_mm256_loadu_si256((__m256i*) & temp[(size1 + 2) % 6]),_mm256_loadu_si256((__m256i*) & temp[size1 % 6]) };
                int size2 = (width - size1) >> 5;
                int size3 = (width - size1) & 31;
                for (int y = height; 0 < y; y--) {
                    auto dst = dst0;
                    dst0 += linesize;
                    memcpy(dst, temp, size1);
                    dst += size1;
                    int i = 2;
                    for (int x = size2; 0 < x; x--) {
                        _mm256_stream_si256((__m256i*)dst, yc256[i]);
                        dst += sizeof(__m256i);
                        i--;
                        if (i < 0) {
                            i = 2;
                        }
                    }
                    memcpy(dst, temp + (36 - size3), size3);
                }
            }
        }*/
    }
}

#endif // ifdef PATCH_SWITCH_FAST_EXFUNC_FILL
