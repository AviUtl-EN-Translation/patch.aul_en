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

#ifdef PATCH_SWITCH_CAMERA_SCENE
#include <memory>

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // カメラ制御とシーンの組み合わせで起こるバグの修正
    inline class camera_scene_t {

        static BOOL __cdecl video_func_main_wrap(ExEdit::FilterProcInfo* efpip, AviUtl::FilterProcInfo* fpip, int end_layer, int frame, int subframe, int scene_idx, ExEdit::ObjectFilterIndex ofi);
        static void __cdecl clear_zbuffer_wrap(void* zbuffer_ptr, int zbuffer_size);
        static void __cdecl clear_shadowmap_wrap(void* shadowmap_ptr, int shadowmap_size);

        inline static AviUtl::SharedMemoryInfo* zorder_smem_handle[4];
        inline static AviUtl::SharedMemoryInfo* zbuf_smem_handle[4];
        inline static AviUtl::SharedMemoryInfo* shadowmap_smem_handle[4];

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "camera_scene";
    public:
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            {
                auto& cursor = GLOBAL::executable_memory_cursor;

                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x025575, 5);
                h.store_i8(0, '\xe8');
                h.replaceNearJmp(1, cursor);

                /*
                10025575 a36c591310        mov   [camera_mode],eax
                ↓
                10025575 e8XxXxXxXx        call  cursor

                10000000 8bbe1c010000      mov   edi,dword ptr [esi+0000011c]
                10000000 8904bdXxXxXxXx    mov   dword ptr [edi*4+camera_mode],eax
                10000000 c3                ret
                */

                static const char code_put[] =
                    "\x8b\xbe\x1c\x01\x00\x00" // mov   edi,dword ptr [esi+0000011c]
                    "\x89\x04\xbdXXXX"         // mov   dword ptr [edi*4+camera_mode],eax
                    "\xc3"                     // ret
                    ;

                memcpy(cursor, code_put, sizeof(code_put) - 1);
                store_i32(cursor + 9, GLOBAL::exedit_base + OFS::ExEdit::camera_mode);
                cursor += sizeof(code_put) - 1;
            }

            {
                /*
                1004cefd 6a00          push     0
                1004ceff e82cb9ffff    call     video_func_main
                ↓
                1004cefd 90            nop
                1004cefe 55            push     ebp
                1004ceff e82cb9ffff    call     video_func_main_wrap
                */

                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x04cefd, 7);
                h.store_i16(0, '\x90\x55');
                h.replaceNearJmp(3, &video_func_main_wrap);
            }
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x07d30a, 4);
                h.replaceNearJmp(0, &clear_zbuffer_wrap);
            }
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x07d37a, 4);
                h.replaceNearJmp(0, &clear_shadowmap_wrap);
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
    } camera_scene;
} // namespace patch

#endif // ifdef PATCH_SWITCH_CAMERA_SCENE
