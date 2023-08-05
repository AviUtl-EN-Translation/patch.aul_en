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

#include "patch_adjust_vmem.hpp"

#ifdef PATCH_SWITCH_ADJUST_VMEM
namespace patch {
	void __fastcall adjust_vmem_t::adjust_vmem(int size) {

		{ // 共有メモリ関数で仮想メモリにマップしている物をアンマップ
			auto current_map1_smipp = reinterpret_cast<AviUtl::SharedMemoryInfo**>(GLOBAL::aviutl_base + 0x27feb0);
			auto current_map1_smip = *current_map1_smipp;
			if (current_map1_smip != nullptr && current_map1_smip->hFileMappingObject != NULL && current_map1_smip->mem_ptr != nullptr) {
				UnmapViewOfFile(current_map1_smip->mem_ptr);
				current_map1_smip->mem_ptr = nullptr;
				auto current_map2_smipp = reinterpret_cast<AviUtl::SharedMemoryInfo**>(GLOBAL::aviutl_base + 0x27febc);
				*current_map1_smipp = *current_map2_smipp;
				*current_map2_smipp = nullptr;
				return;
			}
		}
		
		if (GLOBAL::exedit_hmod != NULL) { // 拡張編集を使っている

			{ // 拡張編集タイムラインアイテムのコピーのバッファを解放
				if (0 < *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::copy_paste_buffer_size)) {
					reinterpret_cast<void(__cdecl*)(void)>(GLOBAL::exedit_base + OFS::ExEdit::init_copy_paste_info)();
					reinterpret_cast<void(__cdecl*)(void)>(GLOBAL::exedit_base + OFS::ExEdit::copy_paste_buffer_free)();
					return;
				}
			}

			{ // 動画ファイルのハンドルを閉じる
				struct AviFileHandleInfo {
					AviUtl::AviFileHandle* afh;
					int id_and_count;
					DWORD tick_count;
					int priority;
					int flag;
					AviUtl::FileInfo fi;
					char path[_MAX_PATH];
				}*afhi = reinterpret_cast<AviFileHandleInfo*>(GLOBAL::exedit_base + OFS::ExEdit::AviFileHandleInfoArray);
				int min_priority = MAXDWORD;
				int del_handle_i = -1;
				int max_num = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::AviFileHandleMaxNum);
				for (int i = 0; i < max_num; i++) {
					if (afhi[i].afh != nullptr) {
						if (afhi[i].priority < min_priority) {
							min_priority = afhi[i].priority;
							del_handle_i = i;
						}
					}
				}
				if (0 <= del_handle_i) {
					auto exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
					exfunc->avi_file_close(afhi[del_handle_i].afh);
					afhi[del_handle_i].afh = nullptr;
					return;
				}
			}
		}

		auto main_hwnd = *reinterpret_cast<HWND*>(GLOBAL::aviutl_base + OFS::AviUtl::main_hwnd);
		MessageBoxA(main_hwnd, "調整できる項目がありませんでした", PATCH_VERSION_NAME, MB_OK | MB_ICONEXCLAMATION);
	}
} // namespace patch
#endif // ifdef PATCH_SWITCH_ADJUST_VMEM

