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

#include "patch_group_id.hpp"

#ifdef PATCH_SWITCH_GROUP_ID
namespace patch {

	void __cdecl group_id_t::exedit_wrap32594(){
        reinterpret_cast<void(__cdecl*)()>(GLOBAL::exedit_base + OFS::ExEdit::InitScrollHorizonal)();
        char* exeditbuf = *(char**)(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);
        if (exeditbuf == NULL)return;

        int gmax = INT_MIN;
        auto obj = *(ExEdit::Object**)(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
        int n = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::ObjectAllocNum);
        for (int i = 0; i < n; i++) {
            int gid = obj[i].group_belong;
            if (gid) {
                exeditbuf[i] = 1;
            } else {
                exeditbuf[i] = 0;
            }
            if (gmax < gid) {
                gmax = gid;
            }
        }
        if (gmax < n && gmax < 8000)return;
        int newid = 1;
        for (int i = 0; i < n; i++) {
            if (exeditbuf[i]) {
                int oldid = obj[i].group_belong;
                for (int j = i; j < n; j++) {
                    if (exeditbuf[j]) {
                        if (obj[j].group_belong == oldid) {
                            exeditbuf[j] = 0;
                            obj[j].group_belong = newid;
                        }
                    }
                }
                newid++;
            }
        }
	}

} // namespace patch
#endif // ifdef PATCH_GROUP_ID