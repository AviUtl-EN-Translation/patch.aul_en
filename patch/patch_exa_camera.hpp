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

#ifdef PATCH_SWITCH_EXA_CAMERA
#include <memory>

#include <exedit.hpp>

#include "global.hpp"
#include "util.hpp"
#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // exaでエフェクトを追加するときにカメラ制御の対象が外れるのを修正
    inline class exa_camera_t {
        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "exa_camera";
        static int __cdecl get_obj_camera_flag(int object_idx);

    public:
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            auto& cursor = GLOBAL::executable_memory_cursor;

            { // exo_write
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x29e26, 5);
                h.store_i8(0, '\xe9');
                h.replaceNearJmp(1, cursor);
                /*
                    10029e26 8bf8               mov     edi,eax
                    10029e28 83c408             add     esp,+08
                    ↓
                    10029e26 e9XxXxXxXx         jmp     cursor

                    10000000 8bf8               mov     edi,eax
                    10000000 56                 push    esi ; obj_idx
                    10000000 e8XxXxXxXx         call    newfunc
                    10000000 8944243c           mov     dword ptr [esp+3c],eax
                    10000000 83c40c             add     esp,+0c
                    10000000 e9XxXxXxXx         jmp     ee+29e2b

                */
                store_i32(cursor, '\x8b\xf8\x56\xe8'); cursor += 4;
                store_i32(cursor, (int)&get_obj_camera_flag - (int)cursor - 4); cursor += 4;
                store_i32(cursor, '\x89\x44\x24\x3c'); cursor += 4;
                store_i32(cursor, '\x83\xc4\x0c\xe9'); cursor += 4;
                store_i32(cursor, GLOBAL::exedit_base + 0x29e2b - (int)cursor - 4); cursor += 4;
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
    } exa_camera;
} // namespace patch

#endif // ifdef PATCH_SWITCH_EXA_CAMERA
