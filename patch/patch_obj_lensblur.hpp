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

#ifdef PATCH_SWITCH_OBJ_LENSBLUR

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // レンズブラーのエラー修正
    
    /* オフセットアドレス exedit + 71449 の修正
        サイズ２～３の図形にレンズブラーを付けて
        サイズ固定にチェックが入った状態でレンズブラー範囲を広げる
    */

    /* オフセットアドレス exedit + 71f95 の修正 → patch_scene_vramで根本的に修正して不要に
        フィルタオブジェクトのレンズブラー(範囲を広げたもの)をシーンオブジェクトで読み込む
        シーンオブジェクト以外でエラーはあまり見られないけど他データを破壊していそうなのでそちらもまとめて修正
    */ 

    inline class obj_LensBlur_t {

        static void* __cdecl lbResize_709a0_wrap_12809(void* pix_edit, int w0, int h0, int w1, int h1, void* pix_temp);
        static void* __cdecl lbResize_71420_wrap_126a6(void* pix_edit, int w0, int h0, int w1, int h1, void* pix_temp);

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "obj_lensblur";
    public:


        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            { // オフセットアドレス exedit + 71449 の修正
                ReplaceNearJmp(GLOBAL::exedit_base + 0x01280a, &lbResize_709a0_wrap_12809);
                ReplaceNearJmp(GLOBAL::exedit_base + 0x0126a7, &lbResize_71420_wrap_126a6);
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
    } LensBlur;
} // namespace patch

#endif // ifdef PATCH_SWITCH_OBJ_LENSBLUR
