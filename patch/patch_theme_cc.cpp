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

#include "patch_theme_cc.hpp"

#ifdef PATCH_SWITCH_THEME_CC
namespace patch {
	

	int __stdcall theme_cc_t::SetScrollInfo_wrap38dee(HWND hwnd, int nBar, SCROLLINFO* lpsi, BOOL redraw) {
		if ((int)lpsi->nPage < horizontal_thumb_min) {
			lpsi->nMax += horizontal_thumb_min - lpsi->nPage;
			lpsi->nPage = horizontal_thumb_min;
		}
		return SetScrollInfo(hwnd, nBar, lpsi, redraw);
	}

	int __stdcall theme_cc_t::SetScrollInfo_wrap38c65(HWND hwnd, int nBar, SCROLLINFO* lpsi, BOOL redraw) {
		if ((int)lpsi->nPage < vertical_thumb_min) {
			lpsi->nMax += vertical_thumb_min - lpsi->nPage;
			lpsi->nPage = vertical_thumb_min;
		}
		return SetScrollInfo(hwnd, nBar, lpsi, redraw);
	}
	

} // namespace patch
#endif // ifdef PATCH_THEME_CC