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

#include "patch_setting_dialog_next.hpp"

#ifdef PATCH_SWITCH_SETTINGDIALOG_NEXT
namespace patch {
    bool is_selecting(int object_idx) {
        int* SelectingObjectIdxArray = (int*)(GLOBAL::exedit_base + OFS::ExEdit::SelectingObjectIdxArray);
        auto SelectingObjectNum_ptr = reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::SelectingObjectNum);
        int i;
        for (i = 0; i < *SelectingObjectNum_ptr; i++) {
            if (SelectingObjectIdxArray[i] == object_idx) {
                return true;
            }
        }
        return false;
    }

	int __cdecl dialog_next_t::get_near_object_idx_wrap(int object_idx, int flag) {

        auto obj = *(ExEdit::Object**)(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
        auto SortedObjectTable = reinterpret_cast<ExEdit::Object**>(GLOBAL::exedit_base + OFS::ExEdit::SortedObjectTable);
        int CurrentSceneObjectNum = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::CurrentSceneObjectNum);

        int* SelectingObjectIdxArray = (int*)(GLOBAL::exedit_base + OFS::ExEdit::SelectingObjectIdxArray);
        auto SelectingObjectNum_ptr = reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::SelectingObjectNum);
        int SelectingObjectNum_cur = *SelectingObjectNum_ptr;

        int frame_cursor = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::frame_cursor);
        if (obj[object_idx].frame_begin <= frame_cursor && frame_cursor < obj[object_idx].frame_end) {
            int cur_dist = 127;
            int cur_idx = -1;
            for (int i = 0; i < CurrentSceneObjectNum; i++) {
                if (SortedObjectTable[i]->layer_disp != obj[object_idx].layer_disp) {
                    if (SortedObjectTable[i]->frame_begin <= frame_cursor && frame_cursor < SortedObjectTable[i]->frame_end) {
                        int dist = abs(SortedObjectTable[i]->layer_disp - obj[object_idx].layer_disp);
                        if (SortedObjectTable[i]->layer_disp < obj[object_idx].layer_disp) {
                            dist++;
                        }
                        if (cur_dist > dist) {
                            int sidx = ((int)SortedObjectTable[i] - (int)obj) / sizeof(ExEdit::Object);
                            if (!is_selecting(sidx)) {
                                cur_dist = dist;
                                cur_idx = sidx;
                            }
                        }
                    }
                }
            }
            if (0 <= cur_idx) {
                return cur_idx;
            }
        }
        int cur_dp_frame = 0;
        int cur_frame_len = 0;
        int cur_dist = 127;
        int cur_idx = -1;
        for (int i = 0; i < CurrentSceneObjectNum; i++) {
            if (SortedObjectTable[i]->layer_disp != obj[object_idx].layer_disp) {
                int dp = min(obj[object_idx].frame_end, SortedObjectTable[i]->frame_end) - max(obj[object_idx].frame_begin, SortedObjectTable[i]->frame_begin);
                if (0 < dp) {
                    int len = obj[object_idx].frame_end - obj[object_idx].frame_begin;
                    int dist = 2 * abs(SortedObjectTable[i]->layer_disp - obj[object_idx].layer_disp);
                    if (SortedObjectTable[i]->layer_disp < obj[object_idx].layer_disp) {
                        dist++;
                    }
                    if (cur_dist > dist || (cur_dist == dist && cur_dp_frame < dp || (cur_dp_frame == dp && cur_frame_len < len))) {
                        int sidx = ((int)SortedObjectTable[i] - (int)obj) / sizeof(ExEdit::Object);
                        if (!is_selecting(sidx)) {
                            cur_dp_frame = dp;
                            cur_frame_len = len;
                            cur_dist = dist;
                            cur_idx = sidx;
                        }
                    }
                }
            }
        }
        if (0 <= cur_idx) {
            return cur_idx;
        }


        int mid_idx = obj[object_idx].index_midpt_leader;
        if (mid_idx < 0) {
            reinterpret_cast<int(__cdecl*)(int)>(GLOBAL::exedit_base + OFS::ExEdit::select_object)(object_idx);
        } else {
            int* next_obj = reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::NextObjectIdxArray);
            while (0 <= next_obj[mid_idx]) {
                reinterpret_cast<int(__cdecl*)(int)>(GLOBAL::exedit_base + OFS::ExEdit::select_object)(mid_idx);
                mid_idx = next_obj[mid_idx];
            }
        }

        int ret = reinterpret_cast<int(__cdecl*)(int, int)>(GLOBAL::exedit_base + OFS::ExEdit::get_near_object_idx)(object_idx, flag);
        *SelectingObjectNum_ptr = SelectingObjectNum_cur;
        return ret;
	}

} // namespace patch
#endif // ifdef PATCH_SWITCH_SETTINGDIALOG_NEXT

