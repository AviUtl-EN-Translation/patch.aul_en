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

    void __cdecl update_any_exdata_and_rename_object(ExEdit::ObjectFilterIndex ofi, char* str, int flag, void* param) {
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
            int leader_filter_idx = reinterpret_cast<int(__cdecl*)(int, int, int)>(GLOBAL::exedit_base + 0x365b0)(select_idx, SettingDialog_ObjIdx, filter_idx);
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

	char* __cdecl any_obj_t::disp_extension_image_file_wrap(ExEdit::Filter* efp, char* path) {
		char* name = reinterpret_cast<char*(__cdecl*)(ExEdit::Filter*, char*)>(GLOBAL::exedit_base + 0xe220)(efp, path);
        if ((int)efp->flag & 0x20) {
            reinterpret_cast<void(__cdecl*)(ExEdit::ObjectFilterIndex, char*)>(GLOBAL::exedit_base + OFS::ExEdit::update_any_exdata)(efp->processing, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_file));
        } else {
            update_any_exdata_and_rename_object(efp->processing, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_file), 1, name);
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

    char* __cdecl any_obj_t::disp_extension_movie_file_wrap(ExEdit::Filter* efp, char* path) {
        char* name = reinterpret_cast<char* (__cdecl*)(ExEdit::Filter*, char*)>(GLOBAL::exedit_base + 0x69d0)(efp, path);
        if ((int)efp->flag & 0x20) {
            reinterpret_cast<void(__cdecl*)(ExEdit::ObjectFilterIndex, char*)>(GLOBAL::exedit_base + OFS::ExEdit::update_any_exdata)(efp->processing, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_file));
        } else {
            update_any_exdata_and_rename_object(efp->processing, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_file), 1, name);
        }
        reinterpret_cast<void(__cdecl*)(ExEdit::ObjectFilterIndex, char*)>(GLOBAL::exedit_base + OFS::ExEdit::update_any_exdata)(efp->processing, 0);
        if (0 <= GetKeyState(VK_CONTROL)) {
            reinterpret_cast<void(__cdecl*)(void)>(GLOBAL::exedit_base + OFS::ExEdit::deselect_object)();
        }

        return name;
    }
    char* __cdecl any_obj_t::disp_extension_audio_file_wrap(ExEdit::Filter* efp, char* path) {
        char* name = reinterpret_cast<char* (__cdecl*)(ExEdit::Filter*, char*)>(GLOBAL::exedit_base + 0x8f960)(efp, path);
        update_any_exdata_and_rename_object(efp->processing, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_file), 3, name);
        reinterpret_cast<void(__cdecl*)(ExEdit::ObjectFilterIndex, char*)>(GLOBAL::exedit_base + OFS::ExEdit::update_any_exdata)(efp->processing, 0);
        if (0 <= GetKeyState(VK_CONTROL)) {
            reinterpret_cast<void(__cdecl*)(void)>(GLOBAL::exedit_base + OFS::ExEdit::deselect_object)();
        }

        return name;
    }

    void __cdecl any_obj_t::set_figure_type_text_wrap(ExEdit::Filter* efp, void* exdata) {
        reinterpret_cast<void(__cdecl*)(ExEdit::Filter*, void*)>(GLOBAL::exedit_base + 0x74740)(efp, exdata);
        int idx = ((int)efp->processing & 0xffff) - 1;
        auto obj = *(ExEdit::Object**)(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
        update_any_exdata_and_rename_object(efp->processing, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_type), 1, obj[idx].dispname);
        update_any_exdata_and_rename_object(efp->processing, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_name), 1, obj[idx].dispname);
        if (0 <= GetKeyState(VK_CONTROL)) {
            reinterpret_cast<void(__cdecl*)(void)>(GLOBAL::exedit_base + OFS::ExEdit::deselect_object)();
        }
    }

    void __cdecl any_obj_t::update_any_range(ExEdit::Filter* efp) { // undoの部分から呼び出す
        if (!any_obj.is_enabled_i())return;

        reinterpret_cast<void(__cdecl*)(ExEdit::ObjectFilterIndex, char*)>(GLOBAL::exedit_base + OFS::ExEdit::update_any_exdata)(efp->processing, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_range));

    }



} // namespace patch
#endif // ifdef PATCH_SWITCH_ANY_OBJ
