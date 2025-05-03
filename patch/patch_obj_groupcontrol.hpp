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

#ifdef PATCH_SWITCH_OBJ_GROUPCONTROL
#include <memory>

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // グループ制御がチェックで無効になっている際にフィルタ効果が実行されるのを修正
    inline class obj_GroupControl_t {
        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "obj_groupcontrol";
    public:
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            auto& cursor = GLOBAL::executable_memory_cursor;

            OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x049782, 4).replaceNearJmp(0, cursor);
            /*
                10049780 0f84f8010000       jz      1004997e
                ↓
                10049780 0f84XxXxXxXx       jz      cursor

                10000000 81fdXxXxXxXx       cmp     ebp,ee+a6ba8 ;efGroupControl
                10000000 0f84XxXxXxXx       jz      ee+497a4
                10000000 e9XxXxXxXx         jmp     ee+4997e
            */
            store_i16(cursor, '\x81\xfd'); cursor += 2;
            store_i32(cursor, GLOBAL::exedit_base + 0x0a6ba8); cursor += 4;
            store_i16(cursor, '\x0f\x84'); cursor += 2;
            store_i32(cursor, GLOBAL::exedit_base + 0x0497a4 - (int)cursor - 4); cursor += 4;
            store_i8(cursor, '\xe9'); cursor++;
            store_i32(cursor, GLOBAL::exedit_base + 0x04997e - (int)cursor - 4); cursor += 4;

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
    } GroupControl;
} // namespace patch

#endif // ifdef PATCH_SWITCH_OBJ_GROUPCONTROL
