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

#ifdef PATCH_SWITCH_RIGHT_TRACKBAR

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // 複数選択状態で右トラックバーの値を変えた時に正常ではないのを修正

    inline class right_trackbar_t {

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "right_trackbar";
    public:

        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x40f63, 1).store_i8(0, 1);
            
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
    } right_trackbar;
} // namespace patch

#endif // ifdef PATCH_SWITCH_RIGHT_TRACKBAR
