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

#ifdef PATCH_SWITCH_PASTE_POS

#include <exedit.hpp>
#include "config_rw.hpp"
#include "util.hpp"

#include "global.hpp"

namespace patch {

    // init at exedit load
    /* オブジェクトの貼り付け位置を修正
    ・クリック時に保存されるのがウィンドウ座標のみのため、スクロールしたり拡大率を変えた後の貼り付けがその時のウィンドウ座標の位置になるのを変更
    ・フレーム数の小さいシーンへ貼り付けを行った際に貼り付け位置がおかしいのを修正
    */
    inline class paste_pos_t {

        static void __cdecl set_pos(int x, int y);
        static int __cdecl get_click_frame(int x);
        static int __cdecl get_click_layer(int y);

        inline static int timeline_click_frame = 0;
        inline static int timeline_click_layer = 0;

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "paste_pos";
    public:
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            { // クリック時に保存されるのがウィンドウ座標のみのため、スクロールしたり拡大率を変えた後の貼り付けがその時のウィンドウ座標の位置になるのを変更
                { // クリック時にウィンドウ座標を取得するのと同時にフレーム位置とレイヤー位置を取得する
                    { // 左クリック
                        /*
                            1003c7f5 83fb40             cmp     ebx,+40
                            1003c7f8 891db4601410       mov     dword ptr [101460b4],ebx
                            1003c7fe 893d44671910       mov     dword ptr [10196744],edi
                            ↓
                            1003c7f5 50                 push    eax
                            1003c7f6 57                 push    edi
                            1003c7f7 53                 push    ebx
                            1003c7f8 e8XxXxXxXx         call    new_func
                            1003c7fd 5b                 pop     ebx
                            1003c7fe 5f                 pop     edi
                            1003c7ff 58                 pop     eax
                            1003c800 90                 nop
                            1003c801 83fb40             cmp     ebx,+40
                        */
                        constexpr int vp_begin = 0x3c7f5;
                        OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x3c804 - vp_begin);
                        h.store_i32(0x3c7f5 - vp_begin, '\x50\x57\x53\xe8');
                        h.replaceNearJmp(0x3c7f9 - vp_begin, &set_pos);
                        h.store_i32(0x3c7fd - vp_begin, '\x5b\x5f\x58\x90');
                        h.store_i32(0x3c800 - vp_begin, '\x90\x83\xfb\x40');
                    }
                    { // 右クリック
                        /*
                            1003d112 83f940             cmp     ecx,+40
                            1003d115 890db4601410       mov     dword ptr [101460b4],ecx
                            1003d11b 891d44671910       mov     dword ptr [10196744],ebx
                            ↓
                            1003d112 50                 push    eax
                            1003d113 53                 push    ebx
                            1003d114 51                 push    ecx
                            1003d115 e8XxXxXxXx         call    new_func
                            1003d11a 59                 pop     ecx
                            1003d11b 5b                 pop     ebx
                            1003d11c 58                 pop     eax
                            1003d11d 90                 nop
                            1003d11e 83f940             cmp     ecx,+40
                        */
                        constexpr int vp_begin = 0x3d115;
                        OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x3d121 - vp_begin);
                        h.store_i32(0x3d112 - vp_begin, '\x50\x53\x51\xe8');
                        h.replaceNearJmp(0x3d116 - vp_begin, &set_pos);
                        h.store_i32(0x3d11a - vp_begin, '\x59\x5b\x58\x90');
                        h.store_i32(0x3d11d - vp_begin, '\x90\x83\xf9\x40');
                    }
                }
                { // 貼り付け時にクリック位置から計算する部分を置き換える
                    constexpr int vp_begin = 0x408d3;
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x408e4 - vp_begin);
                    h.replaceNearJmp(0x408d3 - vp_begin, &get_click_frame);
                    h.replaceNearJmp(0x408e0 - vp_begin, &get_click_layer);
                }
            }

            { // フレーム数の小さいシーンへ貼り付けを行った際に貼り付け位置がおかしいのを修正
                /* 最小値の初期値がcurrent_scene_frame_nと低すぎるのが原因
                    100182fe 8b3da0d31410       mov     edi,dword ptr [current_scene_frame_n]
                    ↓
                    100182fe 90                 nop
                    100182ff bfffffff00         mov     edi,00ffffff
                */
                constexpr int vp_begin = 0x182fe;
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x18304 - vp_begin);
                h.store_i16(0x182fe - vp_begin, '\x90\xbf');
                h.store_i32(0x18300 - vp_begin, 0xffffff);
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
    } paste_pos;
} // namespace patch

#endif // ifdef PATCH_SWITCH_PASTE_POS
