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
#include "patch_fast_lightemission.hpp"
#include "patch_fast_blur.hpp"
#ifdef PATCH_SWITCH_FAST_LIGHTEMISSION
#ifdef PATCH_SWITCH_FAST_BLUR

#include <numbers>

#include "global.hpp"
#include "offset_address.hpp"
#include "util_int.hpp"
#include "debug_log.hpp"
#include <immintrin.h>


namespace patch::fast {
    
    void __cdecl LightEmission_t::vertical_yc_fb_cs_mt_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto le = (efLightEmission_var*)(GLOBAL::exedit_base + OFS::ExEdit::efLightEmission_var_ptr);
        void* buf_ptr = *reinterpret_cast<void**>(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);
        Blur_t::blur_yc_fb_cs_mt(thread_id * le->w / thread_num, (thread_id + 1) * le->w / thread_num, buf_ptr, efpip->frame_temp,
            efpip->scene_line * sizeof(ExEdit::PixelYC), sizeof(ExEdit::PixelYC), le->h, le->size_h);
    }
    
    void __cdecl LightEmission_t::vertical_yc_cs_mt_wrap(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto le = (efLightEmission_var*)(GLOBAL::exedit_base + OFS::ExEdit::efLightEmission_var_ptr);
        void* buf_ptr = *reinterpret_cast<void**>(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);
        Blur_t::blur_yc_cs_mt(thread_id * le->w / thread_num, (thread_id + 1) * le->w / thread_num, buf_ptr, efpip->frame_temp,
            efpip->scene_line * sizeof(ExEdit::PixelYC), sizeof(ExEdit::PixelYC), le->h, le->size_h);
    }
}
#endif // ifdef PATCH_SWITCH_FAST_BLUR
#endif // ifdef PATCH_SWITCH_FAST_LIGHTEMISSION
