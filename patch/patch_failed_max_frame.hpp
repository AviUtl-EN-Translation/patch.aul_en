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

#ifdef PATCH_SWITCH_FAILED_MAX_FRAME

#include <exedit.hpp>

#include "global.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at patch load
    // aup読み込み時、最大フレーム数が足りない時のメッセージを追加
    // ついでにその場合に拡張編集などにオブジェクトが置けてしまうのを修正

    inline class failed_max_frame_t {

        inline static const char str_new_failed_msg[] = "Unable to open %d frame project\nIncrease the maximum number of frames in the system configuration.";
        static void __stdcall AppendMessageBoxA(AviUtl::EditHandle* editp);

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "failed_max_frame";
    public:
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            /*
                00423a09 8123feffff0f       and     dword ptr [ebx],0ffffffe
                ↓
                00423a09 53                 push    ebx
                00423a0a e8XxXxXxXx         call    newfunc
            */

            OverWriteOnProtectHelper h(GLOBAL::aviutl_base + 0x23a09, 6);
            h.store_i16(0, '\x53\xe8');
            h.replaceNearJmp(2, &AppendMessageBoxA);
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
    } failed_max_frame;
} // namespace patch

#endif // ifdef PATCH_SWITCH_FAILED_MAX_FRAME
