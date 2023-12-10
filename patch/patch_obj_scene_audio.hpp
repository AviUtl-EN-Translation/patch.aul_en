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

#ifdef PATCH_SWITCH_OBJ_SCENE_AUDIO

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"
#include "patch_small_filter.hpp"

namespace patch {

    // init at exedit load
    // シーン(音声)でシーンと連携にした時に参照するシーン番号が正常ではないのを修正



    inline class obj_SceneAudio_t {

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "obj_scene_audio";
    public:


        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            auto& cursor = GLOBAL::executable_memory_cursor;

            /*
                10083ed5 8b4104             mov     eax,dword ptr [ecx+04]
                10083ed8 8b0a               mov     ecx,dword ptr [edx]
                ↓
                10083ed5 e8XxXxXxXx         call    cursor

                10000000 8b442434           mov     eax,dword ptr [esp+34]
                10000000 8b00               mov     eax,dword ptr [eax]
                10000000 89442414           mov     dword ptr [esp+14],eax
                10000000 8b4104             mov     eax,dword ptr [ecx+04]
                10000000 8b0a               mov     ecx,dword ptr [edx]
                10000000 c3                 ret
            */
            OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x83ed5, 5);
            h.store_i8(0, '\xe8');
            h.replaceNearJmp(1, cursor);

            store_i32(cursor, '\x8b\x44\x24\x34'); cursor += 4;
            store_i32(cursor, '\x8b\x00\x89\x44'); cursor += 4;
            store_i32(cursor, '\x24\x14\x8b\x41'); cursor += 4;
            store_i32(cursor, '\x04\x8b\x0a\xc3'); cursor += 4;
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
    } SceneAudio;
} // namespace patch

#endif // ifdef PATCH_SWITCH_OBJ_SCENE_AUDIO
