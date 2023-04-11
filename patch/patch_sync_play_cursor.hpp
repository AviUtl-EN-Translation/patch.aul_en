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

#ifdef PATCH_SWITCH_SYNC_PLAY_CURSOR

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // 再生停止をした時に再生開始位置のフレームが表示されることがあるのを修正
    // undo追加修正の「テキスト字間・行間のバグ修正」によりほぼ起こらなくなったが、
    // 再生ウィンドウの■停止を使うとマレに起こるので修正は行う

    inline class sync_play_cursor_t {

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "sync_play_cursor";
    public:

        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x2aff9, 1).store_i8(0, 0x34);

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
    } sync_play_cursor;
} // namespace patch

#endif // ifdef PATCH_SWITCH_SYNC_PLAY_CURSOR
