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
            ExEdit::Object::FilterParam* filter_param = obj[dst_idx].filter_param;
            int i;
            for (i = 1; i < 12 && filter_param->id < 0; i++) {
                filter_param++;
            }
            filter_idx = i - 1;
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


	char* __cdecl any_obj_t::disp_extension_image_file_wrap(ExEdit::Filter* efp, char* path) {
		char* name = reinterpret_cast<char*(__cdecl*)(ExEdit::Filter*, char*)>(GLOBAL::exedit_base + 0xe220)(efp, path);
        reinterpret_cast<void(__cdecl*)(ExEdit::ObjectFilterIndex, char*)>(GLOBAL::exedit_base + OFS::ExEdit::update_any_exdata)(efp->processing, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_file));
        if (!has_flag(efp->flag, ExEdit::Filter::Flag::Effect)) {
            efp->exfunc->rename_object(efp->processing, name);
            update_any_dispname(efp->processing);
        }
        if (0 <= GetKeyState(VK_CONTROL)) {
            reinterpret_cast<void(__cdecl*)(void)>(GLOBAL::exedit_base + OFS::ExEdit::deselect_object)();
        }

        return name;
	}

    int __stdcall any_obj_t::count_section_num_wrap(ExEdit::Filter* efp, ExEdit::ObjectFilterIndex ofi) {
        if (0 < *(int*)(GLOBAL::exedit_base + OFS::ExEdit::SelectingObjectNum)) {
            return 0;
        }
        return efp->exfunc->count_section_num(ofi);
    }

    void __cdecl any_obj_t::calc_milli_second_movie_file_wrap(ExEdit::Filter* efp, void* exdata) {
        reinterpret_cast<void(__cdecl*)(ExEdit::Filter*, void*)>(GLOBAL::exedit_base + 0x6900)(efp, exdata);
        reinterpret_cast<void(__cdecl*)(ExEdit::ObjectFilterIndex, char*)>(GLOBAL::exedit_base + OFS::ExEdit::update_any_exdata)(efp->processing, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_file));
        reinterpret_cast<void(__cdecl*)(ExEdit::ObjectFilterIndex, char*)>(GLOBAL::exedit_base + OFS::ExEdit::update_any_exdata)(efp->processing, 0);
        if (!has_flag(efp->flag, ExEdit::Filter::Flag::Effect)) {
            update_any_dispname(efp->processing);
        }
        if (0 <= GetKeyState(VK_CONTROL)) {
            reinterpret_cast<void(__cdecl*)(void)>(GLOBAL::exedit_base + OFS::ExEdit::deselect_object)();
        }
    }
    void __cdecl any_obj_t::calc_milli_second_audio_file_wrap(ExEdit::Filter* efp, void* exdata) {
        reinterpret_cast<void(__cdecl*)(ExEdit::Filter*, void*)>(GLOBAL::exedit_base + 0x902d0)(efp, exdata);
        update_any_check(efp->processing, 1);
        reinterpret_cast<void(__cdecl*)(ExEdit::ObjectFilterIndex, char*)>(GLOBAL::exedit_base + OFS::ExEdit::update_any_exdata)(efp->processing, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_file));
        reinterpret_cast<void(__cdecl*)(ExEdit::ObjectFilterIndex, char*)>(GLOBAL::exedit_base + OFS::ExEdit::update_any_exdata)(efp->processing, 0);
        update_any_dispname(efp->processing);
        if (0 <= GetKeyState(VK_CONTROL)) {
            reinterpret_cast<void(__cdecl*)(void)>(GLOBAL::exedit_base + OFS::ExEdit::deselect_object)();
        }
    }
    void __cdecl any_obj_t::rename_object_audio_file_wrap(ExEdit::ObjectFilterIndex ofi, char* name) {
        auto exfunc = (ExEdit::Exfunc*)(GLOBAL::exedit_base + OFS::ExEdit::exfunc);
        exfunc->rename_object(ofi, name);
        update_any_dispname(ofi);
    }


    void __cdecl any_obj_t::update_obj_data_waveform_wrap(ExEdit::ObjectFilterIndex ofi) {
        auto exfunc = (ExEdit::Exfunc*)(GLOBAL::exedit_base + OFS::ExEdit::exfunc);
        exfunc->x00(ofi);
        update_any_check(ofi, 3);
        reinterpret_cast<void(__cdecl*)(ExEdit::ObjectFilterIndex, char*)>(GLOBAL::exedit_base + OFS::ExEdit::update_any_exdata)(ofi, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_file));
        reinterpret_cast<void(__cdecl*)(ExEdit::ObjectFilterIndex, char*)>(GLOBAL::exedit_base + OFS::ExEdit::update_any_exdata)(ofi, 0);
        update_any_dispname(ofi);
        if (0 <= GetKeyState(VK_CONTROL)) {
            reinterpret_cast<void(__cdecl*)(void)>(GLOBAL::exedit_base + OFS::ExEdit::deselect_object)();
        }
    }
    BOOL __cdecl any_obj_t::update_any_waveform_type() {
        auto efp = (ExEdit::Filter*)(GLOBAL::exedit_base + OFS::ExEdit::efWaveform);
        auto exdata_use = efp->exdata_use;
        struct DialogParamInfo {
            short type;
            short exdata_use_id;
            char* name;
        }*dlgparam = (DialogParamInfo*)(GLOBAL::exedit_base + 0xba240);
        int i = 0;
        reinterpret_cast<void(__cdecl*)(ExEdit::ObjectFilterIndex, char*)>(GLOBAL::exedit_base + OFS::ExEdit::update_any_exdata)(efp->processing, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_type));
        while (dlgparam[i].type) {
            reinterpret_cast<void(__cdecl*)(ExEdit::ObjectFilterIndex, char*)>(GLOBAL::exedit_base + OFS::ExEdit::update_any_exdata)(efp->processing, (char*)exdata_use[dlgparam[i].exdata_use_id].name);
            i++;
        }
        return TRUE;
    }

    void __cdecl any_obj_t::set_figure_type_text_wrap(ExEdit::Filter* efp, void* exdata) {
        reinterpret_cast<void(__cdecl*)(ExEdit::Filter*, void*)>(GLOBAL::exedit_base + 0x74740)(efp, exdata);
        reinterpret_cast<void(__cdecl*)(ExEdit::ObjectFilterIndex, char*)>(GLOBAL::exedit_base + OFS::ExEdit::update_any_exdata)(efp->processing, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_type));
        reinterpret_cast<void(__cdecl*)(ExEdit::ObjectFilterIndex, char*)>(GLOBAL::exedit_base + OFS::ExEdit::update_any_exdata)(efp->processing, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_name));
        update_any_dispname(efp->processing);
        if (0 <= GetKeyState(VK_CONTROL)) {
            reinterpret_cast<void(__cdecl*)(void)>(GLOBAL::exedit_base + OFS::ExEdit::deselect_object)();
        }
    }

    void __cdecl any_obj_t::update_any_range(ExEdit::Filter* efp) { // undoの部分から呼び出す
        if (!any_obj.is_enabled_i())return;

        reinterpret_cast<void(__cdecl*)(ExEdit::ObjectFilterIndex, char*)>(GLOBAL::exedit_base + OFS::ExEdit::update_any_exdata)(efp->processing, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_range));

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
            reinterpret_cast<void(__cdecl*)(ExEdit::ObjectFilterIndex, char*)>(GLOBAL::exedit_base + OFS::ExEdit::update_any_exdata)(efp->processing, (char*)exdata_use[dlgparam[i].exdata_use_id].name);
            i++;
        }
        if (0 <= GetKeyState(VK_CONTROL)) {
            reinterpret_cast<void(__cdecl*)(void)>(GLOBAL::exedit_base + OFS::ExEdit::deselect_object)();
        }
    }


    void __cdecl any_obj_t::init_setting_dialog_file_script_wrap(ExEdit::Filter* efp, void* exdata, int upd_flag, int sw_flag, short type, char* name, int folder_flag) {
        reinterpret_cast<void(__cdecl*)(ExEdit::Filter*, void*, int, int, short, char*, int)>(GLOBAL::exedit_base + 0x2660)(efp, exdata, upd_flag, sw_flag, type, name, folder_flag);

        // ##################### sdk更新したら変える
        struct exdata_use_fix { // sdkのExdataUse::typeがintになっているため上手くいかない
            short type;
            short size;
            char* name;
        } *exdata_use = (exdata_use_fix*)efp->exdata_use;
        for (int i = 0; i < 4; i++) {
            reinterpret_cast<void(__cdecl*)(ExEdit::ObjectFilterIndex, char*)>(GLOBAL::exedit_base + OFS::ExEdit::update_any_exdata)(efp->processing, (char*)exdata_use[i].name);
        }
        for (int i = 0; i < efp->check_n; i++) {
            update_any_check(efp->processing, i);
        }
        for (int i = 0; i < efp->track_n; i++) {
            update_any_track(efp->processing, i);
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
