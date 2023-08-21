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

#include "patch_obj_text.hpp"

#ifdef PATCH_SWITCH_OBJ_TEXT
namespace patch {

	void __cdecl obj_Text_t::exedit_exfunc_x40_ret_wrap(void* dst, int ofs_x, int ofs_y,
		wchar_t* text, ExEdit::PixelBGR col1, ExEdit::PixelBGR col2, int opacity, HFONT hfont,
		int* ret_w, int* ret_h, int type, int speed, int align, int* space_x_ptr, int flag) {
		if (ret_w && ret_h) {
			if (*ret_w == 0) {
				*ret_h = 0;
			} else if (*ret_h == 0) {
				*ret_w = 0;
			}
		}
	}
	void __cdecl obj_Text_t::yc_buffer_fill_wrap(void* ptr, int ox, int oy, int w, int h, short y, short cb, short cr, short a, int flag) {
		auto exfunc = reinterpret_cast<ExEdit::Exfunc*>(GLOBAL::exedit_base + OFS::ExEdit::exfunc);
		w = min(w + 128, *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::exedit_max_w));
		h = min(h + 128, *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::exedit_max_h));
		exfunc->fill(ptr, ox, oy, w, h, y, cb, cr, a, flag);
		//auto t0 = GetTickCount();
			//reinterpret_cast<void(__cdecl*)(void*, int, int, int, int, short, short, short, short, int)>(GLOBAL::exedit_base + 0x81f90)(ptr, ox, oy, w, h, y, cb, cr, a, flag);
			/*int max_w = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::exedit_max_w);
			int max_h = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::exedit_max_h);
			memset(ptr, 0, max_w * max_h * 8);*/
		//printf("%d\n", GetTickCount() - t0);
	}

} // namespace patch
#endif // ifdef PATCH_SWITCH_OBJ_TEXT
