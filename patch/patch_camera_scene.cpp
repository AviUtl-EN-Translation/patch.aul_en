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

#include "patch_camera_scene.hpp"

#ifdef PATCH_SWITCH_CAMERA_SCENE
namespace patch {
	BOOL __cdecl camera_scene_t::video_func_main_wrap(ExEdit::FilterProcInfo* efpip, AviUtl::FilterProcInfo* fpip, int end_layer, int frame, int subframe, int scene_idx, ExEdit::ObjectFilterIndex ofi) {
		if ((*(int*)&efpip->object_flag & 0x10000) == 0) {
			return reinterpret_cast<BOOL(__cdecl*)(AviUtl::FilterPlugin*, AviUtl::FilterProcInfo*, int, int, int, int, ExEdit::ObjectFilterIndex)>(GLOBAL::exedit_base + OFS::ExEdit::video_func_main)(NULL, fpip, end_layer, frame, subframe, scene_idx, ofi);
		}

		int video_func_idx = efpip->v_func_idx;
		
		zorder_smem_handle[video_func_idx] = (AviUtl::SharedMemoryInfo*)1;

		void** zbuffer_ptr = (void**)(GLOBAL::exedit_base + OFS::ExEdit::zbuffer_ptr);
		if (*zbuffer_ptr != NULL) {
			zbuf_smem_handle[video_func_idx] = (AviUtl::SharedMemoryInfo*)1;
		} else {
			zbuf_smem_handle[video_func_idx] = 0;
		}
		void** shadowmap_ptr = (void**)(GLOBAL::exedit_base + OFS::ExEdit::shadowmap_ptr);
		if (*shadowmap_ptr != NULL) {
			shadowmap_smem_handle[video_func_idx] = (AviUtl::SharedMemoryInfo*)1;
		} else {
			shadowmap_smem_handle[video_func_idx] = 0;
		}

		/*
		auto camera_ofi_ptr = (ExEdit::ObjectFilterIndex*)(GLOBAL::exedit_base + OFS::ExEdit::select_camera_ofi);
		auto camera_ofi = *camera_ofi_ptr;
		auto camera_dlg_visible_ptr = (int*)(GLOBAL::exedit_base + OFS::ExEdit::camera_dlg_visible);
		auto camera_dlg_visible = *camera_dlg_visible_ptr;
		auto camera_value_change_mode_ptr = (int*)(GLOBAL::exedit_base + OFS::ExEdit::camera_value_change_mode);
		auto camera_value_change_mode = *camera_value_change_mode_ptr;
		auto int135a4c_ptr = (int*)(GLOBAL::exedit_base + 0x135a4c);
		auto int135a4c = *int135a4c_ptr;
		auto camera_gui_mode_ptr = (int*)(GLOBAL::exedit_base + 0x134ec8);
		auto camera_gui_mode = *camera_gui_mode_ptr;
		auto camera1359e8_ptr = (int*)(GLOBAL::exedit_base + 0x1359e8);
		auto camera1359e8 = *camera1359e8_ptr;
		auto camera135aa0_ptr = (int*)(GLOBAL::exedit_base + 0x135aa0);
		auto camera135aa0 = *camera135aa0_ptr;
		auto camera135aa4_ptr = (int*)(GLOBAL::exedit_base + 0x135aa4);
		auto camera135aa4 = *camera135aa4_ptr;

		auto int135a84_arr = (int*)(GLOBAL::exedit_base + 0x135a84);
		int int135a84[7];
		memcpy(int135a84, int135a84_arr, 28);
		auto int135a50_arr = (int*)(GLOBAL::exedit_base + 0x135a50);
		int int135a50[7];
		memcpy(int135a50, int135a50_arr, 28);
		*/
		BOOL ret =  reinterpret_cast<BOOL(__cdecl*)(AviUtl::FilterPlugin*, AviUtl::FilterProcInfo*, int, int, int, int, ExEdit::ObjectFilterIndex)>(GLOBAL::exedit_base + OFS::ExEdit::video_func_main)(NULL, fpip, end_layer, frame, subframe, scene_idx, ofi);
		/*
		*camera_ofi_ptr = camera_ofi;
		*camera_dlg_visible_ptr = camera_dlg_visible;
		*camera_value_change_mode_ptr = camera_value_change_mode;
		*int135a4c_ptr = int135a4c;
		*camera_gui_mode_ptr = camera_gui_mode;
		*camera1359e8_ptr = camera1359e8;
		*camera135aa0_ptr = camera135aa0;
		*camera135aa4_ptr = camera135aa4;

		memcpy(int135a84_arr, int135a84, 28);
		memcpy(int135a50_arr, int135a50, 28);
		*/
		
		if (1 < (unsigned int)zorder_smem_handle[video_func_idx]) {
			auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
			void* ptr = a_exfunc->get_shared_mem((int)&zorder_smem_handle[video_func_idx], 0, zorder_smem_handle[video_func_idx]);
			if (ptr != NULL) {
				/*
				auto z_order_ofi = (ExEdit::ObjectFilterIndex*)(GLOBAL::exedit_base + OFS::ExEdit::z_order_ofi);
				int z_order_ofi_size = 100 * sizeof(ExEdit::ObjectFilterIndex);
				memcpy(z_order_ofi, ptr, z_order_ofi_size);
				ptr = (void*)((int)ptr + z_order_ofi_size);
				auto z_order_coord = (int*)(GLOBAL::exedit_base + OFS::ExEdit::z_order_coord);
				memcpy(z_order_coord, ptr, 500 * sizeof(int));
				*/

				memcpy((void*)(GLOBAL::exedit_base + 0x134e80), ptr, 0x135aa8 - 0x134e80);

				a_exfunc->delete_shared_mem((int)&zorder_smem_handle[video_func_idx], zorder_smem_handle[video_func_idx]);
			}
		}

		int* zbuffer_size = (int*)(GLOBAL::exedit_base + OFS::ExEdit::zbuffer_size);
		if (1 < (unsigned int)zbuf_smem_handle[video_func_idx]) {
			auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
			void* ptr = a_exfunc->get_shared_mem((int)&zbuf_smem_handle[video_func_idx], 0, zbuf_smem_handle[video_func_idx]);
			if (ptr != NULL) {
				memcpy(*zbuffer_ptr, ptr, *zbuffer_size);
				a_exfunc->delete_shared_mem((int)&zbuf_smem_handle[video_func_idx], zbuf_smem_handle[video_func_idx]);
			}
		//} else {
		//	if (*zbuffer_ptr != NULL) {
		//		reinterpret_cast<void(__cdecl*)(void*, int)>(GLOBAL::exedit_base + OFS::ExEdit::clear_zbuffer)(*zbuffer_ptr, *zbuffer_size);
		//	}
		}
		int* shadowmap_size = (int*)(GLOBAL::exedit_base + OFS::ExEdit::shadowmap_size);
		if (1 < (unsigned int)shadowmap_smem_handle[video_func_idx]) {
			auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
			void* ptr = a_exfunc->get_shared_mem((int)&shadowmap_smem_handle[video_func_idx], 0, shadowmap_smem_handle[video_func_idx]);
			if (ptr != NULL) {
				memcpy(*shadowmap_ptr, ptr, *shadowmap_size);
				a_exfunc->delete_shared_mem((int)&shadowmap_smem_handle[video_func_idx], shadowmap_smem_handle[video_func_idx]);
			}
		//} else {
		//	if (*shadowmap_ptr != NULL) {
		//		reinterpret_cast<void(__cdecl*)(void*, int)>(GLOBAL::exedit_base + OFS::ExEdit::clear_shadowmap)(*shadowmap_ptr, *shadowmap_size);
		//	}
		}
		return ret;
	}

	void __cdecl camera_scene_t::clear_zbuffer_wrap(void* zbuffer_ptr, int zbuffer_size) {
		int video_func_idx = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::video_func_idx) - 1;
		if (zbuffer_ptr != NULL) {
			for (int i = 0; i < video_func_idx; i++) {
				if ((unsigned int)zbuf_smem_handle[i] == 1) {
					auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
					void* ptr = a_exfunc->create_shared_mem((int)&zbuf_smem_handle[i], 0, zbuffer_size, &zbuf_smem_handle[i]);
					if (ptr != NULL) {
						memcpy(ptr, zbuffer_ptr, zbuffer_size);
					}
					break;
				}
			}
		}
		//reinterpret_cast<void(__cdecl*)(void*, int)>(GLOBAL::exedit_base + OFS::ExEdit::clear_zbuffer)(zbuffer_ptr, zbuffer_size);
		memset(zbuffer_ptr, 0, zbuffer_size);

		for (int i = 0; i < video_func_idx; i++) {
			if ((unsigned int)zorder_smem_handle[i] == 1) {
				/*
				int z_order_ofi_size = 100 * sizeof(ExEdit::ObjectFilterIndex);
				int z_order_coord_size = 500 * sizeof(int);
				auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
				void* ptr = a_exfunc->create_shared_mem((int)&zorder_smem_handle[i], 0, z_order_ofi_size + z_order_coord_size, &zorder_smem_handle[i]);
				if (ptr != NULL) {
					auto z_order_ofi = (ExEdit::ObjectFilterIndex*)(GLOBAL::exedit_base + OFS::ExEdit::z_order_ofi);
					memcpy(ptr, z_order_ofi, z_order_ofi_size);
					ptr = (void*)((int)ptr + z_order_ofi_size);
					auto z_order_coord = (int*)(GLOBAL::exedit_base + OFS::ExEdit::z_order_coord);
					memcpy(ptr, z_order_coord, z_order_coord_size);
				}
				*/
				auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
				void* ptr = a_exfunc->create_shared_mem((int)&zorder_smem_handle[i], 0, 0x135aa8 - 0x134e80, &zorder_smem_handle[i]);
				if (ptr != NULL) {
					memcpy(ptr, (void*)(GLOBAL::exedit_base + 0x134e80), 0x135aa8 - 0x134e80);
				}
				break;
			}
		}

	}

	void __cdecl camera_scene_t::clear_shadowmap_wrap(void* shadowmap_ptr, int shadowmap_size) {
		int video_func_idx = *(int*)(GLOBAL::exedit_base + OFS::ExEdit::video_func_idx) - 1;
		if (shadowmap_ptr != NULL) {
			for (int i = 0; i < video_func_idx; i++) {
				if ((unsigned int)shadowmap_smem_handle[i] == 1) {
					auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
					void* ptr = a_exfunc->create_shared_mem((int)&shadowmap_smem_handle[i], 0, shadowmap_size, &shadowmap_smem_handle[i]);
					if (ptr != NULL) {
						memcpy(ptr, shadowmap_ptr, shadowmap_size);
					}
					break;
				}
			}
		}
		reinterpret_cast<void(__cdecl*)(void*, int)>(GLOBAL::exedit_base + OFS::ExEdit::clear_shadowmap)(shadowmap_ptr, shadowmap_size);
	}
} // namespace patch
#endif // ifdef PATCH_SWITCH_CAMERA_SCENE
