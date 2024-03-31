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
#ifdef PATCH_SWITCH_EXFILTER_FLASH
#include <exedit.hpp>
#include "util_magic.hpp"
#include "offset_address.hpp"
#include "global.hpp"
#include "config_rw.hpp"

#include "patch_exfilter.hpp"

namespace patch::exfilter {
    // init at exedit load
    // 閃光のフィルタオブジェクト追加（拡張編集で実装途中のもの）
    inline class Flash_t {

        inline static char* track_name[4] = { nullptr };
        inline static int track_default[4] = { 1000,0,0,750 };
        inline static int track_s[4] = { 0,-2000,-2000,0 };
        inline static int track_e[4] = { 1000,2000,2000,1000 };
        inline static int track_scale[4] = { 10,1,1,10 };
        inline static int track_link[4] = { 0,1,-1,0 };
        inline static int track_drag_min[4] = { 0,-1000,-1000,0 };
        inline static int track_drag_max[4] = { 1000,1000,1000,1000 };

        static void __cdecl mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
        static void __cdecl mt_color(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

        static BOOL __cdecl func_WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, AviUtl::EditHandle* editp, ExEdit::Filter* efp);

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "exfilter.flash";
    public:
        static BOOL __cdecl func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

        struct efFlash_var { // 1a6b7c
            int r_intensity;
            int cx;
            int cy;
            int temp_w;
            int temp_h;
            short color_cb;
            short _padding1;
            int intensity;
            int pixel_range;
            short color_cr;
            short _padding2;
            int temp_x;
            int range;
            int temp_y;
            short color_y;
        };

        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            auto efp = reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efFlash_Filter_ptr);
            efp->track_n = 4;
            memcpy(track_name, efp->track_name, 3 * sizeof(char*));
            track_name[3] = reinterpret_cast<char*>(GLOBAL::exedit_base + OFS::ExEdit::str_HANNI);
            efp->track_name = track_name;
            efp->track_default = track_default;
            efp->track_s = track_s;
            efp->track_e = track_e;
            (efp->func_proc) = (func_proc);
            (efp->func_WndProc) = (func_WndProc);
            efp->track_gui = reinterpret_cast<ExEdit::Filter::TrackGuiIdx*>(GLOBAL::exedit_base + 0x9fcb0); // 放射ブラーから流用
            efp->track_extra->track_scale = track_scale;
            efp->track_extra->track_link = track_link;
            efp->track_extra->track_drag_min = track_drag_min;
            efp->track_extra->track_drag_max = track_drag_max;
            exfilter.apend_filter(efp);
        }

        void switch_load(ConfigReader& cr) {
            cr.regist(key, [this](json_value_s* value) {
                ConfigReader::load_variable(value, enabled);
                });
        }

        void switch_store(ConfigWriter& cw) {
            cw.append(key, enabled);
        }

    } Flash;
} // namespace patch::exfilter
#endif // ifdef PATCH_SWITCH_EXFILTER_FLASH
