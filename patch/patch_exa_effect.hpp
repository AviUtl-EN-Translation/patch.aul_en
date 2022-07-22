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

#ifdef PATCH_SWITCH_EXA_EFFECT
#include <memory>

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

namespace patch {

    // init at exedit load
    // エフェクトのエイリアスを格納フォルダに保存した時に設定ダイアログの[+]の所に出てこない
    inline class exa_effect_t {
        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "exa_effect";
    public:
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            auto& cursor = GLOBAL::executable_memory_cursor;

            int hMenu_Dialog = *reinterpret_cast<int*>(GLOBAL::exedit_base + 0x039e4b);
            OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x039e49, 6);
            h.store_i16(0, '\x90\xe8');
            h.store_i32(2, cursor - (GLOBAL::exedit_base + 0x039e4f));
            /*
                10039e49 8b0dXxXxXxXx  mov     ecx,dword ptr [hMenu_Dialog]
                10039e4f
                ↓
                10039e49 90e8XxXxXxXx  jmp     executable_memory_cursor
                10039e4f

            */

            static const char code_put[] =
                "\xc7\x44\x24\x14\x00\x00\x00\x00"  // mov  dword ptr [esp+14],0
                "\x8b\x0dXXXX"                      // mov  ecx,dword ptr [hMenu_Dialog]
                "\xc3"                              // ret
                ;

            memcpy(cursor, code_put, sizeof(code_put) - 1);
            cursor += sizeof(code_put) - 1;
            store_i32(cursor - 5, hMenu_Dialog);
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
    } exa_effect;
} // namespace patch

#endif // ifdef PATCH_SWITCH_EXA_EFFECT
