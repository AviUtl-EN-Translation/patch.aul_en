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

#ifdef PATCH_SWITCH_PLAYBACK_POS

#include <exedit.hpp>
#include "config_rw.hpp"
#include "util.hpp"

#include "global.hpp"

namespace patch {

    // init at exedit load
    // 再生位置をトラックバー移動にしてループ再生した時の再生位置が正しくないのを修正
    inline class playback_pos_t {

        static int __cdecl calc_movie_frame(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip, AviUtl::FileInfo* fip, ExEdit::ObjectFilterIndex ofi, ExEdit::struct_exfunc0x04* objinfo);
        static int __cdecl calc_scene_frame(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
        static int __cdecl calc_scene_audio_frame(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "playback_pos";
    public:
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            { // movie_file
                /*
                    100054e8 8b7d64             mov     edi,dword ptr [ebp+64]
                    100054eb ff5714             call    dword ptr [edi+14]
                    100054ee 83c404             add     esp,+04
                    100054f1 50                 push    eax
                    100054f2 ff5704             call    dword ptr [edi+04]
                    100054f5 8b442440           mov     eax,dword ptr [esp+40]
                    100054f9 8b95e4000000       mov     edx,dword ptr [ebp+000000e4]
                    100054ff 83c408             add     esp,+08
                    10005502 8d4c242c           lea     ecx,dword ptr [esp+2c]
                    10005506 8b38               mov     edi,dword ptr [eax]
                    ↓
                    100054e8 8b7d64             mov     edi,dword ptr [ebp+64]
                    100054eb ff5704             call    dword ptr [edi+04]
                    100054ee ff742418           push    dword ptr [esp+18]
                    100054f2 53                 push    ebx
                    100054f3 55                 push    ebp
                    100054f4 e8XxXxXxXx         call    newfunc
                    100054f9 83c40c             add     esp,+0c
                    100054fc 8bf0               mov     esi,eax
                    100054fe 8b442440           mov     eax,dword ptr [esp+40]
                    10005502 8b38               mov     edi,dword ptr [eax]
                    10005504 4f                 dec     edi
                    10005505 eb1f               jmp     skip,+1f
                */
                constexpr int vp_begin = 0x54ed;
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x5507 - vp_begin);
                h.store_i32(0x54ed - vp_begin, '\x04\xff\x74\x24');
                h.store_i32(0x54f1 - vp_begin, '\x18\x53\x55\xe8');
                h.replaceNearJmp(0x54f5 - vp_begin, &calc_movie_frame);
                h.store_i32(0x54f9 - vp_begin, '\x83\xc4\x0c\x8b');
                h.store_i32(0x54fd - vp_begin, '\xf0\x8b\x44\x24');
                h.store_i32(0x5501 - vp_begin, '\x40\x8b\x38\x4f');
                h.store_i16(0x5505 - vp_begin, '\xeb\x1f');
            }
            { // scene
                /*
                    10083471 50                 push    eax
                    10083472 ff5314             call    dword ptr [ebx+14]
                    10083475 83c404             add     esp,+04
                    10083478 50                 push    eax
                    10083479 ff5304             call    dword ptr [ebx+04]
                    1008347c 8b4c2430           mov     ecx,dword ptr [esp+30]
                    10083480 8b6e64             mov     ebp,dword ptr [esi+64]
                    10083483 83c408             add     esp,+08
                    10083486 8b01               mov     eax,dword ptr [ecx]
                    10083488 8b8ee4000000       mov     ecx,dword ptr [esi+000000e4]
                    1008348e 8d0480             lea     eax,dword ptr [eax+eax*4]
                    10083491 8d1480             lea     edx,dword ptr [eax+eax*4]
                    10083494 8d44241c           lea     eax,dword ptr [esp+1c]
                    10083498 50                 push    eax
                    10083499 51                 push    ecx
                    1008349a 8d1c959cffffff     lea     ebx,dword ptr [edx*4+ffffff9c]
                    ↓
                    10083471 50                 push    eax
                    10083472 ff5304             call    dword ptr [ebx+04]
                    10083475 ff742458           push    dword ptr [esp+58]
                    10083479 56                 push    esi
                    1008347a e8XxXxXxXx         call    newfunc
                    1008347f 8b4c2438           mov     ecx,dword ptr [esp+38]
                    10083483 83c408             add     esp,+08
                    10083486 8bf8               mov     edi,eax
                    10083488 8b01               mov     eax,dword ptr [ecx]
                    1008348a 0f1f4000           nop
                    1008348e 8d0480             lea     eax,dword ptr [eax+eax*4]
                    10083491 8d1480             lea     edx,dword ptr [eax+eax*4]
                    10083494 8d1c959cffffff     lea     ebx,dword ptr [edx*4+ffffff9c]
                    1008349b eb0e               jmp     skip,+0e
                */
                constexpr int vp_begin = 0x83474;
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x8349d - vp_begin);
                h.store_i32(0x83474 - vp_begin, '\x04\xff\x74\x24');
                h.store_i32(0x83478 - vp_begin, '\x58\x56\xe8\x00');
                h.replaceNearJmp(0x8347b - vp_begin, &calc_scene_frame);
                h.store_i32(0x8347f - vp_begin, '\x8b\x4c\x24\x38');
                h.store_i8(0x83487 - vp_begin, '\xf8');
                h.store_i32(0x83489 - vp_begin, '\x01\x0f\x1f\x40');
                h.store_i32(0x83495 - vp_begin, '\x1c\x95\x9c\xff');
                h.store_i32(0x83499 - vp_begin, '\xff\xff\xeb\x0e');

            }
            { // scene_audio
                /*
                    10084080 51                 push    ecx
                    10084081 ff5314             call    dword ptr [ebx+14]
                    10084084 83c404             add     esp,+04
                    10084087 50                 push    eax
                    10084088 ff5304             call    dword ptr [ebx+04]
                    1008408b 8b542458           mov     edx,dword ptr [esp+58]
                    1008408f 8b5e64             mov     ebx,dword ptr [esi+64]
                    10084092 83c408             add     esp,+08
                    10084095 8d4c2444           lea     ecx,dword ptr [esp+44]
                    10084099 8b02               mov     eax,dword ptr [edx]
                    1008409b 8b96e4000000       mov     edx,dword ptr [esi+000000e4]
                    100840a1 51                 push    ecx
                    100840a2 52                 push    edx
                    100840a3 8d0480             lea     eax,dword ptr [eax+eax*4]
                    100840a6 8d0480             lea     eax,dword ptr [eax+eax*4]
                    100840a9 8d0480             lea     eax,dword ptr [eax+eax*4]
                    100840ac 8d2cc518fcffff     lea     ebp,dword ptr [eax*8+fffffc18]
                    100840b3 ff5310             call    dword ptr [ebx+10]
                    ↓
                    10084080 51                 push    ecx
                    10084081 ff5304             call    dword ptr [ebx+04]
                    10084084 83c408             add     esp,+08
                    10084087 ff742478           push    dword ptr [esp+78]
                    1008408b 56                 push    esi
                    1008408c e8XxXxXxXx         call    newfunc
                    10084091 8bf8               mov     edi,eax
                    10084093 8b542458           mov     edx,dword ptr [esp+58]
                    10084097 8b02               mov     eax,dword ptr [edx]
                    10084099 eb08               jmp     skip,+08
                    1008409b-100840a2       skip
                    100840a3 8d0480             lea     eax,dword ptr [eax+eax*4]
                    100840a6 8d0480             lea     eax,dword ptr [eax+eax*4]
                    100840a9 8d0480             lea     eax,dword ptr [eax+eax*4]
                    100840ac 8d2cc518fcffff     lea     ebp,dword ptr [eax*8+fffffc18]
                    100840b3 eb08               jmp     skip,+08
                */
                constexpr int vp_begin = 0x84081;
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x840b5 - vp_begin);
                h.store_i8(0x84083 - vp_begin, '\x04');
                h.store_i32(0x84086 - vp_begin, '\x08\xff\x74\x24');
                h.store_i32(0x8408a - vp_begin, '\x78\x56\xe8\x00');
                h.replaceNearJmp(0x8408d - vp_begin, &calc_scene_audio_frame);
                h.store_i16(0x84091 - vp_begin, '\x8b\xf8');
                h.store_i32(0x84093 - vp_begin, '\x8b\x54\x24\x58');
                h.store_i32(0x84097 - vp_begin, '\x8b\x02\xeb\x08');
                h.store_i16(0x840b3 - vp_begin, '\xeb\x08');
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
    } playback_pos;
} // namespace patch

#endif // ifdef PATCH_SWITCH_PLAYBACK_POS
