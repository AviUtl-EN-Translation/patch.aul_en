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

#include "patch_set_frame.hpp"
#ifdef PATCH_SWITCH_SET_FRAME

namespace patch {
    int __cdecl set_frame_t::exfunc_set_frame_n_end(AviUtl::EditHandle* editp, int n) {
        if (n <= editp->select_frame_end) {
            int s;
            if (n <= editp->select_frame_start) {
                s = 0;
            } else {
                s = editp->select_frame_start;
            }
            auto exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
            exfunc->set_select_frame(editp, s, n - 1);
        }
        return n;
    }

    BOOL __cdecl set_frame_t::exfunc_set_select_frame_wrap(AviUtl::EditHandle* editp, int s, int e) {
        if (editp->frame_n <= e) {
            e = editp->frame_n - 1;
        }
        if (editp->frame_n <= s) {
            s = 0;
        }
        return reinterpret_cast<BOOL(__cdecl*)(AviUtl::EditHandle*, int, int)>(GLOBAL::aviutl_base + OFS::AviUtl::exfunc_set_select_frame)(editp, s, e);
    }
} // namespace patch
#endif // ifdef PATCH_SWITCH_SET_FRAME
