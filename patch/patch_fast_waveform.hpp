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
#ifdef PATCH_SWITCH_FAST_WAVEFORM

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	// init at exedit load
	// 音声波形表示の速度アップ
	// 波形モード0の部分の処理をマルチスレッドに
	inline class Waveform_t {
		struct wf_var { // [esp+10]
			int obj_w;
			int audio_n;
			short* audio_data;
			int res_h;
			int obj_h;
			int _other24;
			int mirror;
			int _other2c;
			int _other30;
			int _other34;
			int pad_h;
			int _other3c;
			int pad_w;
			void* _other44;
			int _other48;
			int _other4c;
			int res_w;
			int _other54;
			int _other58;
			int _other5c;
			int _other60;
			int _other64;
			int _other68;
			int _other6c;
			int _other70;
			int sample_w;
		};

		static void __cdecl normal_wrap(wf_var* wf, ExEdit::FilterProcInfo* efpip);
		static void __cdecl mt(int thread_id, int thread_num, void* param1, void* param2);

		inline static int(__cdecl* calc_pos)(int, int, int, int);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.waveform";


	public:

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			calc_pos = reinterpret_cast<decltype(calc_pos)>(GLOBAL::exedit_base + 0x8e010);

			/*
			1008e557 - 1008e8a0 wrap
			jmp 1008eaec

			1008e557 8d442410           lea     eax,dword ptr [esp+10]
			1008e55b 51                 push    ecx ; efpip
			1008e55c 50                 push    eax
			1008e55d e8XxXxXxXx         call    normal_wrap
			1008e562 83c408             add     esp,+08
			1008e565 e982050000         jmp     1008eaec
			1008e56a
			*/

			constexpr int vp_begin = 0x8e557;
			OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x8e56a - vp_begin);
			h.store_i32(0x8e557 - vp_begin, '\x8d\x44\x24\x10');
			h.store_i32(0x8e55b - vp_begin, '\x51\x50\xe8\x00');
			h.replaceNearJmp(0x8e55e - vp_begin, &normal_wrap);
			h.store_i32(0x8e562 - vp_begin, '\x83\xc4\x08\xe9');
			h.store_i32(0x8e566 - vp_begin, '\x82\x05\x00\x00');
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

	} Waveform;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_WAVEFORM
