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

#ifdef PATCH_SWITCH_FAILED_SJIS_MSGBOX

#include <exedit.hpp>

#include "global.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // ファイル名（パス）が原因で読み込み失敗のエラー表記を変更する

    // **ファイルの読み込みに失敗しました 対応していないフォーマットの可能性があります

    inline class failed_sjis_msgbox_t {

        inline static const char str_new_failed_msg_question[] = "ファイルパスに環境依存文字が含まれています\nファイル名やフォルダ名を確認してください";
        inline static const char str_new_failed_msg_not_found[] = "ファイルが見つかりませんでした\n削除されたファイルを読み込ませようとしたか、ファイル名やフォルダ名に環境依存文字が含まれる可能性があります";
        static int __stdcall MessageBoxA_1(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
        static int __stdcall MessageBoxA_2(LPCSTR path, HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType);
        static int __stdcall MessageBoxA_import_exo(HWND hWnd, LPCSTR lpText, LPCSTR path);
        static int __cdecl MessageBoxA_new_project_exo(LPCSTR path, void* param);
        static int __cdecl MessageBoxA_exa(LPCSTR path);

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "failed_sjis_msgbox";
    public:
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            auto& cursor = GLOBAL::executable_memory_cursor;

            { // audio & movie
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x00522b, 6);
                h.store_i16(0, '\x90\xe8'); // nop; call (rel32)
                h.replaceNearJmp(2, &MessageBoxA_1);
            }

            { // image_file_wndproc
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x00e173, 11);
                h.store_i32(0, '\x8d\x44\x24\x24'); // lea  eax,dword ptr [esp+24]
                h.store_i32(4, '\x50\x90\xe8\x00'); // push eax, nop, call (rel32)
                h.replaceNearJmp(7, &MessageBoxA_2);
            }
            { // border_wndproc
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x0521c3, 11);
                h.store_i32(0, '\x8d\x44\x24\x24'); // lea  eax,dword ptr [esp+24]
                h.store_i32(4, '\x50\x90\xe8\x00'); // push eax, nop, call (rel32)
                h.replaceNearJmp(7, &MessageBoxA_2);
            }
            { // shadow_wndproc
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x088ef6, 11);
                h.store_i32(0, '\x8d\x44\x24\x24'); // lea  eax,dword ptr [esp+24]
                h.store_i32(4, '\x50\x90\xe8\x00'); // push eax, nop, call (rel32)
                h.replaceNearJmp(7, &MessageBoxA_2);
            }

            { // exo
                { // new project
                    /*
                        1002a578 33c0               xor     eax,eax
                        1002a57a 5e                 pop     esi
                        1002a57b 83c408             add     edp,+08
                        ↓
                        1002a578 5e                 pop     esi
                        1002a579 e9XxXxXxXx         jmp     cursor
                        
                        10000000 83c408             add     edp,+08
                        10000000 e9XxXxXxxx         jmp     newfunc
                    */
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x02a578, 6);
                    h.store_i16(0, '\x5e\xe9');
                    h.replaceNearJmp(2, cursor);

                    store_i32(cursor, '\x83\xc4\x08\xe9'); cursor += 4;
                    store_i32(cursor, (int)&MessageBoxA_new_project_exo - (int)cursor - 4); cursor += 4;
                }
                { // import
                    /*
                        10029238 6830200400         push    0x42030
                        1002923d 68e0d80910         push    "拡張編集"
                        10029242 68bc460a10         push    "ファイルの読み込みに失敗しました"
                        10029247 53                 push    ebx ; 0
                        10029248 ff1520a30910       call    dword ptr [USER32.MessageBoxA]
                        ↓
                        10029238 ffb42494000000     push    dword ptr [esp+00000094] ; path
                        1002923f 0f1f00             nop
                        10029242 68bc460a10         push    "ファイルの読み込みに失敗しました"
                        10029247 53                 push    ebx ; 0
                        10029248 90                 nop
                        10029249 e8XxXxXxXx         call    newfunc
                    */
                    constexpr int vp_begin = 0x29238;
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x2924e - vp_begin);
                    h.store_i32(0x29238 - vp_begin, '\xff\xb4\x24\x94');
                    h.store_i16(0x2923c - vp_begin, '\x00\x00');
                    h.store_i32(0x2923e - vp_begin, '\x00\x0f\x1f\x00');
                    h.store_i16(0x29248 - vp_begin, '\x90\xe8');
                    h.replaceNearJmp(0x2924a - vp_begin, &MessageBoxA_import_exo);
                }
            }
            { // exa
                /*
                    1004dbb3 0f8ce2000000       jl      1004dc9b

                    1004dc9b 33c0               xor     eax,eax
                    1004dc9d 5e                 pop     esi
                    1004dc9e c3                 ret

                    ↓
                    1004dbb3 0f8cXxXxXxXx       jl      cursor

                    10000000 5e                 pop     esi
                    10000000 e9XxXxXxXx         jmp     newfunc
                */
                ReplaceNearJmp(GLOBAL::exedit_base + 0x4dbb5, cursor);
                store_i16(cursor, '\x5e\xe9'); cursor += 2;
                store_i32(cursor, (int)&MessageBoxA_exa - (int)cursor - 4); cursor += 4;
            
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
    } failed_sjis_msgbox;
} // namespace patch

#endif // ifdef PATCH_SWITCH_FAILED_SJIS_MSGBOX
