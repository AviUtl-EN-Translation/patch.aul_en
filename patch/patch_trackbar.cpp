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

#include "patch_trackbar.hpp"

#ifdef PATCH_SWITCH_TRACKBAR
namespace patch {
    LRESULT __stdcall trackbar_t::SendMessageA_TBM_SETPOS_wrap(HWND hwnd, WPARAM wparam, LPARAM lparam) {
        auto ret = SendMessageA(hwnd, TBM_SETPOS, wparam, lparam);
        if (lparam == -1) {
            SendMessageA(GetParent(hwnd), WM_HSCROLL, MAKEWPARAM(SB_THUMBTRACK, -1), (LPARAM)hwnd);
        }
        return ret;
    }

    /*
    LRESULT __cdecl trackbar_t::trackbar_WndProc_wrap(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
        if (message == WM_MOUSEWHEEL)return 0;
        if (*ini_trackbarclick_ptr != 0 && message == WM_LBUTTONDOWN) {
            int x = LOWORD(lparam);
            int y = HIWORD(lparam);

            RECT rect;
            SendMessageA(hwnd, TBM_GETCHANNELRECT, 0, (LPARAM)&rect);
            int left = rect.left;
            int right = rect.right;
            if (x < left || right < x) {
                return 0;
            }

            SendMessageA(hwnd, TBM_GETTHUMBRECT, 0, (LPARAM)&rect);
            if (y < rect.top || rect.bottom < y) {
                return 0;
            }

            if (x <= rect.left || rect.right <= x) {
                int rlsub = (rect.right - rect.left + 1) / 2;
                left += rlsub;
                right -= rlsub;
                if (right < left) {
                    left = right = (right + left + 1) / 2;
                }
                int track_min = SendMessageA(hwnd, TBM_GETRANGEMIN, 0, 0);
                int track_max = SendMessageA(hwnd, TBM_GETRANGEMAX, 0, 0);
                int track_width = track_max - track_min;
                int track_value = (track_width / 4 + (x - left) * (track_width + 1)) / (right - left + 1) + track_min;
                track_value = std::clamp(track_value, track_min, track_max);

                SendMessageA(hwnd, TBM_SETPOS, 1, track_value);
                if (track_value == -1) {
                    SendMessageA(GetParent(hwnd), WM_HSCROLL, MAKEWPARAM(SB_THUMBTRACK, -1), (LPARAM)hwnd);
                }

                SendMessageA(hwnd, TBM_GETTHUMBRECT, 0, (LPARAM)&rect);
                lparam = MAKELPARAM((rect.right + rect.left + 1) >> 1, y);
            }
        }
        return CallWindowProcA(*reinterpret_cast<WNDPROC*>(GLOBAL::aviutl_base + OFS::AviUtl::trackbar_WndProc), hwnd, message, wparam, lparam);
    }
    */
} // namespace patch
#endif // ifdef PATCH_SWITCH_TRACKBAR