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

#ifdef PATCH_SWITCH_OBJECT_TABLE

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "restorable_patch.hpp"

#include "config_rw.hpp"

namespace patch {
    // init at exedit load
    // 拡張編集オブジェクトのバッファ再確保の際にアドレスが変わることで起こるエラーを対策
    // https://scrapbox.io/ePi5131/000384b8

    // update_ObjectTablesでソートアルゴリズムを選択ソート→コムソートに変更

    inline class object_table_t {

        static BOOL __cdecl object_realloc_wrap(void** pointer, size_t size);
        static void __fastcall comb_sort_current_layer(int sort_s, int sort_e);
        static int __cdecl comb_sort_other_scene();

        bool enabled = true;
        bool enabled_i;

        inline static const char key[] = "object_table";

    public:

        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            // 拡張編集オブジェクトのバッファ再確保の際にアドレスが変わることで起こるエラーを対策
            ReplaceNearJmp(GLOBAL::exedit_base + 0x6d8bc, object_realloc_wrap);


            // update_ObjectTablesでソートアルゴリズムを選択ソート→コムソートに変更
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x2b18b, 7);
                h.store_i8(0, '\xe8');
                h.replaceNearJmp(1, &comb_sort_current_layer);
                h.store_i16(5, '\xeb\x62');
            }
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x2b252, 10);
                h.store_i8(0, '\xe8');
                h.replaceNearJmp(1, &comb_sort_other_scene);
                h.store_i8(5, '\xe9');
                h.store_i32(6, 0xc1); // jmp ee+0x2b31d
            }

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

    } object_table;
} // namespace patch
#endif // ifdef PATCH_SWITCH_OBJECT_TABLE
