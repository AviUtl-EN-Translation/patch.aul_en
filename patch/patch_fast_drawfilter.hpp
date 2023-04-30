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
#ifdef PATCH_SWITCH_FAST_DRAWFILTER

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	// init at exedit load
	// 標準描画や拡張描画の無駄を減らす
	// flag の移植部分
	// X,Y,Z のトラック計算部分
	// RX,RY,RZ のトラック計算部分
	inline class DrawFilter_t {

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.drawfilter";

	public:

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			{ // flag
				static const char code_put[] =
					"\x74\x06"                     // jz      skip,+06
					"\x0b\x83\x40\x01\x00\x00"     // or      eax,dword ptr [ebx+00000140]
					"\x39\xbb\xf4\x00\x00\x00"     // cmp     dword ptr [ebx+000000f4],edi
					"\x75\x02"                     // jnz     skip,+02
					"\x0c\x01"                     // or      al,01
					"\xf6\x43\x01\x01"             // test    byte ptr [ebx+01],01
					"\x74\x02"                     // jz      skip,+02
					"\x0c\x02"                     // or      al,02
					"\xf6\xc1\x08"                 // test    cl,08
					"\x74\x03"                     // jz      skip,+03
					"\x80\xcc\x02"                 // or      ah,02
					"\xf6\xc5\x10"                 // test    ch,10
					"\x74\x02"                     // jz      skip,+02
					"\x0c\x08"                     // or      al,08
					"\xf7\xc1\x00\x00\x00\x40"     // test    ecx,40000000
					"\x74\x0a"                     // jz      skip,+0a
					"\x80\xce\x08"                 // or      dh,08
					"\x89\x94\x24\xf4\x00\x00\x00" // mov     dword ptr [esp+000000f4],edx
					"\x81\xe1\x00\x0f\x00\x00"     // and     ecx,00000f00
					"\xc1\xe1\x09"                 // shl     ecx,09
					"\x0b\xc1"                     // or      eax,ecx
					"\x89\x44\x24\x1c"             // mov     dword ptr [esp+1c],eax
					"\xeb\x3f"                     // jmp     skip,+3f
				;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x191f9, sizeof(code_put) - 1);
				memcpy(reinterpret_cast<void*>(h.address()), code_put, sizeof(code_put) - 1);
			}
			{
				constexpr int vp_begin = 0x19426;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x1959c - vp_begin);
				{ // X,Y,Z
					/*
						x = track[xid];
						if (x != efp->track_scale[xid]) {
							x = 0x1000;
						} else {
							x = (int)(((double)x * 4096.0) / (double)efp->track_scale[xid]);
						}
						↓
						x = track[xid];
						if (x != 0) {
							x = (int)(((double)x * 4096.0) / (double)efp->track_scale[xid]);
						}
					*/
					static const char code_put[] =
						"\x85\xc0"                    // test    eax,eax
						"\x75\x07"                    // jnz     skip,07
						"\xeb\x2a"                    // jmp     10019456
						"\x90\x90\x90\x90\x90"        // nop
						"\x89\x94\x24\x94\x00\x00\x00"// mov     dword ptr [esp+00000094],edx
						"\x89\x44\x24\x40"            // mov     dword ptr [esp+40],eax
					;
					for (int i = 0; i < 225; i += 75) { // 19426 19471 194bc
						memcpy(reinterpret_cast<void*>(h.address(i)), code_put, sizeof(code_put) - 1);
					}
				}
				{ // 回転
					/*
						track_rx_norm = track[rxid] % 36000;
						if (track_rx_norm == 36000) { // falseにしかならない
							track_rx = 0x10000;
						} else {
							track_rx = (int)((double)track_rx_norm * 1.820444444444445); // 65536 / 36000 
						}
						↓
						track_rx_norm = track[rxid] % 36000;
						track_rx = track_rx_norm;
						if (track_rx) {
							track_rx = (track_rx << 11) / 1125;
						}
					*/
					static const char code_put[] =
						"\x8b\xc2"                 // mov     eax,edx
						"\x85\xc0"                 // test    eax,eax
						"\x74\x18"                 // jz      skip +18
						"\xc1\xe0\x0b"             // shl     eax,0b
						"\xb9\x65\x04\x00\x00"     // mov     ecx,00000465
						"\x99"                     // cdq
						"\xf7\xf9"                 // idiv    ecx
						"\xeb\x0b"                 // jmp     skip +0b
					;
					for (int i = 0; i < 121; i += 60) { // 19506 19542 1957e
						memcpy(reinterpret_cast<void*>(h.address(0x19506 - vp_begin + i)), code_put, sizeof(code_put) - 1);
					}
				}
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

	} DrawFilter;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_DRAWFILTER
