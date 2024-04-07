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
#ifdef PATCH_SWITCH_FAST_YC_FILTER_EFFECT

#include <aviutl.hpp>
#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"


#include "patch_fast_sharp.hpp"

namespace patch::fast {
	// init at exedit load

	inline class yc_filter_effect_t {

		static void __stdcall yc_check(ExEdit::FilterProcInfo* efpip, ExEdit::Object** objpp, int oid, int on, int fid);
		static BOOL __cdecl func_proc(BOOL(*func_proc)(ExEdit::Filter*, ExEdit::FilterProcInfo*), BOOL(*filter_func_proc)(ExEdit::Filter*, ExEdit::FilterProcInfo*), ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static BOOL __cdecl efColorCorrection_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		inline static BOOL(__cdecl* efColorCorrection_func_proc_org)(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static BOOL __cdecl efMonochromatic_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		inline static BOOL(__cdecl* efMonochromatic_func_proc_org)(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static BOOL __cdecl efMosaic_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		inline static BOOL(__cdecl* efMosaic_func_proc_org)(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static BOOL __cdecl efSpecialColorConv_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		inline static BOOL(__cdecl* efSpecialColorConv_func_proc_org)(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static BOOL __cdecl efDiffuseLight_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		inline static BOOL(__cdecl* efDiffuseLight_func_proc_org)(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static BOOL __cdecl efLightEmission_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		inline static BOOL(__cdecl* efLightEmission_func_proc_org)(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static BOOL __cdecl efRadiationalBlur_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		inline static BOOL(__cdecl* efRadiationalBlur_func_proc_org)(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static BOOL __cdecl efLensBlur_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		inline static BOOL(__cdecl* efLensBlur_func_proc_org)(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static BOOL __cdecl efFlip_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		inline static BOOL(__cdecl* efFlip_func_proc_org)(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static BOOL __cdecl efFlash_func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		inline static BOOL(__cdecl* efFlash_func_proc_org)(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);

		inline static const int exedit_check_filter_num = 62;
		inline static BOOL(__cdecl* func_check[exedit_check_filter_num])(ExEdit::Object* leaderobj, int fid) = { NULL };
		inline static BOOL first = TRUE;
		static void func_check_listing();

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.yc_filter_effect";

	public:

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			auto& cursor = GLOBAL::executable_memory_cursor;
			{ // main
				/*
					100498a3 f6450028           test    byte ptr [ebp+00],28
					100498a7 7432               jz      100498db
					100498a9 837c241801         cmp     dword ptr [esp+18],+01
					100498ae 752b               jnz     100498db
					100498b0 8b06               mov     eax,dword ptr [esi]
					100498b2 f6c401             test    ah,01
					100498b5 7524               jnz     100498db
					100498b7 8b4c2410           mov     ecx,dword ptr [esp+10]
					100498bb 41                 inc     ecx
					100498bc 83f90c             cmp     ecx,+0c
					100498bf 7d1a               jnl     100498db
					100498c1 8b542414           mov     edx,dword ptr [esp+14]
					100498c5 8b0a               mov     ecx,dword ptr [edx]
					100498c7 85c9               test    ecx,ecx
					100498c9 7c10               jl      100498db
					100498cb 8b0c8d987c1810     mov     ecx,dword ptr [ecx*4+10187c98]
					100498d2 f60140             test    byte ptr [ecx],40
					100498d5 7404               jz      100498db
					100498d7 0c40               or      al,40
					100498d9 8906               mov     dword ptr [esi],eax
					100498db
					↓
					100498a3 f6450008           test    byte ptr [ebp+00],08
					100498a7 7432               jz      100498db
					100498a9 8d8424ec010000     lea     eax,dword ptr [esp+000001ec]
					100498b0 ff742410           push    dword ptr [esp+10]
					100498b4 ff74241c           push    dword ptr [esp+1c]
					100498b8 ff742424           push    dword ptr [esp+24]
					100498bc 50                 push    eax
					100498bd 56                 push    esi
					100498be e8XxXxXxXx         call    newfunc_stdcall
					100498c3 eb16               jmp     100498db
					100498c5
				*/


				constexpr int vp_begin = 0x498a6;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x498c5 - vp_begin);
				h.store_i8(0x498a6 - vp_begin, '\x08');
				h.store_i16(0x498a9 - vp_begin, '\x8d\x84');
				h.store_i32(0x498ab - vp_begin, '\x24\xec\x01\x00');
				h.store_i32(0x498af - vp_begin, '\x00\xff\x74\x24');
				h.store_i32(0x498b3 - vp_begin, '\x10\xff\x74\x24');
				h.store_i32(0x498b7 - vp_begin, '\x1c\xff\x74\x24');
				h.store_i32(0x498bb - vp_begin, '\x24\x50\x56\xe8');
				h.replaceNearJmp(0x498bf - vp_begin, &yc_check);
				h.store_i16(0x498c3 - vp_begin, '\xeb\x16');
			}
			{ // アルファチャンネルのないシーンを対象にする
				/*
					100835e3 7419               jz      100835fe
					100835e5 6803000013         push    13000003
					100835ea eb17               jmp     10083603
					↓
					100835e3 e9XxXxXxXx         jmp     cursor

					10000000 740c               jz      skip,0c
					10000000 6803000013         push    13000003
					10000000 33db               xor     ebx,ebx
					10000000 e9XxXxXxXx         jmp     ee+83603
					10000000 f7450040000000     test    dword ptr [ebp+00],00000040
					10000000 740c               jz      skip,0c
					10000000 6800000013         push    13000000
					10000000 bb01000000         mov     ebx,00000001
					10000000 ebe6               jmp     back,1a
					10000000 33db               xor     ebx,ebx
					10000000 e9XxXxXxXx         jmp     100835fe


					1008362d c785f4000000
					                   00000000 mov     dword ptr [ebp+000000f4],00000000
					↓
					1008362d 899df4000000       mov     dword ptr [ebp+000000f4],ebx
					10083633 0f1f4000           nop
				*/
				constexpr int vp_begin = 0x835e3;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x83637 - vp_begin);
				h.store_i8(0x835e3 - vp_begin, '\xe9');
				h.replaceNearJmp(0x835e4 - vp_begin, cursor);
				h.store_i16(0x8362d - vp_begin, '\x89\x9d');
				h.store_i32(0x83633 - vp_begin, '\x0f\x1f\x40\x00');

				store_i32(cursor, '\x74\x0c\x68\x03'); cursor += 4;
				store_i32(cursor, '\x00\x00\x13\x33'); cursor += 4;
				store_i16(cursor, '\xdb\xe9'); cursor += 2;
				store_i32(cursor, GLOBAL::exedit_base + 0x83603 - (int)cursor - 4); cursor += 4;
				store_i32(cursor, '\xf7\x45\x00\x40'); cursor += 4;
				store_i32(cursor, '\x00\x00\x00\x74'); cursor += 4;
				store_i32(cursor, '\x0c\x68\x00\x00'); cursor += 4;
				store_i32(cursor, '\x00\x13\xbb\x01'); cursor += 4;
				store_i32(cursor, '\x00\x00\x00\xeb'); cursor += 4;
				store_i32(cursor, '\xe6\x33\xdb\xe9'); cursor += 4;
				store_i32(cursor, GLOBAL::exedit_base + 0x835fe - (int)cursor - 4); cursor += 4;
			}

			{ // 初めからアルファチャンネルを扱わないフィルタ効果
				constexpr int list[] = {
					OFS::ExEdit::efCoordinate_ptr, // 座標
					OFS::ExEdit::efZoom_ptr, // 拡大率
					OFS::ExEdit::efAlpha_ptr, // 透明度
					OFS::ExEdit::efAngle_ptr, // 回転
					OFS::ExEdit::efVibration_ptr, // 振動
					OFS::ExEdit::efExColorConfig_ptr, // 拡張色設定
					OFS::ExEdit::efCameraControlOption_ptr, // カメラ制御オプション
					OFS::ExEdit::efGroupControl_ptr // グループ制御
				};
				for (int i = 0; i < sizeof(list) / sizeof(int); i++) {
					ExEdit::Filter* efp = reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + list[i]);
					efp->flag |= static_cast<decltype(efp->flag)>(0x40);
				}
			}
			{ // 簡単な処理変更が必要なフィルタ効果
				{ // 色調補正
					ExEdit::Filter* efp = reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efColorCorrection_ptr);
					efp->flag |= static_cast<decltype(efp->flag)>(0x40);
					(efColorCorrection_func_proc_org) = (efp->func_proc);
					(efp->func_proc) = (efColorCorrection_func_proc);
				}
				{ // 単色化
					ExEdit::Filter* efp = reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efMonochromatic_ptr);
					efp->flag |= static_cast<decltype(efp->flag)>(0x40);
					(efMonochromatic_func_proc_org) = (efp->func_proc);
					(efp->func_proc) = (efMonochromatic_func_proc);
				}
				{ // モザイク
					ExEdit::Filter* efp = reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efMosaic_ptr);
					efp->flag |= static_cast<decltype(efp->flag)>(0x40);
					(efMosaic_func_proc_org) = (efp->func_proc);
					(efp->func_proc) = (efMosaic_func_proc);
				}
				{ // 特定色域変換
#ifdef PATCH_SWITCH_EXFILTER_SPECIALCOLORCONV
#ifdef PATCH_SWITCH_FAST_SPECIALCOLORCONV
					ExEdit::Filter* efp = reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efSpecialColorConv_ptr);
					efp->flag |= static_cast<decltype(efp->flag)>(0x40);
					(efSpecialColorConv_func_proc_org) = (efp->func_proc);
					(efp->func_proc) = (efSpecialColorConv_func_proc);
#endif // ifdef PATCH_SWITCH_FAST_SPECIALCOLORCONV
#endif // ifdef PATCH_SWITCH_EXFILTER_SPECIALCOLORCONV
				}
				{ // クリッピング
					ExEdit::Filter* efp = reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efClipping_ptr);
					efp->flag |= static_cast<decltype(efp->flag)>(0x40);
					/*
						1001461c 6803000013         push    13000003
						↓
						1001461c e9XxXxXxXx         jmp     cursor

						10000000 b800000013         mov     eax,13000000
						10000000 f786f4000000
										   ffffffff test    dword ptr [esi+000000f4],ffffffff
						10000000 7502               jnz     skip,02
						10000000 0c03               or      al,03
						10000000 50                 push    eax
						10000000 e9XxXxXxXx         jmp     ee+14621
					*/
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x1461c, 5);
					h.store_i8(0, '\xe9');
					h.replaceNearJmp(1, cursor);
					store_i32(cursor, '\xb8\x00\x00\x00'); cursor += 4;
					store_i32(cursor, '\x13\xf7\x86\xf4'); cursor += 4;
					store_i32(cursor, '\x00\x00\x00\xff'); cursor += 4;
					store_i32(cursor, '\xff\xff\xff\x75'); cursor += 4;
					store_i32(cursor, '\x02\x0c\x03\x50'); cursor += 4;
					store_i8(cursor, '\xe9'); cursor++;
					store_i32(cursor, GLOBAL::exedit_base + 0x14621 - (int)cursor - 4); cursor += 4;
				}

				// 以下、別の条件付きなので0x40は付けない
				{ // 拡散光
					ExEdit::Filter* efp = reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efDiffuseLight_ptr);
					(efDiffuseLight_func_proc_org) = (efp->func_proc);
					(efp->func_proc) = (efDiffuseLight_func_proc);
				}
				{ // 発光
					ExEdit::Filter* efp = reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efLightEmission_ptr);
					(efLightEmission_func_proc_org) = (efp->func_proc);
					(efp->func_proc) = (efLightEmission_func_proc);
				}
				{ // 放射ブラー
					ExEdit::Filter* efp = reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efRadiationalBlur_ptr);
					(efRadiationalBlur_func_proc_org) = (efp->func_proc);
					(efp->func_proc) = (efRadiationalBlur_func_proc);
				}
				{ // レンズブラー
					ExEdit::Filter* efp = reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efLensBlur_ptr);
					(efLensBlur_func_proc_org) = (efp->func_proc);
					(efp->func_proc) = (efLensBlur_func_proc);
				}
				{ // 反転
					ExEdit::Filter* efp = reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efFlip_ptr);
					(efFlip_func_proc_org) = (efp->func_proc);
					(efp->func_proc) = (efFlip_func_proc);
				}
				{ // 閃光
#ifdef PATCH_SWITCH_EXFILTER_FLASH
					ExEdit::Filter* efp = reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efFlash_ptr);
					(efFlash_func_proc_org) = (efp->func_proc);
					(efp->func_proc) = (efFlash_func_proc);
#endif // ifdef PATCH_SWITCH_FAST_FLASH
				}
			}
			{ // fastなどに組み込み済み
				{ // シャープ
#ifdef PATCH_SWITCH_FAST_BLUR
#ifdef PATCH_SWITCH_FAST_SHARP
#ifdef PATCH_SWITCH_EXFILTER_SHARP
					if (fast::Sharp.is_enabled()) {
						ExEdit::Filter* efp = reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efSharp_ptr);
						efp->flag |= static_cast<decltype(efp->flag)>(0x40);
					}
#endif // ifdef PATCH_SWITCH_EXFILTER_SHARP
#endif // ifdef PATCH_SWITCH_FAST_SHARP
#endif // ifdef PATCH_SWITCH_FAST_BLUR
				}
				{ // 凸エッジ
#ifdef PATCH_SWITCH_FAST_CONVEXEDGE
					ExEdit::Filter* efp = reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efConvexEdge_ptr);
					efp->flag |= static_cast<decltype(efp->flag)>(0x40);
#endif // ifdef PATCH_SWITCH_FAST_CONVEXEDGE
				}

				// 以下、別の条件付きなので0x40は付けない

				{ // ぼかし
					// func_check_listing()内で初期設定
				}
				{ // 方向ブラー
					// func_check_listing()内で初期設定
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

	} yc_filter_effect;
} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_YC_FILTER_EFFECT
