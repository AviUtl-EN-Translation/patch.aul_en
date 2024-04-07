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

#include "patch_fast_blur.hpp"
#include "patch_fast_sharp.hpp"
#include "patch_exfilter_sharp.hpp"
#ifdef PATCH_SWITCH_FAST_BLUR
#ifdef PATCH_SWITCH_FAST_SHARP


//#define PATCH_STOPWATCH

namespace patch::fast {

    BOOL __cdecl Sharp_t::func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        if (efp->track[0] <= 0 || efp->track[1] <= 0 || efpip->obj_w <= 0 || efpip->obj_h <= 0) {
            return TRUE;
        }
#ifdef PATCH_SWITCH_EXFILTER_SHARP
        if (efpip->xf4) {
            auto scene_w = efpip->scene_w;
            auto scene_h = efpip->scene_h;
            auto frame_edit = efpip->frame_edit;
            efpip->scene_w = efpip->obj_w;
            efpip->scene_h = efpip->obj_h;
            efpip->frame_edit = efpip->obj_edit;

            BOOL ret = exfilter::Sharp.func_proc(efp, efpip);

            efpip->scene_w = scene_w;
            efpip->scene_h = scene_h;
            efpip->frame_edit = frame_edit;
            return ret;
        }
#endif // ifdef PATCH_SWITCH_EXFILTER_SHARP
        efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&mt1, efp, efpip);
        efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&mt2, efp, efpip);
        return TRUE;
    }
    void __cdecl Sharp_t::mt1(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        int xb = thread_id * efpip->obj_w / thread_num;
        int xe = (thread_id + 1) * efpip->obj_w / thread_num;
        auto src = (ExEdit::PixelYCA*)efpip->obj_edit + xb;
        auto mem = *reinterpret_cast<ExEdit::PixelYCA**>(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);
        auto dst = mem + xb;
        int line = (efpip->obj_line - xe + xb) * sizeof(*src);
        for (int y = efpip->obj_h; 0 < y; y--) {
            for (int x = xe - xb; 0 < x; x--) {
                int src_a = src->a;
                if (src_a <= 0) {
                    *(int32_t*)&dst->y = *(int32_t*)&dst->cr = 0;
                } else if (src_a < 0x1000) {
                    dst->y = src->y * src_a >> 12;
                    dst->cb = src->cb * src_a >> 12;
                    dst->cr = src->cr * src_a >> 12;
                    dst->a = src_a;
                } else {
                    if (0x1000 < src_a) {
                        src->a = 0x1000;
                    }
                    *dst = *src;
                }
                src++;
                dst++;
            }
            src = (ExEdit::PixelYCA*)((int)src + line);
            dst = (ExEdit::PixelYCA*)((int)dst + line);
        }
        int range = efp->track[1];
        int size = range >> 1;
        Blur.blur_yca_csa_mt(xb, xe, efpip->obj_temp, mem,
            efpip->obj_line * sizeof(ExEdit::PixelYCA), sizeof(ExEdit::PixelYCA), efpip->obj_h, range - size);
        if (size) {
            Blur.blur_yca_csa_mt(xb, xe, mem, efpip->obj_temp,
                efpip->obj_line * sizeof(ExEdit::PixelYCA), sizeof(ExEdit::PixelYCA), efpip->obj_h, size);
        }
    }
    void __cdecl Sharp_t::mt2(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        int yb = thread_id * efpip->obj_h / thread_num;
        int ye = (thread_id + 1) * efpip->obj_h / thread_num;
        auto mem = *reinterpret_cast<ExEdit::PixelYCA**>(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);
        int range = efp->track[1];
        int size = range >> 1;
        if (size) {
            Blur.blur_yca_csa_mt(yb, ye, efpip->obj_temp, mem,
                sizeof(ExEdit::PixelYCA), efpip->obj_line * sizeof(ExEdit::PixelYCA), efpip->obj_w, size);
        }
        Blur.blur_yca_csa_mt(yb, ye, mem, efpip->obj_temp,
            sizeof(ExEdit::PixelYCA), efpip->obj_line * sizeof(ExEdit::PixelYCA), efpip->obj_w, range - size);

        int intensity = (efp->track[0] << 9) / 125;
        auto dst = (ExEdit::PixelYCA*)efpip->obj_edit + yb * efpip->obj_line;
        auto unsharpmask = (ExEdit::PixelYCA*)mem + yb * efpip->obj_line;
        int line = (efpip->obj_line - efpip->obj_w) * sizeof(*dst);
        for (int y = ye - yb; 0 < y; y--) {
            for (int x = efpip->obj_w; 0 < x; x--) {
                int a = unsharpmask->a * dst->a >> 12;
                if (0 < a) {
                    int cy, cb, cr;
                    if (0x1000 <= a) {
                        cy = dst->y - unsharpmask->y;
                        cb = dst->cb - unsharpmask->cb;
                        cr = dst->cr - unsharpmask->cr;
                    } else {
                        cy = (dst->y * dst->a - (unsharpmask->y << 12)) / a;
                        cb = (dst->cb * dst->a - (unsharpmask->cb << 12)) / a;
                        cr = (dst->cr * dst->a - (unsharpmask->cr << 12)) / a;
                    }
                    cy = dst->y + (cy * intensity >> 12);
                    cb = dst->cb + (cb * intensity >> 12);
                    cr = dst->cr + (cr * intensity >> 12);
                    if (cy < 0) {
                        cy += 1024;
                        if (cy <= 0) {
                            cb = 0;
                            cr = 0;
                        } else {
                            cb = cb * cy >> 10;
                            cr = cr * cy >> 10;
                        }
                        cy = 0;
                    }
                    dst->y = cy;
                    dst->cb = cb;
                    dst->cr = cr;
                }
                dst++;
                unsharpmask++;
            }
            dst = (ExEdit::PixelYCA*)((int)dst + line);
            unsharpmask = (ExEdit::PixelYCA*)((int)unsharpmask + line);
        }
    }


} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_SHARP
#endif // ifdef PATCH_SWITCH_FAST_BLUR
