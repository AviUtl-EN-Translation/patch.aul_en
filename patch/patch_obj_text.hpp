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

    inline class obj_Text_t {

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "obj_text";
    public:
        static void __cdecl exedit_exfunc_x40_ret_wrap(void*, int, int, wchar_t*, ExEdit::PixelBGR, ExEdit::PixelBGR, int, HFONT, int*, int*, int, int, int, int*, int);
        static void __cdecl yc_buffer_fill_wrap(void*, int, int, int, int, short, short, short, short, int);
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
            { // テキスト作成時のバッファ初期化の方法を変える

                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x050254, 4);
                h.replaceNearJmp(0, &yc_buffer_fill_wrap);
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
