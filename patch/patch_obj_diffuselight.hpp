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
#ifdef PATCH_SWITCH_OBJ_DIFFUSELIGHT

#include <memory>

#include "util.hpp"

#include "config_rw.hpp"

namespace patch {
    // init at exedit load
    /* 拡散校のバグ修正
    最大画像サイズ+8まで広げられてしまい、不具合を起こすのを修正
    */

    
    inline class obj_diffuselight_t {

        bool enabled = true;
        bool enabled_i;

        inline static const char key[] = "obj_diffuselight";


    public:
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            constexpr int vp_begin = 0x1c392;
            OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x1c3bd - vp_begin);
            /*
                1001c391 8bbeec000000       mov     edi,dword ptr [esi+000000ec]
                ↓
                1001c391 8b3d486719XX       mov     edi,dword ptr [exedit+196748]

                1001c3b7 8b86f0000000       mov     eax,dword ptr [esi+000000f0]
                ↓
                1001c3b7 90                 nop
                1001c3b8 a1e02019XX         mov     eax,dword ptr [exedit+1920e0]

            */
            h.store_i8(0x1c392 - vp_begin, '\x3d');
            h.store_i32(0x1c393 - vp_begin, GLOBAL::exedit_base + OFS::ExEdit::yca_max_w);

            h.store_i16(0x1c3b7 - vp_begin, '\x90\xa1');
            h.store_i32(0x1c3b9 - vp_begin, GLOBAL::exedit_base + OFS::ExEdit::yca_max_h);

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
    } DiffuseLight;
} // namespace patch
#endif // ifdef PATCH_SWITCH_OBJ_DIFFUSELIGHT
