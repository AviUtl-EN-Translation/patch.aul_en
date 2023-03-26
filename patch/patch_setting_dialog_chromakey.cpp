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

#include "patch_setting_dialog_chromakey.hpp"
#ifdef PATCH_SWITCH_SETTINGDIALOG_CHROMAKEY



namespace patch {
	BOOL __cdecl dialog_chromakey_t::dialog_WndProc_wrap(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void* editp, ExEdit::Filter* efp) {
		if (message == 0x702) {
			if (wparam == 0x1e1a) {
				HWND ctrl_hwnd = efp->exfunc->get_hwnd(efp->processing, 3, 1);
				if (ctrl_hwnd) {
					ShowWindow(ctrl_hwnd, efp->check[0]);
				}
				return TRUE;
			}
		}
		return FALSE;
	}
	int __cdecl dialog_chromakey_t::dialog_init_wrap(HINSTANCE hinstance, HWND hwnd, int y, int base_id, int sw_param, ExEdit::Filter* efp) {
		HWND ctrl_hwnd = efp->exfunc->get_hwnd(efp->processing, 3, 0);
		if (ctrl_hwnd) {
			SetWindowPos(ctrl_hwnd, NULL, 0, 0, 112, 16, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
		ctrl_hwnd = efp->exfunc->get_hwnd(efp->processing, 3, 1);
		if (ctrl_hwnd) {
			SetWindowPos(ctrl_hwnd, NULL, 0x80, y - 0x3d, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
			if (efp->check[0] == 0) {
				ShowWindow(ctrl_hwnd, 0);
			}
		}
		ctrl_hwnd = efp->exfunc->get_hwnd(efp->processing, 4, 2);
		if (ctrl_hwnd) {
			SetWindowPos(ctrl_hwnd, NULL, 11, y - 0x29, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
		ctrl_hwnd = efp->exfunc->get_hwnd(efp->processing, 5, 2);
		if (ctrl_hwnd) {
			SetWindowPos(ctrl_hwnd, NULL, 174, y - 0x29, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
		return -21;
	}

} // namespace patch
#endif // ifdef PATCH_SWITCH_SETTINGDIALOG_CHROMAKEY
