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

#ifdef PATCH_SWITCH_GROUP_ID

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "restorable_patch.hpp"

#include "config_rw.hpp"

namespace patch {
    // init at exedit load
    // aupを使いまわしてオブジェクトの追加と削除を繰り返していくとグループIDが増え続けてグループ化できなくなるのを対策
    // プロジェクト読み込み時にグループIDの最大が高すぎる時にを整理します
    inline class group_id_t {

        static void __cdecl exedit_wrap32594();

        bool enabled = true;
        bool enabled_i;

        inline static const char key[] = "group_id";

    public:

        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            ReplaceNearJmp(GLOBAL::exedit_base + 0x32594, exedit_wrap32594);


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

    } group_id;
} // namespace patch
#endif // ifdef PATCH_SWITCH_GROUP_ID
