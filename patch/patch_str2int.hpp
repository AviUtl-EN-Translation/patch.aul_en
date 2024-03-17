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

#ifdef PATCH_SWITCH_STR2INT

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {
    // init at exedit load
    // ダイアログ入力などで数値を読み取る際に全角文字も対象にする
    // 文字の先頭にスペースがあっても除外するように変更

    // グリッドの設定の基準フレーム番号に負号を入力できるようにする

    inline class str2int_t {

        static int __cdecl str2int_wrap(char* str);

        bool enabled = true;
        bool enabled_i;

        inline static const char key[] = "str2int";

    public:

        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            // 文字の先頭にスペースがあっても除外し、全角文字も対象にする
            ReplaceNearJmp(GLOBAL::exedit_base + 0x918b0, &str2int_wrap);

            { // グリッドの設定の基準フレーム番号に負号を入力できるようにする
                OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x2516b9, 1).store_i8(0, 0);
                /*
                    1004347e 33c9               xor     ecx,ecx
                    10043480 3c30               cmp     al,30
                    10043482 7c1a               jl      1004349e
                    ↓
                    1004347e e9XxXxXxXx         jmp     cursor
                    10043483 1a                 err

                    10000000 33c9               xor     ecx,ecx
                    10000000 33ff               xor     edi,edi
                    10000000 3c2d               cmp     al,2d ; '-'
                    10000000 7502               jnz     skip,02
                    10000000 47                 inc     edi
                    10000000 42                 inc     edx
                    10000000 e9XxXxXxXx         jmp     ee+43493


                    10043499 83f901             cmp     ecx,+01
                    1004349c 7d05               jnl     100434a3
                    1004349e b901000000         mov     ecx,00000001
                    ↓
                    10043499 85ff               test    edi,edi
                    1004349b 7406               jz      100434a3
                    1004349d f7d9               neg     ecx
                    1004349f 0f1f4000
                */
                auto& cursor = GLOBAL::executable_memory_cursor;
                constexpr int vp_begin = 0x4347e;
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x434a3 - vp_begin);
                h.store_i8(0x4347e - vp_begin, '\xe9');
                h.replaceNearJmp(0x4347f - vp_begin, cursor);
                h.store_i16(0x43499 - vp_begin, '\x85\xff');
                h.store_i32(0x4349b - vp_begin, '\x74\x06\xf7\xd9');
                h.store_i32(0x4349f - vp_begin, '\x0f\x1f\x40\x00');
                
                store_i32(cursor, '\x33\xc9\x33\xff'); cursor += 4;
                store_i32(cursor, '\x3c\x2d\x75\x02'); cursor += 4;
                store_i32(cursor, '\x47\x42\xe9\x00'); cursor += 3;
                store_i32(cursor, GLOBAL::exedit_base + 0x43493 - (int)cursor - 4); cursor += 4;
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

    } str2int;
} // namespace patch
#endif // ifdef PATCH_SWITCH_STR2INT
