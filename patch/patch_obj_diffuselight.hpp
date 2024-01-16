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
#ifdef PATCH_SWITCH_OBJ_DIFFUSELIGHT

#include <memory>

#include "util.hpp"

#include "config_rw.hpp"

namespace patch {
    // init at exedit load
    /* 拡散校のバグ修正
    強さ 拡散を負の値にすることでエラーが起こるのを修正
    最大画像サイズ+8まで広げられてしまい、不具合を起こすのを修正
    */

    
    inline class obj_diffuselight_t {

        static int __cdecl check_value(void* ecx, void* ebx, DWORD ret, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
            if (efp->track[1] <= 0) return 0;
            if (has_flag(efp->flag, ExEdit::Filter::Flag::Effect)) {
                if (efpip->obj_w <= 0 || efpip->obj_h <= 0) return 0;
            }
            return efp->track[0];
        }

        bool enabled = true;
        bool enabled_i;

        inline static const char key[] = "obj_diffuselight";


    public:
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            constexpr int vp_begin = 0x1c336;
            OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x1c611 - vp_begin);
            { // 拡散を負の値にすることでエラーが起こるのを修正 ついでにオブジェクトサイズが0の時に処理されないように変更
                /*
                    1001c336 8b4344             mov     eax,dword ptr [ebx+44]
                    1001c339 8b00               mov     eax,dword ptr [eax]
                    ↓
                    1001c336 e8XxXxXxXx         call    newfunc
                */
                h.store_i8(0x1c336 - vp_begin, '\xe8');
                h.replaceNearJmp(0x1c337 - vp_begin, &check_value);
            }
            { // 強さを負の値にすることでエラーが起こるのを修正
                /*
                    1001c33d 0f84c0030000       jz      1001c703
                    ↓
                    1001c33d 0f8ec0030000       jle     1001c703
                */
                h.store_i8(0x1c33e - vp_begin, '\x8e');
            }
            { // 最大画像サイズ+8まで広げられてしまい、不具合を起こすのを修正
                /*
                    1001c391 8bbeec000000       mov     edi,dword ptr [esi+000000ec]
                    ↓
                    1001c391 8b3d486719XX       mov     edi,dword ptr [exedit+196748]

                    1001c3b7 8b86f0000000       mov     eax,dword ptr [esi+000000f0]
                    ↓
                    1001c3b7 90                 nop
                    1001c3b8 a1e02019XX         mov     eax,dword ptr [exedit+1920e0]

                */
                h.store_i8(0x1c392 - vp_begin, '\x3d');
                h.store_i32(0x1c393 - vp_begin, GLOBAL::exedit_base + OFS::ExEdit::yca_max_w);

                h.store_i16(0x1c3b7 - vp_begin, '\x90\xa1');
                h.store_i32(0x1c3b9 - vp_begin, GLOBAL::exedit_base + OFS::ExEdit::yca_max_h);
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
    } DiffuseLight;
} // namespace patch
#endif // ifdef PATCH_SWITCH_OBJ_DIFFUSELIGHT
