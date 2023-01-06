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

#ifdef PATCH_SWITCH_OBJ_PORTIONFILTER

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"
#include "patch_small_filter.hpp"

namespace patch {

    // init at exedit load
    // 部分フィルタのバグ修正

    /* 小さいオブジェクトに効果が無いのを修正
        スレッド数より小さいオブジェクトに効果が乗らない
    */


    inline class obj_PortionFilter_t {

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "obj_portionfilter";
    public:


        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

#ifdef PATCH_SWITCH_SMALL_FILTER
            { // 小さいオブジェクトに効果が無いのを修正
                constexpr int ofs[] = { 0x6de4c,0x6dfa8,0x6e100,0x6e1e0 };
                constexpr int n = sizeof(ofs) / sizeof(int);
                constexpr int amin = small_filter.address_min(ofs, n);
                constexpr int size = small_filter.address_max(ofs, n) - amin + 1;
                small_filter(ofs, n, amin, size);
            }
#endif // PATCH_SWITCH_SMALL_FILTER

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
    } PortionFilter;
} // namespace patch

#endif // ifdef PATCH_SWITCH_OBJ_PORTIONFILTER
