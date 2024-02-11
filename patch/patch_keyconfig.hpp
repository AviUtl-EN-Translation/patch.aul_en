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
#ifdef PATCH_SWITCH_KEYCONFIG
#include <exedit.hpp>
#include "util_magic.hpp"
#include "global.hpp"
#include "config_rw.hpp"

namespace patch {
	// init at patch load
	// 言語リソースプラグインなどでメニューの「編集」の部分が変わるとaviutl.keyのプラグイン設定部分が正しく読み込めなくなるのを修正
	inline class KeyConfig_t {

		static int __stdcall lstrcmpA_wrap(char* menu_data_str, char* keyfile_data_str);


		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "keyconfig";


	public:

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			{
				/*
                    004331c2 ff1550f24600       call    lstrcmpA

					↓

					004331c2 90                 nop
					004331c3 e8XxXxXxXx         call    new_function
				*/

				OverWriteOnProtectHelper h(GLOBAL::aviutl_base + 0x331c2, 6);
				h.store_i16(0, '\x90\xe8');
				h.replaceNearJmp(2, &lstrcmpA_wrap);
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

	} KeyConfig;

	inline class ApendKey_t {

		static int __stdcall lstrcmpA_wrap(char* menu_data_str, char* keyfile_data_str);


		bool enabled = true;
		bool enabled_i;
		inline static const char key[] = "apendkey";

#define DEFAULT_KEY_N 91

		inline static const int apend_n = 16;
		// inline static char* key_short_str_list[DEFAULT_KEY_N + 16]; // 元の領域+OFS::AviUtl::key_code_listの領域でchar*[183]まで使える
		inline static char* key_long_str_list[DEFAULT_KEY_N + apend_n];
		inline static int key_code_list[DEFAULT_KEY_N + apend_n];

		inline static char str_F13to24[12][4] = {"F13", "F14", "F15", "F16", "F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24"};

		inline static char str_Apps[] = "Apps";
		inline static char str_Cvt[] = "Cvt";
		inline static char str_Convert[] = "Convert";
		inline static char str_NCvt[] = "NCvt";
		inline static char str_NonConvert[] = "NonConvert";
		inline static char str_Pause[] = "Pause";
	public:

		void init() {
			enabled_i = enabled;
			if (!enabled_i)return;

			char** def_key_short_str_list = reinterpret_cast<char**>(GLOBAL::aviutl_base + OFS::AviUtl::key_short_str_list);
			char** def_key_long_str_list = reinterpret_cast<char**>(GLOBAL::aviutl_base + OFS::AviUtl::key_long_str_list);
			int* def_key_code_ptr = reinterpret_cast<int*>(GLOBAL::aviutl_base + OFS::AviUtl::key_code_list);

			constexpr int insert_ofs = 75;

			memcpy(key_long_str_list, def_key_long_str_list, insert_ofs * sizeof(char*));
			memcpy(key_code_list, def_key_code_ptr, insert_ofs * sizeof(int));

			memcpy(&key_long_str_list[insert_ofs + apend_n], &def_key_long_str_list[insert_ofs], (DEFAULT_KEY_N - insert_ofs) * sizeof(char*));
			memcpy(&key_code_list[insert_ofs + apend_n], &def_key_code_ptr[insert_ofs], (DEFAULT_KEY_N - insert_ofs) * sizeof(int));
			OverWriteOnProtectHelper h(def_key_short_str_list, (DEFAULT_KEY_N + apend_n) * sizeof(char*));
			memmove(&def_key_short_str_list[insert_ofs + apend_n], &def_key_short_str_list[insert_ofs], (DEFAULT_KEY_N - insert_ofs) * sizeof(char*));
			
			int i;
			for (i = 0; i < 12; i++) {
				def_key_short_str_list[i + insert_ofs] = str_F13to24[i];
				key_long_str_list[i + insert_ofs] = str_F13to24[i];
				key_code_list[i + insert_ofs] = VK_F13 + i;
			}
			def_key_short_str_list[i + insert_ofs] = str_Apps;
			key_long_str_list[i + insert_ofs] = str_Apps;
			key_code_list[i + insert_ofs] = VK_APPS;
			i++;
			def_key_short_str_list[i + insert_ofs] = str_Cvt;
			key_long_str_list[i + insert_ofs] = str_Convert;
			key_code_list[i + insert_ofs] = VK_CONVERT;
			i++;
			def_key_short_str_list[i + insert_ofs] = str_NCvt;
			key_long_str_list[i + insert_ofs] = str_NonConvert;
			key_code_list[i + insert_ofs] = VK_NONCONVERT;
			i++;
			def_key_short_str_list[i + insert_ofs] = str_Pause;
			key_long_str_list[i + insert_ofs] = str_Pause;
			key_code_list[i + insert_ofs] = VK_PAUSE;
			i++;

			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x2347d, 4).store_i32(0, &key_code_list[1]);
			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x33797, 4).store_i32(0, &key_code_list[1]);
			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x33d8c, 4).store_i32(0, &key_code_list[1]);
			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x33ec0, 4).store_i32(0, key_code_list);

			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x3419c, 4).store_i32(0, key_long_str_list);

			/* key_short_str_listは新たな領域を確保 F13~24のみ実装
			for (int i = 0; i < 75; i++) {
				key_short_str_list[i] = def_key_short_str_list[i];
				key_long_str_list[i] = def_key_long_str_list[i];
				key_code_list[i] = def_key_code_ptr[i];
			}
			
			for (int i = 0; i < 12; i++) {
				key_short_str_list[n] = str_F13to24[i];
				key_long_str_list[n] = str_F13to24[i];
				key_code_list[n] = VK_F13 + i;
				n++;
			}

			for (int i = 75; i < 91; i++) {
				key_short_str_list[n] = def_key_short_str_list[i];
				key_long_str_list[n] = def_key_long_str_list[i];
				key_code_list[n] = def_key_code_ptr[i];
				n++;
			}

			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x2347d, 4).store_i32(0, &key_code_list[1]);
			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x33797, 4).store_i32(0, &key_code_list[1]);
			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x33d8c, 4).store_i32(0, &key_code_list[1]);
			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x33ec0, 4).store_i32(0, key_code_list);

			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x337ed, 4).store_i32(0, key_short_str_list);
			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x338bc, 4).store_i32(0, key_short_str_list);
			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x33f4b, 4).store_i32(0, key_short_str_list);

			OverWriteOnProtectHelper(GLOBAL::aviutl_base + 0x3419c, 4).store_i32(0, key_long_str_list);
			*/
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

	} ApendKey;


} // namespace patch
#endif // ifdef PATCH_SWITCH_KEYCONFIG
