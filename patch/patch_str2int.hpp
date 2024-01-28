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

#ifdef PATCH_SWITCH_STR2INT

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {
    // init at exedit load
    // ダイアログ入力などで数値を読み取る際に全角文字も対象にする
    // 文字の先頭にスペースがあっても除外するように変更
    inline class str2int_t {

        static int __cdecl str2int_wrap(char* str);

        bool enabled = true;
        bool enabled_i;

        inline static const char key[] = "str2int";

    public:

        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            ReplaceNearJmp(GLOBAL::exedit_base + 0x918b0, &str2int_wrap);


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

    } str2int;
} // namespace patch
#endif // ifdef PATCH_SWITCH_STR2INT
