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
		static void __cdecl set_figure_type_text_wrap(ExEdit::Filter* efp, void* exdata);

		bool enabled = true;
		bool enabled_i;

		inline static const char key[] = "any_obj";


	public:

		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;
			{ // 画像ファイル・画像ファイル合成 の参照ファイル
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0xe18c, 4);
				h.replaceNearJmp(0, &disp_extension_image_file_wrap);
			}
			{ // 動画ファイル・動画ファイル合成 の参照ファイル
				{
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x67fc, 4);
					h.replaceNearJmp(0, &disp_extension_movie_file_wrap);
				}
				{ //ignore_media_param_reset に近いものを実装。複数選択中には再生位置などを維持する
				}
			}

			{ // 図形 の図形の種類
				constexpr int vp_begin = 0x74562;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x74611 - vp_begin);
				h.replaceNearJmp(0, &set_figure_type_text_wrap);
				h.replaceNearJmp(0x7460d - vp_begin, &set_figure_type_text_wrap);
			}


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
