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

#ifdef PATCH_SWITCH_OBJ_WAVEFORM

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // 音声波形表示のバグ修正
    // 横解像度0、音声サンプル数＜横幅 の時に範囲外のおかしな波形が表示されるのを修正
    // 波形タイプが0で参照ファイルより読み込ませる時の横解像度の最大値を上げる

    inline class obj_Waveform_t {

        static int __cdecl calc_audio_n(int audio_n, ExEdit::Filter* efp);

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "obj_waveform";
    public:


        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            { // 波形タイプが0で参照ファイルより読み込ませる時の横解像度の最大値を上げる
                /* ; 浮動小数掛け算の無駄を削減して関数を挟む
                    1008e28c db8700010000       fild    dword ptr [edi+00000100]
                    1008e292 89442424           mov     dword ptr [esp+24],eax
                    1008e296 40                 inc     eax
                    1008e297 89442420           mov     dword ptr [esp+20],eax
                    1008e29b 8b442444           mov     eax,dword ptr [esp+44]
                    1008e29f db4018             fild    dword ptr [eax+18]
                    1008e2a2 db87fc000000       fild    dword ptr [edi+000000fc]
                    1008e2a8 db442420           fild    dword ptr [esp+20]
                    1008e2ac d8cb               fmul    st,st(3)
                    1008e2ae d8ca               fmul    st,st(2)
                    1008e2b0 d8f1               fdiv    st,st(1)
                    1008e2b2 e821380000         call    10091ad8 ; double2int
                    1008e2b7 db442424           fild    dword ptr [esp+24]
                    1008e2bb 8bf0               mov     esi,eax
                    1008e2bd d8cb               fmul    st,st(3)
                    1008e2bf d8ca               fmul    st,st(2)
                    1008e2c1 d8f1               fdiv    st,st(1)
                    1008e2c3 e810380000         call    10091ad8 ; double2int
                    1008e2c8 8b4c2420           mov     ecx,dword ptr [esp+20]
                    1008e2cc 2bf0               sub     esi,eax
                    1008e2ce 8b442434           mov     eax,dword ptr [esp+34]
                    1008e2d2 898d10010000       mov     dword ptr [ebp+00000110],ecx
                    1008e2d8 ddd8               fstp    st(0)
                    1008e2da ddd8               fstp    st(0)
                    1008e2dc 8d1406             lea     edx,dword ptr [esi+eax]
                    1008e2df 85c0               test    eax,eax
                    1008e2e1 ddd8               fstp    st(0)
                    1008e2e3 899514010000       mov     dword ptr [ebp+00000114],edx
                    ↓
                    1008e28c 89442424           mov     dword ptr [esp+24],eax
                    1008e290 40                 inc     eax
                    1008e291 89442420           mov     dword ptr [esp+20],eax
                    1008e295 8b442444           mov     eax,dword ptr [esp+44]
                    1008e299 db4018             fild    dword ptr [eax+18] ; st+1
                    1008e29c da8f00010000       fimul   dword ptr [edi+00000100]
                    1008e2a2 dab7fc000000       fidiv   dword ptr [edi+000000fc]
                    1008e2a8 db442420           fild    dword ptr [esp+20] ; st+1
                    1008e2ac d8c9               fmul    st,st(1)
                    1008e2ae e825380000         call    10091ad8 ; double2int ; st-1
                    1008e2b3 8bf0               mov     esi,eax
                    1008e2b5 da4c2424           fimul   dword ptr [esp+24]
                    1008e2b9 e81a380000         call    10091ad8 ; double2int ; st-1
                    1008e2be 8b4c2420           mov     ecx,dword ptr [esp+20]
                    1008e2c2 2bf0               sub     esi,eax
                    1008e2c4 8b442434           mov     eax,dword ptr [esp+34]
                    1008e2c8 898d10010000       mov     dword ptr [ebp+00000110],ecx
                    1008e2ce 8d1406             lea     edx,dword ptr [esi+eax]
                    1008e2d1 53                 push    ebx
                    1008e2d2 56                 push    esi
                    1008e2d3 899514010000       mov     dword ptr [ebp+00000114],edx
                    1008e2d9 e8XxXxXxXx         call    newfunc
                    1008e2de 83c408             add     esp,+08
                    1008e2e1 8bf0               mov     esi,eax
                    1008e2e3 8b442434           mov     eax,dword ptr [esp+34]
                    1008e2e7 85c0               test    eax,eax

                    push ebx ; efp
                    push esi ; audio_n
                    return audio_n;
                */
                char calc_audio_n_bin[] = {
                    "\x89\x44\x24\x24"         // mov     dword ptr [esp+24],eax
                    "\x40"                     // inc     eax
                    "\x89\x44\x24\x20"         // mov     dword ptr [esp+20],eax
                    "\x8b\x44\x24\x44"         // mov     eax,dword ptr [esp+44]
                    "\xdb\x40\x18"             // fild    dword ptr [eax+18] ; st+1
                    "\xda\x8f\x00\x01\x00\x00" // fimul   dword ptr [edi+00000100]
                    "\xda\xb7\xfc\x00\x00\x00" // fidiv   dword ptr [edi+000000fc]
                    "\xdb\x44\x24\x20"         // fild    dword ptr [esp+20] ; st+1
                    "\xd8\xc9"                 // fmul    st,st(1)
                    "\xe8\x25\x38\x00\x00"     // call    10091ad8 ; double2int ; st-1
                    "\x8b\xf0"                 // mov     esi,eax
                    "\xda\x4c\x24\x24"         // fimul   dword ptr [esp+24]
                    "\xe8\x1a\x38\x00\x00"     // call    10091ad8 ; double2int ; st-1
                    "\x8b\x4c\x24\x20"         // mov     ecx,dword ptr [esp+20]
                    "\x2b\xf0"                 // sub     esi,eax
                    "\x8b\x44\x24\x34"         // mov     eax,dword ptr [esp+34]
                    "\x89\x8d\x10\x01\x00\x00" // mov     dword ptr [ebp+00000110],ecx
                    "\x8d\x14\x06"             // lea     edx,dword ptr [esi+eax]
                    "\x53"                     // push    ebx
                    "\x56"                     // push    esi
                    "\x89\x95\x14\x01\x00\x00" // mov     dword ptr [ebp+00000114],edx
                    "\xe8XXXX"                 // call    newfunc
                    "\x83\xc4\x08"             // add     esp,+08
                    "\x8b\xf0"                 // mov     esi,eax
                    "\x8b\x44\x24\x34"         // mov     eax,dword ptr [esp+34]
                    "\x85\xc0"                 // test    eax,eax
                };
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x8e28c, sizeof(calc_audio_n_bin) - 1);
                memcpy(reinterpret_cast<void*>(h.address()), calc_audio_n_bin, sizeof(calc_audio_n_bin) - 1);
                h.replaceNearJmp(0x8e2da - 0x8e28c, &calc_audio_n);
            }
            {
                /*
                if (res_w <= 0) {
                    audio_n = res_w = obj_w;
                } else if (res_w < audio_n) {
                    audio_n = res_w;
                }
                ↓
                if (res_w <= 0) {
                    res_w = obj_w;
                }
                if (res_w < audio_n) {
                    audio_n = res_w;
                }
                */
                constexpr int vp_begin = 0x8e4dd;
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x8e4e5 - vp_begin);
                h.store_i16(0x8e4dd - vp_begin, '\x66\x90'); // nop
                h.store_i16(0x8e4e3 - vp_begin, '\x8b\xf0'); // mov esi,eax
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
    } Waveform;
} // namespace patch

#endif // ifdef PATCH_SWITCH_OBJ_WAVEFORM
