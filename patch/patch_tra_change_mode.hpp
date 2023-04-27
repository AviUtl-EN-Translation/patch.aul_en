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

#ifdef PATCH_SWITCH_TRA_CHANGE_MODE
#include <memory>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "config_rw.hpp"


namespace patch {

    // init at exedit load
    // 移動無しから変更するときに右トラックバーの値を左と同じにする
    // 移動量指定から他に変えた時に中間点オブジェクトが正常に同じにならないのも修正

    inline class tra_change_mode_t {
        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "tra_change_mode";

    public:
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            { // 移動無しから変更するときに右トラックバーの値を左と同じにする
                /*
                    if (tm_old == 5 || tm_new == 0) { // 移動量指定→任意 、任意→移動無し
                        TrackbarRight_14def0[track_id].track_value = TrackbarLeft_14d4c8[track_id].track_value;
                        update_track(TrackbarRight_14def0 + track_id,1);
                    } else if (tm_new == 5) { // 移動量指定以外→移動量指定
                        TrackbarRight_14def0[track_id].track_value = 0;
                        update_track(TrackbarRight_14def0 + track_id,1);
                    }
                    ↓
                    if ((tm_old == 0 && tm_new != 5) || tm_old == 5 || tm_new == 0) { // 移動無し→移動量指定以外 、移動量指定→任意 、任意→移動無し
                        TrackbarRight_14def0[track_id].track_value = TrackbarLeft_14d4c8[track_id].track_value;
                        update_track(TrackbarRight_14def0 + track_id,1);
                    } else if (tm_new == 5) { // 移動量指定以外→移動量指定
                        TrackbarRight_14def0[track_id].track_value = 0;
                        update_track(TrackbarRight_14def0 + track_id,1);
                    }


                    1002c99b 83fb05             cmp     ebx,+05
                    1002c99e 754c               jnz     1002c9ec
                    ↓
                    1002c99b e9XxXxXxXx         jmp     cursor

                    10000000 83fb05             cmp     ebx,+05
                    10000000 0f84XXXX           jz      ee+2c9a0
                    10000000 85ff               test    edi,edi
                    10000000 0f84XXXX           jz      ee+2c9c4
                    10000000 5f                 pop     edi
                    10000000 5e                 pop     esi
                    10000000 5b                 pop     ebx
                    10000000 c3                 ret
                */

                auto& cursor = GLOBAL::executable_memory_cursor;

                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x2c99b, 5);
                h.store_i8(0, '\xe9');
                h.replaceNearJmp(1, cursor);

                store_i32(cursor, '\x83\xfb\x05\x0f'); cursor += 4;
                store_i8(cursor, '\x84'); cursor++;
                store_i32(cursor, GLOBAL::exedit_base + 0x2c9a0 - (int)cursor - 4); cursor += 4;
                store_i32(cursor, '\x85\xff\x0f\x84'); cursor += 4;
                store_i32(cursor, GLOBAL::exedit_base + 0x2c9c4 - (int)cursor - 4); cursor += 4;
                store_i32(cursor, '\x5f\x5e\x5b\xc3'); cursor += 4;
            }
            { // 移動量指定から他に変えた時に中間点オブジェクトが正常に同じにならないのも修正
                /*
                    ; eax = [101e0fa4]
                    ; ebp = base_idx * sizeof(Object) + 0x2f8 + track_id*4
                    ; ecx = midpt_idx * sizeof(Object) + 0x4c0 + track_id*4
                    100360b7 8b1428             mov     edx,dword ptr [eax+ebp] ; edx=ObjectArrayPointer[base_idx].track_mode[track_id]
                    100360ba 89940138feffff     mov     dword ptr [ecx+eax+fffffe38],edx ; ObjectArrayPointer[midpt_idx].track_mode[track_id]=edx
                    100360c1 a1a40f1e10         mov     eax,[101e0fa4] ; 不要
                    100360c6 8b9428c8010000     mov     edx,dword ptr [eax+ebp+000001c8] ; edx=ObjectArrayPointer[base_idx].track_param[track_id]
                    100360cd 891401             mov     dword ptr [ecx+eax],edx ; edx=ObjectArrayPointer[midpt_idx].track_param[track_id]
                    100360d0 8b0da40f1e10       mov     ecx,dword ptr [101e0fa4] ; 8bc8 mov ecx,eax で良い
                    100360d6 8b0429             mov     eax,dword ptr [ecx+ebp]
                    100360d9 a80f               test    al,0f
                    100360db 7544               jnz     10036121
                    ; edi = base_idx * sizeof(Object)
                    ; ebx = midpt_idx * sizeof(Object)
                    ; esi = track_id
                    100360dd 8d0437             lea     eax,dword ptr [edi+esi]
                    100360e0 8d1437             lea     edx,dword ptr [edi+esi]
                    100360e3 8b9491f8000000     mov     edx,dword ptr [ecx+edx*4+000000f8] ; left
                    100360ea 8d0485f8010000     lea     eax,dword ptr [eax*4+000001f8] ; right
                    100360f1 891408             mov     dword ptr [eax+ecx],edx
                    100360f4 8b15a40f1e10       mov     edx,dword ptr [101e0fa4]
                    100360fa 8d0c33             lea     ecx,dword ptr [ebx+esi]
                    100360fd 8b0410             mov     eax,dword ptr [eax+edx]
                    10036100 8d0c8df8000000     lea     ecx,dword ptr [ecx*4+000000f8]
                    10036107 890411             mov     dword ptr [ecx+edx],eax
                    1003610a a1a40f1e10         mov     eax,dword ptr [101e0fa4]
                    1003610f 8d1433             lea     edx,dword ptr [ebx+esi]
                    10036112 8b0c01             mov     ecx,dword ptr [ecx+eax]
                    10036115 898c90f8010000     mov     dword ptr [eax+edx*4+000001f8],ecx
                    
                    ↓

                    100360b7 8b9428c8010000     mov     edx,dword ptr [eax+ebp+000001c8] ; edx=ObjectArrayPointer[base_idx].track_param[track_id]
                    100360be 891401             mov     dword ptr [ecx+eax],edx ; edx=ObjectArrayPointer[midpt_idx].track_param[track_id]
                    100360c1 8b1428             mov     edx,dword ptr [eax+ebp] ; edx=ObjectArrayPointer[base_idx].track_mode[track_id]
                    100360c4 8d8c0138feffff     mov     ecx,ecx+eax+fffffe38 ; ecx=&ObjectArrayPointer[midpt_idx].track_mode[track_id]
                    100360cb 8b01               mov     eax,dword ptr [ecx] ; eax=ObjectArrayPointer[midpt_idx].track_mode[track_id]
                    100360cd 8911               mov     dword ptr [ecx],edx ; ObjectArrayPointer[midpt_idx].track_mode[track_id]=edx
                    100360cf 80e20f             and     dl,0f
                    100360d2 7428               jz      100360fc
                    100360d4 240f               and     al,0f
                    100360d6 7505               jnz     skip,05
                    100360d8 80fa05             cmp     dl,05
                    100360db 751f               jnz     100360fc
                    100360dd 3c05               cmp     al,05
                    100360df 741b               jz      100360fc
                    100360e1 8b01               mov     eax,dword ptr [ecx]
                    100360e3 eb3c               jmp     10036121
                    100360e5

                    100360fc a1XxXxXxXx         mov     eax,dword ptr [ee+1e0fa4]
                    10036101 8d0c37             mov     ecx,edi+esi
                    10036104 8b8c88f8000000     mov     ecx,dword ptr [eax+ecx*4+000000f8] ; ecx=ObjectArrayPointer[base_idx].track_value_left[track_id]
                    1003610b 8d1433             mov     edx,ebx+esi
                    1003610e 898c90f8000000     mov     dword ptr [eax+edx*4+000000f8],ecx ; ObjectArrayPointer[midpt_idx].track_value_left[track_id]=ecx
                    10036115 898c90f8010000     mov     dword ptr [eax+edx*4+000001f8],ecx ; ObjectArrayPointer[midpt_idx].track_value_right[track_id]=ecx

                */

                constexpr int vp_begin = 0x360b8;
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x36115 - vp_begin);
                char bin360b8[] = {
                    "\x94\x28\xc8\x01\x00\x00"    // mov     edx,dword ptr [eax+ebp+000001c8] ; 1文字目の8bは省略
                    "\x89\x14\x01"                // mov     dword ptr [ecx+eax],edx
                    "\x8b\x14\x28"                // mov     edx,dword ptr [eax+ebp]
                    "\x8d\x8c\x01\x38\xfe\xff\xff"// mov     ecx,ecx+eax+fffffe38
                    "\x8b\x01"                    // mov     eax,dword ptr [ecx]
                    "\x89\x11"                    // mov     dword ptr [ecx],edx
                    "\x80\xe2\x0f"                // and     dl,0f
                    "\x74\x28"                    // jz      100360fc
                    "\x24\x0f"                    // and     al,0f
                    "\x75\x05"                    // jnz     skip,05
                    "\x80\xfa\x05"                // cmp     dl,05
                    "\x75\x1f"                    // jnz     100360fc
                    "\x3c\x05"                    // cmp     al,05
                    "\x74\x1b"                    // jz      100360fc
                    "\x8b\x01"                    // mov     eax,dword ptr [ecx]
                    "\xeb\x3c"                    // jmp     10036121
                };
                memcpy(reinterpret_cast<void*>(h.address()), bin360b8, sizeof(bin360b8) - 1);
                h.store_i8(0x360fc - vp_begin, '\xa1');
                h.store_i32(0x360fd - vp_begin, GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
                char bin36101[] = {
                    "\x8d\x0c\x37"                // mov     ecx,edi+esi
                    "\x8b\x8c\x88\xf8\x00\x00\x00"// mov     ecx,dword ptr [eax+ecx*4+000000f8]
                    "\x8d\x14\x33"                // mov     edx,ebx+esi
                    "\x89\x8c\x90\xf8\x00\x00\x00"// mov     dword ptr [eax+edx*4+000000f8],ecx
                };
                memcpy(reinterpret_cast<void*>(h.address(0x36101 - vp_begin)), bin36101, sizeof(bin36101) - 1);
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
    } tra_change_mode;
} // namespace patch

#endif // ifdef PATCH_SWITCH_TRA_CHANGE_MODE
