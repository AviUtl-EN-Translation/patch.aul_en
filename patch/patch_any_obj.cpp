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

#include "patch_any_obj.hpp"

#ifdef PATCH_SWITCH_ANY_OBJ
namespace patch {
    void any_obj_t::deselect_object_if() {
        if (0 <= GetKeyState(VK_CONTROL)) {
            deselect_object();
        }
    }

    void any_obj_t::deselect_object_tl_activate() {
        int timeline_obj_click_mode = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::timeline_obj_click_mode);
        HWND menuhwnd = FindWindowA("#32768", NULL);
        if (timeline_obj_click_mode == 0 && (!menuhwnd || !IsWindowVisible(menuhwnd))) {
            deselect_object_if();
        }
    }
    void any_obj_t::post_deselect_object_tl_activate() {
        PostMessageA(*(HWND*)(GLOBAL::exedit_base + OFS::ExEdit::exedit_hwnd), PATCH_EXEDIT_EXCOMMAND, PATCH_EXEDIT_EXCOMMAND_ONE_DELAY, PATCH_EXEDIT_EXCOMMAND_DESELECT_OBJECT_TL_ACTIVATE);
    }

    void __cdecl any_obj_t::update_any_exdata_use_idx(ExEdit::Filter* efp, int idx) {
        // ##################### sdk更新したら変える
        struct exdata_use_fix { // sdkのExdataUse::typeがintになっているため上手くいかない
            short type;
            short size;
            char* name;
        } *exdata_use = (exdata_use_fix*)efp->exdata_use;
        update_any_exdata(efp->processing, (char*)exdata_use[idx].name);
    }

    int get_same_filter_idx(int dst_idx, int src_idx, int filter_idx) {
        if (src_idx == dst_idx) return -1;
        auto obj = *(ExEdit::Object**)(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
        
        int filter_param_id = obj[src_idx].filter_param[filter_idx].id;
        if (filter_param_id < 0) return -1;

        if (0 <= obj[src_idx].index_midpt_leader) {
            src_idx = obj[src_idx].index_midpt_leader;
        }
        if (0 <= obj[dst_idx].index_midpt_leader) {
            dst_idx = obj[dst_idx].index_midpt_leader;
        }
        if (src_idx == dst_idx) return filter_idx;

        auto LoadedFilterTable = (ExEdit::Filter**)(GLOBAL::exedit_base + OFS::ExEdit::LoadedFilterTable);
        if (has_flag(LoadedFilterTable[filter_param_id]->flag, ExEdit::Filter::Flag::Output)) {
            filter_idx = reinterpret_cast<int(__cdecl*)(int)>(GLOBAL::exedit_base + OFS::ExEdit::get_last_filter_idx)(dst_idx);
        }
        int id = obj[dst_idx].filter_param[filter_idx].id;
        if (0 <= id && filter_param_id == id) return filter_idx;
        return -1;
    }

    void __cdecl update_any_dispname(ExEdit::ObjectFilterIndex ofi) {
        if ((int)ofi == 0) return;

        int SettingDialog_ObjIdx = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::SettingDialog_ObjIdx);
        if (SettingDialog_ObjIdx < 0) return;

        int SelectingObjectNum = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::SelectingObjectNum);
        if (SelectingObjectNum <= 0) return;

        auto exfunc = (ExEdit::Exfunc*)(GLOBAL::exedit_base + OFS::ExEdit::exfunc);
        auto obj = *(ExEdit::Object**)(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
        int* SelectingObjectIdxArray = (int*)(GLOBAL::exedit_base + OFS::ExEdit::SelectingObjectIdxArray);

        int filter_idx = (int)ofi >> 16;

        if (0 <= obj[SettingDialog_ObjIdx].index_midpt_leader) {
            SettingDialog_ObjIdx = obj[SettingDialog_ObjIdx].index_midpt_leader;
        }

        for (int i = 0; i < SelectingObjectNum; i++) {
            int select_idx = SelectingObjectIdxArray[i];
            int leader_filter_idx = reinterpret_cast<int(__cdecl*)(int, int, int)>(GLOBAL::exedit_base + OFS::ExEdit::get_same_filter_idx_if_leader)(select_idx, SettingDialog_ObjIdx, filter_idx);
            if (0 <= leader_filter_idx) {
                reinterpret_cast<void(__cdecl*)(int, int)>(GLOBAL::exedit_base + OFS::ExEdit::set_undo)(select_idx, 0);
                exfunc->rename_object((ExEdit::ObjectFilterIndex)(select_idx + 1), obj[SettingDialog_ObjIdx].dispname);
            }
        }
    }

    void __cdecl update_any_track(ExEdit::ObjectFilterIndex ofi, int track_id) {
        if ((int)ofi == 0) return;
        if (track_id < 0 && 64 <= track_id)return;

        int SettingDialog_ObjIdx = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::SettingDialog_ObjIdx);
        if (SettingDialog_ObjIdx < 0) return;

        int SelectingObjectNum = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::SelectingObjectNum);
        if (SelectingObjectNum <= 0) return;

        auto obj = *(ExEdit::Object**)(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
        int* SelectingObjectIdxArray = (int*)(GLOBAL::exedit_base + OFS::ExEdit::SelectingObjectIdxArray);

        int filter_idx = (int)ofi >> 16;

        for (int i = 0; i < SelectingObjectNum; i++) {
            int select_idx = SelectingObjectIdxArray[i];
            int leader_filter_idx = get_same_filter_idx(select_idx, SettingDialog_ObjIdx, filter_idx);
            if (0 <= leader_filter_idx) {
                reinterpret_cast<void(__cdecl*)(int, int)>(GLOBAL::exedit_base + OFS::ExEdit::set_undo)(select_idx, 0);
                obj[select_idx].track_value_left[obj[select_idx].filter_param[leader_filter_idx].track_begin + track_id] = obj[SettingDialog_ObjIdx].track_value_left[obj[SettingDialog_ObjIdx].filter_param[filter_idx].track_begin + track_id];
                obj[select_idx].track_value_right[obj[select_idx].filter_param[leader_filter_idx].track_begin + track_id] = obj[SettingDialog_ObjIdx].track_value_right[obj[SettingDialog_ObjIdx].filter_param[filter_idx].track_begin + track_id];
                obj[select_idx].track_mode[obj[select_idx].filter_param[leader_filter_idx].track_begin + track_id] = obj[SettingDialog_ObjIdx].track_mode[obj[SettingDialog_ObjIdx].filter_param[filter_idx].track_begin + track_id];
                obj[select_idx].track_param[obj[select_idx].filter_param[leader_filter_idx].track_begin + track_id] = obj[SettingDialog_ObjIdx].track_param[obj[SettingDialog_ObjIdx].filter_param[filter_idx].track_begin + track_id];
            }
        }
    }

    void __cdecl update_any_check(ExEdit::ObjectFilterIndex ofi, int check_id) {
        if ((int)ofi == 0) return;
        if (check_id < 0 && 48 <= check_id)return;

        int SettingDialog_ObjIdx = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::SettingDialog_ObjIdx);
        if (SettingDialog_ObjIdx < 0) return;

        int SelectingObjectNum = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::SelectingObjectNum);
        if (SelectingObjectNum <= 0) return;

        auto obj = *(ExEdit::Object**)(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
        int* SelectingObjectIdxArray = (int*)(GLOBAL::exedit_base + OFS::ExEdit::SelectingObjectIdxArray);

        int filter_idx = (int)ofi >> 16;

        for (int i = 0; i < SelectingObjectNum; i++) {
            int select_idx = SelectingObjectIdxArray[i];
            int leader_filter_idx = get_same_filter_idx(select_idx, SettingDialog_ObjIdx, filter_idx);
            if (0 <= leader_filter_idx) {
                reinterpret_cast<void(__cdecl*)(int, int)>(GLOBAL::exedit_base + OFS::ExEdit::set_undo)(select_idx, 0);
                obj[select_idx].check_value[obj[select_idx].filter_param[leader_filter_idx].check_begin + check_id] = obj[SettingDialog_ObjIdx].check_value[obj[SettingDialog_ObjIdx].filter_param[filter_idx].check_begin + check_id];
            }
        }
    }

    void __cdecl update_any_object_flag(int object_idx, int flag) {
        if (object_idx < 0) return;

        int SelectingObjectNum = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::SelectingObjectNum);
        if (SelectingObjectNum <= 0) return;

        auto obj = *(ExEdit::Object**)(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
        if (0 <= obj[object_idx].index_midpt_leader) {
            object_idx = obj[object_idx].index_midpt_leader;
        }
        int* SelectingObjectIdxArray = (int*)(GLOBAL::exedit_base + OFS::ExEdit::SelectingObjectIdxArray);

        for (int i = 0; i < SelectingObjectNum; i++) {
            int select_idx = SelectingObjectIdxArray[i];
            if (obj[select_idx].index_midpt_leader < 0 || obj[select_idx].index_midpt_leader == select_idx) {
                if (((int)obj[select_idx].flag & 0x30000) == 0x10000) {
                    if (((int)obj[object_idx].flag & flag) != ((int)obj[select_idx].flag & flag)) {
                        reinterpret_cast<void(__cdecl*)(int, int)>(GLOBAL::exedit_base + OFS::ExEdit::set_undo)(select_idx, 1);
                        *(int*)&obj[select_idx].flag ^= flag;
                        reinterpret_cast<void(__cdecl*)(int)>(GLOBAL::exedit_base + 0x36020)(select_idx);
                        reinterpret_cast<void(__cdecl*)(int)>(GLOBAL::exedit_base + 0x39490)(select_idx);
                    }
                }
            }
        }
    }


	char* __cdecl any_obj_t::disp_extension_image_file_wrap(ExEdit::Filter* efp, char* path) {
		char* name = reinterpret_cast<char*(__cdecl*)(ExEdit::Filter*, char*)>(GLOBAL::exedit_base + 0xe220)(efp, path);
        update_any_exdata_use_idx(efp, 1); // OFS::ExEdit::str_file
        if (!has_flag(efp->flag, ExEdit::Filter::Flag::Effect)) {
            efp->exfunc->rename_object(efp->processing, name);
            update_any_dispname(efp->processing);
        }
        deselect_object_if();

        return name;
	}

    int __stdcall any_obj_t::count_section_num_wrap(ExEdit::Filter* efp, void* e1) {
        if (0 < *(int*)(GLOBAL::exedit_base + OFS::ExEdit::SelectingObjectNum)) {
            return 0;
        }
        return efp->exfunc->count_section_num(efp->processing);
    }

    void __cdecl any_obj_t::calc_milli_second_movie_file_wrap(ExEdit::Filter* efp, void* exdata) {
        reinterpret_cast<void(__cdecl*)(ExEdit::Filter*, void*)>(GLOBAL::exedit_base + 0x6900)(efp, exdata);
        update_any_exdata_use_idx(efp, 0); // OFS::ExEdit::str_file
        update_any_exdata_use_idx(efp, 1); // NULL
        if (!has_flag(efp->flag, ExEdit::Filter::Flag::Effect)) {
            update_any_dispname(efp->processing);
        }
        deselect_object_if();
    }
    void __cdecl any_obj_t::calc_milli_second_audio_file_wrap(ExEdit::Filter* efp, void* exdata) {
        reinterpret_cast<void(__cdecl*)(ExEdit::Filter*, void*)>(GLOBAL::exedit_base + 0x902d0)(efp, exdata);
        update_any_check(efp->processing, 1);
        update_any_exdata_use_idx(efp, 0); // OFS::ExEdit::str_file
        update_any_exdata_use_idx(efp, 1); // NULL
        update_any_dispname(efp->processing);
        deselect_object_if();
    }
    void __cdecl any_obj_t::rename_object_audio_file_wrap(ExEdit::ObjectFilterIndex ofi, char* name) {
        auto exfunc = (ExEdit::Exfunc*)(GLOBAL::exedit_base + OFS::ExEdit::exfunc);
        exfunc->rename_object(ofi, name);
        update_any_dispname(ofi);
    }

    void __cdecl any_obj_t::calc_frame_scene_wrap(ExEdit::Filter* efp) {
        reinterpret_cast<void(__cdecl*)(ExEdit::Filter*)>(GLOBAL::exedit_base + 0x83cc0)(efp);
        update_any_exdata_use_idx(efp, 0); // NULL or "scene" (exo_sceneidx)
        update_any_dispname(efp->processing);
        deselect_object_if();
    }
    void __cdecl any_obj_t::calc_frame_sceneaudio_wrap(ExEdit::Filter* efp) {
        reinterpret_cast<void(__cdecl*)(ExEdit::Filter*)>(GLOBAL::exedit_base + 0x848d0)(efp);
        update_any_check(efp->processing, 1);
        update_any_exdata_use_idx(efp, 0); // NULL or "scene" (exo_sceneidx)
        update_any_dispname(efp->processing);
        deselect_object_if();
    }
    void __stdcall any_obj_t::rename_object_sceneaudio_wrap(ExEdit::ObjectFilterIndex ofi, char* name, ExEdit::Filter* efp, void* exdata) {
        efp->exfunc->rename_object(ofi, name);
        update_any_dispname(ofi);
    }

    void __cdecl any_obj_t::update_obj_data_waveform_wrap(ExEdit::Filter* efp) {
        efp->exfunc->x00(efp->processing);
        update_any_check(efp->processing, 3);
        update_any_exdata_use_idx(efp, 0); // OFS::ExEdit::str_file
        update_any_exdata_use_idx(efp, 1); // NULL
        update_any_dispname(efp->processing);
        deselect_object_if();
    }
    int __cdecl any_obj_t::mov_eax_1_waveform_wrap() {
        auto efp = (ExEdit::Filter*)(GLOBAL::exedit_base + OFS::ExEdit::efWaveform);
        struct DialogParamInfo {
            short type;
            short exdata_use_id;
            char* name;
        }*dlgparam = (DialogParamInfo*)(GLOBAL::exedit_base + 0xba240);
        int i = 0;
        update_any_exdata_use_idx(efp, 2);
        while (dlgparam[i].type) {
            update_any_exdata_use_idx(efp, dlgparam[i].exdata_use_id);
            i++;
        }
        return 1;
    }

    void __cdecl any_obj_t::rename_object_figure_wrap(ExEdit::Filter* efp, void* exdata) {
        reinterpret_cast<void(__cdecl*)(ExEdit::Filter*, void*)>(GLOBAL::exedit_base + 0x74740)(efp, exdata);
        update_any_exdata_use_idx(efp, 0); // OFS::ExEdit::str_type
        update_any_exdata_use_idx(efp, 3); // OFS::ExEdit::str_name
        update_any_dispname(efp->processing);
        deselect_object_if();
    }

    void __cdecl any_obj_t::update_any_range(ExEdit::Filter* efp) { // undoの部分から呼び出す
        if (!any_obj.is_enabled_i())return;

        update_any_exdata_use_idx(efp, 0); // OFS::ExEdit::str_range
    }

    void __cdecl any_obj_t::update_obj_data_extractedge_wrap(ExEdit::ObjectFilterIndex ofi) {
        auto exfunc = (ExEdit::Exfunc*)(GLOBAL::exedit_base + OFS::ExEdit::exfunc);
        exfunc->x00(ofi);
        update_any_check(ofi, 0);
        update_any_check(ofi, 1);
    }

    void __cdecl any_obj_t::update_any_dlg_param_exdata() {
        auto efp = *(ExEdit::Filter**)(GLOBAL::exedit_base + 0x11f30c);
        auto exdata_use = efp->exdata_use;
        struct DialogParamInfo {
            short type;
            short exdata_use_id;
            char* name;
        }*dlgparam = *(DialogParamInfo**)(GLOBAL::exedit_base + 0x11f0c0);
        int i = 0;
        while (dlgparam[i].type) {
            update_any_exdata_use_idx(efp, dlgparam[i].exdata_use_id);
            i++;
        }
        deselect_object_if();
    }

    void __cdecl any_obj_t::update_dlg_chromakey_wrap(ExEdit::Filter* efp, int* exdata) {
        reinterpret_cast<void(__cdecl*)(ExEdit::Filter*, void*)>(GLOBAL::exedit_base + 0x144c0)(efp, exdata);
        if (exdata[2] != 2) {
            update_any_exdata_use_idx(efp, 0); // OFS::ExEdit::str_color_yc
            update_any_exdata_use_idx(efp, 2); // OFS::ExEdit::str_status
        }
        deselect_object_if();
    }
    void __cdecl any_obj_t::update_dlg_colorkey_wrap(ExEdit::Filter* efp, int* exdata) {
        reinterpret_cast<void(__cdecl*)(ExEdit::Filter*, void*)>(GLOBAL::exedit_base + 0x16940)(efp, exdata);
        if (exdata[2] != 2) {
            update_any_exdata_use_idx(efp, 0); // OFS::ExEdit::str_color_yc
            update_any_exdata_use_idx(efp, 2); // OFS::ExEdit::str_status
        }
        deselect_object_if();
    }
    void any_obj_t::update_any_color_specialcolorconv(ExEdit::Filter* efp, int id, short status) {
        short* exdata = (short*)efp->exdata_ptr;
        exdata[3 + id * 4] = status;

        update_any_exdata_use_idx(efp, 0 + id * 2); // OFS::ExEdit::str_color_yc OFS::ExEdit::str_color_yc2
        update_any_exdata_use_idx(efp, 1 + id * 2); // OFS::ExEdit::str_status OFS::ExEdit::str_status2
    }
    void __stdcall any_obj_t::mov_status_0_specialcolorconv(ExEdit::Filter* efp) {
        update_any_color_specialcolorconv(efp, 0, 0);
    }
    void __stdcall any_obj_t::mov_status2_0_specialcolorconv(ExEdit::Filter* efp) {
        update_any_color_specialcolorconv(efp, 1, 0);
    }
    void __stdcall any_obj_t::mov_status_1_specialcolorconv(ExEdit::Filter* efp) {
        update_any_color_specialcolorconv(efp, 0, 1);
    }
    void __stdcall any_obj_t::mov_status2_1_specialcolorconv(ExEdit::Filter* efp) {
        update_any_color_specialcolorconv(efp, 1, 1);
    }

    void __cdecl any_obj_t::init_setting_dialog_script_wrap(ExEdit::Filter* efp, void* exdata, int upd_flag, int sw_flag, short type, char* name, int folder_flag) {
        reinterpret_cast<void(__cdecl*)(ExEdit::Filter*, void*, int, int, short, char*, int)>(GLOBAL::exedit_base + 0x2660)(efp, exdata, upd_flag, sw_flag, type, name, folder_flag);

        for (int i = 0; i < 4; i++) {
            update_any_exdata_use_idx(efp, i);
        }
        for (int i = 0; i < efp->check_n; i++) {
            update_any_check(efp->processing, i);
        }
        for (int i = 0; i < efp->track_n; i++) {
            update_any_track(efp->processing, i);
        }
        update_any_dispname(efp->processing);
    }
    void __cdecl any_obj_t::init_setting_dialog_scenechange_wrap(ExEdit::Filter* efp, void* exdata, LPARAM lparam, int sw_flag, short type) {
        reinterpret_cast<void(__cdecl*)(ExEdit::Filter*, void*, LPARAM, int, short)>(GLOBAL::exedit_base + 0x871a0)(efp, exdata, lparam, sw_flag, type);

        for (int i = 0; i < 4; i++) {
            update_any_exdata_use_idx(efp, i);
        }
        for (int i = 1; i < efp->check_n; i++) { // 0:反転 はそのまま
            update_any_check(efp->processing, i);
        }
        for (int i = 0; i < efp->track_n; i++) {
            update_any_track(efp->processing, i);
        }
    }

    BOOL __cdecl any_obj_t::disp_1st_dlg_script_wrap(HWND hwnd, ExEdit::Filter* efp, void* exdata, short type, char* name) {
        int SelectingObjectNum = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::SelectingObjectNum);

        auto org_ofi = efp->processing;
        int org_object_idx = ((int)org_ofi & 0xffff) - 1;
        int org_filter_idx = (int)org_ofi >> 16;
        auto org_exdata_ptr = (ExEdit::Exdata::efAnimationEffect*)efp->exdata_ptr;
        BOOL ret = reinterpret_cast<BOOL(__cdecl*)(HWND, ExEdit::Filter*, void*, short, char*)>(GLOBAL::exedit_base + 0x30a0)(hwnd, efp, exdata, type, name);

        if (script_dlg_ok_cancel && 0 < SelectingObjectNum) {
            int* SelectingObjectIdxArray = (int*)(GLOBAL::exedit_base + OFS::ExEdit::SelectingObjectIdxArray);

            for (int i = 0; i < SelectingObjectNum; i++) {
                int select_idx = SelectingObjectIdxArray[i];
                int filter_idx = reinterpret_cast<int(__cdecl*)(int, int, int)>(GLOBAL::exedit_base + OFS::ExEdit::get_same_filter_idx_if_leader)(select_idx, org_object_idx, org_filter_idx);
                if (0 <= filter_idx) {
                    auto exdata_ptr = (ExEdit::Exdata::efAnimationEffect*)reinterpret_cast<int(__cdecl*)(ExEdit::ObjectFilterIndex)>(GLOBAL::exedit_base + OFS::ExEdit::get_exdata_ptr)((ExEdit::ObjectFilterIndex)((filter_idx << 16) | select_idx + 1));
                    if (exdata_ptr->type == org_exdata_ptr->type && lstrcmpA(exdata_ptr->name, org_exdata_ptr->name) == 0) {
                        reinterpret_cast<void(__cdecl*)(int, int)>(GLOBAL::exedit_base + OFS::ExEdit::set_undo)(select_idx, 0);
                        memcpy(exdata_ptr->param, org_exdata_ptr->param, 256);
                    }
                }
            }
        }
        deselect_object_if();
        script_dlg_ok_cancel = FALSE;
        return ret;
    }

    
    BOOL __cdecl any_obj_t::update_script_param_wrap(ExEdit::Filter* efp, char* name, char* valuestr) {
        if (!reinterpret_cast<BOOL(__cdecl*)(ExEdit::Filter*, char*, char*)>(GLOBAL::exedit_base + 0x2300)(efp, name, valuestr)) return FALSE;
        
        if (!script_dlg_ok_cancel) return TRUE;

        int SelectingObjectNum = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::SelectingObjectNum);
        if (SelectingObjectNum <= 0) return TRUE;

        int* SelectingObjectIdxArray = (int*)(GLOBAL::exedit_base + OFS::ExEdit::SelectingObjectIdxArray);

        auto org_ofi = efp->processing;
        int org_object_idx = ((int)org_ofi & 0xffff) - 1;
        int org_filter_idx = (int)org_ofi >> 16;
        auto org_exdata_ptr = (ExEdit::Exdata::efAnimationEffect*)efp->exdata_ptr;
        for (int i = 0; i < SelectingObjectNum; i++) {
            int select_idx = SelectingObjectIdxArray[i];
            int filter_idx = reinterpret_cast<int(__cdecl*)(int, int, int)>(GLOBAL::exedit_base + OFS::ExEdit::get_same_filter_idx_if_leader)(select_idx, org_object_idx, org_filter_idx);
            if (0 <= filter_idx) {
                efp->processing = (ExEdit::ObjectFilterIndex)((filter_idx << 16) | select_idx + 1);
                efp->exdata_ptr = (void*)reinterpret_cast<int(__cdecl*)(ExEdit::ObjectFilterIndex)>(GLOBAL::exedit_base + OFS::ExEdit::get_exdata_ptr)(efp->processing);
                if (((ExEdit::Exdata::efAnimationEffect*)efp->exdata_ptr)->type == org_exdata_ptr->type && lstrcmpA(((ExEdit::Exdata::efAnimationEffect*)efp->exdata_ptr)->name, org_exdata_ptr->name) == 0) {
                    reinterpret_cast<void(__cdecl*)(int, int)>(GLOBAL::exedit_base + OFS::ExEdit::set_undo)(select_idx, 0);
                    reinterpret_cast<BOOL(__cdecl*)(ExEdit::Filter*, char*, char*)>(GLOBAL::exedit_base + 0x2300)(efp, name, valuestr);
                }
            }
        }
        efp->processing = org_ofi;
        efp->exdata_ptr = org_exdata_ptr;
        script_dlg_ok_cancel = FALSE;
        return TRUE;
    }
    
    int __cdecl any_obj_t::get_same_track_id_wrap(int dst_idx, int src_idx, int track_idx) {
        auto obj = *(ExEdit::Object**)(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
        auto LoadedFilterTable = (ExEdit::Filter**)(GLOBAL::exedit_base + OFS::ExEdit::LoadedFilterTable);

        auto filter_param = obj[src_idx].filter_param;
        int src_filter_idx;
        for (src_filter_idx = 0; src_filter_idx < 12; src_filter_idx++) {
            if (filter_param->id < 0) return -1;
            if (track_idx < LoadedFilterTable[filter_param->id]->track_n) break;
            track_idx -= LoadedFilterTable[filter_param->id]->track_n;
            filter_param++;
        }
        if (12 <= src_filter_idx) return -1;
        
        int dst_filter_idx;
        if (has_flag(LoadedFilterTable[obj[src_idx].filter_param[src_filter_idx].id]->flag, ExEdit::Filter::Flag::Output)) {
            dst_filter_idx = reinterpret_cast<int(__cdecl*)(int)>(GLOBAL::exedit_base +OFS::ExEdit::get_last_filter_idx)(dst_idx);
        } else {
            dst_filter_idx = src_filter_idx;
        }
        int filter_id = obj[dst_idx].filter_param[dst_filter_idx].id;
        if (filter_id < 0) return -1;
        if (LoadedFilterTable[filter_id]->track_n < track_idx) return -1;
        // if (((int)LoadedFilterTable[obj[dst_idx].filter_param[dst_filter_idx].id]->flag ^ (int)LoadedFilterTable[obj[src_idx].filter_param[src_filter_idx].id]->flag) & 0x38) return -1;
        // if (lstrcmpA(LoadedFilterTable[obj[dst_idx].filter_param[dst_filter_idx].id]->name, LoadedFilterTable[obj[src_idx].filter_param[src_filter_idx].id]->name)) return -1;
        
        if (filter_id != obj[src_idx].filter_param[src_filter_idx].id) return -1;

        switch ((int)LoadedFilterTable[filter_id] - GLOBAL::exedit_base) {
        case OFS::ExEdit::efAnimationEffect_ptr:
        case OFS::ExEdit::efCustomObject_ptr:
        case OFS::ExEdit::efCameraEffect_ptr:
        case OFS::ExEdit::efSceneChange_ptr:
            auto dst_exdata = reinterpret_cast<ExEdit::Exdata::efAnimationEffect*(__cdecl*)(ExEdit::ObjectFilterIndex)>(GLOBAL::exedit_base + OFS::ExEdit::get_exdata_ptr)((ExEdit::ObjectFilterIndex)((dst_filter_idx << 16) | dst_idx + 1));
            auto src_exdata = reinterpret_cast<ExEdit::Exdata::efAnimationEffect*(__cdecl*)(ExEdit::ObjectFilterIndex)>(GLOBAL::exedit_base + OFS::ExEdit::get_exdata_ptr)((ExEdit::ObjectFilterIndex)((src_filter_idx << 16) | src_idx + 1));
            if (dst_exdata->type != src_exdata->type || lstrcmpA(dst_exdata->name, src_exdata->name)) return -1;
        }

        return obj[dst_idx].filter_param[dst_filter_idx].track_begin + track_idx;
    }
    int __cdecl any_obj_t::get_same_check_id_wrap(int dst_idx, int src_idx, int check_idx) {
        auto obj = *(ExEdit::Object**)(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
        auto LoadedFilterTable = (ExEdit::Filter**)(GLOBAL::exedit_base + OFS::ExEdit::LoadedFilterTable);

        auto filter_param = obj[src_idx].filter_param;
        int src_filter_idx;
        for (src_filter_idx = 0; src_filter_idx < 12; src_filter_idx++) {
            if (filter_param->id < 0) return -1;
            if (check_idx < LoadedFilterTable[filter_param->id]->check_n) break;
            check_idx -= LoadedFilterTable[filter_param->id]->check_n;
            filter_param++;
        }
        if (12 <= src_filter_idx) return -1;

        int dst_filter_idx;
        if (has_flag(LoadedFilterTable[obj[src_idx].filter_param[src_filter_idx].id]->flag, ExEdit::Filter::Flag::Output)) {
            dst_filter_idx = reinterpret_cast<int(__cdecl*)(int)>(GLOBAL::exedit_base + OFS::ExEdit::get_last_filter_idx)(dst_idx);
        } else {
            dst_filter_idx = src_filter_idx;
        }
        int filter_id = obj[dst_idx].filter_param[dst_filter_idx].id;
        if (filter_id < 0) return -1;
        if (LoadedFilterTable[filter_id]->check_n < check_idx) return -1;
        // if (((int)LoadedFilterTable[obj[dst_idx].filter_param[dst_filter_idx].id]->flag ^ (int)LoadedFilterTable[obj[src_idx].filter_param[src_filter_idx].id]->flag) & 0x38) return -1;
        // if (lstrcmpA(LoadedFilterTable[obj[dst_idx].filter_param[dst_filter_idx].id]->name, LoadedFilterTable[obj[src_idx].filter_param[src_filter_idx].id]->name)) return -1;

        if (filter_id != obj[src_idx].filter_param[src_filter_idx].id) return -1;

        switch ((int)LoadedFilterTable[filter_id] - GLOBAL::exedit_base) {
        case OFS::ExEdit::efSceneChange_ptr:
            if (check_idx == 0) break; // シーンチェンジの"反転"は別スクリプトでも有効にしておく
            [[fallthrough]];
        case OFS::ExEdit::efAnimationEffect_ptr:
        case OFS::ExEdit::efCustomObject_ptr:
        case OFS::ExEdit::efCameraEffect_ptr:
            auto dst_exdata = reinterpret_cast<ExEdit::Exdata::efAnimationEffect*(__cdecl*)(ExEdit::ObjectFilterIndex)>(GLOBAL::exedit_base + OFS::ExEdit::get_exdata_ptr)((ExEdit::ObjectFilterIndex)((dst_filter_idx << 16) | dst_idx + 1));
            auto src_exdata = reinterpret_cast<ExEdit::Exdata::efAnimationEffect*(__cdecl*)(ExEdit::ObjectFilterIndex)>(GLOBAL::exedit_base + OFS::ExEdit::get_exdata_ptr)((ExEdit::ObjectFilterIndex)((src_filter_idx << 16) | src_idx + 1));
            if (dst_exdata->type != src_exdata->type || lstrcmpA(dst_exdata->name, src_exdata->name)) return -1;
            break;
        }

        return obj[dst_idx].filter_param[dst_filter_idx].check_begin + check_idx;
    }
    

    void __cdecl any_obj_t::update_dlg_mask_wrap(ExEdit::Filter* efp, char* name, int sw_param) {
        reinterpret_cast<void(__cdecl*)(ExEdit::Filter*, char*, int)>(GLOBAL::exedit_base + 0x69f20)(efp, name, sw_param);
        for (int i = 0; i < 3; i++) {
            update_any_exdata_use_idx(efp, i); // OFS::ExEdit::str_type OFS::ExEdit::str_name OFS::ExEdit::str_mode
        }
        deselect_object_if();
    }
    int __cdecl any_obj_t::mov_eax_1_portion_filter_wrap() {
        auto efp = (ExEdit::Filter*)(GLOBAL::exedit_base + OFS::ExEdit::efPortionFilter);
        update_any_exdata_use_idx(efp, 0); // OFS::ExEdit::str_type
        update_any_exdata_use_idx(efp, 1); // OFS::ExEdit::str_name
        deselect_object_if();
        efp->func_window_init(NULL, NULL, 0, 0, 0, efp); // obj_portionfilterによって、ファイルパスを表示できるようになる
        return 1;
    }

    void __cdecl any_obj_t::update_dlg_displacementmap_wrap(ExEdit::Filter* efp, void* exdata, char* name, int sw_param, int edi, int esi, int ebp, int ebx, tagRECT rect, DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp) {
        reinterpret_cast<void(__cdecl*)(ExEdit::Filter*, void*, char*, int)>(GLOBAL::exedit_base + 0x201d0)(efp, exdata, name, sw_param);
        if ((wparam & 0xffff) == 0x1e1b) {
            for (int i = 0; i < 3; i++) {
                update_any_exdata_use_idx(efp, i); // OFS::ExEdit::str_type OFS::ExEdit::str_name OFS::ExEdit::str_mode
            }
        } else if ((wparam & 0xffff) == 0x1e1c) {
            if (wparam >> 0x10 == 0) {
                for (int i = 0; i < 3; i++) {
                    update_any_exdata_use_idx(efp, i); // OFS::ExEdit::str_type OFS::ExEdit::str_name OFS::ExEdit::str_mode
                }
            } else if (wparam >> 0x10 == 1) {
                update_any_exdata_use_idx(efp, 3); // OFS::ExEdit::str_calc
            }
        }
        deselect_object_if();
    }
    void __cdecl any_obj_t::update_dlg_shadow_wrap(ExEdit::Filter* efp, void* exdata) {
        reinterpret_cast<void(__cdecl*)(ExEdit::Filter*, void*)>(GLOBAL::exedit_base + 0x88f40)(efp, exdata);
        update_any_exdata_use_idx(efp, 2); // OFS::ExEdit::str_file
        deselect_object_if();
    }
    void __cdecl any_obj_t::update_dlg_border_wrap(ExEdit::Filter* efp, void* exdata) {
        reinterpret_cast<void(__cdecl*)(ExEdit::Filter*, void*)>(GLOBAL::exedit_base + 0x52200)(efp, exdata);
        update_any_exdata_use_idx(efp, 2); // OFS::ExEdit::str_file
        deselect_object_if();
    }
    int __cdecl any_obj_t::mov_eax_1_use0_e0_wrap(DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp) {
        update_any_exdata_use_idx(efp, 0);
        return 1;
    }
    int __cdecl any_obj_t::mov_eax_1_use2_e0_wrap(DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp) {
        update_any_exdata_use_idx(efp, 2);
        return 1;
    }
    int __cdecl any_obj_t::mov_eax_1_use0_e1_wrap(int e1, DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp) {
        update_any_exdata_use_idx(efp, 0);
        return 1;
    }
    int __cdecl any_obj_t::mov_eax_1_use1_e1_wrap(int e1, DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp) {
        update_any_exdata_use_idx(efp, 1);
        return 1;
    }
    int __cdecl any_obj_t::mov_eax_1_use2_e1_wrap(int e1, DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp) {
        update_any_exdata_use_idx(efp, 2);
        return 1;
    }
    int __cdecl any_obj_t::mov_eax_1_use5_e1_wrap(int e1, DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp) {
        update_any_exdata_use_idx(efp, 5);
        return 1;
    }
    int __cdecl any_obj_t::mov_eax_1_use2_e2_wrap(int e1, int e2, DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp) {
        update_any_exdata_use_idx(efp, 2);
        return 1;
    }
    int __cdecl any_obj_t::mov_eax_1_use0use1_e1_wrap(int e1, DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp) {
        update_any_exdata_use_idx(efp, 0);
        update_any_exdata_use_idx(efp, 1);
        return 1;
    }
    /*
    int __cdecl any_obj_t::mov_eax_1_type_1_wrap(int e1, DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp) {
        update_any_exdata(efp->processing, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_type));
        return 1;
    }
    int __cdecl any_obj_t::mov_eax_1_blend_1_wrap(int e1, DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp) {
        update_any_exdata(efp->processing, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_blend));
        return 1;
    }
    int __cdecl any_obj_t::mov_eax_1_mode_0_wrap(DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp) {
        update_any_exdata(efp->processing, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_mode));
        return 1;
    }
    int __cdecl any_obj_t::mov_eax_1_mode_1_wrap(int e1, DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp) {
        update_any_exdata(efp->processing, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_mode));
        return 1;
    }
    int __cdecl any_obj_t::mov_eax_1_mode_2_wrap(int e1, int e2, DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp) {
        update_any_exdata(efp->processing, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_mode));
        return 1;
    }
    int __cdecl any_obj_t::mov_eax_1_type_name_1_wrap(int e1, DWORD ret, HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp) {
        update_any_exdata(efp->processing, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_type));
        update_any_exdata(efp->processing, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_name));
        return 1;
    }
    */

    void __cdecl  any_obj_t::update_obj_data_before_clipping_wrap(int object_idx) {
        reinterpret_cast<void(__cdecl*)(int)>(GLOBAL::exedit_base + 0x36020)(object_idx);
        if (GetKeyState(VK_CONTROL) < 0) { // グループ化オブジェクト単体で行えなくなるのがつらいため
            update_any_object_flag(object_idx, 0x100);
        }
    }
    void __cdecl  any_obj_t::update_obj_data_camera_target_wrap(int object_idx) {
        reinterpret_cast<void(__cdecl*)(int)>(GLOBAL::exedit_base + 0x36020)(object_idx);
        if (GetKeyState(VK_CONTROL) < 0) { // グループ化オブジェクト単体で行えなくなるのがつらいため
            update_any_object_flag(object_idx, 0x200);
        }
    }

    void __cdecl any_obj_t::delete_filter_effect_wrap(int object_idx, int filter_idx) {
        auto obj = *(ExEdit::Object**)(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
        if (0 <= obj[object_idx].index_midpt_leader) {
            object_idx = obj[object_idx].index_midpt_leader;
        }
        int filter_param_id = obj[object_idx].filter_param[filter_idx].id;

        reinterpret_cast<void(__cdecl*)(int, int)>(GLOBAL::exedit_base + OFS::ExEdit::delete_filter_effect)(object_idx, filter_idx);

        int SelectingObjectNum = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::SelectingObjectNum);
        int* SelectingObjectIdxArray = (int*)(GLOBAL::exedit_base + OFS::ExEdit::SelectingObjectIdxArray);
        for (int i = 0; i < SelectingObjectNum; i++) {
            int select_idx = SelectingObjectIdxArray[i];
            if (select_idx != object_idx) {
                if (obj[select_idx].index_midpt_leader < 0 || obj[select_idx].index_midpt_leader == select_idx) {
                    if (obj[select_idx].filter_param[filter_idx].id == filter_param_id) {
                        reinterpret_cast<void(__cdecl*)(int, int)>(GLOBAL::exedit_base + OFS::ExEdit::set_undo)(select_idx, 1);
                        reinterpret_cast<void(__cdecl*)(int, int)>(GLOBAL::exedit_base + OFS::ExEdit::delete_filter_effect)(select_idx, filter_idx);
                    }
                }
            }
        }
    }



    /*  // OFS::ExEdit::update_any_exdataをちょっと改造 不要になったけど解析データとして使うことがあれば
    void __cdecl update_any_exdata_ex(ExEdit::ObjectFilterIndex ofi, char* str, int flag, void* param) {
        if ((int)ofi == 0) return;

        int SettingDialog_ObjIdx = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::SettingDialog_ObjIdx);
        if (SettingDialog_ObjIdx < 0) return;

        int SelectingObjectNum = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::SelectingObjectNum);
        if (SelectingObjectNum <= 0) return;

        auto obj = *(ExEdit::Object**)(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
        int* SelectingObjectIdxArray = (int*)(GLOBAL::exedit_base + OFS::ExEdit::SelectingObjectIdxArray);
        DWORD ExdataPointer = *(DWORD*)(GLOBAL::exedit_base + OFS::ExEdit::ExdataPointer);
        auto LoadedFilterTable = (ExEdit::Filter**)(GLOBAL::exedit_base + OFS::ExEdit::LoadedFilterTable);

        int filter_idx = (int)ofi >> 16;
        auto efp = LoadedFilterTable[obj[SettingDialog_ObjIdx].filter_param[filter_idx].id];
        if (efp->exdata_use == NULL) return;
        if (efp->exdata_size <= 0) return;

        if (0 <= obj[SettingDialog_ObjIdx].index_midpt_leader) {
            SettingDialog_ObjIdx = obj[SettingDialog_ObjIdx].index_midpt_leader;
        }
        int dlg_exdata_offset = obj[SettingDialog_ObjIdx].exdata_offset;
        int dlg_filter_exdata_offset = obj[SettingDialog_ObjIdx].filter_param[filter_idx].exdata_offset;


        for (int i = 0; i < SelectingObjectNum; i++) {
            int select_idx = SelectingObjectIdxArray[i];
            int leader_filter_idx = reinterpret_cast<int(__cdecl*)(int, int, int)>(GLOBAL::exedit_base + OFS::ExEdit::get_same_filter_idx_if_leader)(select_idx, SettingDialog_ObjIdx, filter_idx);
            if (0 <= leader_filter_idx) {
                if (0 <= obj[select_idx].index_midpt_leader) {
                    select_idx = obj[select_idx].index_midpt_leader;
                }
                int select_filter_exdata_offset = obj[select_idx].filter_param[leader_filter_idx].exdata_offset;
                int select_exdata_offset = obj[select_idx].exdata_offset;
                int exdata_offset = 0;
// ##################### sdk更新したら変える
                struct exdata_use_fix{ // sdkのExdataUse::typeがintになっているため上手くいかない
                    short type;
                    short size;
                    char* name;
                } *exdata_use = (exdata_use_fix*)efp->exdata_use;
                while (exdata_offset < efp->exdata_size) {
                    if (lstrcmpA(exdata_use->name, str) != 0) {
                        exdata_offset += exdata_use->size;
                        exdata_use++;
                    } else {
                        void* select_exdata = (void*)(ExdataPointer + select_exdata_offset + select_filter_exdata_offset + 4 + exdata_offset);
                        void* dlg_exdata = (void*)(ExdataPointer + dlg_exdata_offset + dlg_filter_exdata_offset + 4 + exdata_offset);
                        if (memcmp(select_exdata, dlg_exdata, exdata_use->size)) {
                            reinterpret_cast<void(__cdecl*)(int, int)>(GLOBAL::exedit_base + OFS::ExEdit::set_undo)(select_idx, 0);

                            if (flag & 2) { // 音声ファイルの動画ファイルと連携を外したい
                                reinterpret_cast<void(__cdecl*)(int, int)>(GLOBAL::exedit_base + OFS::ExEdit::set_undo)(select_idx, 1);
                                int idx = select_idx + 1;
                                while (0 < idx) {
                                    obj[idx-1].check_value[obj[idx-1].filter_param[leader_filter_idx].check_begin + 1] = 0;
                                    idx = (int)efp->exfunc->x08((ExEdit::ObjectFilterIndex)(idx)); // get_next_obj
                                }
                            }

                            memcpy(select_exdata, dlg_exdata, exdata_use->size);

                            if (flag & 1) {
                                efp->exfunc->rename_object((ExEdit::ObjectFilterIndex)(select_idx + 1), (char*)param);
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
    */

} // namespace patch
#endif // ifdef PATCH_SWITCH_ANY_OBJ
