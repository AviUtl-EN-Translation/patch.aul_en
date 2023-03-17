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

#ifdef PATCH_SWITCH_OBJ_MASK

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"
#include "patch_small_filter.hpp"

namespace patch {

    // init at exedit load
    // マスクのバグ修正

    /* 小さいオブジェクトに効果が無いのを修正
        スレッド数より小さいオブジェクトに効果が乗らない
    */

    /* マスクサイズが偶数で、ぼかしサイズが大きい時に描画がおかしいことがあったのを修正
    */

    inline class obj_Mask_t {

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "obj_mask";
    public:


        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

#ifdef PATCH_SWITCH_SMALL_FILTER
            { // 小さいオブジェクトに効果が無いのを修正
                constexpr int ofs[] = { 0x695ec, 0x69748, 0x69d15 };
                constexpr int n = sizeof(ofs) / sizeof(int);
                constexpr int amin = small_filter.address_min(ofs, n);
                constexpr int size = small_filter.address_max(ofs, n) - amin + 1;
                small_filter(ofs, n, amin, size);
            }
#endif // PATCH_SWITCH_SMALL_FILTER

            { // マスクサイズが偶数で、ぼかしサイズが大きい時に描画がおかしいことがあったのを修正
                auto& cursor = GLOBAL::executable_memory_cursor;
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x068b31, 7);
                h.store_i32(0, '\x90\x90\xe8\x00');
                h.replaceNearJmp(3, cursor);
                /*
                10068b31 89542444       mov     dword ptr [esp+44],edx
                10068b35 8d1c4a         mov     ebx,edx+ecx*2

                ↓
                10068b31 9090           nop
                10068b33 e8XxXxXxXx     call    cursor

                10000000 4a             dec     edx
                10000000 89542448       mov     dword ptr [esp+48],edx
                10000000 42             inc     edx
                10000000 8d1c4a         mov     ebx,edx+ecx*2
                10000000 4d             dec     ebp
                10000000 c3             ret
                */

                store_i32(cursor, '\x4a\x89\x54\x24');
                store_i32(cursor + 4, '\x48\x42\x8d\x1c');
                store_i32(cursor + 7, '\x1c\x4a\x4d\xc3');

                cursor += 11;
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
    } Mask;
} // namespace patch

#endif // ifdef PATCH_SWITCH_OBJ_MASK
