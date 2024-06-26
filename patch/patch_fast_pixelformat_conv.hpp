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
#ifdef PATCH_SWITCH_FAST_PIXELFORMAT_CONV

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	// init at exedit load
	// PixelYC形式を変換する部分の速度アップ {short,short,short} <-> {float,byte,byte}
	

	inline class pixelformat_conv_t {

		static void __cdecl mt_sss2fbb(int thread_id, int thread_num, void* n1, void* n2);
		static void __cdecl mt_ssss2fbbs(int thread_id, int thread_num, void* n1, void* n2);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.pixelformat_conv";

#pragma pack(push, 1)
		struct PixelFormatConv_var { // 1e42f4
			void* pix;
			int _other1;
			int _other2;
			int w;
			int h;
			void* _other3;
			int _padding;
			double inv_intensity;
			double intensity;
		};
		struct PixelYCA_fbbs {
			float	y;
			int8_t	cb;
			int8_t	cr;
			int16_t	a;
		};
		struct PixelYC_fbb {
			float	y;
			int8_t	cb;
			int8_t	cr;
		};
#pragma pack(pop)

		// inline static const double ln65536 = 11.090354888959124950675713943331;

	public:

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			/*
			{ // ループ内で浮動小数掛け算を行っているのをループ外で済ませる
				{ // yca
					OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x704a0, 4).store_i32(0, &ln65536);
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x704f4, 8);
					h.store_i16(0, '\xd9\xc1');
					h.store_i16(6, '\x66\x90');
				}
				{ // yc
					OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x707a3, 4).store_i32(0, &ln65536);
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x707f3, 8);
					h.store_i16(0, '\xd9\xc1');
					h.store_i16(6, '\x66\x90');
				}
			}
			*/


			if (has_flag(get_CPUCmdSet(), CPUCmdSet::F_AVX2)) {
				OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x7026b, 4).store_i32(0, &mt_ssss2fbbs);
				OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x7059b, 4).store_i32(0, &mt_sss2fbb);
			}
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

	} pixelformat_conv;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_PIXELFORMAT_CONV
