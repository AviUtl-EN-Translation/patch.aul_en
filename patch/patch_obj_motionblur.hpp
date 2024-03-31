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

#ifdef PATCH_SWITCH_OBJ_MOTIONBLUR

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "util_magic.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // モーションブラーのバグ修正
    // 間隔が0未満になると例外ae33


    inline class obj_MotionBlur_t {
        static void track_clamp(ExEdit::Filter* efp) {
            if (efp->track[0] < 0) {
                efp->track[0] = 0;
            }
            if (efp->track[1] < 1) {
                efp->track[1] = 1;
            }
        }

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "obj_motionblur";
    public:

        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;
            {
                InjectionFunction_push_args_cdecl(GLOBAL::exedit_base + 0x6bd30, (int)&track_clamp, 5, 1, 1, 0);
            }
            { // 残像ON
                /*
                    1006c23f 85c9               test    ecx,ecx
                    1006c241 7415               jz      1006c258
                    -
                    1006c258 8b442434           mov     eax,dword ptr [esp+34]
                    1006c25c 51                 push    ecx
                    1006c25d 83c008             add     eax,+08
                    1006c260 53                 push    ebx
                    1006c261 8944243c           mov     dword ptr [esp+3c],eax
                    1006c265 8b442418           mov     eax,dword ptr [esp+18]
                    1006c269 57                 push    edi
                    1006c26a 56                 push    esi
                    1006c26b 50                 push    eax
                    1006c26c e80febf9ff         call    1000ad80
                    1006c271 668b4c2450         mov     cx,[esp+50]
                    1006c276 8b542424           mov     edx,dword ptr [esp+24]
                    1006c27a 8b442428           mov     eax,dword ptr [esp+28]
                    -
                    1006c28e 83c414             add     esp,+14
                    ↓
                    1006c23f 85c9               test    ecx,ecx
                    1006c241 7e2c               jng     1006c26f
                    -
                    1006c258 51                 push    ecx
                    1006c259 53                 push    ebx
                    1006c25a 57                 push    edi
                    1006c25b 56                 push    esi
                    1006c25c ff742420           push    dword ptr [esp+20]
                    1006c260 e81bebf9ff         call    1000ad80
                    1006c265 83c410             add     esp,+10
                    1006c268 59                 pop     ecx
                    1006c269 eb04               jmp     skip,04
                    1006c26b
                    1006c26f 33c9               xor     ecx,ecx
                    1006c271 8344243408         add     dword ptr [esp+34],+08
                    1006c276 8b542410           mov     edx,dword ptr [esp+10]
                    1006c27a 8b442414           mov     eax,dword ptr [esp+14]
                    -
                    1006c28e 0f1f00             nop
                */

                constexpr int vp_begin = 0x6c241;
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x6c291 - vp_begin);
                h.store_i16(0x6c241 - vp_begin, '\x7e\x2c');
                h.store_i32(0x6c258 - vp_begin, '\x51\x53\x57\x56');
                h.store_i32(0x6c25c - vp_begin, '\xff\x74\x24\x20');
                h.store_i32(0x6c260 - vp_begin, '\xe8\x1b\xeb\xf9');
                h.store_i32(0x6c264 - vp_begin, '\xff\x83\xc4\x10');
                h.store_i32(0x6c268 - vp_begin, '\x59\xeb\x04\x00');
                h.store_i32(0x6c26f - vp_begin, '\x33\xc9\x83\x44');
                h.store_i32(0x6c272 - vp_begin, '\x44\x24\x34\x08');
                h.store_i8(0x6c279 - vp_begin, '\x10');
                h.store_i8(0x6c27d - vp_begin, '\x14');
                h.store_i16(0x6c28e - vp_begin, '\x0f\x1f');
                h.store_i8(0x6c290 - vp_begin, '\x00');
            }
            { // 残像OFF
                /*
                    1006c477 7419               jz      skip,19
                    ↓
                    1006c477 7e19               jng     skip,19
                */
                OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x6c477, 1).store_i8(0, '\x7e');
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
    } MotionBlur;
} // namespace patch

#endif // ifdef PATCH_SWITCH_OBJ_MOTIONBLUR
