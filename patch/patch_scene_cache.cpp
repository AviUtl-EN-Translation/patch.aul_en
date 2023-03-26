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

#include "patch_scene_cache.hpp"

#ifdef PATCH_SWITCH_SCENE_CACHE
namespace patch {

	struct SceneCacheInfo {
		int w;
		int h;
		int _padding1;
		int _padding2;
		int buf[1];
	};

	void* __cdecl scene_cache_t::get_scene_image_wrap(ExEdit::ObjectFilterIndex ofi, ExEdit::FilterProcInfo* efpip, int scene_idx, int frame, int subframe, int* w, int* h) {
		int* SceneDisplaying = (int*)(GLOBAL::exedit_base + OFS::ExEdit::SceneDisplaying);
		if (*SceneDisplaying != 0) {
			return get_scene_image(ofi, efpip, scene_idx, frame, subframe, w, h);
		}
		
		auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);


		int key = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::is_saving);
		if (key) {
			key <<= 6;
		} else {
			key = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::fast_process) << 7;
		}
		key += (int)&get_scene_image + scene_idx;

		auto sceneci = (SceneCacheInfo*)a_exfunc->get_shared_mem(key, (frame << 7) | subframe, NULL);
		if (sceneci != NULL) {
			*w = sceneci->w;
			*h = sceneci->h;
			return sceneci->buf;
		}
		int t0 = GetTickCount();
		void* img_ptr = get_scene_image(ofi, efpip, scene_idx, frame, subframe, w, h);
		if (img_ptr == NULL) {
			return NULL;
		}
		if (time_shreshold < GetTickCount() - t0) {
			int yc_size;
			if (reinterpret_cast<BOOL(__cdecl*)(int)>(GLOBAL::exedit_base + OFS::ExEdit::scene_has_alpha)(scene_idx)) {
				yc_size = 8;
			} else {
				yc_size = 6;
			}
			sceneci = (SceneCacheInfo*)a_exfunc->create_shared_mem(key, (frame << 7) | subframe, efpip->obj_line * *h * yc_size + 16, NULL);
			if (sceneci == NULL) {
				return img_ptr;
			}
			sceneci->w = *w;
			sceneci->h = *h;
			memcpy(sceneci->buf, img_ptr, efpip->obj_line * *h * yc_size);
		}
		return img_ptr;
	}
	void* __cdecl scene_cache_t::get_scene_image_mask_wrap(ExEdit::ObjectFilterIndex ofi, ExEdit::FilterProcInfo* efpip, int scene_idx, int frame, int subframe, int* w, int* h) {
		int* SceneDisplaying = (int*)(GLOBAL::exedit_base + OFS::ExEdit::SceneDisplaying);
		if (*SceneDisplaying != 0) {
			return get_scene_image(ofi, efpip, scene_idx, frame, subframe, w, h);
		}

		auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);


		int key = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::is_saving);
		if (key) {
			key <<= 6;
		} else {
			key = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::fast_process) << 7;
		}
		key += (int)&get_scene_image + scene_idx;

		auto sceneci = (SceneCacheInfo*)a_exfunc->get_shared_mem(key, (frame << 7) | subframe, NULL);
		if (sceneci != NULL) {
			if (reinterpret_cast<BOOL(__cdecl*)(int)>(GLOBAL::exedit_base + OFS::ExEdit::scene_has_alpha)(scene_idx)) {
				void* ptr = reinterpret_cast<void* (__cdecl*)(ExEdit::ObjectFilterIndex, int, int, int, int)>(GLOBAL::exedit_base + OFS::ExEdit::GetOrCreateSceneBufYCA)(ofi, sceneci->w, sceneci->h, efpip->v_func_idx + 1, 1);
				if (ptr != NULL) {
					memcpy(ptr, sceneci->buf, efpip->obj_line * sceneci->h * 8);
					return sceneci->buf;
				}
			} else {
				void* ptr = reinterpret_cast<void* (__cdecl*)(ExEdit::ObjectFilterIndex, int, int, int, int)>(GLOBAL::exedit_base + OFS::ExEdit::GetOrCreateSceneBufYC)(ofi, sceneci->w, sceneci->h, efpip->v_func_idx + 1, 1);
				if (ptr != NULL) {
					memcpy(ptr, sceneci->buf, efpip->obj_line * sceneci->h * 6);
					return sceneci->buf;
				}
			}
		}
		int t0 = GetTickCount();
		void* img_ptr = get_scene_image(ofi, efpip, scene_idx, frame, subframe, w, h);
		if (img_ptr == NULL) {
			return NULL;
		}
		if (time_shreshold < GetTickCount() - t0) {
			int yc_size;
			if (reinterpret_cast<BOOL(__cdecl*)(int)>(GLOBAL::exedit_base + OFS::ExEdit::scene_has_alpha)(scene_idx)) {
				yc_size = 8;
			} else {
				yc_size = 6;
			}
			sceneci = (SceneCacheInfo*)a_exfunc->create_shared_mem(key, (frame << 7) | subframe, efpip->obj_line * *h * yc_size + 16, NULL);
			if (sceneci == NULL) {
				return img_ptr;
			}
			sceneci->w = *w;
			sceneci->h = *h;
			memcpy(sceneci->buf, img_ptr, efpip->obj_line * *h * yc_size);
		}
		return img_ptr;
	}
	void scene_cache_t::delete_scene_cache() {
		auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
		for (int flag = 0; flag < 3; flag++) {
			int key = (int)&get_scene_image + (flag << 6);
			for (int i = 1; i < 50; i++) {
				a_exfunc->delete_shared_mem(key + i, NULL);
			}
		}
	}


} // namespace patch
#endif // ifdef PATCH_SWITCH_SCENE_CACHE
