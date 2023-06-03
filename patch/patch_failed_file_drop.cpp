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

#include "patch_failed_file_drop.hpp"

#ifdef PATCH_SWITCH_FAILED_FILE_DROP
namespace patch {

	char __stdcall failed_file_drop_t::init_flag() {
		flag = 0;
		return *(char*)(GLOBAL::exedit_base + OFS::ExEdit::ini_extension_buf);
	}

	int __stdcall failed_file_drop_t::lstrcmpiA_wrap3c235(LPCSTR lpString1, LPCSTR lpString2) {
		int ret = lstrcmpiA(lpString1, lpString2);
		if (ret == 0) {
			flag = 1;
		}
		return ret;
	}
	void __stdcall failed_file_drop_t::MessageBoxA_drop(char* path) {
		if (flag) return;

		LPCSTR lpCaption = reinterpret_cast<char*(__cdecl*)(char*)>(GLOBAL::exedit_base + OFS::ExEdit::extract_extension)(path);
		LPCSTR lpText;
		if (!lstrcmpA(lpCaption, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_dot_aup)) || !lstrcmpA(lpCaption, (char*)(GLOBAL::exedit_base + OFS::ExEdit::str_dot_exedit_backup))) {
			lpText = (LPCSTR)str_failed_pfdrop_msg;
		} else {
			lpText = (LPCSTR)str_failed_drop_msg;
		}
		HWND hWnd = *(HWND*)(GLOBAL::exedit_base + OFS::ExEdit::exedit_hwnd);
		MessageBoxA(hWnd, lpText, lpCaption, MB_TOPMOST | MB_ICONWARNING | MB_TASKMODAL);
	}
} // namespace patch
#endif // ifdef PATCH_SWITCH_FAILED_FILE_DROP
