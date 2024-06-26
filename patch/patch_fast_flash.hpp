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
#include "macro.h"
#ifdef PATCH_SWITCH_FAST_FLASH
#include <exedit.hpp>
#include "util_magic.hpp"
#include "offset_address.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
    // init at exedit load
    // 閃光の高速化
    inline class Flash_t {
        static BOOL no_color_mt_func(AviUtl::MultiThreadFunc original_func_ptr, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
        static BOOL color_mt_func(AviUtl::MultiThreadFunc original_func_ptr, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "fast.flash";
    public:

        struct efFlash_var { // 1a6b7c
            int r_intensity;
            int cx;
            int cy;
            int temp_w;
            int temp_h;
            short color_cb;
            short _padding1;
            int intensity;
            int pixel_range;
            short color_cr;
            short _padding2;
            int temp_x;
            int range;
            int temp_y;
            short color_y;
        };

        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            {
                constexpr int vp_begin = 0x4e8c0;
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x4e8d6 - vp_begin);
                h.store_i16(0x4e8c0 - vp_begin, '\x90\xe8'); // nop; call (rel32)
                h.replaceNearJmp(0x4e8c2 - vp_begin, &no_color_mt_func);
                h.store_i16(0x4e8d0 - vp_begin, '\x90\xe8'); // nop; call (rel32)
                h.replaceNearJmp(0x4e8d2 - vp_begin, &color_mt_func);
            }
        }

        void switching(bool flag) { enabled = flag; }

        bool is_enabled() { return enabled; }
        bool is_enabled_i() { return enabled_i; }

        void switch_load(ConfigReader& cr) {
            cr.regist(key, [this](json_value_s* value) {
                ConfigReader::load_variable(value, enabled);
            });
        }

        void switch_store(ConfigWriter& cw) {
            cw.append(key, enabled);
        }

    } Flash;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_FLASH
