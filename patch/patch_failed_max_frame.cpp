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
#include "patch_failed_max_frame.hpp"
#ifdef PATCH_SWITCH_FAILED_MAX_FRAME


namespace patch {
    void __stdcall failed_max_frame_t::AppendMessageBoxA(AviUtl::EditHandle* editp) {
        char msg[12 + sizeof(str_new_failed_msg)];
        wsprintfA(msg, str_new_failed_msg, editp->frame_n);
        *(int*)&editp->flag &= 0x0ffffffe; // 00423a09 8123feffff0f       and     dword ptr [ebx],0ffffffe
        editp->frame_n = 0;
        MessageBoxA(editp->aviutl_window_info.main_window, msg, reinterpret_cast<char*>(GLOBAL::aviutl_base + OFS::AviUtl::str_AviUtl), MB_OK | MB_ICONWARNING);
    }
} // namespace patch
#endif // ifdef PATCH_SWITCH_FAILED_MAX_FRAME
