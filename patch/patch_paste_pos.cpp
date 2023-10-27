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

#include "patch_paste_pos.hpp"

#ifdef PATCH_SWITCH_PLAYBACK_POS
namespace patch {
	void __cdecl paste_pos_t::set_pos(int x, int y) {
		*reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::timeline_click_x) = x;
		*reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::timeline_click_y) = y;

		timeline_click_frame = reinterpret_cast<int(__cdecl*)(int)>(GLOBAL::exedit_base + OFS::ExEdit::timeline_x2frame)(x);
		timeline_click_layer = reinterpret_cast<int(__cdecl*)(int)>(GLOBAL::exedit_base + OFS::ExEdit::timeline_y2layer)(y);
	}
	int __cdecl paste_pos_t::get_click_frame(int x) {
		return timeline_click_frame;
	}
	int __cdecl paste_pos_t::get_click_layer(int y) {
		return timeline_click_layer;
	}
} // namespace patch
#endif // ifdef PATCH_SWITCH_PASTE_POS