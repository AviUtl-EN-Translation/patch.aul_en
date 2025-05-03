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

#ifdef PATCH_SWITCH_OBJECT_COPY

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "restorable_patch.hpp"

#include "config_rw.hpp"

namespace patch {
    // init at exedit load
    // タイムラインアイテムのコピー情報の保存先を共有メモリに置き換える
    // これによりキャッシュサイズに応じて自動的にアイテムのコピー情報が消える
    // 消えた場合、右クリックメニューの貼り付けのグレーアウト処理は未実装。一度押した際にグレーアウトする

    inline class object_copy_t {

        static void __cdecl next_index();
        static void __cdecl prev_index();
        static void __cdecl set_buffer_ptr();
        static ExEdit::Object* __cdecl mov_eax_ObjectArrayPointer_wrap();
        static BOOL __cdecl realloc_smem(void** ptr, size_t size);
        static int __cdecl mov_eax_offset_wrap();
        static void __cdecl free_smem();

        bool enabled = true;
        bool enabled_i;

        inline static const char key[] = "object_copy";


        inline static AviUtl::SharedMemoryInfo* smi[2] = {};
        inline static int index = 0;


        struct object_copy_var { // 11edc0
            int offset;
            void* buffer_ptr;
            int layer[100];
            int buffer_size;
            int object_num;
        };

    public:

        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            { // copy
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x1818f, 5);
                h.store_i8(0, '\xe8');
                h.replaceNearJmp(1, &mov_eax_ObjectArrayPointer_wrap);
                ReplaceNearJmp(GLOBAL::exedit_base + 0x181da, &realloc_smem);
            }

            { // paste
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x182e3, 5);
                h.store_i8(0, '\xe8');
                h.replaceNearJmp(1, &mov_eax_offset_wrap);
            }

            { // exit
                ReplaceNearJmp(GLOBAL::exedit_base + 0x31850, &free_smem);
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

    } object_copy;
} // namespace patch
#endif // ifdef PATCH_SWITCH_OBJECT_COPY
