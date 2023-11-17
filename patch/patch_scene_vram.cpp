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

#include "patch_scene_vram.hpp"

#ifdef PATCH_SWITCH_SCENE_VRAM


namespace patch {

	void* __cdecl scene_vram_t::GetOrCreateSceneBufYC_fix(ExEdit::ObjectFilterIndex ofi, int w, int h, int v_func_id, int flag) {
		if ((int)ofi == 0) return nullptr;
		int add_h = 2;
		if (patch_level < 30) {
			// add_h = 2;
		} else if (patch_level < 40) {
			add_h = 6;
		} else {
			add_h = 8;
		}
		int old_cache_exists = 0;
		int yc_vram_w = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::yc_vram_w);
		int yc_vram_h = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::yc_vram_h);
		int* cache = (int*)reinterpret_cast<void*(__cdecl*)(ExEdit::ObjectFilterIndex, int , int , int, int, int*)>(GLOBAL::exedit_base + OFS::ExEdit::GetOrCreateCache)(ofi, yc_vram_w, yc_vram_h + add_h, 8 * sizeof(ExEdit::PixelYC), v_func_id, &old_cache_exists);
		if (cache == nullptr) return nullptr;
		// CreateCacheでは16byte多く確保されるので128bitアライメントのために+8できる(結果として256bitアライメント)
		// void* ycp = (void*)((int)cache + (add_h / 2 * yc_vram_w + 4) * sizeof(ExEdit::PixelYC) + 8);
		void* ycp = (void*)((int)cache + add_h * sizeof(ExEdit::PixelYC) / 2 * yc_vram_w + 4 * sizeof(ExEdit::PixelYC) + 8);
		if (flag == 0) {
			if (old_cache_exists == 0) {
				cache[0] = 0;
				return nullptr;
			}
			if (cache[0] != (int)ofi) return nullptr;
			if (w != 0 && cache[1] != w) return nullptr;
			if (h != 0 && *(int*)((int)cache + yc_vram_w * (yc_vram_h + add_h) * sizeof(ExEdit::PixelYC) + 16 - 4) != h) return nullptr;
		} else {
			auto e_exfunc = reinterpret_cast<ExEdit::Exfunc*>(GLOBAL::exedit_base + OFS::ExEdit::exfunc);
			e_exfunc->fill(ycp, 0, 0, w, h, 0, 0, 0, 0, 0);
			cache[0] = (int)ofi;
			cache[1] = w;
			*(int*)((int)cache + yc_vram_w * (yc_vram_h + add_h) * sizeof(ExEdit::PixelYC) + 16 - 4) = h;
		}
		return ycp;
	}

} // namespace patch
#endif // ifdef PATCH_SWITCH_SCENE_VRAM