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

#include "patch_fast_extractedge.hpp"
#ifdef PATCH_SWITCH_FAST_EXTRACTEDGE

//#define PATCH_STOPWATCH

namespace patch::fast {

    void __cdecl ExtractEdge_t::disp_color_str_wrap(ExEdit::Filter* efp, uint8_t* exdata) {
        char str[64];

        if (exdata[3] & 1) {
            wsprintfA(str, (LPCSTR)(GLOBAL::exedit_base + OFS::ExEdit::str_SHITEINASHI_MOTOGAZOUNOIRO));
        } else {
            wsprintfA(str, (LPCSTR)(GLOBAL::exedit_base + OFS::ExEdit::str_RGB_d_d_d), exdata[0], exdata[1], exdata[2]);
        }
        HWND hWnd = efp->exfunc->get_hwnd(efp->processing, 5, 2);
        if (hWnd) {
            SetWindowTextA(hWnd, str);
        }
    }
    void __cdecl ExtractEdge_t::update_any_exdata_wrap(ExEdit::ObjectFilterIndex ofi, char* str) {
        reinterpret_cast<void(__cdecl*)(ExEdit::ObjectFilterIndex, char*)>(GLOBAL::exedit_base + OFS::ExEdit::update_any_exdata)(ofi, str); // str_color
        reinterpret_cast<void(__cdecl*)(ExEdit::ObjectFilterIndex, char*)>(GLOBAL::exedit_base + OFS::ExEdit::update_any_exdata)(ofi, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_no_color));
    }





    void yc0_color_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto edge = (ExtractEdge_t::efExtractEdge_var*)(GLOBAL::exedit_base + 0x134e6c);
        int obj_w = efpip->obj_w;
        int y_begin = efpip->obj_h * thread_id / thread_num;
        int y_end = efpip->obj_h * (thread_id + 1) / thread_num;
        int obj_linesize = efpip->obj_line * sizeof(ExEdit::PixelYCA);
        ExEdit::PixelYCA col0 = { edge->color_y,edge->color_cb,edge->color_cr,0 };

        int offset = obj_linesize * y_begin;
        for (int y = y_end - y_begin; 0 < y; y--) {
            auto src = (ExEdit::PixelYCA*)((int)efpip->obj_edit + offset);
            auto dst = (ExEdit::PixelYCA*)((int)efpip->obj_temp + offset);
            for (int x = obj_w; 0 < x; x--) {
                *dst = col0;
                dst++;
                int a = src->a;
                if (a < 0x1000) {
                    if (a <= 0) {
                        src->y = src->cb = src->cr = 0;
                    } else {
                        src->y = src->y * a >> 12;
                        src->cb = src->cb * a >> 12;
                        src->cr = src->cr * a >> 12;
                    }
                }
                src++;
            }
            offset += obj_linesize;
        }
    }
    void yc0_nocolor_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        int obj_w = efpip->obj_w;
        int y_begin = efpip->obj_h * thread_id / thread_num;
        int y_end = efpip->obj_h * (thread_id + 1) / thread_num;
        int obj_linesize = efpip->obj_line * sizeof(ExEdit::PixelYCA);

        int offset = obj_linesize * y_begin;
        for (int y = y_end - y_begin; 0 < y; y--) {
            auto src = (ExEdit::PixelYCA*)((int)efpip->obj_edit + offset);
            auto dst = (ExEdit::PixelYCA*)((int)efpip->obj_temp + offset);
            for (int x = obj_w; 0 < x; x--) {
                *dst = *src;
                dst++;
                int a = src->a;
                if (a < 0x1000) {
                    if (a <= 0) {
                        src->y = src->cb = src->cr = 0;
                    } else {
                        src->y = src->y * a >> 12;
                        src->cb = src->cb * a >> 12;
                        src->cr = src->cr * a >> 12;
                    }
                }
                src++;
            }
            offset += obj_linesize;
        }
    }
    void yc1_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto edge = (ExtractEdge_t::efExtractEdge_var*)(GLOBAL::exedit_base + 0x134e6c);
        int y_begin = efpip->obj_h * thread_id / thread_num;
        int y_end = efpip->obj_h * (thread_id + 1) / thread_num;
        int obj_line = efpip->obj_line;

        if (y_begin == 0) {
            y_begin++;
            auto dst = (short*)&((ExEdit::PixelYCA*)efpip->obj_temp)->a;
            for (int x = efpip->obj_w; 0 < x; x--) {
                *dst = 0;
                dst += 4;
            }
        }
        if (y_end == efpip->obj_h) {
            y_end--;
        }
        int offset = obj_line * y_begin;

        if (has_flag(get_CPUCmdSet(), CPUCmdSet::F_AVX2)) {
            for (int y = y_end - y_begin; 0 < y; y--) {
                auto src = (ExEdit::PixelYCA*)efpip->obj_edit + offset + 1;
                auto dst = (short*)&((ExEdit::PixelYCA*)efpip->obj_temp + offset)->a;
                *dst = 0;
                dst += 4;
                for (int x = (efpip->obj_w - 2 + 3) >> 2; 0 < x; x--) {
                    __m256i src1 = _mm256_loadu_si256((__m256i*)(src - obj_line - 1));
                    __m256i src2 = _mm256_loadu_si256((__m256i*)(src - obj_line));
                    __m256i src3 = _mm256_loadu_si256((__m256i*)(src - obj_line + 1));

                    __m256i src46 = _mm256_subs_epi16(_mm256_loadu_si256((__m256i*)(src - 1)), _mm256_loadu_si256((__m256i*)(src + 1)));
                    short a[] = { src[0].a,src[1].a,src[2].a,src[3].a };

                    __m256i src37 = _mm256_subs_epi16(src3, _mm256_loadu_si256((__m256i*)(src + obj_line - 1)));
                    __m256i src28 = _mm256_subs_epi16(src2, _mm256_loadu_si256((__m256i*)(src + obj_line)));
                    __m256i src19 = _mm256_subs_epi16(src1, _mm256_loadu_si256((__m256i*)(src + obj_line + 1)));

                    __m256i h256 = _mm256_adds_epi16(_mm256_adds_epi16(src19, src37), src28);
                    __m256i v256 = _mm256_adds_epi16(_mm256_subs_epi16(src19, src37), src46);

                    for (int i = 0; i < 2; i++) {
                        __m256 hf256 = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(*((__m128i*)&h256 + i)));
                        hf256 = _mm256_mul_ps(hf256, hf256);
                        __m256 vf256 = _mm256_cvtepi32_ps(_mm256_cvtepi16_epi32(*((__m128i*)&v256 + i)));
                        vf256 = _mm256_mul_ps(vf256, vf256);
                        __m256 d256 = _mm256_sqrt_ps(_mm256_add_ps(hf256, vf256));

                        for (int j = 0; j < 2; j++) {
                            int e = (int)(d256.m256_f32[0 + j * 4] + d256.m256_f32[1 + j * 4] + d256.m256_f32[2 + j * 4]) - edge->threshold;
                            e = std::clamp(e * edge->intensity >> 12, 0, 0x1000);

                            *dst = (short)(a[i * 2 + j] * e >> 12);
                            dst += 4;
                        }
                    }
                    src += 4;
                    obj_line = -obj_line; // 反転しながらの方がメモリアクセスの効率よさそう
                }
                dst = (short*)&((ExEdit::PixelYCA*)efpip->obj_temp + efpip->obj_w - 1 + offset)->a;
                *dst = 0;
                offset += abs(obj_line);
            }
        } else {
            for (int y = y_end - y_begin; 0 < y; y--) {
                auto src = (ExEdit::PixelYCA*)efpip->obj_edit + offset + 1;
                auto dst = (short*)&((ExEdit::PixelYCA*)efpip->obj_temp + offset)->a;
                *dst = 0;
                dst += 4;
                for (int x = (efpip->obj_w - 2 + 1) >> 1; 0 < x; x--) {
                    __m128i src1 = _mm_loadu_si128((__m128i*)(src - obj_line - 1));
                    __m128i src2 = _mm_loadu_si128((__m128i*)(src - obj_line));
                    __m128i src3 = _mm_loadu_si128((__m128i*)(src - obj_line + 1));

                    __m128i src46 = _mm_subs_epi16(_mm_loadu_si128((__m128i*)(src - 1)), _mm_loadu_si128((__m128i*)(src + 1)));
                    short a[] = { src[0].a,src[1].a };

                    __m128i src37 = _mm_subs_epi16(src3, _mm_loadu_si128((__m128i*)(src + obj_line - 1)));
                    __m128i src28 = _mm_subs_epi16(src2, _mm_loadu_si128((__m128i*)(src + obj_line)));
                    __m128i src19 = _mm_subs_epi16(src1, _mm_loadu_si128((__m128i*)(src + obj_line + 1)));

                    __m128i h128 = _mm_adds_epi16(_mm_adds_epi16(src19, src37), src28);
                    __m128i v128 = _mm_adds_epi16(_mm_subs_epi16(src19, src37), src46);

                    for (int i = 0; i < 2; i++) {
                        __m128 hf128 = _mm_setr_ps((float)h128.m128i_i16[0 + i * 4], (float)h128.m128i_i16[1 + i * 4], (float)h128.m128i_i16[2 + i * 4], 0);
                        hf128 = _mm_mul_ps(hf128, hf128);
                        __m128 vf128 = _mm_setr_ps((float)v128.m128i_i16[0 + i * 4], (float)v128.m128i_i16[1 + i * 4], (float)v128.m128i_i16[2 + i * 4], 0);
                        vf128 = _mm_mul_ps(vf128, vf128);
                        __m128 d128 = _mm_sqrt_ps(_mm_add_ps(hf128, vf128));
                        int e = (int)(d128.m128_f32[0] + d128.m128_f32[1] + d128.m128_f32[2]) - edge->threshold;
                        e = std::clamp(e * edge->intensity >> 12, 0, 0x1000);

                        *dst = (short)(a[i] * e >> 12);
                        dst += 4;
                    }
                    src += 2;
                    obj_line = -obj_line; // 反転しながらの方がメモリアクセスの効率よさそう
                }
                dst -= (efpip->obj_w & 1) * 4;
                *dst = 0;
                offset += abs(obj_line);
            }
        }

        if (y_end + 1 == efpip->obj_h) {
            auto dst = (short*)&((ExEdit::PixelYCA*)efpip->obj_temp + y_end * obj_line)->a;
            for (int x = efpip->obj_w; 0 < x; x--) {
                *dst = 0;
                dst += 4;
            }
        }
    }
    BOOL __cdecl ExtractEdge_t::yc_mt_func_wrap(AviUtl::MultiThreadFunc original_func_ptr, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        if (*((int8_t*)efp->exdata_ptr + 3) & 1) {
            efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&yc0_nocolor_mt, efp, efpip);
        } else {
            efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&yc0_color_mt, efp, efpip);
        }
        return efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&yc1_mt, efp, efpip);
    }

    void y0_color_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto edge = (ExtractEdge_t::efExtractEdge_var*)(GLOBAL::exedit_base + 0x134e6c);
        auto mem0 = *(int**)(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);
        int obj_w = efpip->obj_w;
        int y_begin = efpip->obj_h * thread_id / thread_num;
        int y_end = efpip->obj_h * (thread_id + 1) / thread_num;
        int obj_line = efpip->obj_line;
        ExEdit::PixelYCA col0 = { edge->color_y,edge->color_cb,edge->color_cr,0 };

        int offset = obj_line * y_begin;
        for (int y = y_end - y_begin; 0 < y; y--) {
            auto src = (ExEdit::PixelYCA*)efpip->obj_edit + offset;
            auto dst = (ExEdit::PixelYCA*)efpip->obj_temp + offset;
            auto mem = mem0 + offset;
            for (int x = obj_w; 0 < x; x--) {
                *dst = col0;
                dst++;
                int a = src->a;
                if (0x1000 <= a) {
                    *mem = src->y;
                } else  if (a <= 0) {
                    *mem = 0;
                } else {
                    *mem = src->y * a >> 12;
                }
                mem++;
                src++;
            }
            offset += obj_line;
        }
    }
    void y0_nocolor_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto mem0 = *(int**)(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);
        int obj_w = efpip->obj_w;
        int y_begin = efpip->obj_h * thread_id / thread_num;
        int y_end = efpip->obj_h * (thread_id + 1) / thread_num;
        int obj_line = efpip->obj_line;

        int offset = obj_line * y_begin;
        for (int y = y_end - y_begin; 0 < y; y--) {
            auto src = (ExEdit::PixelYCA*)efpip->obj_edit + offset;
            auto dst = (ExEdit::PixelYCA*)efpip->obj_temp + offset;
            auto mem = mem0 + offset;
            for (int x = obj_w; 0 < x; x--) {
                *dst = *src;
                dst++;
                int a = src->a;
                if (0x1000 <= a) {
                    *mem = src->y;
                } else  if (a <= 0) {
                    *mem = 0;
                } else {
                    *mem = src->y * a >> 12;
                }
                mem++;
                src++;
            }
            offset += obj_line;
        }
    }
    void y1_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto edge = (ExtractEdge_t::efExtractEdge_var*)(GLOBAL::exedit_base + 0x134e6c);
        auto mem0 = *(int**)(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);
        int y_begin = efpip->obj_h * thread_id / thread_num;
        int y_end = efpip->obj_h * (thread_id + 1) / thread_num;
        int obj_line = efpip->obj_line;

        if (y_begin == 0) {
            y_begin++;
            auto dst = (short*)&((ExEdit::PixelYCA*)efpip->obj_temp)->a;
            for (int x = efpip->obj_w; 0 < x; x--) {
                *dst = 0;
                dst += 4;
            }
        }
        if (y_end == efpip->obj_h) {
            y_end--;
        }
        int offset = obj_line * y_begin;
        if (has_flag(get_CPUCmdSet(), CPUCmdSet::F_AVX2)) {
            __m256i threshold256 = _mm256_set1_epi32(edge->threshold);
            __m256i intensity256 = _mm256_set1_epi32(edge->intensity);
            for (int y = y_end - y_begin; 0 < y; y--) {
                auto src = (short*)&((ExEdit::PixelYCA*)efpip->obj_edit + offset + 1)->a;
                auto dst = (short*)&((ExEdit::PixelYCA*)efpip->obj_temp + offset)->a;
                auto mem = mem0 + offset + 1;
                *dst = 0;
                dst += 4;
                for (int x = (efpip->obj_w - 2 + 7) >> 3; 0 < x; x--) {
                    __m256i mem1 = _mm256_loadu_si256((__m256i*)(mem - obj_line - 1));
                    __m256i mem2 = _mm256_loadu_si256((__m256i*)(mem - obj_line));
                    __m256i mem3 = _mm256_loadu_si256((__m256i*)(mem - obj_line + 1));

                    __m256i mem46 = _mm256_sub_epi32(_mm256_loadu_si256((__m256i*)(mem - 1)), _mm256_loadu_si256((__m256i*)(mem + 1)));

                    __m256i mem37 = _mm256_sub_epi32(mem3, _mm256_loadu_si256((__m256i*)(mem + obj_line - 1)));
                    __m256i mem28 = _mm256_sub_epi32(mem2, _mm256_loadu_si256((__m256i*)(mem + obj_line)));
                    __m256i mem19 = _mm256_sub_epi32(mem1, _mm256_loadu_si256((__m256i*)(mem + obj_line + 1)));

                    __m256i h256 = _mm256_add_epi32(_mm256_add_epi32(mem19, mem37), mem28);
                    __m256i v256 = _mm256_add_epi32(_mm256_sub_epi32(mem19, mem37), mem46);

                    __m256 hf256 = _mm256_cvtepi32_ps(h256);
                    hf256 = _mm256_mul_ps(hf256, hf256);
                    __m256 vf256 = _mm256_cvtepi32_ps(v256);
                    vf256 = _mm256_mul_ps(vf256, vf256);

                    __m256i e256 = _mm256_cvtps_epi32(_mm256_sqrt_ps(_mm256_add_ps(hf256, vf256)));
                    e256 = _mm256_sub_epi32(e256, threshold256);
                    e256 = _mm256_mullo_epi32(e256, intensity256);
                    e256 = _mm256_srai_epi32(e256, 12);
                    for (int i = 0; i < 8; i++) {
                        if (0x1000 <= e256.m256i_i32[i]) {
                            *dst = *src;
                        } else  if (e256.m256i_i32[i] <= 0) {
                            *dst = 0;
                        } else {
                            *dst = *src * e256.m256i_i32[i] >> 12;
                        }
                        dst += 4;
                        src += 4;
                    }
                    mem += 8;
                    obj_line = -obj_line; // 反転しながらの方がメモリアクセスの効率よさそう
                }
                dst = (short*)&((ExEdit::PixelYCA*)efpip->obj_temp + efpip->obj_w - 1 + offset)->a;
                *dst = 0;
                offset += abs(obj_line);
            }
        } else {
            __m128i threshold128 = _mm_set1_epi32(edge->threshold);
            for (int y = y_end - y_begin; 0 < y; y--) {
                auto src = (short*)&((ExEdit::PixelYCA*)efpip->obj_edit + offset + 1)->a;
                auto dst = (short*)&((ExEdit::PixelYCA*)efpip->obj_temp + offset)->a;
                auto mem = mem0 + offset + 1;
                *dst = 0;
                dst += 4;
                for (int x = (efpip->obj_w - 2 + 3) >> 2; 0 < x; x--) {
                    __m128i mem1 = _mm_loadu_si128((__m128i*)(mem - obj_line - 1));
                    __m128i mem2 = _mm_loadu_si128((__m128i*)(mem - obj_line));
                    __m128i mem3 = _mm_loadu_si128((__m128i*)(mem - obj_line + 1));

                    __m128i mem46 = _mm_sub_epi32(_mm_loadu_si128((__m128i*)(mem - 1)), _mm_loadu_si128((__m128i*)(mem + 1)));

                    __m128i mem37 = _mm_sub_epi32(mem3, _mm_loadu_si128((__m128i*)(mem + obj_line - 1)));
                    __m128i mem28 = _mm_sub_epi32(mem2, _mm_loadu_si128((__m128i*)(mem + obj_line)));
                    __m128i mem19 = _mm_sub_epi32(mem1, _mm_loadu_si128((__m128i*)(mem + obj_line + 1)));

                    __m128i h128 = _mm_add_epi32(_mm_add_epi32(mem19, mem37), mem28);
                    __m128i v128 = _mm_add_epi32(_mm_sub_epi32(mem19, mem37), mem46);

                    __m128 hf128 = _mm_cvtepi32_ps(h128);
                    hf128 = _mm_mul_ps(hf128, hf128);
                    __m128 vf128 = _mm_cvtepi32_ps(v128);
                    vf128 = _mm_mul_ps(vf128, vf128);

                    __m128i e128 = _mm_cvtps_epi32(_mm_sqrt_ps(_mm_add_ps(hf128, vf128)));
                    e128 = _mm_sub_epi32(e128, threshold128);
                    for (int i = 0; i < 4; i++) { // _mm_mullo_epi32はSSE4から
                        e128.m128i_i32[i] *= edge->intensity;
                    }
                    e128 = _mm_srai_epi32(e128, 12);
                    for (int i = 0; i < 4; i++) {
                        if (0x1000 <= e128.m128i_i32[i]) {
                            *dst = *src;
                        } else  if (e128.m128i_i32[i] <= 0) {
                            *dst = 0;
                        } else {
                            *dst = *src * e128.m128i_i32[i] >> 12;
                        }
                        dst += 4;
                        src += 4;
                    }
                    mem += 4;
                    obj_line = -obj_line; // 反転しながらの方がメモリアクセスの効率よさそう
                }
                dst = (short*)&((ExEdit::PixelYCA*)efpip->obj_temp + efpip->obj_w - 1 + offset)->a;
                *dst = 0;
                offset += abs(obj_line);
            }
        }
        
        if (y_end + 1 == efpip->obj_h) {
            auto dst = (short*)&((ExEdit::PixelYCA*)efpip->obj_temp + y_end * obj_line)->a;
            for (int x = efpip->obj_w; 0 < x; x--) {
                *dst = 0;
                dst += 4;
            }
        }
    }
    BOOL __cdecl ExtractEdge_t::y_mt_func_wrap(AviUtl::MultiThreadFunc original_func_ptr, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        if (*((int8_t*)efp->exdata_ptr + 3) & 1) {
            efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&y0_nocolor_mt, efp, efpip);
        } else {
            efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&y0_color_mt, efp, efpip);
        }
        return efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&y1_mt, efp, efpip);
    }

    void __cdecl ExtractEdge_t::alpha_mt_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        if ((*((int8_t*)efp->exdata_ptr + 3) & 1) == 0) {
            reinterpret_cast<void(__cdecl*)(int, int, ExEdit::Filter*, ExEdit::FilterProcInfo*)>(GLOBAL::exedit_base + 0x23880)(thread_id, thread_num, efp, efpip);
            return;
        }
        auto edge = (efExtractEdge_var*)(GLOBAL::exedit_base + 0x134e6c);
        int y_begin = efpip->obj_h * thread_id / thread_num;
        int y_end = efpip->obj_h * (thread_id + 1) / thread_num;
        int obj_line = efpip->obj_line;

        if (y_begin == 0) {
            y_begin++;
            auto dst = (ExEdit::PixelYCA*)efpip->obj_temp;
            auto src = (ExEdit::PixelYCA*)efpip->obj_edit;
            for (int x = efpip->obj_w; 0 < x; x--) {
                *dst = *src;
                dst->a = 0;
                dst++;
                src++;
            }
        }
        if (y_end == efpip->obj_h) {
            y_end--;
            auto dst = (ExEdit::PixelYCA*)efpip->obj_temp + y_end * obj_line;
            auto src = (ExEdit::PixelYCA*)efpip->obj_edit + y_end * obj_line;
            for (int x = efpip->obj_w; 0 < x; x--) {
                *dst = *src;
                dst->a = 0;
                dst++;
                src++;
            }
        }
        int offset = obj_line * y_begin * sizeof(ExEdit::PixelYCA);
        for (int y = y_end - y_begin; 0 < y; y--) { // 無理にsimdしない方が速かった
            auto src = (ExEdit::PixelYCA*)((int)efpip->obj_edit + offset);
            auto dst = (ExEdit::PixelYCA*)((int)efpip->obj_temp + offset);
            *dst = *src;
            dst->a = 0;
            dst++;
            src = (ExEdit::PixelYCA*)((int)src + 8 + 6);
            for (int x = efpip->obj_w - 2; 0 < x; x--) {
                int va, ha;

                ha = va = -src[-obj_line - 1].y;
                va -= src[-obj_line].y;
                ha += src[-obj_line + 1].y;  va -= src[-obj_line + 1].y;

                ha -= src[-1].y;
                auto pix = *(ExEdit::PixelYCA*)((int)src - 6);
                ha += src[1].y;

                va += src[obj_line - 1].y;  ha -= src[obj_line - 1].y;
                va += src[obj_line].y;
                va += src[obj_line + 1].y;  ha += src[obj_line + 1].y;

                int e = (int)sqrt((double)(ha * ha + va * va)) - edge->threshold;
                e = std::clamp(e * edge->intensity >> 12, 0, 0x1000);

                pix.a = pix.a * e >> 12;
                *dst = pix;
                dst++;
                src++;
                obj_line = -obj_line; // 反転しながらの方がメモリアクセスの効率よさそう
            }
            *dst = *(ExEdit::PixelYCA*)((int)src - 6);
            dst->a = 0;
            offset += abs(obj_line) * sizeof(ExEdit::PixelYCA);
        }
    }


} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_EXTRACTEDGE
