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


namespace patch::fast {

    void vertical_convolution_alpha(int thread_id, int thread_num, void* param1, void* param2);
    void horizontal_convolution_alpha_color(int thread_id, int thread_num, void* param1, void* param2);
    void horizontal_convolution_alpha_image(int thread_id, int thread_num, void* param1, void* param2);

    BOOL Border_t::func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {

        int obj_w = efpip->obj_w;
        int obj_h = efpip->obj_h;
        int size = efp->track[0];
        if (size <= 0 || obj_w <= 0 || obj_h <= 0) return TRUE;
        auto border = (reinterpret_cast<efBorder_var*>(GLOBAL::exedit_base + OFS::ExEdit::efBorder_var_ptr));

        short *src01, *src02;
        int max_space;
        bool usespace = (use_space && 0 == *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::luastateidx));
        if (usespace) {
            max_space = min((obj_h + 1) >> 1, size);
            src01 = &((ExEdit::PixelYCA*)efpip->obj_edit)->a;
            src02 = &((ExEdit::PixelYCA*)efpip->obj_edit + (efpip->obj_h - 1) * efpip->obj_line)->a;
        } else {
            src01 = src02 = nullptr;
            max_space = 0;
        }
        int x, y;
        for (y = 0; y < max_space; y++) {
            auto src1 = src01;
            auto src2 = src02;
            for (x = obj_w; 0 < x; x--) {
                if (0 < *src1 || 0 < *src2) break;
                src1 += 4; src2 += 4;
            }
            if (0 < x) break;
            src01 += efpip->obj_line * 4; src02 -= efpip->obj_line * 4;
        }
        border->shift_y = y;
        obj_h -= border->shift_y * 2;
        if (obj_h <= 0) return TRUE; 
        int max_size = (*reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yca_max_h) - obj_h) >> 1;
        if (max_size <= 0) return TRUE;
        if (max_size < size) {
            size = max_size;
        }

        if (usespace) {
            max_space = min((obj_w + 1) >> 1, size);
            src01 = &((ExEdit::PixelYCA*)efpip->obj_edit + border->shift_y * efpip->obj_line)->a;
            src02 = &((ExEdit::PixelYCA*)efpip->obj_edit + border->shift_y * efpip->obj_line + efpip->obj_w - 1)->a;
        } else {
            max_space = 0;
        }
        for (x = 0; x < max_space; x++) {
            auto src1 = src01;
            auto src2 = src02;
            for (y = obj_h; 0 < y; y--) {
                if (0 < *src1 || 0 < *src2) break;
                src1 += efpip->obj_line * 4; src2 += efpip->obj_line * 4;
            }
            if (0 < y) break;
            src01 += 4; src02 -= 4;
        }
        border->shift_x = x;
        obj_w -= border->shift_x * 2;
        if (obj_w <= 0) return TRUE;
        max_size = (*reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yca_max_w) - obj_w) >> 1;
        if (max_size <= 0) return TRUE;
        if (max_size < size) {
            size = max_size;
            if (size < border->shift_y) {
                obj_h += (border->shift_y - size) * 2;
                border->shift_y = size;
            }
        }

        int range = size * 2;
        border->range = range;

        auto exdata = (ExEdit::Exdata::efBorder*)efp->exdata_ptr;
        border->buf = *(unsigned short**)(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);
        // efpip->obj_tempに画像を読み込む
        int file_w = 0;
        int file_h = 0;
        if (exdata->file[0] != '\0') {
            if (efp->exfunc->load_image((ExEdit::PixelYCA*)border->buf, exdata->file, &file_w, &file_h, 0, 0)) {
                for (int i = -border->shift_y; i < obj_h + range + border->shift_y; i += file_h) {
                    int min_h = min(file_h, obj_h + range + border->shift_y - i);
                    for (int j = -border->shift_x; j < obj_w + range + border->shift_x; j += file_w) {
                        efp->exfunc->bufcpy(efpip->obj_temp, j, i, border->buf, 0, 0, min(file_w, obj_w + range + border->shift_x - j), min_h, 0, 0x13000003);
                    }
                }
            }
        }


        int alpha = 6553600 / (max(0, efp->track[1]) * range + 100);
        int sft = 0;
        while (sft < 16 && 64 < alpha) {
            alpha >>= 1;
            sft++;
        }
        border->alpha = alpha;
        border->_alpha_shift = 16 - sft;

        reinterpret_cast<void(__cdecl*)(short*, short*, short*, int)>(GLOBAL::exedit_base + OFS::ExEdit::rgb2yc)(&border->color_y, &border->color_cb, &border->color_cr, *(int*)&exdata->color & 0xffffff);

        efpip->obj_w = obj_w;
        efpip->obj_h = obj_h;

        efp->aviutl_exfunc->exec_multi_thread_func(vertical_convolution_alpha, border, efpip);

        if (file_w <= 0 || file_h <= 0) { // 画像なし
            efp->aviutl_exfunc->exec_multi_thread_func(horizontal_convolution_alpha_color, border, efpip);
        } else { // 画像あり
            efp->aviutl_exfunc->exec_multi_thread_func(horizontal_convolution_alpha_image, border, efpip);
        }

        std::swap(efpip->obj_temp, efpip->obj_edit);

        efpip->obj_w = obj_w + range;
        efpip->obj_h = obj_h + range;


        return TRUE;
    }


#define ALPHA_TEMP_MAX 0xFFFF

    void vertical_convolution_alpha(int thread_id, int thread_num, void* param1, void* param2) {
        auto border = static_cast<Border_t::efBorder_var*>(param1);
        auto efpip = static_cast<ExEdit::FilterProcInfo*>(param2);
        int x = thread_id * efpip->obj_w / thread_num;
        short* src01 = &((ExEdit::PixelYCA*)efpip->obj_edit + x + border->shift_x + border->shift_y * efpip->obj_line)->a;
        auto src02 = src01;
        auto dst0 = (unsigned short*)border->buf + x;
        auto cnv_a0 = (unsigned int*)((unsigned short*)border->buf + (efpip->obj_h + border->range) * efpip->obj_w) + x;
        int w = (thread_id + 1) * efpip->obj_w / thread_num - x;
        memset(cnv_a0, 0, w * sizeof(unsigned int));

        int loop[4];
        if (border->range < efpip->obj_h) {
            loop[0] = border->range + 1;
            loop[1] = efpip->obj_h - border->range - 1;
            loop[2] = 0;
            loop[3] = border->range;
        } else {
            loop[0] = efpip->obj_h;
            loop[1] = 0;
            loop[2] = border->range - efpip->obj_h + 1;
            loop[3] = efpip->obj_h - 1;
        }

        for (int y = loop[0]; 0 < y; y--) {
            auto src = src01;
            src01 += efpip->obj_line * 4;
            auto dst = dst0;
            dst0 += efpip->obj_w;
            auto cnv_a = cnv_a0;
            for (x = w; 0 < x; x--) {
                *cnv_a += *src;
                src += 4;
                *dst = min(*cnv_a * border->alpha >> border->_alpha_shift, ALPHA_TEMP_MAX);
                dst++;
                cnv_a++;
            }
        }

        for (int y = loop[1]; 0 < y; y--) {
            auto src1 = src01;
            auto src2 = src02;
            src01 += efpip->obj_line * 4;
            src02 += efpip->obj_line * 4;
            auto dst = dst0;
            dst0 += efpip->obj_w;
            auto cnv_a = cnv_a0;
            for (x = w; 0 < x; x--) {
                *cnv_a += *src1 - *src2;
                src1 += 4;
                src2 += 4;
                *dst = min(*cnv_a * border->alpha >> border->_alpha_shift, ALPHA_TEMP_MAX);
                dst++;
                cnv_a++;
            }
        }
        for (int y = loop[2]; 0 < y; y--) {
            auto dst = dst0;
            memcpy(dst0, dst0 - efpip->obj_w, w * sizeof(*dst)); // (unsigned short*)
            dst0 += efpip->obj_w;
        }

        for (int y = loop[3]; 0 < y; y--) {
            auto src = src02;
            src02 += efpip->obj_line * 4;
            auto dst = dst0;
            dst0 += efpip->obj_w;
            auto cnv_a = cnv_a0;
            for (x = w; 0 < x; x--) {
                *cnv_a -= *src;
                src += 4;
                *dst = min(*cnv_a * border->alpha >> border->_alpha_shift, ALPHA_TEMP_MAX);
                dst++;
                cnv_a++;
            }
        }
    }

    void horizontal_convolution_alpha_color(int thread_id, int thread_num, void* param1, void* param2) {
        auto border = static_cast<Border_t::efBorder_var*>(param1);
        auto efpip = static_cast<ExEdit::FilterProcInfo*>(param2);
        ExEdit::PixelYCA color = { border->color_y, border->color_cb, border->color_cr,0 };
        int cnv_w = sizeof(unsigned short) * efpip->obj_w * (thread_num - 1);
        auto src1 = (unsigned short*)border->buf + thread_id * efpip->obj_w;
        auto pix_line = sizeof(ExEdit::PixelYCA) * efpip->obj_line * thread_num;
        auto dst0 = (ExEdit::PixelYCA*)efpip->obj_temp + thread_id * efpip->obj_line;

        int size = border->range >> 1;
        int loop[12];
        if (border->range < efpip->obj_w) {
            loop[0] = border->range + 1;
            loop[1] = efpip->obj_w - border->range - 1;
            loop[2] = 0;
            loop[3] = border->range;

            loop[4] = size;
            loop[5] = 0;
            loop[6] = size + 1;
            loop[7] = efpip->obj_w - border->range - 1;
            loop[8] = 0;
            loop[9] = size;
            loop[10] = 0;
            loop[11] = size;
        } else {
            loop[0] = efpip->obj_w;
            loop[1] = 0;
            loop[2] = border->range - efpip->obj_w + 1;
            loop[3] = efpip->obj_w - 1;
            loop[7] = 0;
            if (size < efpip->obj_w) {
                loop[4] = size;
                loop[5] = 0;
                loop[6] = efpip->obj_w - size;
                loop[8] = border->range - efpip->obj_w + 1;
                loop[9] = efpip->obj_w - size - 1;
                loop[10] = 0;
                loop[11] = size;
            } else {
                loop[4] = efpip->obj_w;
                loop[5] = size - efpip->obj_w;
                loop[6] = 0;
                loop[8] = efpip->obj_w;
                loop[9] = 0;
                loop[10] = size - efpip->obj_w + 1;
                loop[11] = efpip->obj_w - 1;
            }
        }

        int yend[4] = { size, size + efpip->obj_h, size * 2 + efpip->obj_h, 0 };
        int* yep = yend;
        int y = thread_id;
        for (int i = 2; 0 < i; i--) {
            for (; y < *yep; y += thread_num) {
                auto src2 = src1;
                auto dst = dst0;
                dst0 = (ExEdit::PixelYCA*)((int)dst0 + pix_line);

                unsigned int cnv_a = 0;
                for (int x = loop[0]; 0 < x; x--) {
                    cnv_a += *src1;
                    src1++;
                    color.a = min(cnv_a * border->alpha >> border->_alpha_shift, 0x1000);
                    *dst = color;
                    dst++;
                }
                for (int x = loop[1]; 0 < x; x--) {
                    cnv_a += *src1 - *src2;
                    src1++;
                    src2++;
                    color.a = min(cnv_a * border->alpha >> border->_alpha_shift, 0x1000);
                    *dst = color;
                    dst++;
                }
                for (int x = loop[2]; 0 < x; x--) {
                    *dst = color;
                    dst++;
                }
                for (int x = loop[3]; 0 < x; x--) {
                    cnv_a -= *src2;
                    src2++;
                    color.a = min(cnv_a * border->alpha >> border->_alpha_shift, 0x1000);
                    *dst = color;
                    dst++;
                }
                src1 = (unsigned short*)((int)src1 + cnv_w);
            }
            auto src = (ExEdit::PixelYCA*)efpip->obj_edit + (y - *yep + border->shift_y) * efpip->obj_line + border->shift_x;
            yep++;
            for (; y < *yep; y += thread_num) {
                auto src2 = src1;
                auto src3 = src;
                src = (ExEdit::PixelYCA*)((int)src + pix_line);
                auto dst = dst0;
                dst0 = (ExEdit::PixelYCA*)((int)dst0 + pix_line);

                unsigned int a;
                unsigned int cnv_a = 0;
                for (int x = loop[4]; 0 < x; x--) {
                    cnv_a += *src1;
                    src1++;
                    color.a = min(cnv_a * border->alpha >> border->_alpha_shift, 0x1000);
                    *dst = color;
                    dst++;
                }
                if (0 < loop[5]) {
                    for (int x = loop[5]; 0 < x; x--) {
                        *dst = color;
                        dst++;
                    }
                }
                for (int x = loop[6]; 0 < x; x--) {
                    cnv_a += *src1;
                    src1++;
                    if (0x1000 <= src3->a) {
                        *dst = *src3;
                    } else {
                        a = cnv_a * border->alpha >> border->_alpha_shift;
                        if (0x1000 <= a) {
                            dst->y = (((src3->y - color.y) * src3->a) >> 12) + color.y;
                            dst->cb = (((src3->cb - color.cb) * src3->a) >> 12) + color.cb;
                            dst->cr = (((src3->cr - color.cr) * src3->a) >> 12) + color.cr;
                            dst->a = 0x1000;
                        } else if (a <= 0) {
                            *dst = *src3;
                        } else {
                            int new_a = (0x1000800 - (0x1000 - a) * (0x1000 - src3->a)) >> 12;
                            int dst_a = (0x1000 - src3->a) * a / new_a;
                            int src_a = (src3->a << 12) / new_a;
                            dst->y = (color.y * dst_a + src3->y * src_a) >> 12;
                            dst->cb = (color.cb * dst_a + src3->cb * src_a) >> 12;
                            dst->cr = (color.cr * dst_a + src3->cr * src_a) >> 12;
                            dst->a = new_a;
                        }
                    }
                    dst++;
                    src3++;
                }
                for (int x = loop[7]; 0 < x; x--) {
                    cnv_a += *src1 - *src2;
                    src1++;
                    src2++;
                    if (0x1000 <= src3->a) {
                        *dst = *src3;
                    } else {
                        a = cnv_a * border->alpha >> border->_alpha_shift;
                        if (0x1000 <= a) {
                            dst->y = (((src3->y - color.y) * src3->a) >> 12) + color.y;
                            dst->cb = (((src3->cb - color.cb) * src3->a) >> 12) + color.cb;
                            dst->cr = (((src3->cr - color.cr) * src3->a) >> 12) + color.cr;
                            dst->a = 0x1000;
                        } else if (a <= 0) {
                            *dst = *src3;
                        } else {
                            int new_a = (0x1000800 - (0x1000 - a) * (0x1000 - src3->a)) >> 12;
                            int dst_a = (0x1000 - src3->a) * a / new_a;
                            int src_a = (src3->a << 12) / new_a;
                            dst->y = (color.y * dst_a + src3->y * src_a) >> 12;
                            dst->cb = (color.cb * dst_a + src3->cb * src_a) >> 12;
                            dst->cr = (color.cr * dst_a + src3->cr * src_a) >> 12;
                            dst->a = new_a;
                        }
                    }
                    dst++;
                    src3++;
                }
                if (0 < loop[8]) {
                    int x = loop[8];
                    a = cnv_a * border->alpha >> border->_alpha_shift;
                    if (0x1000 <= a) {
                        for (; 0 < x; x--) {
                            if (0x1000 <= src3->a) {
                                *dst = *src3;
                            } else {
                                dst->y = (((src3->y - color.y) * src3->a) >> 12) + color.y;
                                dst->cb = (((src3->cb - color.cb) * src3->a) >> 12) + color.cb;
                                dst->cr = (((src3->cr - color.cr) * src3->a) >> 12) + color.cr;
                                dst->a = 0x1000;
                            }
                            dst++;
                            src3++;
                        }
                    } else if (a <= 0) {
                        memcpy(dst, src3, x * sizeof(ExEdit::PixelYCA));
                        dst += x;
                        src3 += x;
                    } else {
                        for (; 0 < x; x--) {
                            if (0x1000 <= src3->a) {
                                *dst = *src3;
                            } else {
                                int new_a = (0x1000800 - (0x1000 - a) * (0x1000 - src3->a)) >> 12;
                                int dst_a = (0x1000 - src3->a) * a / new_a;
                                int src_a = (src3->a << 12) / new_a;
                                dst->y = (color.y * dst_a + src3->y * src_a) >> 12;
                                dst->cb = (color.cb * dst_a + src3->cb * src_a) >> 12;
                                dst->cr = (color.cr * dst_a + src3->cr * src_a) >> 12;
                                dst->a = new_a;
                            }
                            dst++;
                            src3++;
                        }
                    }
                }
                for (int x = loop[9]; 0 < x; x--) {
                    cnv_a -= *src2;
                    src2++;
                    if (0x1000 <= src3->a) {
                        *dst = *src3;
                    } else {
                        a = cnv_a * border->alpha >> border->_alpha_shift;
                        if (0x1000 <= a) {
                            dst->y = (((src3->y - color.y) * src3->a) >> 12) + color.y;
                            dst->cb = (((src3->cb - color.cb) * src3->a) >> 12) + color.cb;
                            dst->cr = (((src3->cr - color.cr) * src3->a) >> 12) + color.cr;
                            dst->a = 0x1000;
                        } else if (a <= 0) {
                            *dst = *src3;
                        } else {
                            int new_a = (0x1000800 - (0x1000 - a) * (0x1000 - src3->a)) >> 12;
                            int dst_a = (0x1000 - src3->a) * a / new_a;
                            int src_a = (src3->a << 12) / new_a;
                            dst->y = (color.y * dst_a + src3->y * src_a) >> 12;
                            dst->cb = (color.cb * dst_a + src3->cb * src_a) >> 12;
                            dst->cr = (color.cr * dst_a + src3->cr * src_a) >> 12;
                            dst->a = new_a;
                        }
                    }
                    dst++;
                    src3++;
                }
                if (0 < loop[10]) {
                    color.a = min(cnv_a * border->alpha >> border->_alpha_shift, 0x1000);
                    for (int x = loop[10]; 0 < x; x--) {
                        *dst = color;
                        dst++;
                    }
                }
                for (int x = loop[11]; 0 < x; x--) {
                    cnv_a -= *src2;
                    src2++;
                    color.a = min(cnv_a * border->alpha >> border->_alpha_shift, 0x1000);
                    *dst = color;
                    dst++;
                }
                src1 = (unsigned short*)((int)src1 + cnv_w);
            }
            yep++;
        }
    }

    void horizontal_convolution_alpha_image(int thread_id, int thread_num, void* param1, void* param2) {
        auto border = static_cast<Border_t::efBorder_var*>(param1);
        auto efpip = static_cast<ExEdit::FilterProcInfo*>(param2);
        int cnv_w = sizeof(unsigned short) * efpip->obj_w * (thread_num - 1);
        auto src1 = (unsigned short*)border->buf + thread_id * efpip->obj_w;
        auto pix_line = sizeof(ExEdit::PixelYCA) * efpip->obj_line * thread_num;
        auto dst0 = &((ExEdit::PixelYCA*)efpip->obj_temp + thread_id * efpip->obj_line)->a;

        int size = border->range >> 1;
        int loop[12];
        if (border->range < efpip->obj_w) {
            loop[0] = border->range + 1;
            loop[1] = efpip->obj_w - border->range - 1;
            loop[2] = 0;
            loop[3] = border->range;

            loop[4] = size;
            loop[5] = 0;
            loop[6] = size + 1;
            loop[7] = efpip->obj_w - border->range - 1;
            loop[8] = 0;
            loop[9] = size;
            loop[10] = 0;
            loop[11] = size;
        } else {
            loop[0] = efpip->obj_w;
            loop[1] = 0;
            loop[2] = border->range - efpip->obj_w + 1;
            loop[3] = efpip->obj_w - 1;
            loop[7] = 0;
            if (size < efpip->obj_w) {
                loop[4] = size;
                loop[5] = 0;
                loop[6] = efpip->obj_w - size;
                loop[8] = border->range - efpip->obj_w + 1;
                loop[9] = efpip->obj_w - size - 1;
                loop[10] = 0;
                loop[11] = size;
            } else {
                loop[4] = efpip->obj_w;
                loop[5] = size - efpip->obj_w;
                loop[6] = 0;
                loop[8] = efpip->obj_w;
                loop[9] = 0;
                loop[10] = size - efpip->obj_w + 1;
                loop[11] = efpip->obj_w - 1;
            }
        }
        int yend[4] = { size, size + efpip->obj_h, size * 2 + efpip->obj_h, 0 };
        int* yep = yend;
        int y = thread_id;
        for (int i = 2; 0 < i; i--) {
            for (; y < *yep; y += thread_num) {
                auto src2 = src1;
                auto dst = dst0;
                dst0 = (short*)((int)dst0 + pix_line);

                unsigned int a;
                unsigned int cnv_a = 0;
                for (int x = loop[0]; 0 < x; x--) {
                    cnv_a += *src1;
                    src1++;
                    a = cnv_a * border->alpha >> border->_alpha_shift;
                    if (a < 0x1000) {
                        *dst = *dst * a >> 12;
                    }
                    dst += 4;
                }
                for (int x = loop[1]; 0 < x; x--) {
                    cnv_a += *src1 - *src2;
                    src1++;
                    src2++;
                    a = cnv_a * border->alpha >> border->_alpha_shift;
                    if (a < 0x1000) {
                        *dst = *dst * a >> 12;
                    }
                    dst += 4;
                }
                if (0 < loop[2]) {
                    int x = loop[2];
                    if (a < 0x1000) {
                        for (; 0 < x; x--) {
                            *dst = *dst * a >> 12;
                            dst += 4;
                        }
                    } else {
                        dst += x * 4;
                    }
                }
                for (int x = loop[3]; 0 < x; x--) {
                    cnv_a -= *src2;
                    src2++;
                    a = cnv_a * border->alpha >> border->_alpha_shift;
                    if (a < 0x1000) {
                        *dst = *dst * a >> 12;
                    }
                    dst += 4;
                }
                src1 = (unsigned short*)((int)src1 + cnv_w);
            }
            dst0 -= 3;
            auto src = (ExEdit::PixelYCA*)efpip->obj_edit + (y - *yep + border->shift_y) * efpip->obj_line + border->shift_x;
            yep++;
            for (; y < *yep; y += thread_num) {
                auto src2 = src1;
                auto src3 = src;
                src = (ExEdit::PixelYCA*)((int)src + pix_line);
                auto dst = (ExEdit::PixelYCA*)dst0;
                dst0 = (short*)((int)dst0 + pix_line);

                unsigned int a;
                unsigned int cnv_a = 0;
                for (int x = loop[4]; 0 < x; x--) {
                    cnv_a += *src1;
                    src1++;
                    a = cnv_a * border->alpha >> border->_alpha_shift;
                    if (a < 0x1000) {
                        dst->a = dst->a * a >> 12;
                    }
                    dst++;
                }
                if (0 < loop[5]) {
                    int x = loop[5];
                    if (a < 0x1000) {
                        for (; 0 < x; x--) {
                            dst->a = dst->a * a >> 12;
                            dst++;
                        }
                    } else {
                        dst += x;
                    }
                }
                for (int x = loop[6]; 0 < x; x--) {
                    cnv_a += *src1;
                    src1++;
                    if (0x1000 <= src3->a) {
                        *dst = *src3;
                    } else {
                        a = cnv_a * border->alpha >> border->_alpha_shift;
                        if (a < 0x1000) {
                            a = a * dst->a >> 12;
                        } else {
                            a = dst->a;
                        }
                        if (0x1000 <= a) {
                            dst->y += ((src3->y - dst->y) * src3->a) >> 12;
                            dst->cb += ((src3->cb - dst->cb) * src3->a) >> 12;
                            dst->cr += ((src3->cr - dst->cr) * src3->a) >> 12;
                            dst->a = 0x1000;
                        } else if (a <= 0) {
                            *dst = *src3;
                        } else {
                            int new_a = (0x1000800 - (0x1000 - a) * (0x1000 - src3->a)) >> 12;
                            int dst_a = (0x1000 - src3->a) * a / new_a;
                            int src_a = (src3->a << 12) / new_a;
                            dst->y = (dst->y * dst_a + src3->y * src_a) >> 12;
                            dst->cb = (dst->cb * dst_a + src3->cb * src_a) >> 12;
                            dst->cr = (dst->cr * dst_a + src3->cr * src_a) >> 12;
                            dst->a = new_a;
                        }
                    }
                    dst++;
                    src3++;
                }
                for (int x = loop[7]; 0 < x; x--) {
                    cnv_a += *src1 - *src2;
                    src1++;
                    src2++;
                    if (0x1000 <= src3->a) {
                        *dst = *src3;
                    } else {
                        a = cnv_a * border->alpha >> border->_alpha_shift;
                        if (a < 0x1000) {
                            a = a * dst->a >> 12;
                        } else {
                            a = dst->a;
                        }
                        if (0x1000 <= a) {
                            dst->y += ((src3->y - dst->y) * src3->a) >> 12;
                            dst->cb += ((src3->cb - dst->cb) * src3->a) >> 12;
                            dst->cr += ((src3->cr - dst->cr) * src3->a) >> 12;
                            dst->a = 0x1000;
                        } else if (a <= 0) {
                            *dst = *src3;
                        } else {
                            int new_a = (0x1000800 - (0x1000 - a) * (0x1000 - src3->a)) >> 12;
                            int dst_a = (0x1000 - src3->a) * a / new_a;
                            int src_a = (src3->a << 12) / new_a;
                            dst->y = (dst->y * dst_a + src3->y * src_a) >> 12;
                            dst->cb = (dst->cb * dst_a + src3->cb * src_a) >> 12;
                            dst->cr = (dst->cr * dst_a + src3->cr * src_a) >> 12;
                            dst->a = new_a;
                        }
                    }
                    dst++;
                    src3++;
                }
                if (0 < loop[8]) {
                    int x = loop[8];
                    a = cnv_a * border->alpha >> border->_alpha_shift;
                    if (a < 0x1000) {
                        for (; 0 < x; x--) {
                            if (0x1000 <= src3->a) {
                                *dst = *src3;
                            } else {
                                int da = a * dst->a >> 12;
                                if (0x1000 <= da) {
                                    dst->y += ((src3->y - dst->y) * src3->a) >> 12;
                                    dst->cb += ((src3->cb - dst->cb) * src3->a) >> 12;
                                    dst->cr += ((src3->cr - dst->cr) * src3->a) >> 12;
                                    dst->a = 0x1000;
                                } else if (da <= 0) {
                                    *dst = *src3;
                                } else {
                                    int new_a = (0x1000800 - (0x1000 - da) * (0x1000 - src3->a)) >> 12;
                                    int dst_a = (0x1000 - src3->a) * da / new_a;
                                    int src_a = (src3->a << 12) / new_a;
                                    dst->y = (dst->y * dst_a + src3->y * src_a) >> 12;
                                    dst->cb = (dst->cb * dst_a + src3->cb * src_a) >> 12;
                                    dst->cr = (dst->cr * dst_a + src3->cr * src_a) >> 12;
                                    dst->a = new_a;
                                }
                            }
                            dst++;
                            src3++;
                        }
                    } else {
                        for (; 0 < x; x--) {
                            if (0x1000 <= src3->a) {
                                *dst = *src3;
                            } else {
                                if (0x1000 <= dst->a) {
                                    dst->y += ((src3->y - dst->y) * src3->a) >> 12;
                                    dst->cb += ((src3->cb - dst->cb) * src3->a) >> 12;
                                    dst->cr += ((src3->cr - dst->cr) * src3->a) >> 12;
                                } else if (dst->a <= 0) {
                                    *dst = *src3;
                                } else {
                                    int new_a = (0x1000800 - (0x1000 - dst->a) * (0x1000 - src3->a)) >> 12;
                                    int dst_a = (0x1000 - src3->a) * dst->a / new_a;
                                    int src_a = (src3->a << 12) / new_a;
                                    dst->y = (dst->y * dst_a + src3->y * src_a) >> 12;
                                    dst->cb = (dst->cb * dst_a + src3->cb * src_a) >> 12;
                                    dst->cr = (dst->cr * dst_a + src3->cr * src_a) >> 12;
                                    dst->a = new_a;
                                }
                            }
                            dst++;
                            src3++;
                        }
                    }
                }
                for (int x = loop[9]; 0 < x; x--) {
                    cnv_a -= *src2;
                    src2++;
                    if (0x1000 <= src3->a) {
                        *dst = *src3;
                    } else {
                        a = cnv_a * border->alpha >> border->_alpha_shift;
                        if (a < 0x1000) {
                            a = a * dst->a >> 12;
                        } else {
                            a = dst->a;
                        }
                        if (0x1000 <= a) {
                            dst->y += ((src3->y - dst->y) * src3->a) >> 12;
                            dst->cb += ((src3->cb - dst->cb) * src3->a) >> 12;
                            dst->cr += ((src3->cr - dst->cr) * src3->a) >> 12;
                            dst->a = 0x1000;
                        } else if (a <= 0) {
                            *dst = *src3;
                        } else {
                            int new_a = (0x1000800 - (0x1000 - a) * (0x1000 - src3->a)) >> 12;
                            int dst_a = (0x1000 - src3->a) * a / new_a;
                            int src_a = (src3->a << 12) / new_a;
                            dst->y = (dst->y * dst_a + src3->y * src_a) >> 12;
                            dst->cb = (dst->cb * dst_a + src3->cb * src_a) >> 12;
                            dst->cr = (dst->cr * dst_a + src3->cr * src_a) >> 12;
                            dst->a = new_a;
                        }
                    }
                    dst++;
                    src3++;
                }
                if (0 < loop[10]) {
                    int x = loop[10];
                    a = cnv_a * border->alpha >> border->_alpha_shift;
                    if (a < 0x1000) {
                        for (; 0 < x; x--) {
                            dst->a = dst->a * a >> 12;
                            dst++;
                        }
                    } else {
                        dst += x;
                    }
                }
                for (int x = loop[11]; 0 < x; x--) {
                    cnv_a -= *src2;
                    src2++;
                    a = cnv_a * border->alpha >> border->_alpha_shift;
                    if (a < 0x1000) {
                        dst->a = dst->a * a >> 12;
                    }
                    dst++;
                }
                src1 = (unsigned short*)((int)src1 + cnv_w);
            }
            dst0 += 3;
            yep++;
        }
    }
}

#endif // ifdef PATCH_SWITCH_FAST_BORDER