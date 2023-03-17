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

#ifdef PATCH_SWITCH_FAILED_FILE_DROP

#include <exedit.hpp>

#include "global.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // exedit.iniに登録されていないファイルをタイムラインにドロップした時にメッセージを表示する

    inline class failed_file_drop_t {

        inline static const char str_failed_drop_msg[] = "拡張編集へドロップされたファイルの種類が判別できませんでした\nexedit.iniを確認してください";
        inline static const char str_failed_pfdrop_msg[] = "現在、別のプロジェクトが開かれています\n読み込みを行う場合、ファイル＞閉じる などで閉じてください";

        static char __stdcall init_flag();
        inline static int flag;
        static int __stdcall lstrcmpiA_wrap3c235(LPCSTR lpString1, LPCSTR lpString2);
        static int __stdcall MessageBoxA_wrap(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "failed_file_drop";
    public:
        
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            {
                /*
                     1003c203 a058cb1410         mov     al,*ini_extension_buf
                     ↓
                     1003c203 e8XxXxXxXx         call    new_func
                */
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x03c203, 5);
                h.store_i8(0, '\xe8');
                h.replaceNearJmp(1, &init_flag);
            }
            {
                /*
                     1003c233 ff15a4a10910       call    dword ptr[KERNEL32.lstrcmpiA]
                     ↓
                     1003c203 90e8XxXxXxXx       call    new_func
                */
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x03c233, 6);
                h.store_i16(0, '\x90\xe8');
                h.replaceNearJmp(2, &lstrcmpiA_wrap3c235);
            }
            {
                auto& cursor = GLOBAL::executable_memory_cursor;

                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x03c454, 4);
                h.replaceNearJmp(0, cursor);
                /*
                     1003c452 0f84f4760000        jz      10043b4c
                     ↓
                     1003c452 0f84XxXxXxXx        jz      executable_memory_cursor
                */

                static const char code_put[] =
                    "\x8b\x0dXXXX"                //  mov     ecx, [this::flag]
                    "\x85\xc9"                    //  test    ecx, ecx
                    "\x75\x26"                    //  jnz     skip +26
                    "\x8d\x8c\x24\xd0\x00\x00\x00"//  lea     ecx,dword ptr [esp+000000d0]
                    "\x51"                        //  push    ecx
                    "\xe8XXXX"                    //  call    1004e1d0 ; ExtractExtension
                    "\x83\xc4\x04"                //  add     esp,+04
                    "\x68\x30\x20\x04\x00"        //  push    0x42030
                    "\x50"                        //  push    eax
                    "\x68XXXX"                    //  push    &str_failed_drop_msg
                    "\xa1XXXX"                    //  mov     eax,[exedit+exedit_hwnd]
                    "\x50"                        //  push    eax
                    "\xe8XXXX"                    //  call    dword ptr [MessageBoxA_wrap]
                    "\xe9"                        //  jmp     10043b4c
                    ;

                memcpy(cursor, code_put, sizeof(code_put) - 1);
                store_i32(cursor + 2, &flag);
                store_i32(cursor + 19, GLOBAL::exedit_base + 0x04e1d0 - (int)cursor - 23);
                store_i32(cursor + 33, &str_failed_drop_msg);
                store_i32(cursor + 38, GLOBAL::exedit_base + OFS::ExEdit::exedit_hwnd);
                store_i32(cursor + 44, (int)&MessageBoxA_wrap - (int)cursor - 48);
                cursor += sizeof(code_put) - 1 + 4;
                store_i32(cursor - 4, GLOBAL::exedit_base + 0x043b4c - (int)cursor);

                // lstrcmpA 0x9a1c0
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
    } failed_file_drop;
} // namespace patch

#endif // ifdef PATCH_SWITCH_FAILED_FILE_DROP
