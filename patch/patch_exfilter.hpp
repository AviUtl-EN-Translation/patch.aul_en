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

#ifdef PATCH_SWITCH_EXFILTER

#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"


namespace patch::exfilter {
    inline class exfilter_t {

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "exfilter";


#define EXFILTER_MAX 32
        inline static ExEdit::Filter* filter_list[EXFILTER_MAX] = { NULL };
        inline static int filter_count = 0;

        static BOOL __cdecl exedit_video_func_init_mov_eax_1_wrap() {
            auto LoadedFilterTable = (ExEdit::Filter**)(GLOBAL::exedit_base + OFS::ExEdit::LoadedFilterTable);
            auto LoadedFilterCount_ptr = reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::LoadedFilterCount);
            int LoadedFilterCount = *LoadedFilterCount_ptr;
            for (int i = 0; i < filter_count; i++) {
                auto efp = filter_list[i];
                efp->flag |= (ExEdit::Filter::Flag)0x4000000;
                efp->exedit_fp = *reinterpret_cast<AviUtl::FilterPlugin**>(GLOBAL::exedit_base + OFS::ExEdit::exedit_fp);
                efp->aviutl_exfunc = efp->exedit_fp->exfunc;
                efp->exfunc = reinterpret_cast<ExEdit::Exfunc*>(GLOBAL::exedit_base + OFS::ExEdit::exfunc);
                if (efp->track_extra != NULL) {
                    efp->track_scale = efp->track_extra->track_scale;
                    efp->track_link = efp->track_extra->track_link;
                    efp->track_drag_min = efp->track_extra->track_drag_min;
                    efp->track_drag_max = efp->track_extra->track_drag_max;
                }
                if (efp->exdata_size != 0) {
                    efp->flag |= ExEdit::Filter::Flag::HasExdata;
                }
                if (efp->func_init != NULL) {
                    efp->func_init(efp);
                }
                LoadedFilterTable[LoadedFilterCount] = efp;
                LoadedFilterCount++;
            }
            *LoadedFilterCount_ptr = LoadedFilterCount;

            return TRUE; // mov eax,1
        }


    public:
        void init() {
            enabled_i = enabled;
            if (!enabled_i)return;
            
            /*
                10031794 b801000000         mov     eax,1
                ↓
                10031794 e8XxXxXxXx         call    newfunc
            */
            OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x31794, 5);
            h.store_i8(0, '\xe8');
            h.replaceNearJmp(1, &exedit_video_func_init_mov_eax_1_wrap);
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

        void apend_filter(ExEdit::Filter* efp) {
            if (filter_count < EXFILTER_MAX) {
                filter_list[filter_count++] = efp;
            }
        }

    } exfilter;
}

#endif //define PATCH_SWITCH_EXFILTER