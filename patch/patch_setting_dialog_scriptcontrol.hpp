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

#ifdef PATCH_SWITCH_SETTINGDIALOG_SCRIPTCONTROL

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // スクリプト制御を折り畳み状態にしたら次のスクリプト制御のエディットボックスを表示できるようにする

    inline class dialog_scriptcontrol_t {

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "settingdialog_scriptcontrol";
    public:


        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;


            OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x876ee, 9);
            h.store_i32(0, '\x0f\x1f\x40\x00'); // nop
            h.store_i8(4, '\xe8');
            h.store_i32(5, 0x87710 - 0x876f7); // call 10087710

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
    } dialog_scriptcontrol;
} // namespace patch

#endif // ifdef PATCH_SWITCH_SETTINGDIALOG_SCRIPTCONTROL
