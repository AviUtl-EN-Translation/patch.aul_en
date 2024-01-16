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

#ifdef PATCH_SWITCH_PLAYBACK_SPEED

#include <exedit.hpp>
#include "config_rw.hpp"
#include "util.hpp"

#include "global.hpp"

namespace patch {

    // init at exedit load
    // n番目の中間点で再生速度を変化させるとnフレーム遅れて反映されるのを修正
    // 中間点の途中で再生速度トラックバーを動かした時にオブジェクトの長さがおかしくなるのを修正
    // 中間点を動かした後に再生速度トラックバーを動かした時にオブジェクトの長さがおかしくなることがあるのを修正
    // オブジェクトの長さを変えた後に元に戻すをして再生速度トラックバーを動かした時にオブジェクトの長さがおかしくなることがあるのを修正
    inline class playback_speed_t {
        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "playback_speed";

        inline static BOOL __cdecl calc_length_if(DWORD ret, ExEdit::Filter* efp) {
            return (((int)efp->processing & 0xffff) - 1 == *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::SettingDialog_ObjIdx));
        }

    public:
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;
            { // n番目の中間点で再生速度を変化させるとnフレーム遅れて反映されるのを修正
                { // movie_file
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x005fd9, 1);
                    h.store_i8(0, '\x90');
                }
                { // audio_file
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x08faab, 1);
                    h.store_i8(0, '\x90');
                }

                { // scene
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x0836b3, 1);
                    h.store_i8(0, '\x90');
                }
                { // scene_audio
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x084297, 1);
                    h.store_i8(0, '\x90');
                }
            }

            { // 中間点の途中で再生速度トラックバーを動かした時にオブジェクトの長さがおかしくなるのを修正
                /*
                    length = efp->frame_start - efp->frame_start_chain - objinfo.frame_begin + objinfo.frame_end;
                    ↓
                    length = 0 - objinfo.frame_begin + objinfo.frame_end;
                */
                { // audio_file
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x006973, 2);
                    h.store_i16(0, '\x33\xc0');
                }
                { // audio_file
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x090343, 2);
                    h.store_i16(0, '\x33\xc0');
                }

                { // scene
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x083d11, 2);
                    h.store_i16(0, '\x33\xc0');
                }
                { // scene_audio
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x084921, 2);
                    h.store_i16(0, '\x33\xc0');
                }
            }
            { // 中間点を動かした後に再生速度トラックバーを動かした時にオブジェクトの長さがおかしくなるのを修正
                auto& cursor = GLOBAL::executable_memory_cursor;
                int addr[4] = { 0x06900, 0x902d0, 0x83cc0, 0x848d0 };
                byte espsub[4] = { 0x6c, 0x6c, 0x30, 0x30 };
                int vaddr[4] = { 0x0d7368, 0x24de58, 0x230980, 0x2309e0 };

                for (int i = 0; i < 4; i++) {
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + addr[i], 13);
                    h.store_i8(0, '\xe9');
                    h.replaceNearJmp(1, cursor);
                    h.store_i16(10, '\x83\xec');
                    h.store_i8(12, espsub[i]);
                    store_i8(cursor, '\xe8'); cursor++;
                    store_i32(cursor, (int)&calc_length_if - (int)cursor - 4); cursor += 4;
                    store_i32(cursor, '\x85\xc0\x75\x01'); cursor += 4;
                    store_i32(cursor, '\xc3\xc7\x05\x00'); cursor += 3;
                    store_i32(cursor, GLOBAL::exedit_base + vaddr[i]); cursor += 4;
                    store_i32(cursor, 0); cursor += 4;
                    store_i8(cursor, '\xe9'); cursor++;
                    store_i32(cursor, GLOBAL::exedit_base + addr[i] + 10 - (int)cursor - 4); cursor += 4;
                }
                { // movie_file
                    /*
                        10006900 83ec6c               sub     esp,+6c
                        10006903 c70568730d1000000000 mov     dword ptr [100d7368],00000000
                        ↓
                        10006900 e9XxXxXxXx           jmp     cursor
                        10006905 8009231000           error
                        1000690a 83ec6c               sub     esp,+6c

                        00000000 e8XxXxXxXx           call    newfunc
                        00000000 85c0                 test    eax,eax
                        00000000 7501                 jnz     skip,+1
                        00000000 c3                   ret
                        00000000 c705XxXxXxXx00000000 mov     dword ptr [ee+d7368],00000000
                        00000000 e9XxXxXxXx           jmp     ee+0690a
                    */
                }
                { // audio_file
                    /*
                        100902d0 83ec6c               sub     esp,+6c
                        100902d3 c70558de241000000000 mov     dword ptr [1024de58],00000000
                        ↓
                        100902d0 e9XxXxXxXx           jmp     cursor
                        100902d5 8009231000           error
                        100902da 83ec6c               sub     esp,+6c

                        00000000 e8XxXxXxXx           call    newfunc
                        00000000 85c0                 test    eax,eax
                        00000000 7501                 jnz     skip,+1
                        00000000 c3                   ret
                        00000000 c705XxXxXxXx00000000 mov     dword ptr [ee+24de58],00000000
                        00000000 e9XxXxXxXx           jmp     ee+902da
                    */
                }
                { // scene
                    /*
                        10083cc0 83ec30               sub     esp,+30
                        10083cc3 c7058009231000000000 mov     dword ptr [10230980],00000000
                        ↓
                        10083cc0 e9XxXxXxXx           jmp     cursor
                        10083cc5 8009231000           error
                        10083cca 83ec30               sub     esp,+30

                        00000000 e8XxXxXxXx           call    newfunc
                        00000000 85c0                 test    eax,eax
                        00000000 7501                 jnz     skip,+1
                        00000000 c3                   ret
                        00000000 c705XxXxXxXx00000000 mov     dword ptr [ee+230980],00000000
                        00000000 e9XxXxXxXx           jmp     ee+83cca
                    */
                }
                { // scene_audio
                    /*
                        100848d0 83ec30               sub     esp,+30
                        100848d3 c705e009231000000000 mov     dword ptr [102309e0],00000000
                        ↓
                        100848d0 e9XxXxXxXx           jmp     cursor
                        100848d5 e009231000           error
                        100848da 83ec30               sub     esp,+30

                        00000000 e8XxXxXxXx           call    newfunc
                        00000000 85c0                 test    eax,eax
                        00000000 7501                 jnz     skip,+1
                        00000000 c3                   ret
                        00000000 c705XxXxXxXx00000000 mov     dword ptr [ee+2309e0],00000000
                        00000000 e9XxXxXxXx           jmp     ee+848da
                    */
                }
            }
            { // オブジェクトの長さを変えて元に戻して再生速度を変えるとオブジェクトの長さがおかしくなるのを修正
                auto& cursor = GLOBAL::executable_memory_cursor;

                { // movie_file audio_file
                    /* movie_file
                        100060d3 0f87b3000000       ja      1000618c
                        ↓
                        100060d3 0f87XxXxXxXx       ja      cursor

                        00000000 83f812             cmp     eax,+12
                        00000000 750a               jnz     skip,+0a
                        00000000 57                 push    edi
                        00000000 56                 push    esi
                        00000000 e8XxXxXxXx         call    ee+06900
                        00000000 83c408             add     esp,+08
                        00000000 e9XxXxXxXx         jmp     ee+0618c
                    */
                    int addr[2] = { 0x060d5, 0x8fb3b };
                    int calcaddr[2] = { 0x06900, 0x902d0 };
                    int retaddr[2] = { 0x0618c, 0x8fbf8 };

                    for (int i = 0; i < 2; i++) {
                        ReplaceNearJmp(GLOBAL::exedit_base + addr[i], cursor);
                        store_i32(cursor, '\x83\xf8\x12\x75'); cursor += 4;
                        store_i32(cursor, '\x0a\x57\x56\xe8'); cursor += 4;
                        store_i32(cursor, GLOBAL::exedit_base + calcaddr[i] - (int)cursor - 4); cursor += 4;
                        store_i32(cursor, '\x83\xc4\x08\xe9'); cursor += 4;
                        store_i32(cursor, GLOBAL::exedit_base + retaddr[i] - (int)cursor - 4); cursor += 4;
                    }
                }
                { // scene
                    /*
                        1008376b 83f80f             cmp     eax,+0f
                        1008376e 7727               ja      10083797
                        ↓
                        1008376b e9XxXxXxXx         jmp     cursor

                        00000000 83f80f             cmp     eax,+0f
                        00000000 0f86XxXxXxXx       jna     ee+83770
                        00000000 83f812             cmp     eax,+12
                        00000000 750a               jnz     skip,+09
                        00000000 56                 push    esi
                        00000000 e8XxXxXxXx         call    ee+83cc0
                        00000000 83c404             add     esp,+04
                        00000000 e9XxXxXxXx         jmp     ee+83797
                    */
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x8376b, 5);
                    h.store_i8(0, '\xe9');
                    h.replaceNearJmp(1, cursor);
                    store_i32(cursor, '\x83\xf8\x0f\x0f'); cursor += 4;
                    store_i8(cursor, '\x86'); cursor++;
                    store_i32(cursor, GLOBAL::exedit_base + 0x83770 - (int)cursor - 4); cursor += 4;
                    store_i32(cursor, '\x83\xf8\x12\x75'); cursor += 4;
                    store_i32(cursor, '\x0a\x56\xe8\x00'); cursor += 3;
                    store_i32(cursor, GLOBAL::exedit_base + 0x83cc0 - (int)cursor - 4); cursor += 4;
                    store_i32(cursor, '\x83\xc4\x04\xe9'); cursor += 4;
                    store_i32(cursor, GLOBAL::exedit_base + 0x83797 - (int)cursor - 4); cursor += 4;
                }
                { // scene_audio
                    /*
                        1008432d 83ff0f             cmp     edi,+0f
                        10084330 7727               ja      10084359
                        ↓
                        1008432d e9XxXxXxXx         jmp     cursor

                        00000000 83ff0f             cmp     edi,+0f
                        00000000 0f86XxXxXxXx       jna     ee+84332
                        00000000 83ff12             cmp     edi,+12
                        00000000 750a               jnz     skip,+09
                        00000000 56                 push    esi
                        00000000 e8XxXxXxXx         call    ee+848d0
                        00000000 83c404             add     esp,+04
                        00000000 e9XxXxXxXx         jmp     ee+84359
                    */
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x8432d, 5);
                    h.store_i8(0, '\xe9');
                    h.replaceNearJmp(1, cursor);
                    store_i32(cursor, '\x83\xff\x0f\x0f'); cursor += 4;
                    store_i8(cursor, '\x86'); cursor++;
                    store_i32(cursor, GLOBAL::exedit_base + 0x84332 - (int)cursor - 4); cursor += 4;
                    store_i32(cursor, '\x83\xff\x12\x75'); cursor += 4;
                    store_i32(cursor, '\x0a\x56\xe8\x00'); cursor += 3;
                    store_i32(cursor, GLOBAL::exedit_base + 0x848d0 - (int)cursor - 4); cursor += 4;
                    store_i32(cursor, '\x83\xc4\x04\xe9'); cursor += 4;
                    store_i32(cursor, GLOBAL::exedit_base + 0x84359 - (int)cursor - 4); cursor += 4;
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
    } playback_speed;
} // namespace patch

#endif // ifdef PATCH_SWITCH_PLAYBACK_SPEED
