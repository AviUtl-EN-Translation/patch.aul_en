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

#include "patch_load_exedit_backup.hpp"
#ifdef PATCH_SWITCH_LOAD_EXEDIT_BACKUP


namespace patch {
    int __stdcall load_exedit_backup_t::MessageBoxA_wrap(LPCSTR lpCaption, int version) {
        char buf[260];
        wsprintfA(buf, "%s\nVer%d.%d-%d\n正常に読みこめない可能性がありますが続行しますか", (char*)(GLOBAL::exedit_base + 0x9d91c), version / 10000, version / 100 % 100, version % 100);
        return MessageBoxA(NULL, buf, lpCaption, MB_TOPMOST | MB_TASKMODAL | MB_ICONWARNING | MB_YESNO);
    }
} // namespace patch
#endif // ifdef PATCH_SWITCH_LOAD_EXEDIT_BACKUP
