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

#include "patch_avi_file_handle_share.hpp"

#ifdef PATCH_SWITCH_AVI_FILE_HANDLE_SHARE
namespace patch {
	void __cdecl avi_file_handle_share_t::avi_handle_open_begin(ExdataAviFileHandleInfo* exdata, int flag, AviUtl::FileInfo** fipp) {
		if (GetKeyState(VK_MENU) & 0x8000) return;

		if (exdata->path[0] == '\0') return;
		
		auto afhi_array = reinterpret_cast<AviFileHandleInfo*>(GLOBAL::exedit_base + OFS::ExEdit::AviFileHandleInfoArray);
		int avi_handle_max = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::AviFileHandleMaxNum);
		for (int i = 0; i < avi_handle_max; i++) {
			auto afhi = &afhi_array[i];
			if (afhi->afh != NULL) {
				if (afhi->flag == flag && lstrcmpA(afhi->path, exdata->path) == 0) {
					exdata->id_and_count = afhi->id_and_count;
					exdata->tickcount = afhi->tickcount;
					return;
				}
			}
		}
	}
} // namespace patch
#endif // ifdef PATCH_SWITCH_AVI_FILE_HANDLE_SHARE

