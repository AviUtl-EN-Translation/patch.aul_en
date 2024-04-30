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

#include "patch_exfilter_gradation.hpp"
#ifdef PATCH_SWITCH_EXFILTER_GRADATION

namespace patch::exfilter {

	int __fastcall line_c(Gradation_t::efGradation_var* grd, int x, int y) {
		return (grd->w << 15) + (x - grd->cx) * grd->sin + (y - grd->cy) * grd->cos;
	}
	int __fastcall circle_c(Gradation_t::efGradation_var* grd, int x, int y) {
		return (grd->w << 16) - (int)(sqrt((double)((x - grd->cx) * (x - grd->cx) + (y - grd->cy) * (y - grd->cy))) * 65536.0);
	}
	int __fastcall rect_c(Gradation_t::efGradation_var* grd, int x, int y) {
		return (grd->w << 16) - abs((x - grd->cx) * grd->sin + (y - grd->cy) * grd->cos) - abs((grd->cx - x) * grd->cos + (y - grd->cy) * grd->sin);
	}
	int __fastcall convex_c(Gradation_t::efGradation_var* grd, int x, int y) {
		return (grd->w << 16) - abs((x - grd->cx) * grd->sin + (y - grd->cy) * grd->cos);
	}
	int __fastcall circleloop_c(Gradation_t::efGradation_var* grd, int x, int y) {
		return abs((grd->w << 16) - ((int)(sqrt((double)((x - grd->cx) * (x - grd->cx) + (y - grd->cy) * (y - grd->cy))) * 65536.0) + grd->sin) % (grd->w << 17));
	}
	int __fastcall rectloop_c(Gradation_t::efGradation_var* grd, int x, int y) {
		return abs((grd->w << 16) - (abs((x - grd->cx) * grd->cos - (y - grd->cy) * grd->sin) + abs((x - grd->cx) * grd->sin + (y - grd->cy) * grd->cos)) % (grd->w << 17));
	}
	int __fastcall convexloop_c(Gradation_t::efGradation_var* grd, int x, int y) {
		return abs((grd->w << 16) - abs(((x - grd->cx) * grd->sin + (y - grd->cy) * grd->cos) % (grd->w << 17)));
	}
	int(__fastcall* calc_func)(Gradation_t::efGradation_var* grd, int x, int y);

	void __cdecl mt(int y, int thread_num, Gradation_t::efGradation_var* grd, ExEdit::FilterProcInfo* efpip) {
		auto lut_minus_cos_half = reinterpret_cast<int16_t*>(GLOBAL::exedit_base + OFS::ExEdit::lut_minus_cos_half);
		auto dst = (ExEdit::PixelYC*)efpip->frame_edit + efpip->scene_line * y;
		int th_line = (efpip->scene_line * thread_num - efpip->scene_w) * sizeof(*dst);
		for (; y < efpip->scene_h; y += thread_num) {
			for (int x = 0; x < efpip->scene_w; x++) {
				int ofs = calc_func(grd, x, y);
				if (ofs <= 0) {
					grd->blendfunc(dst, grd->y_e, grd->cb_e, grd->cr_e, grd->a_e);
				} else if (ofs < (grd->w << 16)) {
					int rate = lut_minus_cos_half[ofs / (grd->w << 4)];
					grd->blendfunc(dst,
						(((int)grd->y_s - (int)grd->y_e) * rate >> 12) + (int)grd->y_e,
						(((int)grd->cb_s - (int)grd->cb_e) * rate >> 12) + (int)grd->cb_e,
						(((int)grd->cr_s - (int)grd->cr_e) * rate >> 12) + (int)grd->cr_e,
						(((int)grd->a_s - (int)grd->a_e) * rate >> 12) + (int)grd->a_e);
				} else {
					grd->blendfunc(dst, grd->y_s, grd->cb_s, grd->cr_s, grd->a_s);
				}
				dst++;
			}
			*(DWORD*)&dst += th_line;
		}
	}

	BOOL __cdecl Gradation_t::func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
		if (efp->track[0] <= 0) return TRUE;
		auto grd = reinterpret_cast<efGradation_var*>(GLOBAL::exedit_base + OFS::ExEdit::efGradation_var_ptr);
		grd->a_s = grd->a_e = grd->intensity = (efp->track[0] << 9) / 125;

		auto exdata = (ExEdit::Exdata::efGradation*)efp->exdata_ptr;
		reinterpret_cast<void(__cdecl*)(short*, short*, short*, int)>(GLOBAL::exedit_base + OFS::ExEdit::rgb2yc)(&grd->y_s, &grd->cb_s, &grd->cr_s, *(int*)&exdata->color & 0xffffff);
		reinterpret_cast<void(__cdecl*)(short*, short*, short*, int)>(GLOBAL::exedit_base + OFS::ExEdit::rgb2yc)(&grd->y_e, &grd->cb_e, &grd->cr_e, *(int*)&exdata->color2 & 0xffffff);
		if (*(int*)&exdata->color & 0x1000000) {
			grd->y_s = grd->y_e;
			grd->cb_s = grd->cb_e;
			grd->cr_s = grd->cr_e;
			grd->a_s = 0;
		}
		if (*(int*)&exdata->color2 & 0x1000000) {
			grd->y_e = grd->y_s;
			grd->cr_e = grd->cr_s;
			grd->cb_e = grd->cb_s;
			grd->a_e = 0;
		}
		if (grd->a_s <= 0 && grd->a_e <= 0) return TRUE;

		unsigned int blendmode = exdata->blend;
		if (31 < blendmode) {
			blendmode = 0;
		}
		grd->blendfunc = reinterpret_cast<decltype(grd->blendfunc)*>(GLOBAL::exedit_base + OFS::ExEdit::yc_blend_func_list)[blendmode];
		grd->cx = (efpip->scene_w >> 1) + efp->track[1];
		grd->cy = (efpip->scene_h >> 1) + efp->track[2];
		grd->w = efp->track[4];
		auto rad = (double)efp->track[3] * 0.00174532925199433; // PI / 1800
		grd->sin = (int)(sin(rad) * 65536.0);
		grd->cos = (int)(cos(rad) * -65536.0);
		switch (exdata->type) {
		case 1:
			(calc_func) = (circle_c);
			break;
		case 2:
			(calc_func) = (rect_c);
			break;
		case 3:
			(calc_func) = (convex_c);
			break;
		case 4:
			if (grd->w <= 0) grd->w = 1;
			grd->sin = grd->w * (-((efp->track[3] << 13) / 225) & 0x1ffff);
			(calc_func) = (circleloop_c);
			break;
		case 5:
			if (grd->w <= 0) grd->w = 1;
			(calc_func) = (rectloop_c);
			break;
		case 7:
			grd->cx = efp->track[1];
			grd->cy = efp->track[2];
			[[fallthrough]];
		case 6:
			if (grd->w <= 0) grd->w = 1;
			(calc_func) = (convexloop_c);
			break;
		default:
			grd->w++;
			(calc_func) = (line_c);
		}
		efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&mt, grd, efpip);
		reinterpret_cast<BOOL(__cdecl*)(ExEdit::Filter*, ExEdit::FilterProcInfo*, int)>(GLOBAL::exedit_base + OFS::ExEdit::drawfilter_func_proc)(efp, efpip, 0x80d);
		return TRUE;
	}
	BOOL __cdecl Gradation_t::func_WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, AviUtl::EditHandle* editp, ExEdit::Filter* efp) {
		if (reinterpret_cast<BOOL(__cdecl*)(HWND, UINT, WPARAM, LPARAM, AviUtl::EditHandle*, ExEdit::Filter*)>(GLOBAL::exedit_base + 0x59b30)(hwnd, message, wparam, lparam, editp, efp)) { // gradation_wndproc
			return TRUE;
		}
		return reinterpret_cast<BOOL(__cdecl*)(HWND, UINT, WPARAM, LPARAM, AviUtl::EditHandle*, ExEdit::Filter*)>(GLOBAL::exedit_base + OFS::ExEdit::drawfilter_func_WndProc)(hwnd, message, wparam, lparam, editp, efp);
	}


	void __cdecl mt_yca(int y, int thread_num, Gradation_t::efGradation_var* grd, ExEdit::FilterProcInfo* efpip) {
		auto lut_minus_cos_half = reinterpret_cast<int16_t*>(GLOBAL::exedit_base + OFS::ExEdit::lut_minus_cos_half);
		auto dst = (ExEdit::PixelYCA*)efpip->obj_edit + efpip->obj_line * y;
		int th_line = (efpip->obj_line * thread_num - efpip->obj_w) * sizeof(*dst);
		for (; y < efpip->obj_h; y += thread_num) {
			for (int x = 0; x < efpip->obj_w; x++) {
				int ofs = calc_func(grd, x, y);
				if (ofs <= 0) {
					grd->blendfunc((ExEdit::PixelYC*)dst, grd->y_e, grd->cb_e, grd->cr_e, grd->a_e);
				} else if (ofs < (grd->w << 16)) {
					int rate = lut_minus_cos_half[ofs / (grd->w << 4)];
					grd->blendfunc((ExEdit::PixelYC*)dst,
						(((int)grd->y_s - (int)grd->y_e) * rate >> 12) + (int)grd->y_e,
						(((int)grd->cb_s - (int)grd->cb_e) * rate >> 12) + (int)grd->cb_e,
						(((int)grd->cr_s - (int)grd->cr_e) * rate >> 12) + (int)grd->cr_e,
						(((int)grd->a_s - (int)grd->a_e) * rate >> 12) + (int)grd->a_e);
				} else {
					grd->blendfunc((ExEdit::PixelYC*)dst, grd->y_s, grd->cb_s, grd->cr_s, grd->a_s);
				}
				dst++;
			}
			*(DWORD*)&dst += th_line;
		}
	}
	BOOL __stdcall Gradation_t::apend_form(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
		auto grd = reinterpret_cast<efGradation_var*>(GLOBAL::exedit_base + OFS::ExEdit::efGradation_var_ptr);
		auto exdata = (ExEdit::Exdata::efGradation*)efp->exdata_ptr;
		switch (exdata->type) {
		case 4:
			if (grd->w <= 0) grd->w = 1;
			grd->sin = grd->w * (-((efp->track[3] << 13) / 225) & 0x1ffff);
			(calc_func) = (circleloop_c);
			break;
		case 5:
			if (grd->w <= 0) grd->w = 1;
			(calc_func) = (rectloop_c);
			break;
		case 7:
			grd->cx = efp->track[1];
			grd->cy = efp->track[2];
			[[fallthrough]];
		case 6:
			if (grd->w <= 0) grd->w = 1;
			(calc_func) = (convexloop_c);
			break;
		default:
			return FALSE; // line
		}
		efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&mt_yca, grd, efpip);
		return TRUE;
	}
} // namespace patch::exfilter
#endif // ifdef PATCH_SWITCH_EXFILTER_GRADATION
