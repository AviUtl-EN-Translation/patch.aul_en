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

#ifdef PATCH_SWITCH_BLEND

#include <memory>
#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "restorable_patch.hpp"

#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	// 合成モード関数の修正(主にアルファチャンネル有りシーンオブジェクト)
	inline class blend_t {
		static void __cdecl blend_yca_add(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a);
		static void __cdecl blend_yca_sub(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a);
		static void __cdecl blend_yca_mul(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a);
		static void __cdecl blend_yca_screen(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a);
		static void __cdecl blend_yca_overlay(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a);
		static void __cdecl blend_yca_cmpmax(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a);
		static void __cdecl blend_yca_cmpmin(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a);
		static void __cdecl blend_yca_luminance(ExEdit::PixelYCA* dst, short src_y, short src_cb, short src_cr, short src_a);
		static void __cdecl blend_yca_colordiff(ExEdit::PixelYCA* dst, short src_y, short src_cb, short src_cr, short src_a);
		static void __cdecl blend_yca_shadow(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a);
		static void __cdecl blend_yca_lightdark(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a);
		static void __cdecl blend_yca_difference(ExEdit::PixelYCA* dst, int src_y, int src_cb, int src_cr, int src_a);

		bool enabled = true;
		bool enabled_i;

		inline static const char key[] = "blend";


	public:

		inline static void(__cdecl* blend_yca_normal)(void* dst, int src_y, int src_cb, int src_cr, int src_a);

		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			blend_yca_normal = reinterpret_cast<decltype(blend_yca_normal)>(GLOBAL::exedit_base + OFS::ExEdit::blend_yca_normal_func);

			{
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x9fbb4, 48);
				h.store_i32(0, &blend_yca_add);
				h.store_i32(4, &blend_yca_sub);
				h.store_i32(8, &blend_yca_mul);
				h.store_i32(12, &blend_yca_screen);
				h.store_i32(16, &blend_yca_overlay);
				h.store_i32(20, &blend_yca_cmpmax);
				h.store_i32(24, &blend_yca_cmpmin);
				h.store_i32(28, &blend_yca_luminance);
				h.store_i32(32, &blend_yca_colordiff);
				h.store_i32(36, &blend_yca_shadow);
				h.store_i32(40, &blend_yca_lightdark);
				h.store_i32(44, &blend_yca_difference);
			}

            char blend_yca_normal_bin[] = {

                "\x8b\x4c\x24\x04"         // mov     ecx,dword ptr [esp+04]
                "\x8b\x54\x24\x14"         // mov     edx,dword ptr [esp+14]
                "\xb8\x00\x10\x00\x00"     // mov     eax,00001000
                "\x3b\xd0"                 // cmp     edx,eax
                "\x7c\x1b"                 // jl      skip,+1b
                "\x66\x8b\x54\x24\x0c"     // mov     dx,[esp+0c]
                "\xc1\xe0\x10"             // shl     eax,10
                "\xc1\xe2\x10"             // shl     edx,10
                "\x66\x8b\x44\x24\x10"     // mov     ax,[esp+10]
                "\x66\x8b\x54\x24\x08"     // mov     dx,[esp+08]
                "\x89\x41\x04"             // mov     dword ptr [ecx+04],eax
                "\x89\x11"                 // mov     dword ptr [ecx],edx
                "\xc3"                     // ret
                "\x56"                     // push    esi
                "\x0f\xbf\x71\x06"         // movsx   esi,dword ptr [ecx+06]
                "\x3b\xf0"                 // cmp     esi,eax
                "\x7c\x3c"                 // jl      skip,+3c
                "\x5e"                     // pop     esi
                "\x66\x89\x41\x06"         // mov     [ecx+06],ax
                "\xf7\xda"                 // neg     edx
                "\x0f\xbf\x01"             // movsx   eax,dword ptr [ecx]
                "\x2b\x44\x24\x08"         // sub     eax,dword ptr [esp+08]
                "\x0f\xaf\xc2"             // imul    eax,edx
                "\xc1\xf8\x0c"             // sar     eax,0c
                "\x66\x01\x01"             // add     [ecx],ax
                "\x0f\xbf\x41\x02"         // movsx   eax,dword ptr [ecx+02]
                "\x2b\x44\x24\x0c"         // sub     eax,dword ptr [esp+0c]
                "\x0f\xaf\xc2"             // imul    eax,edx
                "\xc1\xf8\x0c"             // sar     eax,0c
                "\x66\x01\x41\x02"         // add     [ecx+02],ax
                "\x0f\xbf\x41\x04"         // movsx   eax,dword ptr [ecx+04]
                "\x2b\x44\x24\x10"         // sub     eax,dword ptr [esp+10]
                "\x0f\xaf\xc2"             // imul    eax,edx
                "\xc1\xf8\x0c"             // sar     eax,0c
                "\x66\x01\x41\x04"         // add     [ecx+04],ax
                "\xc3"                     // ret
                "\x85\xf6"                 // test    esi,esi
                "\x7f\x1c"                 // jg      skip,+1c
                "\x5e"                     // pop     esi
                "\xc1\xe2\x10"             // shl     edx,10
                "\x66\x8b\x44\x24\x0c"     // mov     ax,[esp+0c]
                "\x66\x8b\x54\x24\x10"     // mov     dx,[esp+10]
                "\xc1\xe0\x10"             // shl     eax,10
                "\x66\x8b\x44\x24\x08"     // mov     ax,[esp+08]
                "\x89\x51\x04"             // mov     dword ptr [ecx+04],edx
                "\x89\x01"                 // mov     dword ptr [ecx],eax
                "\xc3"                     // ret
                "\x53"                     // push    ebx
                "\x8b\xd8"                 // mov     ebx,eax
                "\x2b\xc2"                 // sub     eax,edx
                "\x2b\xde"                 // sub     ebx,esi
                "\x0f\xaf\xd8"             // imul    ebx,eax
                "\x0f\xaf\xc6"             // imul    eax,esi
                "\x8b\xf1"                 // mov     esi,ecx
                "\xb9\x00\x08\x00\x01"     // mov     ecx,01000800
                "\x2b\xcb"                 // sub     ecx,ebx
                "\xc1\xf9\x0c"             // sar     ecx,0c
                "\x33\xd2"                 // xor     edx,edx ; cdq
                "\xf7\xf9"                 // idiv    ecx
                "\x8b\xd8"                 // mov     ebx,eax
                "\x8b\x44\x24\x1c"         // mov     eax,dword ptr [esp+1c]
                "\x66\x89\x4e\x06"         // mov     [esi+06],cx
                "\xc1\xe0\x0c"             // shl     eax,0c
                "\x33\xd2"                 // xor     edx,edx ; cdq
                "\xf7\xf9"                 // idiv    ecx
                "\x0f\xbf\x56\x04"         // movsx   edx,dword ptr [esi+04]
                "\x8b\x4c\x24\x18"         // mov     ecx,dword ptr [esp+18]
                "\x0f\xaf\xd3"             // imul    edx,ebx
                "\x0f\xaf\xc8"             // imul    ecx,eax
                "\x03\xca"                 // add     ecx,edx
                "\xc1\xf9\x0c"             // sar     ecx,0c
                "\x0f\xbf\x56\x02"         // movsx   edx,dword ptr [esi+02]
                "\x66\x89\x4e\x04"         // mov     [esi+04],cx
                "\x8b\x4c\x24\x14"         // mov     ecx,dword ptr [esp+14]
                "\x0f\xaf\xd3"             // imul    edx,ebx
                "\x0f\xaf\xc8"             // imul    ecx,eax
                "\x03\xca"                 // add     ecx,edx
                "\x0f\xbf\x16"             // movsx   edx,dword ptr [esi]
                "\x0f\xaf\x44\x24\x10"     // imul    eax,dword ptr [esp+10]
                "\x0f\xaf\xd3"             // imul    edx,ebx
                "\x03\xd0"                 // add     edx,eax
                "\xc1\xe1\x04"             // shl     ecx,04
                "\xc1\xfa\x0c"             // sar     edx,0c
                "\x5b"                     // pop     ebx
                "\x66\x8b\xca"             // mov     cx,dx
                "\x89\x0e"                 // mov     dword ptr [esi],ecx
                "\x5e"                     // pop     esi
                "\xc3"                     // ret
            };
            {
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + OFS::ExEdit::blend_yca_normal_func, sizeof(blend_yca_normal_bin) - 1);
                memcpy(reinterpret_cast<void*>(h.address()), blend_yca_normal_bin, sizeof(blend_yca_normal_bin) - 1);
            }
            {
                char blend_yc_normal_bin[] = {

                    "\x8b\x4c\x24\x04"         // mov     ecx,dword ptr [esp+04]
                    "\x8b\x54\x24\x14"         // mov     edx,dword ptr [esp+14]
                    "\x81\xfa\x00\x10\x00\x00" // cmp     edx,00001000
                    "\x7c\x35"                 // jl      skip,+35
                    "\x66\x8b\x44\x24\x10"     // mov     ax,[esp+10]
                    "\xf7\xc1\x02\x00\x00\x00" // test    ecx,00000002
                    "\x75\x14"                 // jnz     skip,+14
                    "\x66\x8b\x54\x24\x0c"     // mov     dx,[esp+0c]
                    "\xc1\xe2\x10"             // shl     edx,10
                    "\x66\x8b\x54\x24\x08"     // mov     dx,[esp+08]
                    "\x89\x11"                 // mov     dword ptr [ecx],edx
                    "\x66\x89\x41\x04"         // mov     [ecx+04],ax
                    "\xc3"                     // ret
                    "\xc1\xe0\x10"             // shl     eax,10
                    "\x66\x8b\x44\x24\x0c"     // mov     ax,[esp+0c]
                    "\x66\x8b\x54\x24\x08"     // mov     dx,[esp+08]
                    "\x66\x89\x11"             // mov     [ecx],dx
                    "\x89\x41\x02"             // mov     dword ptr [ecx+02],eax
                    "\xc3"                     // ret
                    /*
                    "\xf7\xda"                 // neg     edx
                    "\x0f\xbf\x01"             // movsx   eax,dword ptr [ecx]
                    "\x2b\x44\x24\x08"         // sub     eax,dword ptr [esp+08]
                    "\x0f\xaf\xc2"             // imul    eax,edx
                    "\xc1\xf8\x0c"             // sar     eax,0c
                    "\x66\x01\x01"             // add     [ecx],ax
                    "\x0f\xbf\x41\x02"         // movsx   eax,dword ptr [ecx+02]
                    "\x2b\x44\x24\x0c"         // sub     eax,dword ptr [esp+0c]
                    "\x0f\xaf\xc2"             // imul    eax,edx
                    "\xc1\xf8\x0c"             // sar     eax,0c
                    "\x66\x01\x41\x02"         // add     [ecx+02],ax
                    "\x0f\xbf\x41\x04"         // movsx   eax,dword ptr [ecx+04]
                    "\x2b\x44\x24\x10"         // sub     eax,dword ptr [esp+10]
                    "\x0f\xaf\xc2"             // imul    eax,edx
                    "\xc1\xf8\x0c"             // sar     eax,0c
                    "\x66\x01\x41\x04"         // add     [ecx+04],ax
                    "\xc3"                     // ret
                    */
                };
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + OFS::ExEdit::blend_yc_normal_func, 124);
                memcpy(reinterpret_cast<void*>(h.address()), blend_yc_normal_bin, sizeof(blend_yc_normal_bin) - 1);
                memcpy(reinterpret_cast<void*>(h.address() + sizeof(blend_yc_normal_bin) - 1), &blend_yca_normal_bin[58], 55);
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

	} blend;
} // namespace patch
#endif // ifdef PATCH_SWITCH_BLEND
