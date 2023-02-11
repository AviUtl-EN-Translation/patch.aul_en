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
#include "patch_fast_border.hpp"
#ifdef PATCH_SWITCH_FAST_BORDER

#include <numbers>

#include "global.hpp"
#include "offset_address.hpp"
#include "util_int.hpp"
#include <immintrin.h>


namespace patch::fast {

    BOOL enable_avx2() {
        return has_flag(get_CPUCmdSet(), CPUCmdSet::F_AVX2);
    }

    void horizontal_convolution_alpha(int thread_id, int thread_num, void* param1, void* param2);
    void horizontal_convolution_alpha2(int thread_id, int thread_num, void* param1, void* param2);
    void vertical_convolution_alpha_and_put_color(int thread_id, int thread_num, void* param1, void* param2);
    void vertical_convolution_alpha_and_put_color2(int thread_id, int thread_num, void* param1, void* param2);
    void vertical_convolution_alpha(int thread_id, int thread_num, void* param1, void* param2);
    void vertical_convolution_alpha2(int thread_id, int thread_num, void* param1, void* param2);

    BOOL Border_t::func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {

        auto border = (reinterpret_cast<efBorder_var*>(GLOBAL::exedit_base + OFS::ExEdit::efBorder_var_ptr));
        auto memory_ptr = (void**)(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);

        if (efp->track[0] <= 0) return TRUE;

        auto exdata = (ExEdit::Exdata::efBorder*)efp->exdata_ptr;

        int obj_w = efpip->obj_w;
        int obj_h = efpip->obj_h;

        int add_size = efp->track[0] * 2;

        add_size = (std::min)({ add_size, efpip->obj_line - obj_w, efpip->obj_max_h - obj_h });
        add_size &= 0xfffffffe;

        obj_w += add_size;
        obj_h += add_size;

        // efpip->obj_tempに画像を読み込む
        int file_w = 0;
        int file_h = 0;
        if (exdata->file[0] != '\0') {
            if (efp->exfunc->load_image((ExEdit::PixelYCA*)*memory_ptr, exdata->file, &file_w, &file_h, 0, 0)) {
                for (int i = 0; i < obj_h; i += file_h) {
                    for (int j = 0; j < obj_w; j += file_w) {
                        efp->exfunc->bufcpy(efpip->obj_temp, j, i, *memory_ptr, 0, 0, file_w, file_h, 0, 0x13000003);
                    }
                }
            }
        }


        border->ExEditMemory = (unsigned short*)*memory_ptr;

        border->add_size = add_size;
        border->alpha = (int)round(65536.0 / ((double)efp->track[1] * add_size * 0.01 + 1.0));
        int temp = border->alpha;
        int sft = 0;
        while (sft < 16 && 64 < temp) {
            temp >>= 1;
            sft++;
        }
        border->alpha = temp;
        border->_alpha_shift = 16 - sft;

        reinterpret_cast<void(__cdecl*)(short*, short*, short*, int)>(GLOBAL::exedit_base + OFS::ExEdit::rgb2yc)(&border->color_y, &border->color_cb, &border->color_cr, *(int*)&exdata->color & 0xffffff);

        if (add_size < efpip->obj_w) {
            efp->aviutl_exfunc->exec_multi_thread_func(horizontal_convolution_alpha, border, efpip);
        } else {
            efp->aviutl_exfunc->exec_multi_thread_func(horizontal_convolution_alpha2, border, efpip);
        }

        if (file_w == 0 || file_h == 0) { // 画像なし
            if (add_size < efpip->obj_h) {
                efp->aviutl_exfunc->exec_multi_thread_func(vertical_convolution_alpha_and_put_color, border, efpip);
            } else {
                efp->aviutl_exfunc->exec_multi_thread_func(vertical_convolution_alpha_and_put_color2, border, efpip);
            }
        } else { // 画像あり
            if (add_size < efpip->obj_h) {
                efp->aviutl_exfunc->exec_multi_thread_func(vertical_convolution_alpha, border, efpip);
            } else {
                efp->aviutl_exfunc->exec_multi_thread_func(vertical_convolution_alpha2, border, efpip);
            }
        }


        efp->exfunc->bufcpy(efpip->obj_temp, add_size / 2, add_size / 2, efpip->obj_edit, 0, 0, efpip->obj_w, efpip->obj_h, 0, 3);

        std::swap(efpip->obj_temp, efpip->obj_edit);

        efpip->obj_w = obj_w;
        efpip->obj_h = obj_h;


        return TRUE;
    }



#define ALPHA_TEMP_MAX 0xFFFF

    void horizontal_convolution_alpha(int thread_id, int thread_num, void* param1, void* param2) {
        auto border = static_cast<Border_t::efBorder_var*>(param1);
        auto efpip = static_cast<ExEdit::FilterProcInfo*>(param2);
        int alpha = border->alpha;
        int shift_r = border->_alpha_shift;
        int border_range = border->add_size;
        int loop2 = efpip->obj_w - border_range - 1;
        int y = (efpip->obj_h * thread_id / thread_num + 7) & 0xfffffff8;
        int src_line = efpip->obj_line * sizeof(ExEdit::PixelYCA);
        auto src0 = (short*)((int)efpip->obj_edit + y * src_line) + 2;
        int dst_line = efpip->obj_line * sizeof(unsigned short);
        auto dst0 = (unsigned short*)((int)border->ExEditMemory + y * dst_line);

        y = min((efpip->obj_h * (thread_id + 1) / thread_num + 7) & 0xfffffff8, efpip->obj_h) - y;

        if (enable_avx2() && 8 <= y) {
            __m256i offset256 = _mm256_mullo_epi32(_mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0), _mm256_set1_epi32(src_line));
            __m256i border_alpha256 = _mm256_set1_epi32(alpha);
            __m256i a_dst_max256 = _mm256_set1_epi32(ALPHA_TEMP_MAX);
            int src_line8 = src_line * 8;
            int dst_line8 = dst_line * 8;
            int y7 = y & 7;
            for (y >>= 3; 0 < y; y--) {
                auto src1 = (int*)src0;
                auto src2 = src1;
                src0 = (short*)((int)src0 + src_line8);
                auto dst1 = dst0;
                dst0 = (unsigned short*)((int)dst0 + dst_line8);

                __m256i sum_a256 = _mm256_setzero_si256();
                for (int x = border_range; 0 <= x; x--) {
                    __m256i a256 = _mm256_srli_epi32(_mm256_i32gather_epi32(src1, offset256, 1), 16);
                    sum_a256 = _mm256_add_epi32(sum_a256, a256);
                    a256 = _mm256_mullo_epi32(sum_a256, border_alpha256);
                    a256 = _mm256_srli_epi32(a256, shift_r);
                    a256 = _mm256_min_epu32(a256, a_dst_max256);

                    auto dst = dst1;
                    for (int i = 0; i < 16; i += 2) {
                        *dst = a256.m256i_u16[i];
                        dst = (unsigned short*)((int)dst + dst_line);
                    }

                    src1 += 2;
                    dst1++;
                }
                for (int x = loop2; 0 < x; x--) {
                    __m256i a256 = _mm256_srli_epi32(_mm256_i32gather_epi32(src2, offset256, 1), 16);
                    sum_a256 = _mm256_sub_epi32(sum_a256, a256);
                    a256 = _mm256_srli_epi32(_mm256_i32gather_epi32(src1, offset256, 1), 16);
                    sum_a256 = _mm256_add_epi32(sum_a256, a256);
                    a256 = _mm256_mullo_epi32(sum_a256, border_alpha256);
                    a256 = _mm256_srli_epi32(a256, shift_r);
                    a256 = _mm256_min_epu32(a256, a_dst_max256);

                    auto dst = dst1;
                    for (int i = 0; i < 16; i += 2) {
                        *dst = a256.m256i_u16[i];
                        dst = (unsigned short*)((int)dst + dst_line);
                    }

                    src1 += 2;
                    src2 += 2;
                    dst1++;
                }
                for (int x = border_range; 0 < x; x--) {
                    __m256i a256 = _mm256_srli_epi32(_mm256_i32gather_epi32(src2, offset256, 1), 16);
                    sum_a256 = _mm256_sub_epi32(sum_a256, a256);
                    a256 = _mm256_mullo_epi32(sum_a256, border_alpha256);
                    a256 = _mm256_srli_epi32(a256, shift_r);
                    a256 = _mm256_min_epu32(a256, a_dst_max256);

                    auto dst = dst1;
                    for (int i = 0; i < 16; i += 2) {
                        *dst = a256.m256i_u16[i];
                        dst = (unsigned short*)((int)dst + dst_line);
                    }

                    src2 += 2;
                    dst1++;
                }
            }
            y = y7;
        }

        src0++; // &pix.cr -> &pix.a 
        for (; 0 < y; y--) {
            auto src1 = src0;
            auto src2 = src1;
            src0 = (short*)((int)src0 + src_line);
            auto dst = dst0;
            dst0 = (unsigned short*)((int)dst0 + dst_line);

            int sum_a = 0;
            for (int x = border_range; 0 <= x; x--) {
                sum_a += *src1;
                *dst = min(sum_a * alpha >> shift_r, ALPHA_TEMP_MAX);

                src1 += 4;
                dst++;
            }
            for (int x = loop2; 0 < x; x--) {
                sum_a += *src1 - *src2;
                *dst = min(sum_a * alpha >> shift_r, ALPHA_TEMP_MAX);

                src1 += 4;
                src2 += 4;
                dst++;
            }
            for (int x = border_range; 0 < x; x--) {
                sum_a -= *src2;
                *dst = min(sum_a * alpha >> shift_r, ALPHA_TEMP_MAX);

                src2 += 4;
                dst++;
            }
        }
    }

    void horizontal_convolution_alpha2(int thread_id, int thread_num, void* param1, void* param2) {
        auto border = static_cast<Border_t::efBorder_var*>(param1);
        auto efpip = static_cast<ExEdit::FilterProcInfo*>(param2);
        int alpha = border->alpha;
        int shift_r = border->_alpha_shift;
        int border_range = border->add_size;
        int obj_w = efpip->obj_w;
        int loop2 = border_range - obj_w;
        int y = (efpip->obj_h * thread_id / thread_num + 7) & 0xfffffff8;
        int src_line = efpip->obj_line * sizeof(ExEdit::PixelYCA);
        auto src0 = (short*)((int)efpip->obj_edit + y * src_line) + 2;
        int dst_line = efpip->obj_line * sizeof(unsigned short);
        auto dst0 = (unsigned short*)((int)border->ExEditMemory + y * dst_line);

        y = min((efpip->obj_h * (thread_id + 1) / thread_num + 7) & 0xfffffff8, efpip->obj_h) - y;

        if (enable_avx2() && 8 <= y) {
            __m256i offset256 = _mm256_mullo_epi32(_mm256_set_epi32(7, 6, 5, 4, 3, 2, 1, 0), _mm256_set1_epi32(src_line));
            __m256i border_alpha256 = _mm256_set1_epi32(alpha);
            __m256i a_dst_max256 = _mm256_set1_epi32(ALPHA_TEMP_MAX);
            int src_line8 = src_line * 8;
            int dst_line8 = dst_line * 8;
            int y7 = y & 7;
            for (y >>= 3; 0 < y; y--) {
                auto src = (int*)src0;
                auto dst1 = dst0;
                dst0 = (unsigned short*)((int)dst0 + dst_line8);

                __m256i a256;
                __m256i sum_a256 = _mm256_setzero_si256();
                for (int x = obj_w; 0 < x; x--) {
                    a256 = _mm256_srli_epi32(_mm256_i32gather_epi32(src, offset256, 1), 16);
                    sum_a256 = _mm256_add_epi32(sum_a256, a256);
                    a256 = _mm256_mullo_epi32(sum_a256, border_alpha256);
                    a256 = _mm256_srli_epi32(a256, shift_r);
                    a256 = _mm256_min_epu32(a256, a_dst_max256);

                    auto dst = dst1;
                    for (int i = 0; i < 16; i += 2) {
                        *dst = a256.m256i_u16[i];
                        dst = (unsigned short*)((int)dst + dst_line);
                    }

                    src += 2;
                    dst1++;
                }
                /*
                for (int x = loop2; 0 < x; x--) {
                    auto dst = dst1;
                    for (int i = 0; i < 8; i++) {
                        *dst = a256.m256i_i32[i];
                        dst = (int*)((int)dst + dst_line);
                    }
                    dst1++;
                }
                */
                auto dst2 = (unsigned short*)((int)dst1 + dst_line8);
                for (int i = 14; 0 <= i; i -= 2) {
                    dst1 = dst2 = (unsigned short*)((int)dst2 - dst_line);
                    int a = a256.m256i_u16[i];
                    for (int x = loop2; 0 < x; x--) {
                        *dst1 = a;
                        dst1++;
                    }
                }
                src = (int*)src0;
                src0 = (short*)((int)src0 + src_line8);
                for (int x = obj_w; 0 < x; x--) {
                    a256 = _mm256_srli_epi32(_mm256_i32gather_epi32(src, offset256, 1), 16);
                    sum_a256 = _mm256_sub_epi32(sum_a256, a256);
                    a256 = _mm256_mullo_epi32(sum_a256, border_alpha256);
                    a256 = _mm256_srli_epi32(a256, shift_r);
                    a256 = _mm256_min_epu32(a256, a_dst_max256);

                    auto dst = dst1;
                    for (int i = 0; i < 16; i += 2) {
                        *dst = a256.m256i_u16[i];
                        dst = (unsigned short*)((int)dst + dst_line);
                    }

                    src += 2;
                    dst1++;
                }
            }
            y = y7;
        }

        src0++; // &pix.cr -> &pix.a 
        for (; 0 < y; y--) {
            auto src = src0;
            auto dst = dst0;
            dst0 = (unsigned short*)((int)dst0 + dst_line);

            int sum_a = 0;
            for (int x = obj_w; 0 < x; x--) {
                sum_a += *src;
                *dst = min(sum_a * alpha >> shift_r, ALPHA_TEMP_MAX);

                src += 4;
                dst++;
            }
            unsigned short a = *(dst - 1);
            for (int x = loop2; 0 < x; x--) {
                *dst = a;
                dst++;
            }
            src = src0;
            src0 = (short*)((int)src0 + src_line);
            for (int x = obj_w; 0 < x; x--) {
                sum_a -= *src;
                *dst = min(sum_a * alpha >> shift_r, ALPHA_TEMP_MAX);

                src += 4;
                dst++;
            }
        }
    }

    void vertical_convolution_alpha_and_put_color(int thread_id, int thread_num, void* param1, void* param2) {
        auto border = static_cast<Border_t::efBorder_var*>(param1);
        auto efpip = static_cast<ExEdit::FilterProcInfo*>(param2);

        ExEdit::PixelYCA color = { border->color_y, border->color_cb, border->color_cr,0 };

        unsigned int alpha = border->alpha;
        int shift_r = border->_alpha_shift;
        int border_range = border->add_size;
        int loop2 = efpip->obj_h - border_range - 1;
        int x = ((efpip->obj_w + border_range) * thread_id / thread_num + 3) & 0xfffffffc;
        auto src0 = (unsigned short*)border->ExEditMemory + x;
        int src_line = efpip->obj_line * sizeof(unsigned short);
        auto dst0 = (ExEdit::PixelYCA*)efpip->obj_temp + x;
        int dst_line = efpip->obj_line * sizeof(ExEdit::PixelYCA);

        x = min(((efpip->obj_w + border_range) * (thread_id + 1) / thread_num + 3) & 0xfffffffc, efpip->obj_w + border_range) - x;

        if (enable_avx2() && 4 <= x) {
            __m256i color256 = _mm256_set1_epi64x(*(long long*)&color);
            __m256i border_alpha256 = _mm256_set1_epi32(alpha);
            __m256i a_dst_max256 = _mm256_set1_epi32(0x1000);
            int x3 = x & 3;
            for (x >>= 2; 0 < x; x--) {
                auto src1 = (__m128i*)src0;
                auto src2 = src1;
                src0 += 4;
                auto dst = (__m256i*)dst0;
                dst0 += 4;

                __m256i a_sum256 = _mm256_setzero_si256();
                __m256i pix256;

                for (int y = border_range; 0 <= y; y--) {
                    a_sum256 = _mm256_add_epi64(a_sum256, _mm256_cvtepu16_epi64(*src1));
                    pix256 = _mm256_mullo_epi32(a_sum256, border_alpha256);
                    pix256 = _mm256_srli_epi32(pix256, shift_r);
                    pix256 = _mm256_min_epi32(pix256, a_dst_max256);
                    pix256 = _mm256_slli_epi64(pix256, 48);
                    pix256 = _mm256_or_si256(pix256, color256);
                    _mm256_store_si256(dst, pix256);

                    src1 = (__m128i*)((int)src1 + src_line);
                    dst = (__m256i*)((int)dst + dst_line);
                }

                for (int y = loop2; 0 < y; y--) {
                    a_sum256 = _mm256_sub_epi64(a_sum256, _mm256_cvtepu16_epi64(*src2));
                    a_sum256 = _mm256_add_epi64(a_sum256, _mm256_cvtepu16_epi64(*src1));
                    pix256 = _mm256_mullo_epi32(a_sum256, border_alpha256);
                    pix256 = _mm256_srli_epi32(pix256, shift_r);
                    pix256 = _mm256_min_epi32(pix256, a_dst_max256);
                    pix256 = _mm256_slli_epi64(pix256, 48);
                    pix256 = _mm256_or_si256(pix256, color256);
                    _mm256_store_si256(dst, pix256);

                    src1 = (__m128i*)((int)src1 + src_line);
                    src2 = (__m128i*)((int)src2 + src_line);
                    dst = (__m256i*)((int)dst + dst_line);
                }

                for (int y = border_range; 0 < y; y--) {
                    a_sum256 = _mm256_sub_epi64(a_sum256, _mm256_cvtepu16_epi64(*src2));
                    pix256 = _mm256_mullo_epi32(a_sum256, border_alpha256);
                    pix256 = _mm256_srli_epi32(pix256, shift_r);
                    pix256 = _mm256_min_epi32(pix256, a_dst_max256);
                    pix256 = _mm256_slli_epi64(pix256, 48);
                    pix256 = _mm256_or_si256(pix256, color256);
                    _mm256_store_si256(dst, pix256);

                    src2 = (__m128i*)((int)src2 + src_line);
                    dst = (__m256i*)((int)dst + dst_line);
                }
            }
            x = x3;
        }

        for (; 0 < x; x--) {
            auto src1 = src0;
            auto src2 = src1;
            src0++;
            auto dst = dst0;
            dst0++;

            unsigned int a_sum = 0;
            for (int y = border_range; 0 <= y; y--) {
                a_sum += *src1;
                color.a = (short)min(a_sum * border->alpha >> border->_alpha_shift, 0x1000);
                *dst = color;

                src1 = (unsigned short*)((int)src1 + src_line);
                dst = (ExEdit::PixelYCA*)((int)dst + dst_line);
            }

            for (int y = loop2; 0 < y; y--) {
                a_sum += *src1 - *src2;
                color.a = (short)min(a_sum * border->alpha >> border->_alpha_shift, 0x1000);
                *dst = color;

                src1 = (unsigned short*)((int)src1 + src_line);
                src2 = (unsigned short*)((int)src2 + src_line);
                dst = (ExEdit::PixelYCA*)((int)dst + dst_line);
            }

            for (int y = border_range; 0 < y; y--) {
                a_sum -= *src2;
                color.a = (short)min(a_sum * border->alpha >> border->_alpha_shift, 0x1000);
                *dst = color;

                src2 = (unsigned short*)((int)src2 + src_line);
                dst = (ExEdit::PixelYCA*)((int)dst + dst_line);
            }
        }
    }

    void vertical_convolution_alpha_and_put_color2(int thread_id, int thread_num, void* param1, void* param2) {
        auto border = static_cast<Border_t::efBorder_var*>(param1);
        auto efpip = static_cast<ExEdit::FilterProcInfo*>(param2);

        ExEdit::PixelYCA color = { border->color_y, border->color_cb, border->color_cr,0 };

        unsigned int alpha = border->alpha;
        int shift_r = border->_alpha_shift;
        int border_range = border->add_size;
        int obj_h = efpip->obj_h;
        int loop2 = border_range - obj_h;
        int x = ((efpip->obj_w + border_range) * thread_id / thread_num + 3) & 0xfffffffc;
        auto src0 = (unsigned short*)border->ExEditMemory + x;
        int src_line = efpip->obj_line * sizeof(unsigned short);
        auto dst0 = (ExEdit::PixelYCA*)efpip->obj_temp + x;
        int dst_line = efpip->obj_line * sizeof(ExEdit::PixelYCA);

        x = min(((efpip->obj_w + border_range) * (thread_id + 1) / thread_num + 3) & 0xfffffffc, efpip->obj_w + border_range) - x;

        if (enable_avx2() && 4 <= x) {
            __m256i color256 = _mm256_set1_epi64x(*(long long*)&color);
            __m256i border_alpha256 = _mm256_set1_epi32(alpha);
            __m256i a_dst_max256 = _mm256_set1_epi32(0x1000);
            int x3 = x & 3;
            for (x >>= 2; 0 < x; x--) {
                auto src = (__m128i*)src0;
                auto dst = (__m256i*)dst0;
                dst0 += 4;

                __m256i a_sum256 = _mm256_setzero_si256();
                __m256i pix256;

                for (int y = obj_h; 0 < y; y--) {
                    a_sum256 = _mm256_add_epi64(a_sum256, _mm256_cvtepu16_epi64(*src));
                    pix256 = _mm256_mullo_epi32(a_sum256, border_alpha256);
                    pix256 = _mm256_srli_epi32(pix256, shift_r);
                    pix256 = _mm256_min_epi32(pix256, a_dst_max256);
                    pix256 = _mm256_slli_epi64(pix256, 48);
                    pix256 = _mm256_or_si256(pix256, color256);
                    _mm256_store_si256(dst, pix256);

                    src = (__m128i*)((int)src + src_line);
                    dst = (__m256i*)((int)dst + dst_line);
                }

                for (int y = loop2; 0 < y; y--) {
                    _mm256_store_si256(dst, pix256);

                    dst = (__m256i*)((int)dst + dst_line);
                }

                src = (__m128i*)src0;
                src0 += 4;
                for (int y = obj_h; 0 < y; y--) {
                    a_sum256 = _mm256_sub_epi64(a_sum256, _mm256_cvtepu16_epi64(*src));
                    pix256 = _mm256_mullo_epi32(a_sum256, border_alpha256);
                    pix256 = _mm256_srli_epi32(pix256, shift_r);
                    pix256 = _mm256_min_epi32(pix256, a_dst_max256);
                    pix256 = _mm256_slli_epi64(pix256, 48);
                    pix256 = _mm256_or_si256(pix256, color256);
                    _mm256_store_si256(dst, pix256);

                    src = (__m128i*)((int)src + src_line);
                    dst = (__m256i*)((int)dst + dst_line);
                }
            }
            x = x3;
        }

        for (; 0 < x; x--) {
            auto src = src0;
            auto dst = dst0;
            dst0++;

            unsigned int a_sum = 0;

            for (int y = obj_h; 0 < y; y--) {
                a_sum += *src;
                color.a = (short)min(a_sum * border->alpha >> border->_alpha_shift, 0x1000);
                *dst = color;
                
                src = (unsigned short*)((int)src + src_line);
                dst = (ExEdit::PixelYCA*)((int)dst + dst_line);
            }

            for (int y = loop2; 0 < y; y--) {
                *dst = color;

                dst = (ExEdit::PixelYCA*)((int)dst + dst_line);
            }

            src = src0;
            src0++;
            for (int y = obj_h; 0 < y; y--) {
                a_sum -= *src;
                color.a = (short)min(a_sum * border->alpha >> border->_alpha_shift, 0x1000);
                *dst = color;

                src = (unsigned short*)((int)src + src_line);
                dst = (ExEdit::PixelYCA*)((int)dst + dst_line);
            }
        }
    }

    void vertical_convolution_alpha(int thread_id, int thread_num, void* param1, void* param2) {
        auto border = static_cast<Border_t::efBorder_var*>(param1);
        auto efpip = static_cast<ExEdit::FilterProcInfo*>(param2);

        unsigned int alpha = border->alpha;
        int shift_r = border->_alpha_shift;
        int border_range = border->add_size;
        int loop2 = efpip->obj_h - border_range - 1;
        int x = ((efpip->obj_w + border_range) * thread_id / thread_num + 7) & 0xfffffff8;
        auto src0 = (unsigned short*)border->ExEditMemory + x;
        int src_line = efpip->obj_line * sizeof(unsigned short);
        auto dst0 = (short*)((ExEdit::PixelYCA*)efpip->obj_temp + x) + 2;
        int dst_line = efpip->obj_line * sizeof(ExEdit::PixelYCA);

        x = min(((efpip->obj_w + border_range) * (thread_id + 1) / thread_num + 7) & 0xfffffff8, efpip->obj_w + border_range) - x;

        if (enable_avx2() && 8 <= x) {
            __m256i offset256 = _mm256_set_epi32(14, 12, 10, 8, 6, 4, 2, 0);
            __m256i border_alpha256 = _mm256_set1_epi32(alpha);
            __m256i a_dst_max256 = _mm256_set1_epi32(0x1000);
            int x7 = x & 7;
            for (x >>= 3; 0 < x; x--) {
                auto src1 = (__m128i*)src0;
                auto src2 = src1;
                src0 += 8;
                auto dst1 = (int*)dst0;
                dst0 += 32;

                __m256i sum_a256 = _mm256_setzero_si256();
                for (int y = border_range; 0 <= y; y--) {
                    sum_a256 = _mm256_add_epi32(sum_a256, _mm256_cvtepu16_epi32(*src1));
                    __m256i a256 = _mm256_mullo_epi32(sum_a256, border_alpha256);
                    a256 = _mm256_srli_epi32(a256, shift_r);
                    a256 = _mm256_min_epu32(a256, a_dst_max256);
                    __m256i dsta256 = _mm256_srli_epi32(_mm256_i32gather_epi32(dst1, offset256, 4), 16);
                    dsta256 = _mm256_mullo_epi32(dsta256, a256);
                    dsta256 = _mm256_srli_epi32(dsta256, 12);
                    auto dst = (short*)dst1 + 1;
                    for (int i = 0; i < 16; i += 2) {
                        *dst = dsta256.m256i_u16[i];
                        dst += 4;
                    }

                    src1 = (__m128i*)((int)src1 + src_line);
                    dst1 = (int*)((int)dst1 + dst_line);
                }

                for (int y = loop2; 0 < y; y--) {
                    sum_a256 = _mm256_sub_epi32(sum_a256, _mm256_cvtepu16_epi32(*src2));
                    sum_a256 = _mm256_add_epi32(sum_a256, _mm256_cvtepu16_epi32(*src1));
                    __m256i a256 = _mm256_mullo_epi32(sum_a256, border_alpha256);
                    a256 = _mm256_srli_epi32(a256, shift_r);
                    a256 = _mm256_min_epu32(a256, a_dst_max256);
                    __m256i dsta256 = _mm256_srli_epi32(_mm256_i32gather_epi32(dst1, offset256, 4), 16);
                    dsta256 = _mm256_mullo_epi32(dsta256, a256);
                    dsta256 = _mm256_srli_epi32(dsta256, 12);
                    auto dst = (short*)dst1 + 1;
                    for (int i = 0; i < 16; i += 2) {
                        *dst = dsta256.m256i_u16[i];
                        dst += 4;
                    }

                    src1 = (__m128i*)((int)src1 + src_line);
                    src2 = (__m128i*)((int)src2 + src_line);
                    dst1 = (int*)((int)dst1 + dst_line);
                }

                for (int y = border_range; 0 < y; y--) {
                    sum_a256 = _mm256_sub_epi32(sum_a256, _mm256_cvtepu16_epi32(*src2));
                    __m256i a256 = _mm256_mullo_epi32(sum_a256, border_alpha256);
                    a256 = _mm256_srli_epi32(a256, shift_r);
                    a256 = _mm256_min_epu32(a256, a_dst_max256);
                    __m256i dsta256 = _mm256_srli_epi32(_mm256_i32gather_epi32(dst1, offset256, 4), 16);
                    dsta256 = _mm256_mullo_epi32(dsta256, a256);
                    dsta256 = _mm256_srli_epi32(dsta256, 12);
                    auto dst = (short*)dst1 + 1;
                    for (int i = 0; i < 16; i += 2) {
                        *dst = dsta256.m256i_u16[i];
                        dst += 4;
                    }

                    src2 = (__m128i*)((int)src2 + src_line);
                    dst1 = (int*)((int)dst1 + dst_line);
                }
            }
            x = x7;
        }

        dst0++;
        for (; 0 < x; x--) {
            auto src1 = src0;
            auto src2 = src1;
            src0++;
            auto dst = dst0;
            dst0 += 4;

            unsigned int sum_a = 0;
            for (int y = border_range; 0 <= y; y--) {
                sum_a += *src1;
                int a = sum_a * alpha >> shift_r;
                if (a < 0x1000) {
                    *dst = (short)(*dst * a >> 12);
                }

                src1 = (unsigned short*)((int)src1 + src_line);
                dst = (short*)((int)dst + dst_line);
            }

            for (int y = loop2; 0 < y; y--) {
                sum_a += *src1 - *src2;
                int a = sum_a * alpha >> shift_r;
                if (a < 0x1000) {
                    *dst = (short)(*dst * a >> 12);
                }

                src1 = (unsigned short*)((int)src1 + src_line);
                src2 = (unsigned short*)((int)src2 + src_line);
                dst = (short*)((int)dst + dst_line);
            }

            for (int y = border_range; 0 < y; y--) {
                sum_a -= *src2;
                int a = sum_a * alpha >> shift_r;
                if (a < 0x1000) {
                    *dst = (short)(*dst * a >> 12);
                }

                src2 = (unsigned short*)((int)src2 + src_line);
                dst = (short*)((int)dst + dst_line);
            }
        }
    }

    void vertical_convolution_alpha2(int thread_id, int thread_num, void* param1, void* param2) {
        auto border = static_cast<Border_t::efBorder_var*>(param1);
        auto efpip = static_cast<ExEdit::FilterProcInfo*>(param2);

        unsigned int alpha = border->alpha;
        int shift_r = border->_alpha_shift;
        int border_range = border->add_size;
        int obj_h = efpip->obj_h;
        int loop2 = border_range - obj_h;
        int x = ((efpip->obj_w + border_range) * thread_id / thread_num + 7) & 0xfffffff8;
        auto src0 = (unsigned short*)border->ExEditMemory + x;
        int src_line = efpip->obj_line * sizeof(unsigned short);
        auto dst0 = (short*)((ExEdit::PixelYCA*)efpip->obj_temp + x) + 2;
        int dst_line = efpip->obj_line * sizeof(ExEdit::PixelYCA);

        x = min(((efpip->obj_w + border_range) * (thread_id + 1) / thread_num + 7) & 0xfffffff8, efpip->obj_w + border_range) - x;

        if (enable_avx2() && 8 <= x) {
            __m256i offset256 = _mm256_set_epi32(14, 12, 10, 8, 6, 4, 2, 0);
            __m256i border_alpha256 = _mm256_set1_epi32(alpha);
            __m256i a_dst_max256 = _mm256_set1_epi32(0x1000);
            int x7 = x & 7;
            for (x >>= 3; 0 < x; x--) {
                auto src = (__m128i*)src0;
                auto dst1 = (int*)dst0;
                dst0 += 32;

                __m256i a256;
                __m256i sum_a256 = _mm256_setzero_si256();
                for (int y = obj_h; 0 < y; y--) {
                    sum_a256 = _mm256_add_epi32(sum_a256, _mm256_cvtepu16_epi32(*src));
                    a256 = _mm256_mullo_epi32(sum_a256, border_alpha256);
                    a256 = _mm256_srli_epi32(a256, shift_r);
                    a256 = _mm256_min_epu32(a256, a_dst_max256);
                    __m256i dsta256 = _mm256_srli_epi32(_mm256_i32gather_epi32(dst1, offset256, 4), 16);
                    dsta256 = _mm256_mullo_epi32(dsta256, a256);
                    dsta256 = _mm256_srli_epi32(dsta256, 12);
                    auto dst = (short*)dst1 + 1;
                    for (int i = 0; i < 16; i += 2) {
                        *dst = dsta256.m256i_u16[i];
                        dst += 4;
                    }

                    src = (__m128i*)((int)src + src_line);
                    dst1 = (int*)((int)dst1 + dst_line);
                }

                __m256i flag256 = _mm256_srli_epi32(a256, 12);
                int n;
                for (n = 7; 0 <= n; n--) { // 全alphaが0x1000なら丸ごと省略するってのを書いたけど効率良いのか不明
                    if (flag256.m256i_i32[n] == 0) {
                        for (int y = loop2; 0 < y; y--) {
                            __m256i dsta256 = _mm256_srli_epi32(_mm256_i32gather_epi32(dst1, offset256, 4), 16);
                            dsta256 = _mm256_mullo_epi32(dsta256, a256);
                            dsta256 = _mm256_srli_epi32(dsta256, 12);
                            auto dst = (short*)dst1 + 1;
                            for (int i = 0; i < 16; i += 2) {
                                *dst = dsta256.m256i_u16[i];
                                dst += 4;
                            }
                            dst1 = (int*)((int)dst1 + dst_line);
                        }
                        break;
                    }
                }
                if (n == -1) {
                    dst1 = (int*)((int)dst1 + dst_line * loop2);
                }

                src = (__m128i*)src0;
                src0 += 8;
                for (int y = obj_h; 0 < y; y--) {
                    sum_a256 = _mm256_sub_epi32(sum_a256, _mm256_cvtepu16_epi32(*src));
                    a256 = _mm256_mullo_epi32(sum_a256, border_alpha256);
                    a256 = _mm256_srli_epi32(a256, shift_r);
                    a256 = _mm256_min_epu32(a256, a_dst_max256);
                    __m256i dsta256 = _mm256_srli_epi32(_mm256_i32gather_epi32(dst1, offset256, 4), 16);
                    dsta256 = _mm256_mullo_epi32(dsta256, a256);
                    dsta256 = _mm256_srli_epi32(dsta256, 12);
                    auto dst = (short*)dst1 + 1;
                    for (int i = 0; i < 16; i += 2) {
                        *dst = dsta256.m256i_u16[i];
                        dst += 4;
                    }

                    src = (__m128i*)((int)src + src_line);
                    dst1 = (int*)((int)dst1 + dst_line);
                }
            }
            x = x7;
        }

        dst0++;
        for (; 0 < x; x--) {
            auto src = src0;
            auto dst = dst0;
            dst0 += 4;

            int a;
            unsigned int sum_a = 0;
            for (int y = obj_h; 0 < y; y--) {
                sum_a += *src;
                a = sum_a * alpha >> shift_r;
                if (a < 0x1000) {
                    *dst = (short)(*dst * a >> 12);
                }

                src = (unsigned short*)((int)src + src_line);
                dst = (short*)((int)dst + dst_line);
            }
            if (a < 0x1000) {
                for (int y = loop2; 0 < y; y--) {
                    *dst = (short)(*dst * a >> 12);
                    dst = (short*)((int)dst + dst_line);
                }
            } else {
                dst = (short*)((int)dst + dst_line * loop2);
            }

            src = src0;
            src0++;
            for (int y = obj_h; 0 < y; y--) {
                sum_a -= *src;
                a = sum_a * alpha >> shift_r;
                if (a < 0x1000) {
                    *dst = (short)(*dst * a >> 12);
                }

                src = (unsigned short*)((int)src + src_line);
                dst = (short*)((int)dst + dst_line);
            }
        }
    }


}
#endif // ifdef PATCH_SWITCH_FAST_BORDER
