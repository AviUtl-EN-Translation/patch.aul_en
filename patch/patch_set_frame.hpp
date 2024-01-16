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

#ifdef PATCH_SWITCH_SET_FRAME

#include <aviutl.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at patch load
    // exfunc->set_frame_nを改造する
    // 選択フレームがそれ以上の値になっている場合に調整する

    // exfunc->set_select_frameを改造する
    // 選択フレームが総フレーム数以上の値になっている場合に調整する

    inline class set_frame_t {
        static int __cdecl exfunc_set_frame_n_end(AviUtl::EditHandle* editp, int n);
        static BOOL __cdecl exfunc_set_select_frame_wrap(AviUtl::EditHandle* editp, int s, int e);

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "set_frame";
    public:
        void init() {
            enabled_i = enabled;
            if (!enabled_i)return;
            { // exfunc->set_frame_n
                OverWriteOnProtectHelper h(GLOBAL::aviutl_base + 0x21892, 5);
                h.store_i8(0, 0xe9);
                h.replaceNearJmp(1, &exfunc_set_frame_n_end);
            }
            { // exfunc->set_select_frame
                OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x2cfa0, 4).store_i32(0, &exfunc_set_select_frame_wrap);
            }
        }

        void switching(bool flag) {
            enabled = flag;
        }

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
    } set_frame;
} // namespace patch

#endif // ifdef PATCH_SWITCH_SET_FRAME
