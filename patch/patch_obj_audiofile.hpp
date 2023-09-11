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

#include "patch_read_audio.hpp"
#include "patch_playback_pos.hpp"

namespace patch {
	// init at exedit load
	// 音声ファイルの再生速度トラックの最大値2000.0のはずが800.0となってしまう処理があるのを修正
	inline class AudioFile_t {
		
		static BOOL __cdecl set_trackvalue_wrap8f9b5(ExEdit::Filter* efp, int track_s, int track_e, int scale);
		static BOOL __cdecl func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip0);

		static char* __cdecl update_dialog_wrap(ExEdit::Filter* efp, void* exdata);

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "obj_audiofile";

		inline static int mflag = 0;

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
			{   // 動画ファイルと連携を外した後にオブジェクトの長さを触ると1フレームになるのを修正
				
				ReplaceNearJmp(GLOBAL::exedit_base + 0x90269, &update_dialog_wrap);
			}

			if (playback_pos.is_enabled() && read_audio.is_enabled()) { // 逆再生に対応
				OverWriteOnProtectHelper(GLOBAL::exedit_base + 0xba5a0, 4).store_i32(0, &func_proc);

				ExEdit::Filter* efp = (ExEdit::Filter*)(GLOBAL::exedit_base + 0xba570);
				efp->track_s[1] = -20000;
				efp->track_extra->track_drag_min[1] = -4000;
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
