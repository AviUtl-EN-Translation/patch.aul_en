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
#ifdef PATCH_SWITCH_FAST_AUDIO_SPECTRUM

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	// init at exedit load
	// 音声波形などの高速フーリエ変換の速度を上げる
	inline class AudioSpectrum_t {
		static void __cdecl fft_wrap();

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.audio_spectrum";

	public:

		struct AudioSpectrum_var { // 244e30
			short buf1[1024]; // 244e30
			short buf2[1024]; // 245630
			double buf3[2048]; // 245e30
			double buf4[2048]; // 249e30
		};
		

		inline static AviUtl::SharedMemoryInfo* smi;
		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			ReplaceNearJmp(GLOBAL::exedit_base + 0x8dbaf, &fft_wrap);
			ReplaceNearJmp(GLOBAL::exedit_base + 0x8de0f, &fft_wrap);
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

	} audio_spectrum;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_AUDIO_SPECTRUM
