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
#include "patch_fast_shadow.hpp"
#ifdef PATCH_SWITCH_FAST_SHADOW

#include <numbers>

#include "global.hpp"
#include "offset_address.hpp"
#include "util_int.hpp"

#include <immintrin.h>


namespace patch::fast {

    void mt1_ver(int thread_id, int thread_num, Shadow_t::efShadow_var* shadow, ExEdit::FilterProcInfo* efpip) {
        int x = efpip->obj_w * thread_id / thread_num;
        int w = efpip->obj_w * (thread_id + 1) / thread_num - x;
        short* dst0 = (short*)shadow->buf1 + x;
        short* src0 = &((ExEdit::PixelYCA*)efpip->obj_edit + x)->a;
        short* src01 = src0;
        int* cnv0 = (int*)shadow->buf2 + x;

        memset(cnv0, 0, w * sizeof(int));

        int loop[4];
        if (shadow->diffuse1 < efpip->obj_h) {
            loop[0] = shadow->diffuse1;
            loop[1] = efpip->obj_h - shadow->diffuse1;
            loop[2] = 0;
            loop[3] = shadow->diffuse1 - 1;
        } else {
            loop[0] = efpip->obj_h;
            loop[1] = 0;
            loop[2] = shadow->diffuse1 - efpip->obj_h;
            loop[3] = efpip->obj_h - 1;
        }

        for (int y = loop[0]; 0 < y; y--) {
            auto dst = dst0;
            auto src = src0;
            auto cnv = cnv0;
            for (x = w; 0 < x; x--) {
                *cnv += *src;
                *dst = *cnv / shadow->diffuse1;
                dst++;
                src += 4;
                cnv++;
            }
            dst0 += efpip->obj_w;
            src0 += efpip->obj_line * 4;
        }
        for (int y = loop[1]; 0 < y; y--) {
            auto dst = dst0;
            auto src = src0;
            auto src1 = src01;
            auto cnv = cnv0;
            for (x = w; 0 < x; x--) {
                *cnv += *src - *src1;
                *dst = *cnv / shadow->diffuse1;
                dst++;
                src += 4;
                src1 += 4;
                cnv++;
            }
            dst0 += efpip->obj_w;
            src0 += efpip->obj_line * 4;
            src01 += efpip->obj_line * 4;
        }
        for (int y = loop[2]; 0 < y; y--) {
            memcpy(dst0, dst0 - efpip->obj_w, w * sizeof(short));
            dst0 += efpip->obj_w;
        }
        for (int y = loop[3]; 0 < y; y--) {
            auto dst = dst0;
            auto src1 = src01;
            auto cnv = cnv0;
            for (x = w; 0 < x; x--) {
                *cnv -= *src1;
                *dst = *cnv / shadow->diffuse1;
                dst++;
                src1 += 4;
                cnv++;
            }
            dst0 += efpip->obj_w;
            src01 += efpip->obj_line * 4;
        }
    }

    void mt23_hor(int thread_id, int thread_num, short* dst, short* src, int obj_w, int obj_h, int diffuse) {
        int y = obj_h * thread_id / thread_num;
        dst += y * (obj_w + diffuse - 1);
        src += y * obj_w;
        auto src1 = src;

        int loop[4];
        if (diffuse < obj_w) {
            loop[0] = diffuse;
            loop[1] = obj_w - diffuse;
            loop[2] = 0;
            loop[3] = diffuse - 1;
        } else {
            loop[0] = obj_w;
            loop[1] = 0;
            loop[2] = diffuse - obj_w;
            loop[3] = obj_w - 1;
        }

        for (y = obj_h * (thread_id + 1) / thread_num - y; 0 < y; y--) {
            int cnv = 0;
            for (int x = loop[0]; 0 < x; x--) {
                cnv += *src;
                *dst = cnv / diffuse;
                dst++;
                src++;
            }
            for (int x = loop[1]; 0 < x; x--) {
                cnv += *src - *src1;
                *dst = cnv / diffuse;
                dst++;
                src++;
                src1++;
            }
            if (0 < loop[2]) {
                short a = cnv / diffuse;
                for (int x = loop[2]; 0 < x; x--) {
                    *dst = a;
                    dst++;
                }
            }
            for (int x = loop[3]; 0 < x; x--) {
                cnv -= *src1;
                *dst = cnv / diffuse;
                dst++;
                src1++;
            }
            src1++;
        }
    }
    void mt2_hor(int thread_id, int thread_num, Shadow_t::efShadow_var* shadow, ExEdit::FilterProcInfo* efpip) {
        mt23_hor(thread_id, thread_num,
            (short*)shadow->buf2, // short* dst
            (short*)shadow->buf1, // short* src
            efpip->obj_w, // int obj_w
            efpip->obj_h + shadow->diffuse1 - 1, // int obj_h
            shadow->diffuse1 // int diffuse
        );
    }
    void mt3_hor(int thread_id, int thread_num, Shadow_t::efShadow_var* shadow, ExEdit::FilterProcInfo* efpip) {
        mt23_hor(thread_id, thread_num,
            (short*)shadow->buf1, // short* dst
            (short*)shadow->buf2, // short* src
            efpip->obj_w + shadow->diffuse1 - 1, // int obj_w
            efpip->obj_h + shadow->diffuse1 - 1, // int obj_h
            shadow->diffuse2 // int diffuse
        );
    }

    void mt4_ver_color(int thread_id, int thread_num, Shadow_t::efShadow_var* shadow, ExEdit::FilterProcInfo* efpip) {

        int obj_w = efpip->obj_w + shadow->diffuse1 - 1 + shadow->diffuse2 - 1;
        int obj_h = efpip->obj_h + shadow->diffuse1 - 1;
        int x = obj_w * thread_id / thread_num;
        int w = obj_w * (thread_id + 1) / thread_num - x;

        short* dst0 = &((ExEdit::PixelYCA*)efpip->obj_temp + shadow->oy * efpip->obj_line + shadow->ox + x)->a;
        short* src0 = (short*)shadow->buf1 + x;
        short* src01 = src0;
        int* cnv0 = (int*)shadow->buf2 + x;

        cnv0 = (int*)(((int)cnv0 + 31 + thread_id * 32) & 0xffffffe0);
        memset(cnv0, 0, w * sizeof(int));

        if (7 < w && has_flag(get_CPUCmdSet(), CPUCmdSet::F_AVX2)) {
            __m256i dif256 = _mm256_set1_epi32(shadow->diffuse2);
            __m256i int256 = _mm256_set1_epi32(shadow->intensity << 4);

            for (int y = shadow->diffuse2; 0 < y; y--) {
                auto dst = dst0;
                auto src = src0;
                auto cnv = cnv0;
                for (x = w >> 3; 0 < x; x--) {
                    __m256i src256 = _mm256_cvtepi16_epi32(_mm_loadu_epi16(src));
                    __m256i cnv256 = _mm256_add_epi32(_mm256_load_si256((__m256i*)cnv), src256);
                    _mm256_store_si256((__m256i*)cnv, cnv256);
                    cnv256 = _mm256_div_epi32(cnv256, dif256);
                    cnv256 = _mm256_mullo_epi32(cnv256, int256);
                    for (int i = 1; i < 16; i += 2) {
                        *dst = cnv256.m256i_i16[i];
                        dst += 4;
                    }
                    src += 8;
                    cnv += 8;
                }
                for (x = w & 7; 0 < x; x--) {
                    *cnv += *src;
                    *dst = *cnv / shadow->diffuse2 * shadow->intensity >> 12;
                    dst += 4;
                    src++;
                    cnv++;
                }
                dst0 += efpip->obj_line * 4;
                src0 += obj_w;
            }

            for (int y = obj_h - shadow->diffuse2; 0 < y; y--) {
                auto dst = dst0;
                auto src = src0;
                auto src1 = src01;
                auto cnv = cnv0;
                for (x = w >> 3; 0 < x; x--) {
                    __m256i src256 = _mm256_cvtepi16_epi32(_mm_sub_epi16(_mm_loadu_epi16(src), _mm_loadu_epi16(src1)));
                    __m256i cnv256 = _mm256_add_epi32(_mm256_load_si256((__m256i*)cnv), src256);
                    _mm256_store_si256((__m256i*)cnv, cnv256);
                    cnv256 = _mm256_div_epi32(cnv256, dif256);
                    cnv256 = _mm256_mullo_epi32(cnv256, int256);
                    for (int i = 1; i < 16; i += 2) {
                        *dst = cnv256.m256i_i16[i];
                        dst += 4;
                    }
                    src += 8;
                    src1 += 8;
                    cnv += 8;
                }
                for (x = w & 7; 0 < x; x--) {
                    *cnv += *src - *src1;
                    *dst = *cnv / shadow->diffuse2 * shadow->intensity >> 12;
                    dst += 4;
                    src++;
                    src1++;
                    cnv++;
                }
                dst0 += efpip->obj_line * 4;
                src0 += obj_w;
                src01 += obj_w;
            }
            for (int y = shadow->diffuse2 - 1; 0 < y; y--) {
                auto dst = dst0;
                auto src1 = src01;
                auto cnv = cnv0;
                for (x = w >> 3; 0 < x; x--) {
                    __m256i src256 = _mm256_cvtepi16_epi32(_mm_loadu_epi16(src1));
                    __m256i cnv256 = _mm256_sub_epi32(_mm256_load_si256((__m256i*)cnv), src256);
                    _mm256_store_si256((__m256i*)cnv, cnv256);
                    cnv256 = _mm256_div_epi32(cnv256, dif256);
                    cnv256 = _mm256_mullo_epi32(cnv256, int256);
                    for (int i = 1; i < 16; i += 2) {
                        *dst = cnv256.m256i_i16[i];
                        dst += 4;
                    }
                    src1 += 8;
                    cnv += 8;
                }
                for (x = w & 7; 0 < x; x--) {
                    *cnv -= *src1;
                    *dst = *cnv / shadow->diffuse2 * shadow->intensity >> 12;
                    dst += 4;
                    src1++;
                    cnv++;
                }
                dst0 += efpip->obj_line * 4;
                src01 += obj_w;
            }
        } else {
            for (int y = shadow->diffuse2; 0 < y; y--) {
                auto dst = dst0;
                auto src = src0;
                auto cnv = cnv0;
                for (x = w; 0 < x; x--) {
                    *cnv += *src;
                    *dst = *cnv / shadow->diffuse2 * shadow->intensity >> 12;
                    dst += 4;
                    src++;
                    cnv++;
                }
                dst0 += efpip->obj_line * 4;
                src0 += obj_w;
            }

            for (int y = obj_h - shadow->diffuse2; 0 < y; y--) {
                auto dst = dst0;
                auto src = src0;
                auto src1 = src01;
                auto cnv = cnv0;
                for (x = w; 0 < x; x--) {
                    *cnv += *src - *src1;
                    *dst = *cnv / shadow->diffuse2 * shadow->intensity >> 12;
                    dst += 4;
                    src++;
                    src1++;
                    cnv++;
                }
                dst0 += efpip->obj_line * 4;
                src0 += obj_w;
                src01 += obj_w;
            }
            for (int y = shadow->diffuse2 - 1; 0 < y; y--) {
                auto dst = dst0;
                auto src1 = src01;
                auto cnv = cnv0;
                for (x = w; 0 < x; x--) {
                    *cnv -= *src1;
                    *dst = *cnv / shadow->diffuse2 * shadow->intensity >> 12;
                    dst += 4;
                    src1++;
                    cnv++;
                }
                dst0 += efpip->obj_line * 4;
                src01 += obj_w;
            }
        }
    }

    void mt4_ver_image(int thread_id, int thread_num, Shadow_t::efShadow_var* shadow, ExEdit::FilterProcInfo* efpip) {
        int obj_w = efpip->obj_w + shadow->diffuse1 - 1 + shadow->diffuse2 - 1;
        int obj_h = efpip->obj_h + shadow->diffuse1 - 1;
        int x = obj_w * thread_id / thread_num;
        int w = obj_w * (thread_id + 1) / thread_num - x;

        short* dst0 = &((ExEdit::PixelYCA*)efpip->obj_temp + shadow->oy * efpip->obj_line + shadow->ox + x)->a;
        short* src0 = (short*)shadow->buf1 + x;
        short* src01 = src0;
        int* cnv0 = (int*)shadow->buf2 + x;

        cnv0 = (int*)(((int)cnv0 + 31 + thread_id * 32) & 0xffffffe0);
        memset(cnv0, 0, w * sizeof(int));
        if (7 < w && has_flag(get_CPUCmdSet(), CPUCmdSet::F_AVX2)) {

            __m256i dif256 = _mm256_set1_epi32(shadow->diffuse2);
            __m256i int256 = _mm256_set1_epi32(shadow->intensity << 4);

            for (int y = shadow->diffuse2; 0 < y; y--) {
                auto dst = dst0;
                auto src = src0;
                auto cnv = cnv0;
                for (x = w >> 3; 0 < x; x--) {
                    __m256i src256 = _mm256_cvtepi16_epi32(_mm_loadu_epi16(src));
                    __m256i cnv256 = _mm256_add_epi32(_mm256_load_si256((__m256i*)cnv), src256);
                    _mm256_store_si256((__m256i*)cnv, cnv256);
                    cnv256 = _mm256_div_epi32(cnv256, dif256);
                    cnv256 = _mm256_mullo_epi32(cnv256, int256);
                    for (int i = 1; i < 16; i += 2) {
                        *dst = cnv256.m256i_i16[i] * *dst >> 12;
                        dst += 4;
                    }
                    src += 8;
                    cnv += 8;
                }
                for (x = w & 7; 0 < x; x--) {
                    *cnv += *src;
                    *dst = (*cnv / shadow->diffuse2 * shadow->intensity >> 12) * *dst >> 12;
                    dst += 4;
                    src++;
                    cnv++;
                }
                dst0 += efpip->obj_line * 4;
                src0 += obj_w;
            }

            for (int y = obj_h - shadow->diffuse2; 0 < y; y--) {
                auto dst = dst0;
                auto src = src0;
                auto src1 = src01;
                auto cnv = cnv0;
                for (x = w >> 3; 0 < x; x--) {
                    __m256i src256 = _mm256_cvtepi16_epi32(_mm_sub_epi16(_mm_loadu_epi16(src), _mm_loadu_epi16(src1)));
                    __m256i cnv256 = _mm256_add_epi32(_mm256_load_si256((__m256i*)cnv), src256);
                    _mm256_store_si256((__m256i*)cnv, cnv256);
                    cnv256 = _mm256_div_epi32(cnv256, dif256);
                    cnv256 = _mm256_mullo_epi32(cnv256, int256);
                    for (int i = 1; i < 16; i += 2) {
                        *dst = cnv256.m256i_i16[i] * *dst >> 12;
                        dst += 4;
                    }
                    src += 8;
                    src1 += 8;
                    cnv += 8;
                }
                for (x = w & 7; 0 < x; x--) {
                    *cnv += *src - *src1;
                    *dst = (*cnv / shadow->diffuse2 * shadow->intensity >> 12) * *dst >> 12;
                    dst += 4;
                    src++;
                    src1++;
                    cnv++;
                }
                dst0 += efpip->obj_line * 4;
                src0 += obj_w;
                src01 += obj_w;
            }
            for (int y = shadow->diffuse2 - 1; 0 < y; y--) {
                auto dst = dst0;
                auto src1 = src01;
                auto cnv = cnv0;
                for (x = w >> 3; 0 < x; x--) {
                    __m256i src256 = _mm256_cvtepi16_epi32(_mm_loadu_epi16(src1));
                    __m256i cnv256 = _mm256_sub_epi32(_mm256_load_si256((__m256i*)cnv), src256);
                    _mm256_store_si256((__m256i*)cnv, cnv256);
                    cnv256 = _mm256_div_epi32(cnv256, dif256);
                    cnv256 = _mm256_mullo_epi32(cnv256, int256);
                    for (int i = 1; i < 16; i += 2) {
                        *dst = cnv256.m256i_i16[i] * *dst >> 12;
                        dst += 4;
                    }
                    src1 += 8;
                    cnv += 8;
                }
                for (x = w & 7; 0 < x; x--) {
                    *cnv -= *src1;
                    *dst = (*cnv / shadow->diffuse2 * shadow->intensity >> 12) * *dst >> 12;
                    dst += 4;
                    src1++;
                    cnv++;
                }
                dst0 += efpip->obj_line * 4;
                src01 += obj_w;
            }
        } else {
            for (int y = shadow->diffuse2; 0 < y; y--) {
                auto dst = dst0;
                auto src = src0;
                auto cnv = cnv0;
                for (x = w; 0 < x; x--) {
                    *cnv += *src;
                    *dst = (*cnv / shadow->diffuse2 * shadow->intensity >> 12) * *dst >> 12;
                    dst += 4;
                    src++;
                    cnv++;
                }
                dst0 += efpip->obj_line * 4;
                src0 += obj_w;
            }

            for (int y = obj_h - shadow->diffuse2; 0 < y; y--) {
                auto dst = dst0;
                auto src = src0;
                auto src1 = src01;
                auto cnv = cnv0;
                for (x = w; 0 < x; x--) {
                    *cnv += *src - *src1;
                    *dst = (*cnv / shadow->diffuse2 * shadow->intensity >> 12) * *dst >> 12;
                    dst += 4;
                    src++;
                    src1++;
                    cnv++;
                }
                dst0 += efpip->obj_line * 4;
                src0 += obj_w;
                src01 += obj_w;
            }
            for (int y = shadow->diffuse2 - 1; 0 < y; y--) {
                auto dst = dst0;
                auto src1 = src01;
                auto cnv = cnv0;
                for (x = w; 0 < x; x--) {
                    *cnv -= *src1;
                    *dst = (*cnv / shadow->diffuse2 * shadow->intensity >> 12) * *dst >> 12;
                    dst += 4;
                    src1++;
                    cnv++;
                }
                dst0 += efpip->obj_line * 4;
                src01 += obj_w;
            }
        }

        for (int y = shadow->diffuse2 - 1; 0 < y; y--) {
            auto dst = dst0;
            auto src1 = src01;
            auto cnv = cnv0;
            for (x = w; 0 < x; x--) {
                *cnv -= *src1;
                *dst = (*cnv / shadow->diffuse2 * shadow->intensity >> 12) * *dst >> 12;
                dst += 4;
                src1++;
                cnv++;
            }
            dst0 += efpip->obj_line * 4;
            src01 += obj_w;
        }
    }


    void calc_blur(int len, short* ptr, fast::Shadow_t::efShadow_var* shadow) {
        short* dst = ptr + shadow->diffuse2;
        int n = min(shadow->diffuse1, len);
        for (int i = 1; i <= n; i++) {
            *dst = i;
            dst++;
        }
        for (int i = abs(len - shadow->diffuse1); 0 < i; i--) {
            *dst = n;
            dst++;
        }
        for (int i = n - 1; 0 < i; i--) {
            *dst = i;
            dst++;
        }
        dst = ptr;
        short* src = ptr + shadow->diffuse2;
        int cnv = 0;
        n = shadow->diffuse1 * shadow->diffuse2;
        for (int i = shadow->diffuse2; 0 < i; i--) {
            cnv += *src;
            *dst = (cnv << 12) / n;
            dst++;
            src++;
        }
        for (int i = len + shadow->diffuse1 - 1  - shadow->diffuse2; 0 < i; i--) {
            cnv += *src - *dst;
            *dst = (cnv << 12) / n;
            dst++;
            src++;
        }
        for (int i = shadow->diffuse2 - 1; 0 < i; i--) {
            cnv -= *dst;
            *dst = (cnv << 12) / n;
            dst++;
        }
    }
    void mt_yc_color(int thread_id, int thread_num, Shadow_t::efShadow_var* shadow, ExEdit::FilterProcInfo* efpip) {
        int obj_w = efpip->obj_w + shadow->diffuse1 - 1 + shadow->diffuse2 - 1;
        int obj_h = efpip->obj_h + shadow->diffuse1 - 1 + shadow->diffuse2 - 1;
        int y = obj_h * thread_id / thread_num;
        auto dst = &((ExEdit::PixelYCA*)efpip->obj_temp + (y + shadow->oy) * efpip->obj_line + shadow->ox)->a;
        int line = (efpip->obj_line - obj_w) * sizeof(ExEdit::PixelYCA);
        auto srcy = (short*)shadow->buf2 + y;
        for (y = obj_h * (thread_id + 1) / thread_num - y; 0 < y; y--) {
            auto srcx = (short*)shadow->buf1;
            for (int x = obj_w; 0 < x; x--) {
                *dst = (*srcx * *srcy >> 12) * shadow->intensity >> 12;
                dst += 4;
                srcx++;
            }
            srcy++;
            dst = (short*)((int)dst + line);
        }
    }
    void mt_yc_image(int thread_id, int thread_num, Shadow_t::efShadow_var* shadow, ExEdit::FilterProcInfo* efpip) {
        int obj_w = efpip->obj_w + shadow->diffuse1 - 1 + shadow->diffuse2 - 1;
        int obj_h = efpip->obj_h + shadow->diffuse1 - 1 + shadow->diffuse2 - 1;
        int y = obj_h * thread_id / thread_num;
        auto dst = &((ExEdit::PixelYCA*)efpip->obj_temp + (y + shadow->oy) * efpip->obj_line + shadow->ox)->a;
        int line = (efpip->obj_line - obj_w) * sizeof(ExEdit::PixelYCA);
        auto srcy = (short*)shadow->buf2 + y;
        for (y = obj_h * (thread_id + 1) / thread_num - y; 0 < y; y--) {
            auto srcx = (short*)shadow->buf1;
            for (int x = obj_w; 0 < x; x--) {
                *dst = ((*srcx * *srcy >> 12) * shadow->intensity >> 12) * *dst >> 12;
                dst += 4;
                srcx++;
            }
            srcy++;
            dst = (short*)((int)dst + line);
        }
    }

    BOOL __cdecl Shadow_t::func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        if (efp->track[2] <= 0 || efpip->obj_w <= 0 || efpip->obj_h <= 0) return 1;
        auto shadow = reinterpret_cast<efShadow_var*>(GLOBAL::exedit_base + OFS::ExEdit::efShadow_var_ptr); // 0x231f90

        shadow->intensity = (efp->track[2] << 9) / 125; // * 4096 / 1000

        if (efp->check[0]) {
            shadow->ox = 0;
            shadow->oy = 0;
        } else {
            shadow->ox = efp->track[0];
            shadow->oy = efp->track[1];
        }
        int max_w = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::yca_max_w);
        int max_h = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::yca_max_h);
        int obj_w = efpip->obj_w + abs(shadow->ox);
        int obj_h = efpip->obj_h + abs(shadow->oy);
        int r = max_w - obj_w;
        if (r < 0) {
            if (shadow->ox < 0) {
                shadow->ox -= r;
            } else {
                shadow->ox += r;
            }
            obj_w = max_w;
        }
        r = max_h - obj_h;
        if (r < 0) {
            if (shadow->oy < 0) {
                shadow->oy -= r;
            } else {
                shadow->oy += r;
            }
            obj_h = max_h;
        }
        int diffuse = efp->track[3];
        diffuse = min(diffuse, min((max_w - efpip->obj_w) / 2, max_w - obj_w));
        diffuse = min(diffuse, min((max_h - efpip->obj_h) / 2, max_h - obj_h));
        diffuse = max(0, diffuse);
        int left, right, top, bottom;
        left = right = top = bottom = diffuse;
        if (0 < shadow->ox) {
            left = max(0, left - shadow->ox);
        } else {
            right = max(0, right + shadow->ox);
        }
        if (0 < shadow->oy) {
            top = max(0, top - shadow->oy);
        } else {
            bottom = max(0, bottom + shadow->oy);
        }


        obj_w += left + right;
        obj_h += top + bottom;
        efpip->obj_data.cx -= (shadow->ox + right - left) << 11;
        efpip->obj_data.cy -= (shadow->oy + bottom - top) << 11;
        shadow->ox -= diffuse;
        shadow->oy -= diffuse;
        int ou;
        int ov;
        if (shadow->ox < 0) {
            ou = -shadow->ox;
            shadow->ox = 0;
        } else {
            ou = 0;
        }
        if (shadow->oy < 0) {
            ov = -shadow->oy;
            shadow->oy = 0;
        } else {
            ov = 0;
        }
        int img_w = 0;
        int img_h = 0;
        auto exdata = (ExEdit::Exdata::efShadow*)efp->exdata_ptr;
        auto memory_ptr = *(void**)(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);
        if (exdata->file[0] != '\0' && efp->exfunc->load_image((ExEdit::PixelYCA*)memory_ptr, exdata->file, &img_w, &img_h, 0, 0)) {
            efp->exfunc->fill(efpip->obj_temp, 0, 0, obj_w, obj_h, 0, 0, 0, 0, 2);
            int shadow_w = efpip->obj_w + diffuse * 2 + shadow->ox;
            int shadow_h = efpip->obj_h + diffuse * 2 + shadow->oy;
            for (int y = shadow->oy; y < shadow_h; y += img_h) {
                for (int x = shadow->ox; x < shadow_w; x += img_w) {
                    efp->exfunc->bufcpy(efpip->obj_temp, x, y, memory_ptr, 0, 0, min(img_w, shadow_w - x), min(img_h, shadow_h - y), 0, 0x13000003);
                }
            }
        } else {
            reinterpret_cast<void(__cdecl*)(short*, short*, short*, int)>(GLOBAL::exedit_base + OFS::ExEdit::rgb2yc)(&shadow->color_y, &shadow->color_cb, &shadow->color_cr, *(int*)&exdata->color & 0xffffff);
            efp->exfunc->fill(efpip->obj_temp, 0, 0, obj_w, obj_h, shadow->color_y, shadow->color_cb, shadow->color_cr, 0, 2);
        }
        shadow->buf1 = (short*)memory_ptr;
        shadow->buf2 = (short*)memory_ptr + efpip->obj_line * obj_h;
        shadow->diffuse2 = diffuse | 1; // (diffuse / 2) * 2 + 1
        shadow->diffuse1 = (diffuse - diffuse / 2) * 2 + 1;

        if (efpip->xf4) {
            calc_blur(efpip->obj_w, shadow->buf1, shadow);
            calc_blur(efpip->obj_h, shadow->buf2, shadow);
            if (img_w == 0 || img_h == 0) {
                efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&mt_yc_color, shadow, efpip);
            } else {
                efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&mt_yc_image, shadow, efpip);
            }
        } else {
            efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&mt1_ver, shadow, efpip);
            efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&mt2_hor, shadow, efpip);
            efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&mt3_hor, shadow, efpip);
            if (img_w == 0 || img_h == 0) {
                efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&mt4_ver_color, shadow, efpip);
            } else {
                efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&mt4_ver_image, shadow, efpip);
            }
        }
        if (efp->check[0]) {
            shadow->ox = efp->track[0];
            shadow->oy = efp->track[1];
            efpip->obj_data.ox += shadow->ox << 12;
            efpip->obj_data.oy += shadow->oy << 12;
            efpip->obj_w += diffuse * 2;
            efpip->obj_h += diffuse * 2;
            std::swap(efpip->obj_edit, efpip->obj_temp);
            auto no_alpha = efpip->xf4;
            efpip->xf4 = 0;
            reinterpret_cast<void(__cdecl*)(ExEdit::ObjectFilterIndex, ExEdit::FilterProcInfo*, int)>(GLOBAL::exedit_base + OFS::ExEdit::do_after_filter_effect)(efp->processing, efpip, 0x10fffff);
            efpip->xf4 = no_alpha;
            efpip->obj_w -= diffuse * 2;
            efpip->obj_h -= diffuse * 2;
            efpip->obj_data.ox -= shadow->ox << 12;
            efpip->obj_data.oy -= shadow->oy << 12;
        } else {
            efp->exfunc->bufcpy(efpip->obj_temp, ou, ov, efpip->obj_edit, 0, 0, efpip->obj_w, efpip->obj_h, 0, 3);
            efpip->obj_w = obj_w;
            efpip->obj_h = obj_h;
        }
        std::swap(efpip->obj_edit, efpip->obj_temp);
        return 1;
    }
   
}

#endif // ifdef PATCH_SWITCH_FAST_SHADOW
