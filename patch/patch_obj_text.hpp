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

#ifdef PATCH_SWITCH_OBJ_TEXT

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // 改行のみのテキストが幅0 高さ1以上となってしまい、エラーの原因となるのを修正

    // テキストで制御文字を使用した時に変な描画がされることがあるのを簡易修正
    // フレームレートの分母が大きい時に正常に表示されないのを修正
    inline class obj_Text_t {

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "obj_text";
    public:
        static void __cdecl exedit_exfunc_x40_ret_wrap(void*, int, int, wchar_t*, ExEdit::PixelBGR, ExEdit::PixelBGR, int, HFONT, int*, int*, int, int, int, int*, int);
        static void __cdecl yc_buffer_fill_wrap(void*, int, int, int, int, short, short, short, short, int);
        static void __cdecl FUN_100877e0(HMENU hmenu, int param);
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            { // テキストの幅か高さが0であれば両方0にする
                /*
                    1004fee8 c3        ret
                    1004fee9 90909090  nop
                */
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x04fee8, 5);
                h.store_i8(0, '\xe9'); // jmp
                h.replaceNearJmp(1, &exedit_exfunc_x40_ret_wrap);
            }
            { // テキスト作成時のバッファ初期化の範囲を増やす
                
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x050254, 4);
                h.replaceNearJmp(0, &yc_buffer_fill_wrap);
            }
            { // フレームレートの分母が大きい時に正常に表示されないのを修正
                /*
                    1008a92a 8b8700010000       mov     eax,dword ptr [edi+00000100]
                    1008a930 8954243c           mov     dword ptr [esp+3c],edx
                    1008a934 8d0480             lea     eax,dword ptr [eax+eax*4]
                    1008a937 db44243c           fild    dword ptr [esp+3c]
                    1008a93b 8d0480             lea     eax,dword ptr [eax+eax*4]
                    1008a93e 8d0c80             lea     ecx,dword ptr [eax+eax*4]
                    1008a941 c1e103             shl     ecx,03
                    1008a944 894c243c           mov     dword ptr [esp+3c],ecx
                    1008a948 db44243c           fild    dword ptr [esp+3c]
                    1008a94c
                    ↓
                    1008a92a 8954243c           mov     dword ptr [esp+3c],edx
                    1008a92e db44243c           fild    dword ptr [esp+3c]
                    1008a932 db8700010000       fild    dword ptr [edi+00000100]
                    1008a938 dc0dXxXxXxXx       fmul    qword ptr [ee+9a3d0] ; 1000.0d
                    1008a93e eb0c               jmp     skip,+0c
                    1008a940
                */

                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x8a92a, 22);
                h.store_i32(0, '\x89\x54\x24\x3c');
                h.store_i32(4, '\xdb\x44\x24\x3c');
                h.store_i32(8, '\xdb\x87\x00\x01');
                h.store_i32(12, '\x00\x00\xdc\x0d');
                h.store_i32(16, GLOBAL::exedit_base + OFS::ExEdit::double_1000);
                h.store_i16(20, '\xeb\x0c');
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
    } Text;
} // namespace patch

#endif // ifdef PATCH_SWITCH_OBJ_TEXT
