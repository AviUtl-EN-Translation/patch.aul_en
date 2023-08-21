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

#include "patch_init_window_pos.hpp"

#ifdef PATCH_SWITCH_INIT_WINDOW_POS
namespace patch {
	BOOL __stdcall init_window_pos_t::GetWindowRect_wrap(HWND hWnd, LPRECT lpRect) {
		SetWindowPos(*reinterpret_cast<HWND*>(GLOBAL::exedit_base + OFS::ExEdit::settingdialog_hwnd), nullptr, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE);
		return GetWindowRect(hWnd, lpRect);
	}
} // namespace patch
#endif // ifdef PATCH_SWITCH_INIT_WINDOW_POS
