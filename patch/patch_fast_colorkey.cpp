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

    void __declspec(noinline) main_arg_mt(int y, ExEdit::PixelYCA* src0,int obj_w,int linesize,
        short min_y, short max_y, short min_cb, short max_cb, short min_cr, short max_cr) {

        for (; 0 < y; y--) {
            auto src = src0;
            src0 = (ExEdit::PixelYCA*)((int)src0 + linesize);
            for (int x = obj_w; 0 < x; x--) {
                if (min_y <= src->y && src->y <= max_y && min_cb <= src->cb && src->cb <= max_cb && min_cr <= src->cr && src->cr <= max_cr) {
                    src->a = 0;
                }
                src++;
            }
        }
    }

    void __cdecl Colorkey_t::main_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto key = (ExEdit::PixelYC*)efp->exdata_ptr;
        short min_y = key->y - efp->track[0];
        short max_y = key->y + efp->track[0];
        short min_cb = key->cb - efp->track[1];
        short max_cb = key->cb + efp->track[1];
        short min_cr = key->cr - efp->track[1];
        short max_cr = key->cr + efp->track[1];
        int linesize = efpip->obj_line * sizeof(ExEdit::PixelYCA);
        auto src0 = (ExEdit::PixelYCA*)((int)efpip->obj_edit + linesize * thread_id);
        linesize *= thread_num;
        int y = (efpip->obj_h + thread_num - 1 - thread_id) / thread_num;
        main_arg_mt(y, src0, efpip->obj_w, linesize, min_y, max_y, min_cb, max_cb, min_cr, max_cr);
    }

    void __cdecl Colorkey_t::conv1_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        int border_size = efp->track[2];
        int loop3 = efpip->obj_h - border_size * 2 - 1;
        int linesize = efpip->obj_line * sizeof(ExEdit::PixelYCA);
        int x = efpip->obj_w * thread_id / thread_num;
        int* dst0 = (int*)efpip->obj_temp + x;
        short* src0 = (short*)((ExEdit::PixelYCA*)efpip->obj_edit + x) + 3;
        for (x = efpip->obj_w * (thread_id + 1) / thread_num - x; 0 < x; x--) {
            auto src1 = src0;
            auto src2 = src1;
            src0 += 4;
            auto dst = dst0;
            dst0++;
            int sum_a = 0;
            for (int y = border_size; 0 < y; y--) {
                sum_a += *src1;
                src1 = (short*)((int)src1 + linesize);
            }
            for (int y = border_size; 0 <= y; y--) {
                sum_a += *src1;
                *dst = sum_a;
                src1 = (short*)((int)src1 + linesize);
                dst = (int*)((int)dst + linesize);
            }
            for (int y = loop3; 0 < y; y--) {
                sum_a += *src1 - *src2;
                *dst = sum_a;
                src1 = (short*)((int)src1 + linesize);
                src2 = (short*)((int)src2 + linesize);
                dst = (int*)((int)dst + linesize);
            }
            for (int y = border_size; 0 < y; y--) {
                sum_a -= *src2;
                *dst = sum_a;
                src2 = (short*)((int)src2 + linesize);
                dst = (int*)((int)dst + linesize);
            }
        }
    }
    void __cdecl Colorkey_t::conv2_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        int border_size = efp->track[2];
        int border_range = border_size * 2 + 1;
        int loop3 = efpip->obj_w - border_range;
        int border_sq_range = border_range * border_range;
        int oa = (1 - border_size) << 12;
        int thres = (-oa) / border_size;
        int linesize = efpip->obj_line * sizeof(ExEdit::PixelYCA);
        int* src0 = (int*)((int)efpip->obj_temp + linesize * thread_id);
        short* dst0 = (short*)((int)efpip->obj_edit + linesize * thread_id) + 3;
        linesize *= thread_num;

        for (int y = (efpip->obj_h + thread_num - 1 - thread_id) / thread_num; 0 < y; y--) {
            auto src1 = src0;
            auto src2 = src1;
            src0 = (int*)((int)src0 + linesize);
            auto dst = dst0;
            dst0 = (short*)((int)dst0 + linesize);

            int sum_a = 0;
            for (int x = border_size; 0 < x; x--) {
                sum_a += *src1;
                src1++;
            }
            for (int x = border_size; 0 <= x; x--) {
                sum_a += *src1;
                src1++;
                int a = (sum_a / border_sq_range * *dst) >> 12;
                /*
                if (a == 0) {
                  *dst = 0;
                } else {
                  a = (a - 0x1000) * border_size + 0x1000;
                  if (a <= 0) {
                    *dst = 0;
                  } else {
                    *dst = (short)((*dst * a) >> 12);
                  }
                }
                */
                if (a <= thres) {
                    *dst = 0;
                } else {
                    *dst = (short)(*dst * (a * border_size + oa) >> 12);
                }
                dst += 4;
            }
            for (int x = loop3; 0 < x; x--) {
                sum_a += *src1 - *src2;
                src1++;
                src2++;
                int a = (sum_a / border_sq_range * *dst) >> 12;
                if (a <= thres) {
                    *dst = 0;
                } else {
                    *dst = (short)(*dst * (a * border_size + oa) >> 12);
                }
                dst += 4;
            }

            for (int x = border_size; 0 < x; x--) {
                sum_a -= *src2;
                src2++;
                int a = (sum_a / border_sq_range * *dst) >> 12;
                if (a <= thres) {
                    *dst = 0;
                } else {
                    *dst = (short)(*dst * (a * border_size + oa) >> 12);
                }
                dst += 4;
            }
        }
    }

} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_COLORKEY
