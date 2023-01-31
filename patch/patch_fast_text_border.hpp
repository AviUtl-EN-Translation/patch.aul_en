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

#ifdef PATCH_SWITCH_FAST_TEXTBORDER

#include <exedit.hpp>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch::fast {
	inline class TextBorder_t {
		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "fast.textborder";

	public:
		static void __cdecl draw_font_border(ExEdit::PixelYCA*, int, int, void*, int, int, int, int, int, int, int, int, int);
		static void short_font_border(int thread_id, int thread_num, void* n1, void* n2);
		static void short_font_border_pre_h_mt(int thread_id, int thread_num, void* n1, void* n2);
		static void short_font_border_pre_v_mt(int thread_id, int thread_num, void* n1, void* n2);
		static void byte_font_border(int thread_id, int thread_num, void* n1, void* n2);
		static void byte_font_border_pre_h_mt(int thread_id, int thread_num, void* n1, void* n2);
		static void byte_font_border_pre_v_mt(int thread_id, int thread_num, void* n1, void* n2);

		static void short_font_border_only(int thread_id, int thread_num, void* n1, void* n2);
		static void byte_font_border_only(int thread_id, int thread_num, void* n1, void* n2);
		inline static void(__cdecl* do_multi_thread_func)(AviUtl::MultiThreadFunc*, BOOL);
		inline static void(__cdecl* blend_yca_normal_func)(ExEdit::PixelYCA*, int, int, int, int);
		
		struct CreateFontBorder {
			void* src; // 1a6bb0
			int dst_h; // 1a6bb4
			int src_line; // 1a6bb8
			int dst_w; // 1a6bbc
			int src_ox; // 1a6bc0
			int src_oy; // 1a6bc4
			int color_cb; // 1a6bc8
			int color_cr; // 1a6bcc
			char textstr[32768]; // 1a6bd0
			int color_y; // 1aebd0
			int _unknown1;
			int _unknown2;
			int _unknown3;
			int _unknown4; // 1aebe0
			int _unknown5;
			int alpha; // 1aebe8
			ExEdit::PixelYCA* dst; // 1aebec
			int dst_ox; // 1aebf0
			int dst_oy; // 1aebf4
			int border; // 1aebf8
			int alpha_rate; // 1aebfc
			int _unknown6; // 1aec00
			int _unknown7;
			int _unknown8;
			int _unknown9;
			int src_h; // 1aec10
			int _unknown10;
			int circle_w[129]; // 1aec18
			int src_w; // 1aee1c
		};

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			do_multi_thread_func = reinterpret_cast<decltype(do_multi_thread_func)>(GLOBAL::exedit_base + OFS::ExEdit::do_multi_thread_func);
			blend_yca_normal_func = reinterpret_cast<decltype(blend_yca_normal_func)>(GLOBAL::exedit_base + OFS::ExEdit::blend_yca_normal_func);
			
			ReplaceNearJmp(GLOBAL::exedit_base + 0x50ea3, &draw_font_border);
			
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

	} textborder;

}
#endif
