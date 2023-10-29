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

#ifdef PATCH_SWITCH_AUP_LOAD

#include <exedit.hpp>

#include "global.hpp"
#include "util.hpp"
#include "restorable_patch.hpp"

#include "config_rw.hpp"

namespace patch {
    // init at exedit load
    // プロジェクト読み込み時のバグ修正

    // 異常なオブジェクトを削除するように変更

    inline class aup_load_t {
        //static int __cdecl func_project_load_end();

        bool enabled = true;
        bool enabled_i;

        inline static const char key[] = "aup_load";


    public:

        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            { // 選択中のオブジェクトのIDが異常値になっている時にエラーとなるのを修正
                /*
                    100325a1 8b4718             mov     eax,dword ptr [edi+18]
                    100325a4 83c414             add     esp,+14
                    100325a7 85c0               test    eax,eax
                    ↓
                    100325a1 83c414             add     esp,+14
                    100325a4 e8XxXxXxXx         call    cursor

                    10000000 8b4718             mov     eax,dword ptr [edi+18]
                    10000000 3b05XxXxXxXx       cmp     eax,dword ptr [ee+1e0fa0]
                    10000000 7c06               jnl     skip,03
                    10000000 83c8ff             or      eax,0xffffffff
                    10000000 894718             mov     dword ptr [edi+18],eax
                    10000000 85c0               test    eax,eax
                    10000000 c3                 ret

                */
                auto& cursor = GLOBAL::executable_memory_cursor;

                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x325a1, 8);
                h.store_i32(0, '\x83\xc4\x14\xe8');
                h.replaceNearJmp(4, cursor);

                store_i32(cursor, '\x8b\x47\x18\x3b'); cursor += 4;
                store_i8(cursor, '\x05'); cursor++;
                store_i32(cursor, GLOBAL::exedit_base + OFS::ExEdit::ObjectAllocNum); cursor += 4;
                store_i32(cursor, '\x7c\x06\x83\xc8'); cursor += 4;
                store_i32(cursor, '\xff\x89\x47\x18'); cursor += 3;
                store_i32(cursor, '\x18\x85\xc0\xc3'); cursor += 4;
            }
            /*
            { // 読み込み後の最後に実行する関数を追加
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x3261c, 5);
                h.store_i8(0, '\xe8');
                h.replaceNearJmp(1, &func_project_load_end);
            }
            */
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

    } aup_load;
} // namespace patch
#endif // ifdef PATCH_SWITCH_AUP_LOAD
