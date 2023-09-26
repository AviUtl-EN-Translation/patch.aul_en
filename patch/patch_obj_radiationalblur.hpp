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

#ifdef PATCH_SWITCH_OBJ_RADIATIONALBLUR

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // 放射ブラーでアーチファクトが出るのを修正

    inline class obj_RadiationalBlur_t {

        static int __stdcall eax_mul_range_div_1000(int eax) {
            return eax * *range_ptr / 1000;
        }

        inline static int* range_ptr;

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "obj_radiationalblur";
    public:

        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            range_ptr = reinterpret_cast<int*>(GLOBAL::exedit_base + 0xd75c0);
            /*
                1000b78b 0fafc2             imul    eax,edx
                1000b78e 99                 cdq
                1000b78f f7f9               idiv    ecx

                1000bc8e 0fafc2             imul    eax,edx
                1000bc91 99                 cdq
                1000bc92 f7f9               idiv    ecx

                1000bf3e 0fafc3             imul    eax,ebx
                1000bf41 99                 cdq
                1000bf42 f7f9               idiv    ecx

                ↓

                1000b78b 50                 push    eax
                1000b78c e8XxXxXxXx         call    newfunc
            */
            constexpr int vp_begin = 0xb78b;
            OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0xbf44 - vp_begin);
            h.store_i16(0xb78b - vp_begin, '\x50\xe8');
            h.replaceNearJmp(0xb78d - vp_begin, &eax_mul_range_div_1000);

            h.store_i16(0xbc8e - vp_begin, '\x50\xe8');
            h.replaceNearJmp(0xbc90 - vp_begin, &eax_mul_range_div_1000);

            h.store_i16(0xbf3e - vp_begin, '\x50\xe8');
            h.replaceNearJmp(0xbf40 - vp_begin, &eax_mul_range_div_1000);
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
    } RadiationalBlur;
} // namespace patch

#endif // ifdef PATCH_SWITCH_OBJ_RADIATIONALBLUR
