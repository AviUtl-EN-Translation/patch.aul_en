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

#ifdef PATCH_SWITCH_SETTINGDIALOG_NEXT
#include <memory>

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // 中間点オブジェクトを消した際に次の選択がされないことがあるのを修正
    // オブジェクトを消した際、次に選択されるオブジェクトを変更（左右のオブジェクトより上下を優先）

    inline class dialog_next_t {

        static int __cdecl get_near_object_idx_wrap(int object_idx, int flag);

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "settingdialog_next";
    public:
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            ReplaceNearJmp(GLOBAL::exedit_base + 0x446ec, &get_near_object_idx_wrap);
            

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
    } dialog_next;
} // namespace patch

#endif // ifdef PATCH_SWITCH_SETTINGDIALOG_NEXT
