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

#include "patch_obj_glow.hpp"

#ifdef PATCH_SWITCH_OBJ_GLOW
namespace patch {

    void y2a(ExEdit::PixelYCA* dst) {
        int ya = dst->y;
        if (0x1000 <= ya) {
            dst->a = 0x1000;
        } else if (ya <= 0) {
            dst->y = dst->cb = dst->cr = dst->a = 0;
        } else {
            dst->y = 0x1000;
            dst->cb = ((int)dst->cb << 12) / ya;
            dst->cr = ((int)dst->cr << 12) / ya;
            dst->a = ya;
        }
    }

    void __cdecl obj_Glow_t::mt_mix_calc_alpha(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto glow = (efGlow_var*)(GLOBAL::exedit_base + OFS::ExEdit::efGlow_var_ptr);

        int dst_w = glow->src_w + glow->diffusion_w * 2;
        int dst_h = glow->src_h + glow->diffusion_h * 2;
        int y_end = (thread_id + 1) * dst_h / thread_num;
        if (efp->check[2]) {
            for (int y = thread_id * dst_h / thread_num; y < y_end; y++) {
                auto dst = (ExEdit::PixelYCA*)efpip->obj_temp + y * efpip->obj_line;
                for (int x = dst_w; 0 < x; x--) {
                    y2a(dst);
                    dst++;
                }
            }
            return;
        }
        for (int y = thread_id * dst_h / thread_num; y < y_end; y++) {
            auto dst = (ExEdit::PixelYCA*)efpip->obj_temp + y * efpip->obj_line;
            if (y < glow->diffusion_h || glow->src_h + glow->diffusion_h <= y) {
                for (int x = dst_w; 0 < x; x--) {
                    y2a(dst);
                    dst++;
                }
            } else {
                for (int x = glow->diffusion_w; 0 < x; x--) {
                    y2a(dst);
                    dst++;
                }
                auto src = (ExEdit::PixelYCA*)efpip->obj_edit + (y - glow->diffusion_h) * efpip->obj_line;
                for (int x = glow->src_w; 0 < x; x--) {
                    int srca = (int)src->a;
                    if (0x1000 <= srca) {
                        dst->y += src->y;
                        dst->cb += src->cb;
                        dst->cr += src->cr;
                        dst->a = src->a;
                    } else if (srca <= 0) {
                        y2a(dst);
                    } else {
                        int ya = dst->y + srca;
                        if (0x1000 <= ya) {
                            dst->y += (src->y * srca) >> 12;
                            dst->cb += (src->cb * srca) >> 12;
                            dst->cr += (src->cr * srca) >> 12;
                            dst->a = 0x1000;
                        } else if (ya <= 0) {
                            dst->y = dst->cb = dst->cr = dst->a = 0;
                        } else {
                            dst->y = (src->y * srca + (dst->y << 12)) / ya;
                            dst->cb = (src->cb * srca + (dst->cb << 12)) / ya;
                            dst->cr = (src->cr * srca + (dst->cr << 12)) / ya;
                            dst->a = ya;
                        }
                    }
                    src++;
                    dst++;
                }
                for (int x = glow->diffusion_w; 0 < x; x--) {
                    y2a(dst);
                    dst++;
                }
            }
        }
    }


} // namespace patch
#endif // ifdef PATCH_SWITCH_OBJ_GLOW
