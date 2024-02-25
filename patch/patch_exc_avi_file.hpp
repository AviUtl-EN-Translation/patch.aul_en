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

#ifdef PATCH_SWITCH_EXC_AVI_FILE

#include "Shlwapi.h"

#include <exedit.hpp>
#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

    // init at exedit load
    // exc
    inline class exc_avi_file_t {
        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "exc_avi_file";
        static BOOL __cdecl read_object_file_data_wrap(LPSTR data, ExEdit::Object* eop, int filter_idx);

    public:
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            ReplaceNearJmp(GLOBAL::exedit_base + 0x29c33, &read_object_file_data_wrap);
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
    } exc_avi_file;
} // namespace patch

#endif // ifdef PATCH_SWITCH_EXC_AVI_FILE
