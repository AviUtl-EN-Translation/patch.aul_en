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

#include "patch_obj_portionfilter.hpp"
#ifdef PATCH_SWITCH_OBJ_PORTIONFILTER

namespace patch {
    BOOL __stdcall obj_PortionFilter_t::SetWindowTextA_wrap(ExEdit::Filter* efp, HWND hWnd, LPCSTR lpString){
        char* path = (char*)efp->exdata_ptr + 4;
        if (path[0] == '*') {
            return SetWindowTextA(hWnd, path + 1);
        } else {
            return SetWindowTextA(hWnd, lpString);
        }
    }

} // namespace patch
#endif // ifdef PATCH_SWITCH_OBJ_PORTIONFILTER
