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
#include "patch_exfilter_sharp.hpp"
#ifdef PATCH_SWITCH_FAST_BLUR
#ifdef PATCH_SWITCH_EXFILTER_SHARP


//#define PATCH_STOPWATCH

namespace patch::exfilter {


    BOOL __cdecl Sharp_t::func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        if (efp->track[0] <= 0 || efp->track[1] <= 0) {
            return TRUE;
        }
        efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&mt1, efp, efpip);
        efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&mt2, efp, efpip);
        return TRUE;
    }
    void __cdecl Sharp_t::mt1(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        int xb = thread_id * efpip->scene_w / thread_num;
        int xe = (thread_id + 1) * efpip->scene_w / thread_num;
        auto mem = *reinterpret_cast<ExEdit::PixelYC**>(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);
        int range = efp->track[1];
        int size = range >> 1;
        fast::Blur.blur_yc_csa_mt(xb, xe, efpip->frame_temp, efpip->frame_edit,
            efpip->scene_line * sizeof(ExEdit::PixelYC), sizeof(ExEdit::PixelYC), efpip->scene_h, range - size);
        if (size) {
            fast::Blur.blur_yc_csa_mt(xb, xe, mem, efpip->frame_temp,
                efpip->scene_line * sizeof(ExEdit::PixelYC), sizeof(ExEdit::PixelYC), efpip->scene_h, size);
        }
    }
    void __cdecl Sharp_t::mt2(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        int yb = thread_id * efpip->scene_h / thread_num;
        int ye = (thread_id + 1) * efpip->scene_h / thread_num;
        auto mem = *reinterpret_cast<ExEdit::PixelYC**>(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);
        int range = efp->track[1];
        int size = range >> 1;
        if (size) {
            fast::Blur.blur_yc_csa_mt(yb, ye, efpip->frame_temp, mem,
                sizeof(ExEdit::PixelYC), efpip->scene_line * sizeof(ExEdit::PixelYC), efpip->scene_w, size);
        }
        fast::Blur.blur_yc_csa_mt(yb, ye, mem, efpip->frame_temp,
            sizeof(ExEdit::PixelYC), efpip->scene_line * sizeof(ExEdit::PixelYC), efpip->scene_w, range - size);

        int intensity = (efp->track[0] << 9) / 125;
        auto dst = (ExEdit::PixelYC*)efpip->frame_edit + yb * efpip->scene_line;
        auto unsharpmask = (ExEdit::PixelYC*)mem + yb * efpip->scene_line;
        int line = (efpip->scene_line - efpip->scene_w) * sizeof(*dst);
        for (int y = ye - yb; 0 < y; y--) {
            for (int x = efpip->scene_w; 0 < x; x--) {
                int cy = dst->y + ((dst->y - unsharpmask->y) * intensity >> 12);
                int cb = dst->cb + ((dst->cb - unsharpmask->cb) * intensity >> 12);
                int cr = dst->cr + ((dst->cr - unsharpmask->cr) * intensity >> 12);
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
                dst++;
                unsharpmask++;
            }
            dst = (ExEdit::PixelYC*)((int)dst + line);
            unsharpmask = (ExEdit::PixelYC*)((int)unsharpmask + line);
        }
    }

} // namespace patch::exfilter
#endif // ifdef PATCH_SWITCH_EXFILTER_SHARP
#endif // ifdef PATCH_SWITCH_FAST_BLUR
