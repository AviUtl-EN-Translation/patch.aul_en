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

#include "patch_object_table.hpp"


#ifdef PATCH_SWITCH_OBJECT_TABLE
namespace patch {

	void update_obj_ptr(int ptr_sub) {
		auto SortedObjectTable = reinterpret_cast<ExEdit::Object**>(GLOBAL::exedit_base + OFS::ExEdit::SortedObjectTable);
		int objnum= *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::ValidObjectNum);
		for (int i = 0; i < objnum; i++) {
			SortedObjectTable[i] = (ExEdit::Object*)((int)SortedObjectTable[i] + ptr_sub);
		}

		auto RangeObjectTable = reinterpret_cast<ExEdit::Object**>(GLOBAL::exedit_base + OFS::ExEdit::RangeObjectTable);
		for (int i = 0; i < 4096; i++) {
			if (RangeObjectTable[i] == NULL) {
				break;
			}
			RangeObjectTable[i] = (ExEdit::Object*)((int)RangeObjectTable[i] + ptr_sub);
		}

	}


    BOOL __cdecl object_table_t::object_realloc_wrap(void** pointer, size_t size) {
		int obj_ptr_old = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
        BOOL r = reinterpret_cast<BOOL(__cdecl*)(void**, size_t)>(GLOBAL::exedit_base + OFS::ExEdit::MyRealloc)(pointer, size);
		int obj_ptr_new = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
        if (obj_ptr_new != obj_ptr_old) {
			update_obj_ptr(obj_ptr_new - obj_ptr_old);
			/*
            reinterpret_cast<int(__cdecl*)()>(GLOBAL::exedit_base + OFS::ExEdit::update_ObjectTables)();
            reinterpret_cast<void(__cdecl*)()>(GLOBAL::exedit_base + OFS::ExEdit::set_range_object)();
			*/
        }
        return r;
    }

	void __fastcall object_table_t::comb_sort_current_layer(int sort_s, int sort_e) {
		int s = sort_s;
		int e = sort_e;
		int n = sort_e - sort_s;
		// if (n < 1) return; // この関数の前にある
		auto SortedObjectTable = reinterpret_cast<ExEdit::Object**>(GLOBAL::exedit_base + OFS::ExEdit::SortedObjectTable);
		while (true) {
			int lastswap = 0;
			int i = sort_s;
			int j = i + n;
			while (j <= sort_e) {
				if (SortedObjectTable[i]->frame_begin > SortedObjectTable[j]->frame_begin) {
					std::swap(SortedObjectTable[i], SortedObjectTable[j]);
					lastswap = i;
				}
				i++; j++;
			}
			switch (n) {
			case 12: case 13: case 14: case 15:
				n = 11;
				break;
			case 1:
				if (lastswap == 0) return;
				sort_e = lastswap;
				break;
			default:
				n = n * 10 / 13;
			}
		}
	}

    int __cdecl object_table_t::comb_sort_other_scene() {
		int sort_s = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::CurrentSceneObjectNum);
		int sort_e = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::ValidObjectNum) - 1;
		int n = sort_e - sort_s;
		if (n < 1) return *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::ValidObjectNum);
		auto SortedObjectTable = reinterpret_cast<ExEdit::Object**>(GLOBAL::exedit_base + OFS::ExEdit::SortedObjectTable);
		int lastswap;
		while (true) {
			lastswap = 0;
			int i = sort_s;
			int j = i + n;
			while (j <= sort_e) {
				auto obj1 = SortedObjectTable[i];
				auto obj2 = SortedObjectTable[j];
				if (obj1->scene_set > obj2->scene_set || obj1->scene_set == obj2->scene_set && (obj1->layer_set > obj2->layer_set || obj1->layer_set == obj2->layer_set && obj1->frame_begin > obj2->frame_begin)) {
					std::swap(SortedObjectTable[i], SortedObjectTable[j]);
					lastswap = i;
				}
				i++; j++;
			}
			switch (n) {
			case 12: case 13: case 14: case 15:
				n = 11;
				break;
			case 1:
				if (lastswap == 0) return *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::ValidObjectNum);
				sort_e = lastswap;
				break;
			default:
				n = n * 10 / 13;
			}
		}
    }

} // namespace patch
#endif // ifdef PATCH_SWITCH_OBJECT_TABLE
