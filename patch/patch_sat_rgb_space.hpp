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

#ifdef PATCH_SWITCH_SAT_RGB_SPACE

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // RGB色空間で飽和 の計算を0.93rc1と同じように微調整

    inline class sat_rgb_space_t {

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "sat_rgb_space";
    public:


        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            store_i8(GLOBAL::exedit_base + 0xa992c, 0x4d); // 4b -> 4d
            store_i8(GLOBAL::exedit_base + 0xa992e, 0x01); // 00 -> 01
            store_i8(GLOBAL::exedit_base + 0xa9930, 0xd2); // d1 -> d2
            store_i8(GLOBAL::exedit_base + 0xa9934, 0x92); // 91 -> 92
            store_i8(GLOBAL::exedit_base + 0xa9936, 0xd2); // d1 -> d2
            store_i8(GLOBAL::exedit_base + 0xa993c, 0x24); // 22 -> 24
            store_i8(GLOBAL::exedit_base + 0xa9940, 0x01); // 00 -> 01

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
    } sat_rgb_space;
} // namespace patch

#endif // ifdef PATCH_SWITCH_SAT_RGB_SPACE
