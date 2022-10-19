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

#ifdef PATCH_SWITCH_SECOND_CACHE

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // 従来のキャッシュが削除される前に、共有メモリへ保存する

    inline class second_cache_t {

        static void* __cdecl GetCache_end(LPCSTR name, int* w, int* h, int* bitcount);
        static void init_secondcache();
        static int get_secondcache_id(char* name);
        static int __stdcall lstrcmpiA_wrapcef0(int cid, LPCSTR name, LPCSTR str_cachetemp);

        static void __cdecl sceneobj_blend_wrap(ExEdit::FilterProcInfo* dst, int dst_x, int dst_y, void* src, int src_x, int src_y, int w, int h, int alpha, int flag);

        
        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "second_cache";
    public:

        void init() {
            enabled_i = enabled;
            
            if (!enabled_i)return;
            
            init_secondcache();
            {
                /*
                    1000d162 c3              ret
                    1000d163 90909090        nop
                    ↓
                    1000d162 e9XxXxXxXx      ret
                */

                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x00d162, 5);
                h.store_i8(0, '\xe9'); // jmp
                h.replaceNearJmp(1, &GetCache_end);
            }
            {
                /*
                    1000ceee ff15a4a10910    call    lstrcmpiA
                    ↓
                    1000ceee 56              push    esi ; cache_id
                    1000ceef e8XxXxXxXx      call    lstrcmpiA_wrapcef0
                */

                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x00ceee, 6);
                h.store_i16(0, '\x56\xe8'); // push esi, call (rel32)
                h.replaceNearJmp(2, &lstrcmpiA_wrapcef0);
            }




            { // scene_obj
                /*
                    10083610 52              push    edx ; efpip->obj_edit
                    10083611 e82ae5ffff      call    exedit_func_blend (exfunc->0x44)
                 
                    ↓
                    10083610 55              push    ebp ; efpip
                    10083611 e8XxXxXxXx      call    sceneobj_blend_wrap
                */
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x083610, 6);
                h.store_i8(0, '\x55'); // push ebp
                h.replaceNearJmp(2, &sceneobj_blend_wrap);
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
    } second_cache;
} // namespace patch

#endif // ifdef PATCH_SWITCH_SECOND_CACHE
