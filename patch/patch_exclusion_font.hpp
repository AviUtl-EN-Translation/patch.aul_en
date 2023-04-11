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

#ifdef PATCH_SWITCH_EXCLUSION_FONT

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // 32バイト以上のフォントはバグるので表示しないように変更

    inline class exclusion_font_t {

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "exclusion_font";
    public:

        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            /*
                1008cc88 ff15a8a10910       call    dword ptr [KERNEL32.lstrlenA]
                ↓
                1008cc88 90                 nop
                1008cc89 e9XxXxXxXx         jmp     cursor

                10000000 ff15a8a109Xx       call    exedit + 0x9a1a8
                10000000 83f820             cmp     eax,+20
                10000000 0f8cXxXxXxXx       jl      exedit + 0x8cc8e
                10000000 59                 pop     ecx
                10000000 e9XxXxXxXx         jmp     exedit + 0x8ccc2
            */
            
            auto& cursor = GLOBAL::executable_memory_cursor;

            OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x8cc88, 6);
            h.store_i16(0, '\x90\xe9');
            h.replaceNearJmp(2, cursor);

            store_i16(cursor, '\xff\x15'); cursor += 2;
            store_i32(cursor, GLOBAL::exedit_base + 0x9a1a8); cursor += 4;
            store_i32(cursor, '\x83\xf8\x20\x0f'); cursor += 4;
            store_i8(cursor, '\x8c'); cursor += 1;
            store_i32(cursor, GLOBAL::exedit_base + 0x8cc8e - (int)cursor - 4); cursor += 4;
            store_i16(cursor, '\x59\xe9'); cursor += 2;
            store_i32(cursor, GLOBAL::exedit_base + 0x8ccc2 - (int)cursor - 4); cursor += 4;


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
    } exclusion_font;
} // namespace patch

#endif // ifdef PATCH_SWITCH_EXCLUSION_FONT
