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
#ifdef PATCH_SWITCH_EXFILTER_GLARE

#include <exedit.hpp>

#include "patch_exfilter.hpp"

namespace patch::exfilter {
	// init at exedit load
	// 実装途中のグレアを追加
	inline class Glare_t {

		bool enabled = false;
		bool enabled_i;
		inline static const char key[] = "exfilter.glare";

	public:

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			exfilter.apend_filter(reinterpret_cast<ExEdit::Filter*>(GLOBAL::exedit_base + OFS::ExEdit::efGlare_ptr));
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

	} Glare;
} // namespace patch::exfilter
#endif // ifdef PATCH_SWITCH_EXFILTER_GLARE
