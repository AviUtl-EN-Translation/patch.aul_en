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

#ifdef PATCH_SWITCH_SCENE_VRAM

#include <chrono>

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {

	// init at exedit load
	// 主にアルファチャンネルの無いシーンを呼び出す時の問題点を修正
	/*
	・元の関数の問題点
	領域確保がSYSINFO.vram_w*(SYSINFO.vram_h+2)*6+16となっている
	SYSINFOのバグかvram_hはmax_hと同じ値となっているためvram_h+8しなければ本来の領域分確保できない。 ※プラグインへの影響が強く懸念されるためSYSINFO側は変えられない
	それに加え、FILTER.ycp_editの開始位置はbuf+(SYSINFO.vram_w*4+4)*sizeof(PIXEL_YC)+8としているが（画像外の上下左右に4pixずつ処理用の領域+128bitアライメント用に8byte がある状態）
	元の関数ではbuf+SYSINFO.vram_w*sizeof(PIXEL_YC)としている（画像外の上下に1pix、右に8pixの処理用の領域 という状態）
	本体の一部フィルタではycp_edit[-3,-3]、レンズブラーではycp_edit[-1,-1]の部分を使用するため例外となる
	さらに、buf[0～2]にはシーンサイズなどの情報を入れてしまっており、フィルタ次第で書き換えられてしまう

	・対処内容
	Level10：ycp_editの開始位置を4*sizeof(PIXEL_YC)+8増やす（処理用の領域が右に8pix→左右に4pixずつ、デメリットなしで[-1,-1]アクセス例外は直る）
	 領域確保は指定サイズより+16大きくされているので、その部分にシーンサイズなどの情報を入れるようにする（デメリットなし、YCAの方の関数でも直す）
	Level30：領域確保サイズを増やし、上下3pixずつの領域が出来るようにする（キャッシュ占有量がSYSINFO.vram_w*4増えるため画像データのキャッシュ数に関するトラブルの発生率が少し上昇、[-3,-3]アクセス例外が直る）
	Level40：領域確保サイズを増やし、上下4pixずつの領域が出来るようにする（キャッシュ占有量がSYSINFO.vram_w*6増える同上、本来の領域になり[-4,-4]アクセス例外が直る）
	*/
	inline class scene_vram_t {

		static void* __cdecl GetOrCreateSceneBufYC_fix(ExEdit::ObjectFilterIndex ofi, int w, int h, int v_func_id, int flag);
		
		inline static auto patch_level = 64;
		inline static const char key_threshold_time[] = "patch_level";

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "scene_vram";

	public:

		void init() {
			enabled_i = enabled;

			if (!enabled_i)return;

			{   // scene_obj
				constexpr int vp_begin = 0x2a770;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x2a775 - vp_begin);
				h.store_i8(0x2a770 - vp_begin, '\xe9');
				h.replaceNearJmp(0x2a771 - vp_begin, &GetOrCreateSceneBufYC_fix);
			}

			{
				/* アルファチャンネル有りの方のシーンサイズなどの情報の場所を改善する
				  1002a867 8b0d685c1310       mov     ecx,dword ptr [10135c68]
				  1002a86d 8b442420           mov     eax,dword ptr [esp+20]
				  1002a871 41                 inc     ecx
				  1002a872 8bf7               mov     esi,edi
				  1002a874 c1e105             shl     ecx,05
				  1002a877 03f9               add     edi,ecx
				  1002a879 85c0               test    eax,eax
				  ↓
				  1002a867 8b0d685c1310       mov     ecx,dword ptr [10135c68]
				  1002a86d 8b442420           mov     eax,dword ptr [esp+20]
				  1002a871 41                 inc     ecx
				  1002a872 e8XxXxXxXx         call    cursor
				  1002a877 03f9               add     edi,ecx
				  1002a879 85c0               test    eax,eax

				  10000000 8d51ff             mov     edx,ecx-01
				  10000000 0faf15XxXxXxXx     imul    edx,dword ptr [ee+135c64]
				  10000000 8d34d7             mov     esi,edi+edx*8
				  10000000 83c604             add     esi,+04
				  10000000 c1e105             shl     ecx,05
				  10000000 c3                 ret

				  (int)cache_ptr += (yca_vram_w * yca_vram_h * sizeof(ExEdit::PixelYCA) + 16 - 12);
				*/
				
				auto& cursor = GLOBAL::executable_memory_cursor;

				constexpr int vp_begin = 0x2a872;
				OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x2a877 - vp_begin);
				h.store_i8(0x2a872 - vp_begin, '\xe8');
				h.replaceNearJmp(0x2a873 - vp_begin, cursor);

				store_i32(cursor, '\x8d\x51\xff\x0f'); cursor += 4;
				store_i16(cursor, '\xaf\x15'); cursor += 2;
				store_i32(cursor, GLOBAL::exedit_base + OFS::ExEdit::yca_vram_h); cursor += 4;
				store_i32(cursor, '\x8d\x34\xd7\x83'); cursor += 4;
				store_i32(cursor, '\xc6\x04\xc1\xe1'); cursor += 4;
				store_i16(cursor, '\x05\xc3'); cursor += 2;
				
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

		void config_load(ConfigReader& cr) {
			cr.regist(key_threshold_time, [this](json_value_s* value) {
				ConfigReader::load_variable(value, patch_level);
				});
		}

		void config_store(ConfigWriter& cw) {
			cw.append(key_threshold_time, patch_level);
		}

	} scene_vram;
} // namespace patch

#endif // ifdef PATCH_SWITCH_SCENE_VRAM