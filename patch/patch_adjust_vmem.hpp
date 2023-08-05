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

#ifdef PATCH_SWITCH_ADJUST_VMEM

#include <aviutl.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at patch load
    // 「メモリの確保に失敗しました。キャッシュサイズを調整して再試行しますか？」の処理が意味のあるものになるようにする

    inline class adjust_vmem_t {
        static void __fastcall adjust_vmem(int size);


        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "adjust_vmem";
    public:
        void init() {
            enabled_i = enabled;
            if (!enabled_i)return;

            OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x21e3, 4).replaceNearJmp(0, &adjust_vmem);
            OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x34886, 4).replaceNearJmp(0, &adjust_vmem);
            OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x5ba49, 4).replaceNearJmp(0, &adjust_vmem);
            OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x5bac9, 4).replaceNearJmp(0, &adjust_vmem);

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
    } adjust_vmem;
} // namespace patch

#endif // ifdef PATCH_SWITCH_ADJUST_VMEM
