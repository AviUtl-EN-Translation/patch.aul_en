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

#ifdef PATCH_SWITCH_AVI_FILE_HANDLE_SHARE

#include <aviutl.hpp>
#include <exedit.hpp>
#include "global.hpp"
#include "util.hpp"
#include "restorable_patch.hpp"

#include "config_rw.hpp"

namespace patch {
    // init at exedit load
    // 同じ動画に対して動画ファイルのハンドルを共有させる（L-SMASH WorksのHandle cache、InputPipePluginのハンドルキャッシュと似た機能。拡張編集でもつハンドルから共有させる）
    // Altを押しながら読み込ませるときは無効（InputPipePluginで#junkを作ることが可能）
    inline class avi_file_handle_share_t {
        // exedit92 + 0x0d4748 array[32]
        struct AviFileHandleInfo {
            AviUtl::AviFileHandle* afh;
            int32_t id_and_count;
            int32_t tickcount;
            int32_t priority;
            int32_t flag;
            AviUtl::FileInfo fi;
            char path[260];
        };
        struct ExdataAviFileHandleInfo {
            char path[260];
            int id_and_count;
            int tickcount;
        };

        static void __cdecl avi_handle_open_begin(ExdataAviFileHandleInfo* exdata, int flag, AviUtl::FileInfo** fipp);
        bool enabled = true;
        bool enabled_i;

        inline static const char key[] = "avi_file_handle_share";

    public:

        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            InjectionFunction_push_args_cdecl(GLOBAL::exedit_base + 0x4ea0, (int)&avi_handle_open_begin, 6, 1, 3, 0);
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

    } avi_file_handle_share;
} // namespace patch
#endif // ifdef PATCH_SWITCH_AVI_FILE_HANDLE_SHARE
