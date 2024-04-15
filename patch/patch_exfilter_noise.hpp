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
#ifdef PATCH_SWITCH_EXFILTER_NOISE
#include <exedit.hpp>
#include "util_magic.hpp"
#include "offset_address.hpp"
#include "global.hpp"
#include "config_rw.hpp"

#include "patch_exfilter.hpp"

namespace patch::exfilter {
    // init at exedit load
    // ノイズのフィルタオブジェクト追加
    inline class Noise_t {
        inline static ExEdit::Filter ef;
        inline static char* check_name[3] = { nullptr };

        static void __cdecl mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "exfilter.noise";
    public:
        static BOOL __cdecl func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

        struct efNoise_var { // 1bad50
            int cycle_x; // 1bad50
            int cycle_y; //1bad54
            int n; // 1bad58
            int _padding1;
            short random_wave[65536]; // 1bad60
            short cos_down[256]; // 1dad60
            int inv_threshold; // 1daf60
            int _padding2;
            short cos_4096[4097]; // 1daf68
            short _padding3;
            int speed_x; // 1dcf6c
            int speed_y; // 1dcf70
            int speed_t; // 1dcf74
            short cos_up[4097]; // 1dcf78
            short _padding4;
            int intensity; // 1def7c
            short sin_4096[4097]; // 1def80
            short _padding5;
            int threshold; // 1e0f84
            int(__cdecl* func)(int, int, int); // 1e0f88
        };

        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            auto efp = reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efNoise_ptr);
            ef = *efp;

            ef.flag = (ExEdit::Filter::Flag)0;
            /*
            ef.check_n = 2;
            ef.check_name = (char**)(GLOBAL::exedit_base + 0xa8ec4 + 4);
            ef.check_default = (int*)(GLOBAL::exedit_base + 0xa8ed0 + 4);
            */
            check_name[0]= (char*)(GLOBAL::exedit_base + 0xa9049);
            check_name[1] = (char*)(GLOBAL::exedit_base + 0xa9010);
            check_name[2] = (char*)(GLOBAL::exedit_base + 0x9d6a4);
            ef.check_name = check_name;
            (ef.func_proc) = (func_proc);

            exfilter.apend_filter(&ef);
        }

        void switch_load(ConfigReader& cr) {
            cr.regist(key, [this](json_value_s* value) {
                ConfigReader::load_variable(value, enabled);
                });
        }

        void switch_store(ConfigWriter& cw) {
            cw.append(key, enabled);
        }

    } Noise;
} // namespace patch::exfilter
#endif // ifdef PATCH_SWITCH_EXFILTER_NOISE
