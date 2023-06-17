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
	// 回転X,回転Y,回転Z のトラック計算部分
	// 拡大率 のトラック計算部分
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


			static const char code_put_rot[] =
				"\x8b\xc2"                 // mov     eax,edx
				"\x85\xc0"                 // test    eax,eax
				"\x74\x18"                 // jz      skip +18
				"\xc1\xe0\x0b"             // shl     eax,0b
				"\xb9\x6b\x29\x82\x74"     // mov     ecx,7482296b
				"\xf7\xe9"                 // imul    ecx
				"\xc1\xfa\x09"             // sar     edx,09
				"\x8b\xc2"                 // mov     eax,edx
				"\xc1\xe8\x1f"             // shr     eax,1f
				"\x03\xc2"                 // add     eax,edx
				"\x0f\x1f\x40"//\x00"      // nop
			;
			static const char code_put_zoom[] =
				"\xb9\xad\x8b\xdb\x68"     // mov     ecx,68db8bad
				"\xc1\xe0\x0c"             // shl     eax,0c
				"\xf7\xe9"                 // imul    ecx
				"\xc1\xfa\x08"             // sar     edx,08
				"\x8b\xc2"                 // mov     eax,edx
			;
			{
				constexpr int vp_begin = 0x19426;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x195d3 - vp_begin);
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
						"\x74\x2c"                    // jz      10019456
						"\x0f\x1f\x80\x00\x00\x00\x00"// nop
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
					for (int i = 0; i < 121; i += 60) { // 19506 19542 1957e
						memcpy(reinterpret_cast<void*>(h.address(0x19506 - vp_begin + i)), code_put_rot, sizeof(code_put_rot) - 1);
					}
				}
				{ // 拡大率
					/*
						zoom = (int)((double)track_zoom * 6.5536);
						↓
						zoom = (track_zoom << 12) / 625;
					*/
					memcpy(reinterpret_cast<void*>(h.address(0x195c4 - vp_begin)), code_put_zoom, sizeof(code_put_zoom) - 1);
				}
			}
			{ // get_geometry_track_data(0x18640)
				constexpr int vp_begin = 0x186bb;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x18826 - vp_begin);
				{ // X,Y,Z
					static const char code_put[] =
						"\x85\xc0"                    // test    eax,eax
						"\x74\x22"                    // jz      skip,22
						"\x0f\x1f\x80\x00\x00\x00\x00"// nop
						"\x89\x44\x24\x10"            // mov     dword ptr [esp+10],eax
						"\x89\x4c\x24\x14"            // mov     dword ptr [esp+14],ecx
						;
					for (int i = 0; i < 123; i += 61) { // 186bb 186f8 18735
						memcpy(reinterpret_cast<void*>(h.address(i)), code_put, sizeof(code_put) - 1);
					}
					h.store_i8(0x186d1 - vp_begin, '\x10');
					h.store_i8(0x186db - vp_begin, '\x14');
				}
				{ // 回転
					for (int i = 0; i < 105; i += 52) { // 18771 187a5 187d9
						memcpy(reinterpret_cast<void*>(h.address(0x18771 - vp_begin + i)), code_put_rot, sizeof(code_put_rot) - 1);
					}
				}
				{ // 拡大率
					memcpy(reinterpret_cast<void*>(h.address(0x18817 - vp_begin)), code_put_zoom, sizeof(code_put_zoom) - 1);
				}
			}
			{ // ModifyTrackToLua(0x188d0)
				constexpr int vp_begin = 0x18959;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x18ac3 - vp_begin);
				{ // X,Y,Z
					static const char code_put[] =
						"\x85\xc0"                    // test    eax,eax
						"\x74\x22"                    // jz      skip,22
						"\x0f\x1f\x80\x00\x00\x00\x00"// nop
						"\x89\x4c\x24\x10"            // mov     dword ptr [esp+10],ecx
						"\x89\x44\x24\x0c"            // mov     dword ptr [esp+0c],eax
						;
					h.store_i8(0x1896f - vp_begin, '\x0c');
					h.store_i8(0x18979 - vp_begin, '\x10');
					memcpy(reinterpret_cast<void*>(h.address(0x18959 - vp_begin)), code_put, sizeof(code_put) - 1);
					memcpy(reinterpret_cast<void*>(h.address(0x18995 - vp_begin)), code_put, sizeof(code_put) - 1);
					memcpy(reinterpret_cast<void*>(h.address(0x189d2 - vp_begin)), code_put, sizeof(code_put) - 1);
				}
				{ // 回転
					for (int i = 0; i < 105; i += 52) { // 18a0e 18a42 18a76
						memcpy(reinterpret_cast<void*>(h.address(0x18a0e - vp_begin + i)), code_put_rot, sizeof(code_put_rot) - 1);
					}
				}
				{ // 拡大率
					memcpy(reinterpret_cast<void*>(h.address(0x18ab4 - vp_begin)), code_put_zoom, sizeof(code_put_zoom) - 1);
				}

			}

			{ // グループ制御処理
				constexpr int vp_begin = 0x5a16f;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x5a461 - vp_begin);
				{ // X
					h.store_i32(0, '\x67\x66\x66\x66');
					static const char code_put[] =
						"\x85\xc0"                 // test    eax,eax
						"\x74\x1a"                 // jz      1005a194
						"\xc1\xe0\x0b"             // shl     eax,0b
						"\xf7\xef"                 // imul    edi
						"\xd1\xfa"                 // sar     edx,1
						"\x8b\xc2"                 // mov     eax,edx
						"\xc1\xe8\x1f"             // shr     eax,1f
						"\x03\xc2"                 // add     eax,edx
						"\xeb\x0a"                 // jmp     1005a194
						;
					memcpy(reinterpret_cast<void*>(h.address(0x5a176 - vp_begin)), code_put, sizeof(code_put) - 1);
				}
				{ // YZ
					h.store_i16(0x5a1a9 - vp_begin, '\x89\x16');
					static const char code_put[] =
						"\x85\xc0"                 // test    eax,eax
						"\x74\x15"                 // jz      1005a1c4
						"\xc1\xe0\x0b"             // shl     eax,0b
						"\xf7\xef"                 // imul    edi
						"\xd1\xfa"                 // sar     edx,1
						"\x8b\xc2"                 // mov     eax,edx
						"\xc1\xe8\x1f"             // shr     eax,1f
						"\x03\xc2"                 // add     eax,edx
						"\x0f\x1f\x80\x00\x00\x00"//\x00"// nop
						;
					memcpy(reinterpret_cast<void*>(h.address(0x5a1ab - vp_begin)), code_put, sizeof(code_put) - 1);
					memcpy(reinterpret_cast<void*>(h.address(0x5a1d3 - vp_begin)), code_put, sizeof(code_put) - 1);
				}
				{ // 回転、 入れ子 回転
					for (int i = 0; i < 100; i += 47) { // 5a22f 5a25e 5a28d  5a3e6 5a415 5a444
						memcpy(reinterpret_cast<void*>(h.address(0x5a22f - vp_begin + i)), code_put_rot, sizeof(code_put_rot) - 1);
						memcpy(reinterpret_cast<void*>(h.address(0x5a3e6 - vp_begin + i)), code_put_rot, sizeof(code_put_rot) - 1);
					}
				}
				{ // 拡大率、 入れ子 拡大率
					memcpy(reinterpret_cast<void*>(h.address(0x5a209 - vp_begin)), code_put_zoom, sizeof(code_put_zoom) - 1);
					memcpy(reinterpret_cast<void*>(h.address(0x5a3ba - vp_begin)), code_put_zoom, sizeof(code_put_zoom) - 1);
				}

				{ // 入れ子 XYZ
					h.store_i32(0x5a31f - vp_begin, '\x67\x66\x66\x66');
					h.store_i16(0x5a34f - vp_begin, '\x89\x16');
					static const char code_put[] =
						"\x85\xc0"                 // test    eax,eax
						"\x74\x19"                 // jz      1005a343
						"\xc1\xe0\x0b"             // shl     eax,0b
						"\xf7\xeb"                 // imul    ebx
						"\xd1\xfa"                 // sar     edx,1
						"\x8b\xc2"                 // mov     eax,edx
						"\xc1\xe8\x1f"             // shr     eax,1f
						"\x03\xc2"                 // add     eax,edx
						"\xeb\x09"                 // jmp     1005a343
						;
					memcpy(reinterpret_cast<void*>(h.address(0x5a326 - vp_begin)), code_put, sizeof(code_put) - 1);
					memcpy(reinterpret_cast<void*>(h.address(0x5a351 - vp_begin)), code_put, sizeof(code_put) - 1);
					memcpy(reinterpret_cast<void*>(h.address(0x5a37d - vp_begin)), code_put, sizeof(code_put) - 1);
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
