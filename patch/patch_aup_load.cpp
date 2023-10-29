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

#include "patch_aup_load.hpp"

#ifdef PATCH_SWITCH_AUP_LOAD
namespace patch {
	/* // redo.cppのset_undoの部分にて、別シーンであれば切り替えをしなければまずそう
	int __cdecl aup_load_t::func_project_load_end() {
		int n1 = 0;
		int n2 = 0;

		auto obj_num = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::ObjectAllocNum);
		auto obj = *reinterpret_cast<ExEdit::Object**>(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
		for (int i = 0; i < obj_num; i++) {
			if ((byte)obj[i].flag & 1) {
				if (obj[i].frame_end < 0 || obj[i].frame_end < obj[i].frame_begin) {
					n1++;
					if (0 < n2)break;
				}else if (obj[i].frame_begin < 0) {
					n2++;
					if (0 < n1)break;
				}
			}
		}
		if (0 < n1) {
			int yesno = MessageBoxA(*reinterpret_cast<HWND*>(GLOBAL::exedit_base + OFS::ExEdit::exedit_hwnd),"長さ0以下のオブジェクトが見つかりました\n削除しますか？\n削除後も元に戻す(Ctrl+Z)が可能です", PATCH_VERSION_NAME, MB_YESNO);
			if (yesno == IDYES) {
				reinterpret_cast<void(__cdecl*)()>(GLOBAL::exedit_base + OFS::ExEdit::next_undo)();
				for (int i = 0; i < obj_num; i++) {
					if ((byte)obj[i].flag & 1) {
						if (obj[i].frame_end < 0 || obj[i].frame_end < obj[i].frame_begin) {
							reinterpret_cast<void(__cdecl*)(int)>(GLOBAL::exedit_base + OFS::ExEdit::delete_object)(i);
						}
					}
				}
			}
		}
		if (0 < n2) {
			int yesno = MessageBoxA(*reinterpret_cast<HWND*>(GLOBAL::exedit_base + OFS::ExEdit::exedit_hwnd), "開始フレーム0以下のオブジェクトが見つかりました\n1からになるように変更しますか？\n変更後も元に戻す(Ctrl+Z)が可能です", PATCH_VERSION_NAME, MB_YESNO);
			if (yesno == IDYES) {
				reinterpret_cast<void(__cdecl*)()>(GLOBAL::exedit_base + OFS::ExEdit::next_undo)();
				for (int i = 0; i < obj_num; i++) {
					if ((byte)obj[i].flag & 1) {
						if (obj[i].frame_begin < 0) {
							reinterpret_cast<void(__cdecl*)(int, int)>(GLOBAL::exedit_base + OFS::ExEdit::set_undo)(i, 8);
							obj[i].frame_begin = 0;
						}
					}
				}
			}
		}
		return 1; // 元々mov eax,1を書き換えているため
	}
	*/
} // namespace patch
#endif // ifdef PATCH_SWITCH_AUP_LOAD

