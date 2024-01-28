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
#ifdef PATCH_SWITCH_FAST_COLORKEY

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	// init at exedit load
	// カラーキーを少しだけ速度アップ
	inline class Colorkey_t {
		static void __cdecl main_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl conv1_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static void __cdecl conv2_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.colorkey";

	public:

		/*
		struct efColorkey_var {
			int border_range; // 11ed34
			int border_size; // 11ed38
		};
		*/

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			/*
			100162d8 6840630110         push    10016340
			100162dd ff91cc000000       call    dword ptr [ecx+000000cc]
			100162e3 8b5644             mov     edx,dword ptr [esi+44]
			100162e6 83c40c             add     esp,+0c
			100162e9 8b4208             mov     eax,dword ptr [edx+08]
			100162ec 85c0               test    eax,eax
			100162ee 7441               jz      skip,+41 (10016331)
			100162f0 a338ed1110         mov     [1011ed38],eax
			100162f5 8d440001           lea     eax,dword ptr [eax+eax+01]
			100162f9 a334ed1110         mov     [1011ed34],eax
			100162fe 8b4e60             mov     ecx,dword ptr [esi+60]
			10016301 57                 push    edi
			10016302 56                 push    esi
			10016303 6840630110         push    10016340
			10016308 ff91cc000000       call    dword ptr [ecx+000000cc]
			1001630e 8b5660             mov     edx,dword ptr [esi+60]
			10016311 57                 push    edi
			10016312 56                 push    esi
			10016313 6830640110         push    10016430
			10016318 ff92cc000000       call    dword ptr [edx+000000cc]
			1001631e 8b4660             mov     eax,dword ptr [esi+60]
			10016321 57                 push    edi
			10016322 56                 push    esi
			10016323 68d0650110         push    100165d0
			10016328 ff90cc000000       call    dword ptr [eax+000000cc]
			1001632e 83c424             add     esp,+24
			10016331
			↓
			100162d8 68XxXxXxXx         push    &main_mt
			100162dd ff91cc000000       call    dword ptr [ecx+000000cc]
			100162e3 8b5644             mov     edx,dword ptr [esi+44]
			100162e6 83c40c             add     esp,+0c
			100162e9 8b4208             mov     eax,dword ptr [edx+08]
			100162ec 85c0               test    eax,eax
			100162ee 7e41               jle     skip,+41 (10016331)
			100162f0 a338ed1110         mov     [1011ed38],eax
			100162f5 8d440001           lea     eax,dword ptr [eax+eax+01]
			100162f9 a334ed1110         mov     [1011ed34],eax
			100162fe eb0e               jmp     skip,+0e (1001630e)
			10016300 --
			10016301 --
			10016302 --
			10016303 ----------
			10016308 ------------
			1001630e 8b5660             mov     edx,dword ptr [esi+60]
			10016311 57                 push    edi
			10016312 56                 push    esi
			10016313 68XxXxXxXx         push    &conv1_mt
			10016318 ff92cc000000       call    dword ptr [edx+000000cc]
			1001631e 8b4660             mov     eax,dword ptr [esi+60]
			10016321 57                 push    edi
			10016322 56                 push    esi
			10016323 68XxXxXxXx         push    &conv2_mt
			10016328 ff90cc000000       call    dword ptr [eax+000000cc]
			1001632e 83c418             add     esp,+18
			10016331


			*/

			constexpr int vp_begin = 0x162d9;
			OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x016331 - vp_begin);
			h.store_i32(0x0162d9 - vp_begin, &main_mt);
			h.store_i8(0x0162ee - vp_begin, '\x7e');
			h.store_i16(0x0162fe - vp_begin, '\xeb\x0e');
			h.store_i32(0x016314 - vp_begin, &conv1_mt);
			h.store_i32(0x016324 - vp_begin, &conv2_mt);
			h.store_i8(0x016330 - vp_begin, '\x18');
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

	} Colorkey;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_COLORKEY
