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
#include "patch_failed_sjis_msgbox.hpp"
#ifdef PATCH_SWITCH_FAILED_SJIS_MSGBOX

#include <shlwapi.h>

namespace patch {
    int __stdcall failed_sjis_msgbox_t::MessageBoxA_1(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
        auto end = lpCaption + strlen(lpCaption);
        // sjisに0x3fは含まれないのでこれでよい
        if (std::find(lpCaption, end, '?') != end) {
            lpText = str_new_failed_msg_question;
        } else if (!PathFileExistsA(lpCaption)) {
            lpText = str_new_failed_msg_not_found;
            /*
            char path[261];
            lstrcpyA(path, lpCaption);
            for (int i = lstrlenA(path); 2 < i; i--) {
                path[i] = '\0';
                if (PathFileExistsA(path)) {
                    printf("%s\n", path);
                }
            }*/
        }
        return MessageBoxA(hWnd, lpText, lpCaption, uType);
    }

    int __stdcall failed_sjis_msgbox_t::MessageBoxA_2(LPCSTR path, HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) {
        int ret = MessageBoxA_1(hWnd, lpText, path, uType);
        *(char*)path = '\0';
        return ret;
    }

    int __stdcall failed_sjis_msgbox_t::MessageBoxA_import_exo(HWND hWnd, LPCSTR lpText, LPCSTR path) {
        return MessageBoxA_1(hWnd, lpText, path, MB_TOPMOST | MB_ICONWARNING | MB_TASKMODAL);
    }
    int __cdecl failed_sjis_msgbox_t::MessageBoxA_new_project_exo(LPCSTR path, void* param) {
        MessageBoxA_1(NULL, (LPCSTR)(GLOBAL::exedit_base + 0xa46bc), path, MB_TOPMOST | MB_ICONWARNING | MB_TASKMODAL);
        return 0;
    }

    int __cdecl failed_sjis_msgbox_t::MessageBoxA_exa(LPCSTR path) {
        MessageBoxA_1(NULL, (LPCSTR)(GLOBAL::exedit_base + 0xa46bc), path, MB_TOPMOST | MB_ICONWARNING | MB_TASKMODAL);
        return 0;
    }
} // namespace patch
#endif // ifdef PATCH_SWITCH_FAILED_SJIS_MSGBOX
