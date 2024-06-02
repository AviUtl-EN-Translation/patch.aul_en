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

#ifdef PATCH_SWITCH_OBJ_BLUR

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"
#include "patch_small_filter.hpp"

namespace patch {

    // init at exedit load
    // ぼかしのバグ修正
    /* 曲線移動などで 範囲 が負の数になった時にエラーが出るのを修正
        フィルタオブジェクトで0xc0000094 オフセットアドレス[0xfe64, 0x100d3, 0x115d1, 0x1188b]
        エフェクトで0xc0000005 オフセットアドレス[0xec7b, 0xf573, 0x1034c, 0x10c8a]
    */
    // 縦横比が-100未満や100超で表示がおかしくなるのを修正

    /* 小さいオブジェクトに効果が無いのを修正 (fast.blurにより不要に)
        スレッド数より小さいオブジェクトに効果が乗らない
    */

    inline class obj_Blur_t {

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "obj_blur";
    public:

        void init() {
            enabled_i = enabled;
            if (!enabled_i)return;

            { // 曲線移動などで 範囲 が負の数になった時にエラーが出るのを修正
                /*
                    if(range == 0) return; // jz
                    ↓
                    if(range <= 0) return; // jle
                */
                OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x0e301, 1).store_i8(0, '\x8e');
            }
            
            { // 縦横比が-100未満や100超で表示がおかしくなるのを修正
                /*
                    1000e31f b8d34d6210         mov     eax,10624dd3

                    1000e33f b8d34d6210         mov     eax,10624dd3

                    ↓

                    10000000 85c9               test    ecx,ecx
                    10000000 7d02               jnl     skip,02
                    10000000 33c9               xor     ecx,ecx
                    10000000 b8d34d6210         mov     eax,10624dd3
                    10000000 c3                 ret
                */
                auto& cursor = GLOBAL::executable_memory_cursor;

                static const char code_put[] = {
                    "\x85\xc9"                 // test    ecx,ecx
                    "\x7d\x02"                 // jnl     skip,02
                    "\x33\xc9"                 // xor     ecx,ecx
                    "\xb8\xd3\x4d\x62\x10"     // mov     eax,10624dd3
                    "\xc3"                     // ret
                };

                constexpr int vp_begin = 0x0e31f;
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x0e344 - vp_begin);
                h.store_i8(0x0e31f - vp_begin, '\xe8');
                h.replaceNearJmp(0x0e320 - vp_begin, cursor);
                h.store_i8(0x0e33f - vp_begin, '\xe8');
                h.replaceNearJmp(0x0e340 - vp_begin, cursor);

                memcpy(cursor, code_put, sizeof(code_put) - 1);
                cursor += sizeof(code_put) - 1;
            }

            /*
#ifdef PATCH_SWITCH_SMALL_FILTER
            { // 小さいオブジェクトに効果が無いのを修正
                constexpr int ofs[] = { 0x0eb05,0x0ef11,0x0f34c,0x0f818,0x0fce2,0x0ff72,0x101cc,0x10618,0x10a6c,0x10f58,0x11432,0x11702 };
                constexpr int n = sizeof(ofs) / sizeof(int);
                constexpr int amin = small_filter.address_min(ofs, n);
                constexpr int size = small_filter.address_max(ofs, n) - amin + 1;
                small_filter(ofs, n, amin, size);
            }
#endif // PATCH_SWITCH_SMALL_FILTER
*/

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
    } Blur;
} // namespace patch

#endif // ifdef PATCH_SWITCH_OBJ_BLUR
