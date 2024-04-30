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
#ifdef PATCH_SWITCH_EXFILTER_AUDIO_AUF

#include <exedit.hpp>

#include "patch_exfilter.hpp"

namespace patch::exfilter {
	// init at exedit load
	// 音声フィルタプラグインをエフェクトに変換して追加
	inline class Audio_auf_t {

		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "exfilter.audio_auf";

#define AUDIO_AUF_MAX 32
		inline static AviUtl::FilterPlugin af[AUDIO_AUF_MAX];
		inline static int audio_auf_count = 0;
		inline static int start_idx = 0;

		inline static BOOL(__cdecl* func_proc_org[32])(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip);
		static BOOL __cdecl func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
			int idx = -1;
			for (int i = 0; i < audio_auf_count; i++) {
				if (&af[i] == (void*)efp) {
					idx = i;
					break;
				}
			}
			if (idx < 0) return TRUE;
			auto audio_p = efpip->audio_p;
			efpip->audio_p = efpip->audio_data;
			auto ret = func_proc_org[idx](efp, efpip);
			// efpip->audio_data = efpip->audio_p;
			efpip->audio_p = audio_p;

			return ret;
		}
	public:

		void init() {
			enabled_i = enabled;
			OverWriteOnProtectHelper(GLOBAL::exedit_base + 0x39bd4, 2).store_i16(0, '\xeb\x04'); 
		}

		void apend() {
			if (!enabled_i)return;
			auto LoadedFilterTable = (AviUtl::FilterPlugin**)(GLOBAL::exedit_base + OFS::ExEdit::LoadedFilterTable);
			auto LoadedFilterCount_ptr = reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::LoadedFilterCount);
			int LoadedFilterCount = *LoadedFilterCount_ptr;
			start_idx = LoadedFilterCount;
			for (int i = 0; i < start_idx; i++) {
				auto fp = LoadedFilterTable[i];
				if (!has_flag(fp->flag, (AviUtl::FilterPlugin::Flag)ExEdit::Filter::Flag::ExEditFilter) && has_flag(fp->flag, (AviUtl::FilterPlugin::Flag)ExEdit::Filter::Flag::Audio)) {
					af[audio_auf_count] = *fp;
					af[audio_auf_count].flag |= (AviUtl::FilterPlugin::Flag)ExEdit::Filter::Flag::Effect;
					(func_proc_org[audio_auf_count]) = reinterpret_cast<decltype(*func_proc_org)>(af[audio_auf_count].func_proc);
					(af[audio_auf_count].func_proc) = reinterpret_cast<decltype(af->func_proc)>(func_proc);
					//ef[audio_auf_count].track_scale = NULL;
					LoadedFilterTable[LoadedFilterCount] = &af[audio_auf_count];
					audio_auf_count++;
					LoadedFilterCount++;
					if (512 <= LoadedFilterCount || AUDIO_AUF_MAX <= audio_auf_count) break;
				}
			}
			*LoadedFilterCount_ptr = LoadedFilterCount;

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

	} Audio_auf;
} // namespace patch::exfilter
#endif // ifdef PATCH_SWITCH_EXFILTER_AUDIO_AUF
