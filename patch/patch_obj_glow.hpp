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

#ifdef PATCH_SWITCH_OBJ_GLOW

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"
#include "patch_small_filter.hpp"

namespace patch {

    // init at exedit load
    // グローのバグ修正

    /* オフセットアドレス exedit + 55625 の修正
        曲線移動などで しきい値 が負の数になった時にエラーが出る
    */

    /* 小さいオブジェクトに効果が無いのを修正
        スレッド数より小さいオブジェクトに効果が乗らない
    */

    // 光成分のみチェックがある時に透過情報が失われるのを修正

    inline class obj_Glow_t {

        static void __cdecl mt_mix_calc_alpha(int thi, int thn, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "obj_glow";
    public:

        struct efGlow_var { // 1b2010
            int src_h;
            int src_w;
            int intensity;
            int blur;
            int diffusion_h;
            int diffusion_w;
            int diffusion_length;
            short light_cb;
            short light_cr;
            short light_y;
            short _padding;
            void* buf_temp;
            void* buf_temp2;
            int threshold; // 1b203c
        };

        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            { // オフセットアドレス exedit + 55625 の修正

                auto& cursor = GLOBAL::executable_memory_cursor;

                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x054ed5, 6);
                h.store_i16(0, '\x90\xe8');
                h.replaceNearJmp(2, cursor);
                /*
                    10054ed5 8b4908             mov     ecx,dword ptr [ecx+08]
                    10054ed8 c1e10c             shl     ecx,0c
                    ↓
                    10054ed5 90                 nop
                    10054ed6 e8xXxXxXxX         call    &executable_memory_cursor

                    ; しきい値 track[2] が0未満の時に0にする
                */

                static const char code_put[] =
                    "\x8b\x49\x08"             // mov     ecx,dword ptr [ecx+08]
                    "\x85\xc9"                 // test    ecx,ecx
                    "\x7c\x04"                 // jl      skip,4
                    "\xc1\xe1\x0c"             // shl     ecx,0c
                    "\xc3"                     // ret
                    "\x33\xc9"                 // xor     ecx,ecx
                    "\xc3"                     // ret
                    ;

                memcpy(cursor, code_put, sizeof(code_put) - 1);
                cursor += sizeof(code_put) - 1;
            }

#ifdef PATCH_SWITCH_SMALL_FILTER
            { // 小さいオブジェクトに効果が無いのを修正
                constexpr int ofs[] = { 0x55574,0x556c2,0x557d2,0x55912,0x55a54,0x55cb6,0x55f10,0x56258,0x56a18,0x5710c,0x5776d,0x57dcc,0x5846c };
                constexpr int n = sizeof(ofs) / sizeof(int);
                constexpr int amin = small_filter.address_min(ofs, n);
                constexpr int size = small_filter.address_max(ofs, n) - amin + 1;
                small_filter(ofs, n, amin, size);
            }
#endif // PATCH_SWITCH_SMALL_FILTER

            {
                /*
                if (efp->check[2] == 0) {
                    efp->aviutl_exfunc->exec_multi_thread_func(mt_mix_calc_alpha_original,efp,efpip);
                }
                ↓
                efp->aviutl_exfunc->exec_multi_thread_func(mt_mix_calc_alpha,efp,efpip);
                */
                constexpr int vp_begin = 0x55486;
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x5549a - vp_begin);
                h.store_i16(0x55486 - vp_begin, '\xeb\x08');
                h.store_i32(0x55496 - vp_begin, &mt_mix_calc_alpha);
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
    } Glow;
} // namespace patch

#endif // ifdef PATCH_SWITCH_OBJ_GLOW
