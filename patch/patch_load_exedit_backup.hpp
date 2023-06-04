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

#ifdef PATCH_SWITCH_LOAD_EXEDIT_BACKUP

#include <exedit.hpp>

#include "global.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // 違うバージョンのバックアップファイルの読み込みを続行出来るように変更
    inline class load_exedit_backup_t {

        static int __stdcall MessageBoxA_wrap(LPCSTR lpCaption, int version);;

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "load_exedit_backup";
    public:
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            { // exedit_dlg_get_load_name
                /*
                    10004df6 56                 push    esi
                    10004df7 ff158ca10910       call    dword ptr [KERNEL32.CloseHandle]
                    10004dfd 6830200400         push    00042030
                    10004e02 68e0d80910         push    1009d8e0
                    10004e07 681cd90910         push    1009d91c
                    10004e0c 6a00               push    +00
                    10004e0e ff1520a30910       call    dword ptr [USER32.MessageBoxA]
                    ↓
                    10004df6 ff742408           push    dword ptr [esp+08]
                    10004dfa 68XxXxXxXx         push    ee+9d8e0
                    10004dff e8XxXxXxXx         call    newfunc
                    10004e04 83f806             cmp     eax,+06
                    10004e07 0f84ccfeffff       jz      ee+4cd9
                    10004e0d 56                 push    esi
                    10004e0e ff15XxXxXxXx       call    ee+9a18c
                */
                constexpr int vp_begin = 0x4df6;
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x4e14 - vp_begin);
                h.store_i32(0x4df6 - vp_begin, '\xff\x74\x24\x08');
                h.store_i8(0x4dfa - vp_begin, '\x68');
                h.store_i32(0x4dfb - vp_begin, GLOBAL::exedit_base + 0x9d8e0); // "拡張編集"
                h.store_i8(0x4dff - vp_begin, '\xe8');
                h.replaceNearJmp(0x4e00 - vp_begin, &MessageBoxA_wrap);
                h.store_i32(0x4e04 - vp_begin, '\x83\xf8\x06\x0f');
                h.store_i32(0x4e08 - vp_begin, '\x84\xcc\xfe\xff');
                h.store_i16(0x4e0c - vp_begin, '\xff\x56');
                h.store_i32(0x4e10 - vp_begin, GLOBAL::exedit_base + 0x9a18c); // [KERNEL32.CloseHandle]
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
    } load_exedit_backup;
} // namespace patch

#endif // ifdef PATCH_SWITCH_LOAD_EXEDIT_BACKUP
