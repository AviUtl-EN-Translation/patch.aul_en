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
#ifdef PATCH_SWITCH_OBJ_AUDIOFILE
#include <exedit.hpp>
#include "util_magic.hpp"
#include "offset_address.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	// 音声ファイルの再生速度トラックの最大値2000.0のはずが800.0となってしまう処理があるのを修正
	inline class AudioFile_t {
		
		static BOOL __cdecl set_trackvalue_wrap8f9b5(ExEdit::Filter* efp, int track_s, int track_e, int scale);
		static void __cdecl rev_audio_data(ExEdit::FilterProcInfo* efpip);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "obj_audiofile";

		int mflag = 0;

	public:

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			{   // 音声ファイルの再生速度トラックの最大値2000.0のはずが800.0となってしまう処理があるのを修正
				/*
					1008f9b5 6a01               push    +01
					1008f9b7 51                 push    ecx
					1008f9b8 ff5050             call    dword ptr [eax+50] ; set_trackvalue
					1008f9bb 8b5c242c           mov     ebx,dword ptr [esp+2c]
					1008f9bf 83c414             add     esp,+14

					↓

					1008f9b5 57                 push    edi ; efp
					1008f9b6 e8XxXxXxXx         call
					1008f9bb 8b5c2428           mov     ebx,dword ptr [esp+28]
					1008f9bf 83c410             add     esp,+10
				*/

				OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x8f9b5, 13);
				h.store_i16(0, '\x57\xe8');
				h.replaceNearJmp(2, &set_trackvalue_wrap8f9b5);
				h.store_i32(9, '\x28\x83\xc4\x10');
			}
			{   // 再生速度トラックの下限値を引き下げる
				ExEdit::Filter* efp = (ExEdit::Filter*)(GLOBAL::exedit_base + 0xba570);
				efp->track_s[1] = -20000;
				efp->track_extra->track_drag_min[1] = -4000;

				auto& cursor = GLOBAL::executable_memory_cursor;
				{
					/*
						1008f5bb 3d10270000         cmp     eax,10000
						1008f5c0 89442470           mov     dword ptr [esp+70],eax
						1008f5c4 7d14               jnl     1008f5da
						↓
						1008f5bb 909090             nop
						1008f5be 85c0               test    eax,eax
						1008f5c0 89442470           mov     dword ptr [esp+70],eax
						1008f5c4 7514               jnz     1008f5da
					*/
					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x8f5bb, 10);
					h.store_i8(0, '\x90');
					h.store_i32(1, '\x90\x90\x85\xc0');
					h.store_i8(9, '\x75');
				}
				{
					/*
						1008f6a3 8b4c2410           mov     ecx,dword ptr [esp+10]
						1008f6a7 89442470           mov     dword ptr [esp+70],eax
						↓
						1008f6a3 909090             nop
						1008f6a6 e8XxXxXxXx         call    executable_memory_cursor

						if (audio_rate < 0){
						  audio_rate = -audio_rate;
						  mflag = 1;
						}
					*/
					static const char code_put[] =
						"\x33\xc9"                 // xor     ecx,ecx
						"\x85\xc0"                 // test    eax,eax
						"\x7d\x03"                 // jnl     skip,+03
						"\xf7\xd8"                 // neg     eax
						"\x41"                     // inc     ecx
						"\x89\x0dXXXX"             // mov     dword ptr [mflag],ecx
						"\x8b\x4c\x24\x14"         // mov     ecx,dword ptr [esp+14]
						"\x89\x44\x24\x74"         // mov     dword ptr [esp+74],eax
						"\xc3"                     // ret
						;

					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x8f6a3, 8);
					h.store_i32(0, '\x90\x90\x90\xe8');
					h.replaceNearJmp(4, cursor);

					memcpy(cursor, code_put, sizeof(code_put) - 1);
					store_i32(cursor + 11, &mflag);
					cursor += sizeof(code_put) - 1;
				}
				{
					/*
						exdata->current_pos = start + efpip->audio_n;
						1008f85e 8b4f30             mov     ecx,dword ptr [edi+30] ; ecx = efpip->audio_n
						1008f861 03c8               add     ecx,eax
						1008f863 898a14010000       mov     dword ptr [edx+00000114],ecx ; exdata->current_pos = ecx


						1008f85e 8b4f30             mov     ecx,dword ptr [edi+30]
						1008f861 03c8               add     ecx,eax
						↓
						1008f85e e8XxXxXxXx         call    executable_memory_cursor

						if (mflag){
						  start -= efpip->audio_n;
						  exdata->current_pos = start;
						} else {
						  exdata->current_pos = start + efpip->audio_n;
						}
					*/
					static const char code_put[] =
						"\x8b\x1dXXXX"             // mov     ebx,dword ptr [mflag]
						"\x8b\x4f\x30"             // mov     ecx,dword ptr [edi+30]
						"\x85\xdb"                 // test    ebx,ebx
						"\x75\x03"                 // jnz     skip,+03
						"\x03\xc8"                 // add     ecx,eax
						"\xc3"                     // ret
						"\x2b\xc1"                 // sub     eax,ecx
						"\x8b\xc8"                 // mov     ecx,eax
						"\xc3"                     // ret
						;

					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x8f85e, 5);
					h.store_i8(0, '\xe8');
					h.replaceNearJmp(1, cursor);

					memcpy(cursor, code_put, sizeof(code_put) - 1);
					store_i32(cursor + 2, &mflag);
					cursor += sizeof(code_put) - 1;
				}
				{
					/*
						int length = end_pos - start_pos;
						if (length <= 0) {
							return;
						}
						1008f871 8b4c2418           mov     ecx,dword ptr [esp+18] ; end_pos
						1008f875 8b5c2420           mov     ebx,dword ptr [esp+20] ; start_pos
						1008f879 2bcb               sub     ecx,ebx
						1008f87b 85c9               test    ecx,ecx
						1008f87d 0f8ec1000000       jng     1008f944
						1008f883


						1008f87d 0f8ec1000000       jng     1008f944
						↓
						1008f87d 0f8eXxXxXxXx       jng     executable_memory_cursor

						int length = end_pos - start_pos;
						if (length <= 0) {
							if (length == 0) return;
							swap(end_pos, start_pos);
							length = -length;
							mflag = 1;
						}

					*/
					static const char code_put[] =
						"\x0f\x84XXXX"                 //   jz      exedit_base + 0x8f944
						"\xf7\xd9"                     //   neg     ecx
						"\x8b\x54\x24\x18"             //   mov     edx,dword ptr [esp+18]
						"\x89\x5c\x24\x18"             //   mov     dword ptr [esp+18],ebx
						"\x89\x54\x24\x20"             //   mov     dword ptr [esp+20],edx
						"\x8b\xda"                     //   mov     ebx,edx
						"\xc7\x05XXXX\x01\x00\x00\x00" //   mov     dword ptr [mflag],00000001
						"\xe9"                         //   jmp     exedit_base + 0x8f883
						;

					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x8f87f, 4);
					h.replaceNearJmp(0, cursor);

					memcpy(cursor, code_put, sizeof(code_put) - 1);
					ReplaceNearJmp((int)cursor + 2, (void*)(GLOBAL::exedit_base + 0x8f944));
					store_i32(cursor + 24, &mflag);
					ReplaceNearJmp((int)cursor + 33, (void*)(GLOBAL::exedit_base + 0x8f883));
					cursor += sizeof(code_put) - 1 + 4;
				}
				{
					/*
						1008f937 5f                 pop     edi
						1008f938 5e                 pop     esi
						1008f939 5d                 pop     ebp
						1008f93a b801000000         mov     eax,00000001
						↓
						1008f937 e8XxXxXxXx         call    executable_memory_cursor
						1008f93c 5f                 pop     edi
						1008f93d 5e                 pop     esi
						1008f93e 5d                 pop     ebp

						if (mflag) {
							rev_audio_data(efpip);
						}
						return_value = 1;

					*/
					static const char code_put[] =
						"\x8b\x1dXXXX"             // mov     ebx,dword ptr [mflag]
						"\x85\xdb"                 // test    ebx,ebx
						"\x74\x09"                 // jz      skip,+09
						"\x57"                     // push    edi ; efpip
						"\xe8XXXX"                 // call    rev_audio_data()
						"\x83\xc4\x04"             // add     esp,+04
						"\xb8\x01\x00\x00\x00"     // mov     eax,00000001
						"\xc3"                     // ret
						;

					OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x8f937, 8);
					h.store_i8(0, '\xe8');
					h.store_i32(4, '\x00\x5f\x5e\x5d');
					h.replaceNearJmp(1, cursor);

					memcpy(cursor, code_put, sizeof(code_put) - 1);
					store_i32(cursor + 2, &mflag);
					ReplaceNearJmp((int)cursor + 12, &rev_audio_data);
					cursor += sizeof(code_put) - 1;
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

	} AudioFile;
} // namespace patch
#endif // ifdef PATCH_SWITCH_OBJ_AUDIOFILE
