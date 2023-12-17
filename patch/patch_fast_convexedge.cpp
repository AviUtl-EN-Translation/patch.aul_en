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

#include "patch_fast_convexedge.hpp"
#include "patch_fast_cl.hpp"
#include "debug_log.hpp"

//#define PATCH_STOPWATCH

namespace patch::fast {

#ifdef PATCH_SWITCH_FAST_CONVEXEDGE
    void __cdecl ConvexEdge_t::mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto ce = reinterpret_cast<efConvexEdge_var*>(GLOBAL::exedit_base + OFS::ExEdit::efConvexEdge_var_ptr);
        int maxx = ce->width * abs(ce->step_x16) >> 16;
        int maxy = ce->width * abs(ce->step_y16) >> 16;
        for (int y = thread_id; y < efpip->obj_h; y += thread_num) {
            int y1, y2;
            if (maxy <= y) {
                y1 = ce->width;
            } else {
                y1 = ((y << 16) + 0xffff) / abs(ce->step_y16);
            }
            if (maxy <= efpip->obj_h - y - 1) {
                y2 = ce->width;
            } else {
                y2 = (((efpip->obj_h - y - 1) << 16) + 0xffff) / abs(ce->step_y16);
            }
            if (ce->step_y16 < 0) {
                std::swap(y1, y2);
            }

            auto src = (ExEdit::PixelYCA*)efpip->obj_edit + y * efpip->obj_line;
            auto dst = (ExEdit::PixelYCA*)efpip->obj_temp + y * efpip->obj_line;
            for (int x = 0; x < efpip->obj_w; x++) {
                if (0 < src->a) {
                    int x1, x2;
                    if (maxx <= x) {
                        x1 = ce->width;
                    } else {
                        x1 = ((x << 16) + 0xffff) / abs(ce->step_x16);
                    }
                    if (maxx <= efpip->obj_w - x - 1) {
                        x2 = ce->width;
                    } else {
                        x2 = (((efpip->obj_w - x - 1) << 16) + 0xffff) / abs(ce->step_x16);
                    }
                    if (ce->step_x16 < 0) {
                        std::swap(x1, x2);
                    }
                    int xx = 0;
                    int yy = 0;
                    int a = 0;
                    short* srca = &src->a;
                    for (int n = min(x1, y1); 0 < n; n--) {
                        xx += ce->step_x16;
                        yy += ce->step_y16;
                        a -= srca[-((yy >> 16) * efpip->obj_line + (xx >> 16)) * 4];
                    }
                    xx = 0;
                    yy = 0;
                    for (int n = min(x2, y2); 0 < n; n--) {
                        xx += ce->step_x16;
                        yy += ce->step_y16;
                        a += srca[((yy >> 16) * efpip->obj_line + (xx >> 16)) * 4];
                    }
                    /*
                    for (int n = ce->width; 0 < n; n--) {
                        xx += ce->step_x16;
                        yy += ce->step_y16;
                        int xxx = x + (xx >> 16);
                        int yyy = y + (yy >> 16);
                        if (0 <= xxx && xxx < efpip->obj_w && 0 <= yyy && yyy < efpip->obj_h) {
                            a += efpip->obj_edit[yyy * efpip->obj_line + xxx].a;
                        }
                        xxx = x - (xx >> 16);
                        yyy = y - (yy >> 16);
                        if (0 <= xxx && xxx < efpip->obj_w && 0 <= yyy && yyy < efpip->obj_h) {
                            a -= efpip->obj_edit[yyy * efpip->obj_line + xxx].a;
                        }
                    }
                    */
                    a = src->y + (int)round((double)a * ce->height_rate);
                    if (src->y <= a || src->y <= 0) {
                        dst->y = a;
                        dst->cb = src->cb;
                        dst->cr = src->cr;
                    } else if (a < 0) {
                        *(int*)&dst->y = 0;
                        // dst->y = 0;
                        // dst->cb = 0;
                        dst->cr = 0;
                    } else {
                        dst->y = a;
                        a = (a << 12) / src->y;
                        dst->cb = src->cb * a >> 12;
                        dst->cr = src->cr * a >> 12;
                    }
                }
                dst->a = src->a;
                src++;
                dst++;
            }
        }
    }
#endif // ifdef PATCH_SWITCH_FAST_CONVEXEDGE


#ifdef PATCH_SWITCH_FAST_CONVEXEDGE_CL
    BOOL __cdecl ConvexEdgeCL_t::mt_func(AviUtl::MultiThreadFunc original_func_ptr, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto ce = reinterpret_cast<efConvexEdge_var*>(GLOBAL::exedit_base + OFS::ExEdit::efConvexEdge_var_ptr);
        if (efpip->obj_w * efpip->obj_h * ce->width < 0x1000000) {
            return efp->aviutl_exfunc->exec_multi_thread_func(original_func_ptr, efp, efpip);
        }
        try {
            const auto buf_size = cl_t::calc_size(efpip->obj_w, efpip->obj_h, efpip->obj_line) * sizeof(ExEdit::PixelYCA);
            cl::Buffer clmem_src(cl.context, CL_MEM_READ_ONLY, buf_size);
            cl.queue.enqueueWriteBuffer(clmem_src, CL_TRUE, 0, buf_size, efpip->obj_edit);

            cl::Buffer clmem_dst(cl.context, CL_MEM_WRITE_ONLY, buf_size);
            auto kernel = cl.readyKernel(
                "ConvexEdge",
                clmem_dst,
                clmem_src,
                efpip->obj_w,
                efpip->obj_h,
                efpip->obj_line,
                ce->width,
                static_cast<float>(ce->height_rate),
                ce->step_x16,
                ce->step_y16
            );
            cl.queue.enqueueNDRangeKernel(kernel, { 0,0 }, { (size_t)efpip->obj_w ,(size_t)efpip->obj_h });

            cl.queue.enqueueReadBuffer(clmem_dst, CL_TRUE, 0, buf_size, efpip->obj_temp);
        } catch (const cl::Error& err) {
            debug_log("OpenCL Error\n({}) {}", err.err(), err.what());
            return efp->aviutl_exfunc->exec_multi_thread_func(original_func_ptr, efp, efpip);
        }
        return TRUE;
    }
#endif // ifdef PATCH_SWITCH_FAST_CONVEXEDGE_CL


} // namespace patch::fast
