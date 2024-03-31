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
#ifdef PATCH_SWITCH_FAST_SCENECHANGE

#include <aviutl.hpp>
#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	// init at exedit load
	/* シーンチェンジの速度アップ
		ワイプ(円)
		ワイプ(時計)
		ワイプ(横)
		ワイプ(縦)
		ランダムライン
	*/
	/* 他で実装済み（発光から下はスクリプトなので省略）
		放射ブラー (fast.radiationalblurにて実装）
		ぼかし（fast.blurにて実装）
	*/

	inline class SceneChange_t {
		struct SceneChange_var {
			ExEdit::PixelYC* dst;
			int w;
			int h;
			ExEdit::PixelYC* src1;
			ExEdit::PixelYC* src2;
			int rate;
			int track1;
			int line;
		};
		static void __cdecl scCircleWipe(ExEdit::PixelYC* dst, int w, int h, ExEdit::PixelYC* src1, ExEdit::PixelYC* src2, int rate, int track1, int line, ExEdit::Filter* efp);
		static void __cdecl scFanWipe(ExEdit::PixelYC* dst, int w, int h, ExEdit::PixelYC* src1, ExEdit::PixelYC* src2, int rate, int track1, int line, ExEdit::Filter* efp);
		static void __cdecl scFanWipe_mt(int thread_id, int thread_num, ExEdit::Filter* efp, SceneChange_var* sc);
		static void __cdecl scHorizontalWipe(ExEdit::PixelYC* dst, int w, int h, ExEdit::PixelYC* src1, ExEdit::PixelYC* src2, int rate, int track1, int line, ExEdit::Filter* efp);
		static void __cdecl scVerticalWipe(ExEdit::PixelYC* dst, int w, int h, ExEdit::PixelYC* src1, ExEdit::PixelYC* src2, int rate, int track1, int line, ExEdit::Filter* efp);
		static void __cdecl scRandomLine(ExEdit::PixelYC* dst, int w, int h, ExEdit::PixelYC* src1, ExEdit::PixelYC* src2, int rate, int track1, int line, ExEdit::Filter* efp);
		static void __cdecl scRandomLine_v_mt1(int thread_id, int thread_num, ExEdit::Filter* efp, SceneChange_var* sc);
		static void __cdecl scRandomLine_v_mt2(int thread_id, int thread_num, ExEdit::Filter* efp, SceneChange_var* sc);
		
		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.scenechange";

	public:

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			constexpr int vp_begin = 0xb6f3c;
			OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0xb6f90 - vp_begin);

			h.store_i32(0xb6f3c - vp_begin, &scCircleWipe);
			h.store_i32(0xb6f44 - vp_begin, &scFanWipe);
			h.store_i32(0xb6f7c - vp_begin, &scHorizontalWipe);
			h.store_i32(0xb6f80 - vp_begin, &scVerticalWipe);
			h.store_i32(0xb6f8c - vp_begin, &scRandomLine);

		}

		void switching(bool flag) { enabled = flag; }

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

	} SceneChange;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_SCENECHANGE
