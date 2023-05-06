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

#pragma once
#include "macro.h"

#ifdef PATCH_SWITCH_ANY_OBJ

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "restorable_patch.hpp"

#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	/* 複数選択で変更しても変わらない部分を変わるようにする
	*/
	inline class any_obj_t {
		static char* __cdecl disp_extension_image_file_wrap(ExEdit::Filter* efp, char* path);
		static char* __cdecl disp_extension_movie_file_wrap(ExEdit::Filter* efp, char* path);
		static char* __cdecl disp_extension_audio_file_wrap(ExEdit::Filter* efp, char* path);
		static void __cdecl set_figure_type_text_wrap(ExEdit::Filter* efp, void* exdata);
		static int __stdcall count_section_num_wrap(ExEdit::Filter* efp, ExEdit::ObjectFilterIndex ofi);
		bool enabled = true;
		bool enabled_i;

		inline static const char key[] = "any_obj";


	public:

		static void __cdecl update_any_range(ExEdit::Filter* efp);

		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;
			{ // 画像ファイル・画像ファイル合成 の参照ファイル
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0xe18c, 4);
				h.replaceNearJmp(0, &disp_extension_image_file_wrap);
			}
			{ // 動画ファイル・動画ファイル合成 の参照ファイル
				constexpr int vp_begin = 0x6744;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x6800 - vp_begin);
				{ //ignore_media_param_reset に近いものを実装。複数選択中には再生位置などを維持する
					/*
						10006744 ff5218             call    dword ptr [edx+18]
						10006747 83c404             add     esp,+04
						↓
						10006744 56                 push    esi
						10006745 e8XxXxXxXx         call    new_func
					*/
					h.store_i16(0, '\x56\xe8');
					h.replaceNearJmp(2, &count_section_num_wrap);
				}
				{ // 本実装
					h.replaceNearJmp(0x67fc - vp_begin, &disp_extension_movie_file_wrap);
				}
			}
			{ // 音声ファイル の参照ファイル
				constexpr int vp_begin = 0x9010d;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x9026d - vp_begin);
				{ //ignore_media_param_reset に近いものを実装。複数選択中には再生位置などを維持する
					/*
						1009010d ff5218             call    dword ptr [edx+18]
						10090110 83c404             add     esp,+04
						↓
						1009010d 56                 push    esi
						1009010e e8XxXxXxXx         call    new_func
					*/
					h.store_i16(0x9010d - vp_begin, '\x56\xe8');
					h.replaceNearJmp(0x9010f - vp_begin, &count_section_num_wrap);
				}
				{ // 本実装
					h.replaceNearJmp(0x90196 - vp_begin, &disp_extension_audio_file_wrap);
				}
			}

			{ // 図形 の図形の種類
				constexpr int vp_begin = 0x74562;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x74611 - vp_begin);
				h.replaceNearJmp(0, &set_figure_type_text_wrap);
				h.replaceNearJmp(0x7460d - vp_begin, &set_figure_type_text_wrap);
			}

			/* 時間制御・グループ制御・カメラ制御 の対象レイヤー数
			    undo.cppの部分から呼び出す
			*/

		}
		void switching(bool flag) {
			enabled = flag;
		}

		bool is_enabled() { return enabled; }
		bool is_enabled_i() { return enabled_i; }

		void switch_load(ConfigReader& cr) {
			cr.regist(key, [this](json_value_s* value) {
				ConfigReader::load_variable(value, enabled);
				});
		}

		void switch_store(ConfigWriter& cw) {
			cw.append(key, enabled);
		}

	} any_obj;
} // namespace patch
#endif // ifdef PATCH_SWITCH_ANY_OBJ
