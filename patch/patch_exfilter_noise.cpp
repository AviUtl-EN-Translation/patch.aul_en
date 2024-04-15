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

#include "patch_exfilter_noise.hpp"
#ifdef PATCH_SWITCH_EXFILTER_NOISE

namespace patch::exfilter {
	BOOL Noise_t::func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
		reinterpret_cast<BOOL(__cdecl*)(ExEdit::Filter*, ExEdit::FilterProcInfo*)>(GLOBAL::exedit_base + OFS::ExEdit::efNoise_func_proc)(efp, efpip);

		auto noise = reinterpret_cast<efNoise_var*>(GLOBAL::exedit_base + OFS::ExEdit::efNoise_var_ptr);
		noise->speed_x -= efpip->scene_w * noise->cycle_x / 2;
		noise->speed_y -= efpip->scene_h * noise->cycle_y / 2;
		efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&mt, efp, efpip);
		return TRUE;
	}

	void __cdecl Noise_t::mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
		auto noise = reinterpret_cast<efNoise_var*>(GLOBAL::exedit_base + OFS::ExEdit::efNoise_var_ptr);
		int y = thread_id * efpip->scene_h / thread_num;
		auto dst = (short*)((ExEdit::PixelYC*)efpip->frame_edit + y * efpip->scene_line);
		int line = (efpip->scene_line - efpip->scene_w) * sizeof(ExEdit::PixelYC);
		int intensity = noise->intensity;
		int ny = noise->cycle_y * y + noise->speed_y;
		y = (thread_id + 1) * efpip->scene_h / thread_num - y;
		if (noise->intensity < 0x1000) {
			for (; 0 < y; y--) {
				int nx = noise->speed_x;
				for (int x = efpip->scene_w; 0 < x; x--) {
					int v = noise->func(nx, ny, noise->speed_t);
					*dst += (*dst * (v - 0x1000) >> 12) * intensity >> 12;
					dst = (short*)((int)dst + sizeof(ExEdit::PixelYC));
					nx += noise->cycle_x;
				}
				dst = (short*)((int)dst + line);
				ny += noise->cycle_y;
			}
		} else {
			intensity = 0x2000 - intensity;
			for (; 0 < y; y--) {
				int nx = noise->speed_x;
				for (int x = efpip->scene_w; 0 < x; x--) {
					int v = noise->func(nx, ny, noise->speed_t);
					*dst = (*dst * v >> 12) * intensity >> 12;
					dst = (short*)((int)dst + sizeof(ExEdit::PixelYC));
					nx += noise->cycle_x;
				}
				dst = (short*)((int)dst + line);
				ny += noise->cycle_y;
			}
		}
	}

} // namespace patch::exfilter
#endif // ifdef PATCH_SWITCH_EXFILTER_NOISE
