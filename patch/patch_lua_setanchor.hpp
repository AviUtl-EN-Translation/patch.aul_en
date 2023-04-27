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

#ifdef PATCH_SWITCH_LUA_SETANCHOR
#include <memory>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "config_rw.hpp"


namespace patch {

    // init at exedit load
    // obj.setanchorを使っているスクリプトが正常に描画できないことがあるのを修正（簡易変形や多角形など）

    inline class lua_setanchor_t {
        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "lua_setanchor";

    public:
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            {
                /*
                    62945-6297c と 6297d-62a8b を入れ替え
                    e8 call や e9 jmp などの値を修正する必要あり
                */

                constexpr int s = 0x62945;
                constexpr int p = 0x6297d;
                constexpr int e = 0x62a8c;
                constexpr int size1 = p - 4 - s;
                constexpr int size2 = e - p;

                OverWriteOnProtectHelper h(GLOBAL::exedit_base + s, e - s);

                byte data62945[size1];
                memcpy(data62945, (void*)h.address(), size1);

                memmove((void*)h.address(), (void*)h.address(p - s), size2);
                h.replaceNearJmp(0x629bc - p, (void*)(GLOBAL::exedit_base + 0x91ad8));
                h.replaceNearJmp(0x62a30 - p, (void*)(GLOBAL::exedit_base + 0x91ad8));

                memcpy((void*)h.address(size2), data62945, size1);
                h.replaceNearJmp(0x62946 - s + size2, (void*)(GLOBAL::exedit_base + 0x63640));
                h.replaceNearJmp(0x62951 - s + size2, (void*)(GLOBAL::exedit_base + 0x63399));
                h.replaceNearJmp(0x62979 - s + size2, (void*)(GLOBAL::exedit_base + 0x63399));

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
    } lua_setanchor;
} // namespace patch

#endif // ifdef PATCH_SWITCH_LUA_SETANCHOR
