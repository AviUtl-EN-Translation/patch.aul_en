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

#include "patch_fast_yc_filter_effect.hpp"
#ifdef PATCH_SWITCH_FAST_YC_FILTER_EFFECT


//#define PATCH_STOPWATCH
#include "patch_fast_blur.hpp"
#include "patch_fast_directionalblur.hpp"
#include "patch_exfilter_specialcolorconv.hpp"
#include "patch_exfilter_flash.hpp"

namespace patch::fast {
    BOOL __cdecl check0(ExEdit::Object* leaderobj, int fid) {
        return leaderobj->check_value[leaderobj->filter_param[fid].check_begin + 0] != 0;
    }
    BOOL __cdecl check4_not(ExEdit::Object* leaderobj, int fid) {
        return leaderobj->check_value[leaderobj->filter_param[fid].check_begin + 4] == 0;
    }
    BOOL __cdecl check2_exdata1(ExEdit::Object* leaderobj, int fid) {
        int* exdata = (int*)(4 + leaderobj->filter_param[fid].exdata_offset + leaderobj->exdata_offset + *reinterpret_cast<DWORD*>(GLOBAL::exedit_base + OFS::ExEdit::ExdataPointer));
        return leaderobj->check_value[leaderobj->filter_param[fid].check_begin + 2] != 0 && (exdata[1] == 0 || exdata[1] == 1);
    }
    void yc_filter_effect_t::func_check_listing() {
        auto LoadedFilterTable = (ExEdit::Filter**)(GLOBAL::exedit_base + OFS::ExEdit::LoadedFilterTable);
        for (int i = 0; i < exedit_check_filter_num; i++) {
            switch ((int)LoadedFilterTable[i] - GLOBAL::exedit_base) {
            case OFS::ExEdit::efBlur_ptr:
                if (Blur.is_enabled_i()) {
                    (func_check[i]) = (check0);
                }
                break;
            case OFS::ExEdit::efDirectionalBlur_ptr:
                if (DirectionalBlur.is_enabled_i()) {
                    (func_check[i]) = (check0);
                }
                break;
            case OFS::ExEdit::efDiffuseLight_ptr:
            case OFS::ExEdit::efLightEmission_ptr:
            case OFS::ExEdit::efRadiationalBlur_ptr:
            case OFS::ExEdit::efLensBlur_ptr:
                (func_check[i]) = (check0);
                break;
            case OFS::ExEdit::efFlip_ptr:
                (func_check[i]) = (check4_not);
                break;
            case OFS::ExEdit::efFlash_ptr:
                (func_check[i]) = (check2_exdata1);
                break;
            }
        }
    }
    void __stdcall yc_filter_effect_t::yc_check(ExEdit::FilterProcInfo* efpip, ExEdit::Object** objpp, int oid, int on, int fid) {
        if (first) {
            func_check_listing();
            first = FALSE;
        }

        auto LoadedFilterTable = (ExEdit::Filter**)(GLOBAL::exedit_base + OFS::ExEdit::LoadedFilterTable);
        auto ObjectArrayPointer = *(ExEdit::Object**)(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
        efpip->xf4 = 0;
        fid++;
        for (; oid < on; oid++) {
            for (; fid < 12; fid++) {
                auto obj = objpp[oid];
                int filter_idx = obj->filter_param[fid].id;
                if (filter_idx < 0) {
                    break;
                }
                auto efp = LoadedFilterTable[filter_idx];
                if (!has_flag(efp->flag, ExEdit::Filter::Flag::Output)) {
                    if (0 <= obj->index_midpt_leader) {
                        obj = &ObjectArrayPointer[obj->index_midpt_leader];
                    }
                    if (has_flag(obj->filter_status[fid], ExEdit::Object::FilterStatus::Active)) {
                        if (((byte)efp->flag & 0x40) == 0) {
                            if (exedit_check_filter_num <= filter_idx || func_check[filter_idx] == NULL || !func_check[filter_idx](obj, fid)) {
                                return;
                            }
                        }
                    }
                }
            }
            fid = 0;
        }
        *(byte*)&efpip->flag |= 0x40;
    }

    BOOL __cdecl yc_filter_effect_t::func_proc(BOOL(*func_proc)(ExEdit::Filter*, ExEdit::FilterProcInfo*), BOOL(*filter_func_proc)(ExEdit::Filter*, ExEdit::FilterProcInfo*), ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        if (efpip->xf4) {
            auto scene_w = efpip->scene_w;
            auto scene_h = efpip->scene_h;
            auto scene_line = efpip->scene_line;
            auto frame_edit = efpip->frame_edit;
            auto frame_temp = efpip->frame_temp;
            efpip->scene_w = efpip->obj_w;
            efpip->scene_h = efpip->obj_h;
            efpip->scene_line = efpip->obj_line;
            efpip->frame_edit = reinterpret_cast<decltype(efpip->frame_edit)>(efpip->obj_edit);
            efpip->frame_temp = reinterpret_cast<decltype(efpip->frame_temp)>(efpip->obj_temp);
            
            efp->flag ^= static_cast<decltype(efp->flag)>(0x20);
            BOOL ret = filter_func_proc(efp, efpip);
            efp->flag ^= static_cast<decltype(efp->flag)>(0x20);

            efpip->obj_edit = reinterpret_cast<decltype(efpip->obj_edit)>(efpip->frame_edit);
            efpip->obj_temp = reinterpret_cast<decltype(efpip->obj_temp)>(efpip->frame_temp);
            efpip->scene_w = scene_w;
            efpip->scene_h = scene_h;
            efpip->scene_line = scene_line;
            efpip->frame_edit = frame_edit;
            efpip->frame_temp = frame_temp;
            return ret;
        } else {
            return func_proc(efp, efpip);
        }
    }

    BOOL __cdecl yc_filter_effect_t::efColorCorrection_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        return func_proc(efColorCorrection_func_proc_org, efColorCorrection_func_proc_org, efp, efpip);
    }
    BOOL __cdecl yc_filter_effect_t::efMonochromatic_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        return func_proc(efMonochromatic_func_proc_org, efMonochromatic_func_proc_org, efp, efpip);
    }
    BOOL __cdecl yc_filter_effect_t::efMosaic_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        return func_proc(efMosaic_func_proc_org, efMosaic_func_proc_org, efp, efpip);
    }
    BOOL __cdecl yc_filter_effect_t::efSpecialColorConv_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        return func_proc(efSpecialColorConv_func_proc_org, exfilter::SpecialColorConv_t::func_proc, efp, efpip);
    }
    BOOL __cdecl yc_filter_effect_t::efDiffuseLight_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        return func_proc(efDiffuseLight_func_proc_org, efDiffuseLight_func_proc_org, efp, efpip);
    }
    BOOL __cdecl yc_filter_effect_t::efLightEmission_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        return func_proc(efLightEmission_func_proc_org, efLightEmission_func_proc_org, efp, efpip);
    }
    BOOL __cdecl yc_filter_effect_t::efRadiationalBlur_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        return func_proc(efRadiationalBlur_func_proc_org, reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efRadiationalBlur_Filter_ptr)->func_proc, efp, efpip);
    }
    BOOL __cdecl yc_filter_effect_t::efLensBlur_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        return func_proc(efLensBlur_func_proc_org, efLensBlur_func_proc_org, efp, efpip);
    }
    BOOL __cdecl yc_filter_effect_t::efFlip_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        return func_proc(efFlip_func_proc_org, efFlip_func_proc_org, efp, efpip);
    }
    BOOL __cdecl yc_filter_effect_t::efFlash_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        if (efpip->xf4) {
            if (((int*)efp->exdata_ptr)[1] == 1) {
                return TRUE;
            }
            efp->track[3] = 750;
        }
        return func_proc(efFlash_func_proc_org, exfilter::Flash_t::func_proc, efp, efpip);
    }

} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_YC_FILTER_EFFECT
