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

#ifdef PATCH_SWITCH_TRA_CHANGE_MODE
#include <memory>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"
#include "config_rw.hpp"


namespace patch {

    // init at exedit load
    // 移動無しから変更するときに右トラックバーの値を左と同じにする
    inline class tra_change_mode_t {
        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "tra_change_mode";
    public:
        void init() {
            enabled_i = enabled;

            if (!enabled_i)return;

            /*
                if (tm_old == 5 || tm_new == 0) { // 移動量指定→任意 、任意→移動無し
                    TrackbarRight_14def0[track_id].track_value = TrackbarLeft_14d4c8[track_id].track_value;
                    update_track(TrackbarRight_14def0 + track_id,1);
                } else if (tm_new == 5) { // 移動量指定以外→移動量指定
                    TrackbarRight_14def0[track_id].track_value = 0;
                    update_track(TrackbarRight_14def0 + track_id,1);
                }
                ↓
                if ((tm_old == 0 && tm_new != 5) || tm_old == 5 || tm_new == 0) { // 移動無し→移動量指定以外 、移動量指定→任意 、任意→移動無し
                    TrackbarRight_14def0[track_id].track_value = TrackbarLeft_14d4c8[track_id].track_value;
                    update_track(TrackbarRight_14def0 + track_id,1);
                } else if (tm_new == 5) { // 移動量指定以外→移動量指定
                    TrackbarRight_14def0[track_id].track_value = 0;
                    update_track(TrackbarRight_14def0 + track_id,1);
                }


                1002c99b 83fb05             cmp     ebx,+05
                1002c99e 754c               jnz     1002c9ec
                ↓
                1002c99b e9XxXxXxXx         jmp     cursor

                10000000 83fb05             cmp     ebx,+05
                10000000 0f84XXXX           jz      ee+2c9a0
                10000000 85ff               test    edi,edi
                10000000 0f84XXXX           jz      ee+2c9c4
                10000000 5f                 pop     edi
                10000000 5e                 pop     esi
                10000000 5b                 pop     ebx
                10000000 c3                 ret
            */

            auto& cursor = GLOBAL::executable_memory_cursor;

            OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x2c99b, 5);
            h.store_i8(0, '\xe9');
            h.replaceNearJmp(1, cursor);

            store_i32(cursor, '\x83\xfb\x05\x0f'); cursor += 4;
            store_i8(cursor, '\x84'); cursor++;
            store_i32(cursor, GLOBAL::exedit_base + 0x2c9a0 - (int)cursor - 4); cursor += 4;
            store_i32(cursor, '\x85\xff\x0f\x84'); cursor += 4;
            store_i32(cursor, GLOBAL::exedit_base + 0x2c9c4 - (int)cursor - 4); cursor += 4;
            store_i32(cursor, '\x5f\x5e\x5b\xc3'); cursor += 4;

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
    } tra_change_mode;
} // namespace patch

#endif // ifdef PATCH_SWITCH_TRA_CHANGE_MODE
