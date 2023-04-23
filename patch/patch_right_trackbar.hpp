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

#ifdef PATCH_SWITCH_RIGHT_TRACKBAR

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // 複数選択状態で右トラックバーの値を変えた時に正常ではないのを修正

    // 移動無し状態の左トラック値を変えた時に右トラックの値が異なる状態になるのを修正
    inline class right_trackbar_t {

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "right_trackbar";
    public:

        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            { // 複数選択状態で右トラックバーの値を変えた時に正常ではないのを修正
                OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x40f63, 1).store_i8(0, 1);
            }

            { // 移動無し状態の左トラック値を変えた時に右トラックの値が異なる状態になるのを修正
            /*
                SendMessageA(exedit_hwnd_177a44,0x111,0x3eb,idx | 0x10000);
                if (TrackMode_14d3b0[idx] == NULL) {
                    TrackbarRight_14def0[idx].track_value = track_data_left->track_value;
                    update_track_2c470(TrackbarRight_14def0[idx],1);
                }
                ↓
                if (TrackMode_14d3b0[idx] == NULL) {
                    TrackbarRight_14def0[idx].track_value = track_data_left->track_value;
                    update_track_2c470(TrackbarRight_14def0[idx],1);
                }
                SendMessageA(exedit_hwnd_177a44,0x111,0x3eb,idx | 0x10000);
            */

                { // 左トラックの左右ボタンを押した時
                /*
                    "\x83\xc4\x08"             // add     esp,+08
                    "\x8b\x04\x9dXXXX"         // mov     eax,dword ptr [ebx*4+1014d3b0]
                    "\x85\xc0"                 // test    eax,eax
                    "\x75\x15"                 // jnz     skip,15
                    "\x8b\x0e"                 // mov     ecx,dword ptr [esi]
                    "\x8d\x85XXXX"             // lea     eax,dword ptr [ebp+1014def0]
                    "\x6a\x01"                 // push    +01
                    "\x50"                     // push    eax
                    "\x89\x08"                 // mov     dword ptr [eax],ecx
                    "\xe8XXXX"                 // call    1002c470
                    "\x83\xc4\x08"             // add     esp,+08
                    "\x8b\xcb"                 // mov     ecx,ebx
                    "\x81\xc9\x00\x00\x01\x00" // or      ecx,00010000
                    "\x51"                     // push    ecx
                    "\x68\xeb\x03\x00\x00"     // push    000003eb
                    "\x8b\x15XXXX"             // mov     edx,dword ptr [10177a44]
                    "\x68\x11\x01\x00\x00"     // push    00000111
                    "\x52"                     // push    edx
                    "\xff\xd7"                 // call    edi
                */
                    int ofs = 0;
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x2d194, 63);
                    h.store_i32(ofs, '\x83\xc4\x08\x8b'); ofs += 4;
                    h.store_i16(ofs, '\x04\x9d'); ofs += 2;
                    h.store_i32(ofs, GLOBAL::exedit_base + OFS::ExEdit::TrackModeInfoArray); ofs += 4;
                    h.store_i32(ofs, '\x85\xc0\x75\x15'); ofs += 4;
                    h.store_i32(ofs, '\x8b\x0e\x8d\x85'); ofs += 4;
                    h.store_i32(ofs, GLOBAL::exedit_base + OFS::ExEdit::TrackRightInfoArray); ofs += 4;
                    h.store_i16(ofs, '\x6a\x01'); ofs += 2;
                    h.store_i32(ofs, '\x50\x89\x08\xe8'); ofs += 4;
                    h.replaceNearJmp(ofs, (void*)(GLOBAL::exedit_base + OFS::ExEdit::update_track)); ofs += 4;
                    h.store_i32(ofs, '\x83\xc4\x08\x8b'); ofs += 4;
                    h.store_i32(ofs, '\xcb\x81\xc9\x00'); ofs += 4;
                    h.store_i32(ofs, '\x00\x01\x00\x51'); ofs += 4;
                    h.store_i32(ofs, '\x68\xeb\x03\x00'); ofs += 4;
                    h.store_i32(ofs, '\x00\x8b\x15\x00'); ofs += 3;
                    h.store_i32(ofs, GLOBAL::exedit_base + OFS::ExEdit::exedit_hwnd); ofs += 4;
                    h.store_i32(ofs, '\x68\x11\x01\x00'); ofs += 4;
                    h.store_i32(ofs, '\x00\x52\xff\xd7');

                }
                { // 左トラックの数字を入力した時
                /*
                    "\x8b\x04\xbdXXXX"         // mov     eax,dword ptr [edi*4+ee+14d3b0]
                    "\x85\xc0"                 // test    eax,eax
                    "\x0f\x85\x15\x00\x00\x00" // jnz     skip,15
                    "\x8b\x0e"                 // mov     ecx,dword ptr [esi]
                    "\x8d\x83XXXX"             // lea     eax,dword ptr [ebx+ee+14def0]
                    "\x6a\x01"                 // push    +01
                    "\x50"                     // push    eax
                    "\x89\x08"                 // mov     dword ptr [eax],ecx
                    "\xe8XXXX"                 // call    ee+2c470
                    "\x83\xc4\x08"             // add     esp,+08
                    "\xa1XXXX"                 // mov     eax,[ee+177a44]
                    "\x8b\xd7"                 // mov     edx,edi
                    "\x83\xc4\x08"             // add     esp,+08
                    "\x81\xca\x00\x00\x01\x00" // or      edx,00010000
                    "\x52"                     // push    edx
                    "\x68\xeb\x03\x00\x00"     // push    000003eb
                    "\x68\x11\x01\x00\x00"     // push    00000111
                    "\x50"                     // push    eax
                    "\xff\x15XXXX"             // call    dword ptr [ee+9a334]
                */
                    int ofs = 0;
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x2d6a5, 70);
                    h.store_i32(ofs, '\x8b\x04\xbd\x00'); ofs += 3;
                    h.store_i32(ofs, GLOBAL::exedit_base + OFS::ExEdit::TrackModeInfoArray); ofs += 4;
                    h.store_i32(ofs, '\x85\xc0\x0f\x85'); ofs += 4;
                    h.store_i32(ofs, '\x15\x00\x00\x00'); ofs += 4;
                    h.store_i32(ofs, '\x8b\x0e\x8d\x83'); ofs += 4;
                    h.store_i32(ofs, GLOBAL::exedit_base + OFS::ExEdit::TrackRightInfoArray); ofs += 4;
                    h.store_i16(ofs, '\x6a\x01'); ofs += 2;
                    h.store_i32(ofs, '\x50\x89\x08\xe8'); ofs += 4;
                    h.replaceNearJmp(ofs, (void*)(GLOBAL::exedit_base + OFS::ExEdit::update_track)); ofs += 4;
                    h.store_i32(ofs, '\x83\xc4\x08\xa1'); ofs += 4;
                    h.store_i32(ofs, GLOBAL::exedit_base + OFS::ExEdit::exedit_hwnd); ofs += 4;
                    h.store_i32(ofs, '\x8b\xd7\x83\xc4'); ofs += 4;
                    h.store_i32(ofs, '\x08\x81\xca\x00'); ofs += 4;
                    h.store_i32(ofs, '\x00\x01\x00\x52'); ofs += 4;
                    h.store_i32(ofs, '\x68\xeb\x03\x00'); ofs += 4;
                    h.store_i8(ofs, '\x00'); ofs++;
                    h.store_i32(ofs, '\x68\x11\x01\x00'); ofs += 4;
                    h.store_i32(ofs, '\x00\x50\xff\x15'); ofs += 4;
                    h.store_i32(ofs, GLOBAL::exedit_base + 0x9a334);
                }
                { // 左トラックの数字をドラッグ移動した時
                /*
                    "\x83\xc4\x08"             // add     esp,+08
                    "\x8b\x04\xbdXXXX"         // mov     eax,dword ptr [edi*4+ee+14d3b0]
                    "\x85\xc0"                 // test    eax,eax
                    "\x75\x15"                 // jnz     skip,15
                    "\x8b\x13"                 // mov     edx,dword ptr [ebx]
                    "\x8d\x86XXXX"             // lea     eax,dword ptr [esi+ee+14def0]
                    "\x6a\x01"                 // push    +01
                    "\x50"                     // push    eax
                    "\x89\x10"                 // mov     dword ptr [eax],edx
                    "\xe8XXXX"                 // call    ee+2c470
                    "\x83\xc4\x08"             // add     esp,+08
                    "\x8b\xc7"                 // mov     eax,edi
                    "\x0d\x00\x00\x01\x00"     // or      eax,00010000
                    "\x50"                     // push    eax
                    "\x68\xeb\x03\x00\x00"     // push    000003eb
                    "\x8b\x0dXXXX"             // mov     ecx,dword ptr [ee+177a44]
                    "\x68\x11\x01\x00\x00"     // push    00000111
                    "\x51"                     // push    ecx
                    "\xff\x15XXXX"             // call    dword ptr [ee+9a334]

                */
                    int ofs = 0;
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x2f68c, 66);
                    h.store_i32(ofs, '\x83\xc4\x08\x8b'); ofs += 4;
                    h.store_i16(ofs, '\x04\xbd'); ofs += 2;
                    h.store_i32(ofs, GLOBAL::exedit_base + OFS::ExEdit::TrackModeInfoArray); ofs += 4;
                    h.store_i32(ofs, '\x85\xc0\x75\x15'); ofs += 4;
                    h.store_i32(ofs, '\x8b\x13\x8d\x86'); ofs += 4;
                    h.store_i32(ofs, GLOBAL::exedit_base + OFS::ExEdit::TrackRightInfoArray); ofs += 4;
                    h.store_i16(ofs, '\x6a\x01'); ofs += 2;
                    h.store_i32(ofs, '\x50\x89\x10\xe8'); ofs += 4;
                    h.replaceNearJmp(ofs, (void*)(GLOBAL::exedit_base + OFS::ExEdit::update_track)); ofs += 4;
                    h.store_i32(ofs, '\x83\xc4\x08\x8b'); ofs += 4;
                    h.store_i32(ofs, '\xc7\x0d\x00\x00'); ofs += 4;
                    h.store_i32(ofs, '\x01\x00\x50\x68'); ofs += 4;
                    h.store_i32(ofs, '\xeb\x03\x00\x00'); ofs += 4;
                    h.store_i16(ofs, '\x8b\x0d'); ofs += 2;
                    h.store_i32(ofs, GLOBAL::exedit_base + OFS::ExEdit::exedit_hwnd); ofs += 4;
                    h.store_i32(ofs, '\x68\x11\x01\x00'); ofs += 4;
                    h.store_i32(ofs, '\x00\x51\xff\x15'); ofs += 4;
                    h.store_i32(ofs, GLOBAL::exedit_base + 0x9a334);

                }
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
    } right_trackbar;
} // namespace patch

#endif // ifdef PATCH_SWITCH_RIGHT_TRACKBAR
