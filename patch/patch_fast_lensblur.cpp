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

#include "patch_fast_lensblur.hpp"
#ifdef PATCH_SWITCH_FAST_LENSBLUR

#include "debug_log.hpp"
#include "patch_fast_cl.hpp"


namespace patch::fast {

    BOOL LensBlur_t::media_mt_func(AviUtl::MultiThreadFunc original_func_ptr, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        if (efpip->obj_w < 8 || efpip->obj_h < 8) {
            return efp->aviutl_exfunc->exec_multi_thread_func(original_func_ptr, efp, efpip);
        }
        try {
            const auto buf_size = cl_t::calc_size(efpip->obj_w, efpip->obj_h, efpip->obj_line) * sizeof(ExEdit::PixelYCA);
            cl::Buffer clmem_src(cl.context, CL_MEM_READ_ONLY, buf_size);
            cl.queue.enqueueWriteBuffer(clmem_src, CL_TRUE, 0, buf_size, efpip->obj_edit);

            cl::Buffer clmem_dst(cl.context, CL_MEM_WRITE_ONLY, buf_size);
            efLensBlur_var& lensblur = *(efLensBlur_var*)uintptr_t(reinterpret_cast<efLensBlur_var*>(GLOBAL::exedit_base + OFS::ExEdit::efLensBlur_var_ptr));
            auto kernel = cl.readyKernel(
                "LensBlur_Media",
                clmem_dst,
                clmem_src,
                efpip->obj_w,
                efpip->obj_h,
                efpip->obj_line,
                lensblur.range,
                lensblur.rangep05_sqr,
                lensblur.range_t3m1,
                lensblur.rangem1_sqr
            );
            cl.queue.enqueueNDRangeKernel(kernel, { 0,0 }, { (size_t)efpip->obj_w ,(size_t)efpip->obj_h });

            cl.queue.enqueueReadBuffer(clmem_dst, CL_TRUE, 0, buf_size, efpip->obj_temp);
        } catch (const cl::Error& err) {
            debug_log("OpenCL Error\n({}) {}", err.err(), err.what());
            return efp->aviutl_exfunc->exec_multi_thread_func(original_func_ptr, efp, efpip);
        }
        return TRUE;
    }

    BOOL LensBlur_t::filter_mt_func(AviUtl::MultiThreadFunc original_func_ptr, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        try {
            const auto buf_size = cl_t::calc_size(efpip->scene_w, efpip->scene_h, efpip->scene_line) * sizeof(ExEdit::PixelYC);
            cl::Buffer clmem_src(cl.context, CL_MEM_READ_ONLY, buf_size);
            cl.queue.enqueueWriteBuffer(clmem_src, CL_TRUE, 0, buf_size, efpip->frame_edit);

            cl::Buffer clmem_dst(cl.context, CL_MEM_WRITE_ONLY, buf_size);
            efLensBlur_var& lensblur = *(efLensBlur_var*)uintptr_t(reinterpret_cast<efLensBlur_var*>(GLOBAL::exedit_base + OFS::ExEdit::efLensBlur_var_ptr));
            auto kernel = cl.readyKernel(
                "LensBlur_Filter",
                clmem_dst,
                clmem_src,
                efpip->scene_w,
                efpip->scene_h,
                efpip->scene_line,
                lensblur.range,
                lensblur.rangep05_sqr,
                lensblur.range_t3m1,
                lensblur.rangem1_sqr
            );
            cl.queue.enqueueNDRangeKernel(kernel, { 0,0 }, { (size_t)efpip->scene_w ,(size_t)efpip->scene_h });

            cl.queue.enqueueReadBuffer(clmem_dst, CL_TRUE, 0, buf_size, efpip->frame_temp);
        } catch (const cl::Error& err) {
            debug_log("OpenCL Error\n({}) {}", err.err(), err.what());
            return efp->aviutl_exfunc->exec_multi_thread_func(original_func_ptr, efp, efpip);
        }

        return TRUE;
    }

} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_LENSBLUR
