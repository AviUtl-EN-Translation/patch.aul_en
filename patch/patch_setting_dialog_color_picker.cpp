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

#include "patch_setting_dialog_color_picker.hpp"

#include "patch_any_obj.hpp"

#ifdef PATCH_SWITCH_SETTINGDIALOG_COLOR_PICKER
namespace patch {

	void __cdecl yc2rgb(byte* r, byte* g, byte* b, short y, short cb, short cr) {
		ExEdit::PixelBGRA bgra[2] = {};
		ExEdit::PixelYCA yca[2] = { y, cb, cr, 0x1000 };
		reinterpret_cast<void(__cdecl*)(void*, void*, int, int, int)>(GLOBAL::exedit_base + 0x6fb10)(bgra, yca, 2, 1, 0);
		*r = bgra[0].r;
		*g = bgra[0].g;
		*b = bgra[0].b;
		return;
	}

	void __cdecl dialog_color_picker_t::color_dialog_chromakey(ExEdit::Filter* efp) {
		auto exdata = reinterpret_cast<ExEdit::Exdata::efChromakey*>(efp->exdata_ptr);
		
		byte color[3];
		yc2rgb(color, color + 1, color + 2, exdata->color.y, exdata->color.cb, exdata->color.cr);
		if (efp->exfunc->x6c(efp, &color, 0)) { // color_dialog
			reinterpret_cast<void(__cdecl*)(short*, short*, short*, int)>(GLOBAL::exedit_base + OFS::ExEdit::rgb2yc)(&exdata->color.y, &exdata->color.cb, &exdata->color.cr, *(int*)color & 0xffffff);
			any_obj.update_any_exdata_use_idx(efp, 0);
			exdata->status = 1;
		} else {
			exdata->status = 0;
		}
		/* any_obj側で実装済み
		if (any_obj.is_enabled_i()) {
			any_obj.update_any_exdata_use_idx(efp, 2);
		}
		*/
	}

	void color_dialog_specialcolorconv(ExEdit::Filter* efp, int n) {
		auto exdata = reinterpret_cast<ExEdit::Exdata::efSpecialColorConversion*>((int)efp->exdata_ptr + n * 8);
		byte color[3];
		yc2rgb(color, color + 1, color + 2, exdata->color_yc.y, exdata->color_yc.cb, exdata->color_yc.cr);
		if (efp->exfunc->x6c(efp, &color, 0)) { // color_dialog
			reinterpret_cast<void(__cdecl*)(short*, short*, short*, int)>(GLOBAL::exedit_base + OFS::ExEdit::rgb2yc)(&exdata->color_yc.y, &exdata->color_yc.cb, &exdata->color_yc.cr, *(int*)color & 0xffffff);
			any_obj.update_any_exdata_use_idx(efp, 0 + n * 2);
			exdata->color_yc.status = 1;
		} else {
			exdata->color_yc.status = 0;
		}
		any_obj.update_any_exdata_use_idx(efp, 1 + n * 2);
	}
	void __stdcall dialog_color_picker_t::color_dialog_specialcolorconv1(ExEdit::Filter* efp) {
		color_dialog_specialcolorconv(efp, 0);
	}
	void __stdcall dialog_color_picker_t::color_dialog_specialcolorconv2(ExEdit::Filter* efp) {
		color_dialog_specialcolorconv(efp, 1);
	}

} // namespace patch
#endif // ifdef PATCH_SWITCH_SETTINGDIALOG_COLOR_PICKER

