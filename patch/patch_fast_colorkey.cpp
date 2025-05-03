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

#include "patch_fast_colorkey.hpp"
#ifdef PATCH_SWITCH_FAST_COLORKEY


//#define PATCH_STOPWATCH

namespace patch::fast {

    void __cdecl main_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto ck = reinterpret_cast<Colorkey_t::efColorkey_var*>(GLOBAL::exedit_base + OFS::ExEdit::efColorkey_var_ptr);
        auto key = (ExEdit::PixelYC*)efp->exdata_ptr;
        short min_y = key->y - efp->track[0];
        short max_y = key->y + efp->track[0];
        short min_cb = key->cb - efp->track[1];
        short max_cb = key->cb + efp->track[1];
        short min_cr = key->cr - efp->track[1];
        short max_cr = key->cr + efp->track[1];
        int y = thread_id * efpip->obj_h / thread_num;
        auto src = reinterpret_cast<ExEdit::PixelYCA*>(efpip->obj_edit) + efpip->obj_line * y;
        int sstep = (efpip->obj_line - efpip->obj_w) * sizeof(*src);
        auto dst = reinterpret_cast<int*>(efpip->obj_temp) + efpip->obj_w * (y + ck->border_size + 1);
        for (y = (thread_id + 1) * efpip->obj_h / thread_num - y; 0 < y; y--) {
            for (int x = efpip->obj_w; 0 < x; x--) {
                if (min_y <= src->y && src->y <= max_y && min_cb <= src->cb && src->cb <= max_cb && min_cr <= src->cr && src->cr <= max_cr && 0 < src->a) {
                    *dst = src->a;
                    src->a = 0;
                } else {
                    *dst = 0;
                }
                src++; dst++;
            }
            src = reinterpret_cast<decltype(src)>((int)src + sstep);
        }
    }

    void __cdecl conv_v_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto ck = reinterpret_cast<Colorkey_t::efColorkey_var*>(GLOBAL::exedit_base + OFS::ExEdit::efColorkey_var_ptr);
        int x = thread_id * efpip->obj_w / thread_num;
        auto dst = reinterpret_cast<int*>(efpip->obj_temp) + x;
        auto src = dst + efpip->obj_w * (ck->border_size + 1);
        int w = (thread_id + 1) * efpip->obj_w / thread_num - x;
        int line = efpip->obj_w;

        memset(dst, 0, w * sizeof(*dst));
        for (int y = ck->border_size; 0 <= y; y--) {
            for (x = w; 0 < x; x--) {
                *dst += *src;
                src++;
                dst++;
            }
            src += line - w;
            dst -= w;
        }
        dst += line;
        for (int y = ck->border_size; 0 < y; y--) {
            for (x = w; 0 < x; x--) {
                *dst = *(dst - line) + *src;
                src++;
                dst++;
            }
            src += line - w;
            dst += line - w;
        }
        for (int y = efpip->obj_h - ck->border_range; 0 < y; y--) {
            for (x = w; 0 < x; x--) {
                *dst = *(dst - line) + *src - *dst;
                src++;
                dst++;
            }
            src += line - w;
            dst += line - w;
        }
        for (int y = ck->border_size; 0 < y; y--) {
            for (x = w; 0 < x; x--) {
                *dst = *(dst - line) - *dst;
                dst++;
            }
            dst += line - w;
        }
    }

    void __cdecl conv_h_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto ck = reinterpret_cast<Colorkey_t::efColorkey_var*>(GLOBAL::exedit_base + OFS::ExEdit::efColorkey_var_ptr);
        int loop3 = efpip->obj_w - ck->border_range;
        int oa = (1 - ck->border_size) << 12;
        int thres = (-oa) / ck->border_size;
        int ye = (thread_id + 1) * efpip->obj_h / thread_num;
        int y = thread_id * efpip->obj_h / thread_num;
        auto src1 = reinterpret_cast<int*>(efpip->obj_temp) + efpip->obj_w * y;
        short* dst0 = &(reinterpret_cast<ExEdit::PixelYCA*>(efpip->obj_edit) + efpip->obj_line * y)->a;

        for (; y < ye; y++) {
            int yrange = min(ck->border_size + min(y + 1, efpip->obj_h - y), ck->border_range);

            auto src2 = src1;
            auto dst = dst0;
            dst0 += efpip->obj_line * 4;

            int cnv = 0;
            for (int x = ck->border_size; 0 < x; x--) {
                cnv += *src1;
                src1++;
            }
            int xrange = ck->border_size;
            for (int x = ck->border_size; 0 <= x; x--) {
                cnv += *src1;
                src1++;
                xrange++;
                if (0 < *dst) {
                    int a = cnv / (xrange * yrange);
                    if (0 < a) {
                        a = ((0x1000 - a) * *dst) >> 12;
                        if (a <= thres) {
                            *dst = 0;
                        } else {
                            *dst = (short)(*dst * (a * ck->border_size + oa) >> 12);
                        }
                    }
                }
                dst += 4;
            }
            int border_sq_range = (xrange * yrange);
            for (int x = loop3; 0 < x; x--) {
                cnv += *src1 - *src2;
                src1++;
                src2++;
                if (0 < *dst) {
                    int a = cnv / border_sq_range;
                    if (0 < a) {
                        a = ((0x1000 - a) * *dst) >> 12;
                        if (a <= thres) {
                            *dst = 0;
                        } else {
                            *dst = (short)(*dst * (a * ck->border_size + oa) >> 12);
                        }
                    }
                }
                dst += 4;
            }

            for (int x = ck->border_size; 0 < x; x--) {
                cnv -= *src2;
                src2++;
                xrange--;
                if (0 < *dst) {
                    int a = cnv / (xrange * yrange);
                    if (0 < a) {
                        a = ((0x1000 - a) * *dst) >> 12;
                        if (a <= thres) {
                            *dst = 0;
                        } else {
                            *dst = (short)(*dst * (a * ck->border_size + oa) >> 12);
                        }
                    }
                }
                dst += 4;
            }
        }
    }

    BOOL __cdecl Colorkey_t::func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto exdata = reinterpret_cast<ExEdit::Exdata::efColorKey*>(efp->exdata_ptr);

        if (exdata->status != 1) {
            return TRUE;
        }
        auto ck = reinterpret_cast<efColorkey_var*>(GLOBAL::exedit_base + OFS::ExEdit::efColorkey_var_ptr);
        int border = min(efp->track[2], (min(efpip->obj_w, efpip->obj_h) - 1) >> 1);
        if (0 < border) {
            ck->border_size = border;
            ck->border_range = border * 2 + 1;
            efp->aviutl_exfunc->exec_multi_thread_func(reinterpret_cast<AviUtl::MultiThreadFunc>(&main_mt), efp, efpip);
            efp->aviutl_exfunc->exec_multi_thread_func(reinterpret_cast<AviUtl::MultiThreadFunc>(&conv_v_mt), efp, efpip);
            efp->aviutl_exfunc->exec_multi_thread_func(reinterpret_cast<AviUtl::MultiThreadFunc>(&conv_h_mt), efp, efpip);
        } else {
            efp->aviutl_exfunc->exec_multi_thread_func(reinterpret_cast<AviUtl::MultiThreadFunc>(GLOBAL::exedit_base + 0x016340), efp, efpip);
        }
        return TRUE;
    }


} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_COLORKEY
