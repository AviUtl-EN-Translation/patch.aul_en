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

#ifdef PATCH_SWITCH_SMALL_FILTER

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at exedit load

    /* 小さいオブジェクトに効果が無いのを修正
        スレッド数より小さいオブジェクトに効果が乗らないのを修正する
    */


    inline class small_filter_t {

        public:

        void operator()(const int* address_list, int n, int ofs_begin, int ofs_size) {
            OverWriteOnProtectHelper h(GLOBAL::exedit_base + ofs_begin, ofs_size);
            for (int i = 0; i < n; i++) {
                h.store_i8(address_list[i] - ofs_begin, '\xeb'); // if_jmp -> jmp
            }
            
        }

        constexpr int address_max(const int* address_list, int n) {
            int amax = address_list[0];
            for (int i = 1; i < n; i++) {
                amax = max(amax, address_list[i]);
            }
            return amax;
        }
        constexpr int address_min(const int* address_list, int n) {
            int amin = address_list[0];
            for (int i = 1; i < n; i++) {
                amin = min(amin, address_list[i]);
            }
            return amin;
        }

    } small_filter;
} // namespace patch

#endif // ifdef PATCH_SWITCH_SMALL_FILTER
