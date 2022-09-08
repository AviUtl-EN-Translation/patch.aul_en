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

#include "patch_obj_audiofile.hpp"
#ifdef PATCH_SWITCH_OBJ_AUDIOFILE

namespace patch {
    struct TrackInfo {
        int value;
        int value_s;
        int value_e;
        int scale;
        int value_def;
        HWND trackbar_hwnd;
        HWND edit_hwnd;
        HWND updown_hwnd;
        int pos_y;
        int scale2;
    };

    BOOL __cdecl AudioFile_t::set_trackvalue_wrap8f9b5(ExEdit::Filter* efp, int track_s, int track_e, int scale) {
        ExEdit::ObjectFilterIndex ofi = efp->processing;
        int obj_idx = ((int)ofi & 0xffff) - 1;

        int* SettingDialog_ObjIdx = (int*)(GLOBAL::exedit_base + OFS::ExEdit::SettingDialog_ObjIdx);
        if (*SettingDialog_ObjIdx != obj_idx) {
            return 0;
        }

        ExEdit::Object** ObjectArrayPointer = (ExEdit::Object**)(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
        int track_id = (*ObjectArrayPointer)[obj_idx].filter_param[(int)ofi >> 16].track_begin + 1;

        if (scale == 0) {
            scale = 1;
        }

        TrackInfo* trackinfol = (TrackInfo*)(GLOBAL::exedit_base + OFS::ExEdit::TrackInfoArrayLeft);
        TrackInfo* trackinfor = (TrackInfo*)(GLOBAL::exedit_base + OFS::ExEdit::TrackInfoArrayRight);

        trackinfor[track_id].value_s = trackinfol[track_id].value_s = efp->track_s[1];
        trackinfor[track_id].value_e = trackinfol[track_id].value_e = efp->track_e[1];
        trackinfor[track_id].scale2 = trackinfol[track_id].scale2 = scale;

        SendMessageA(trackinfol[track_id].trackbar_hwnd, 0x407,
            0, efp->track_drag_min[1] / trackinfol[track_id].scale2);
        SendMessageA(trackinfol[track_id].trackbar_hwnd, 0x408,
            1, efp->track_drag_max[1] / trackinfol[track_id].scale2);
        SendMessageA(trackinfor[track_id].trackbar_hwnd, 0x407,
            0, efp->track_drag_min[1] / trackinfor[track_id].scale2);
        SendMessageA(trackinfor[track_id].trackbar_hwnd, 0x408,
            1, efp->track_drag_max[1] / trackinfor[track_id].scale2);
        SendMessageA(trackinfol[track_id].updown_hwnd, 0x46f,
            efp->track_s[1] / trackinfol[track_id].scale2, efp->track_e[1] / trackinfol[track_id].scale2);
        SendMessageA(trackinfor[track_id].updown_hwnd, 0x46f,
            efp->track_s[1] / trackinfor[track_id].scale2, efp->track_e[1] / trackinfor[track_id].scale2);

        reinterpret_cast<void(__cdecl*)(int)>(GLOBAL::exedit_base + 0x2c1e0)(track_id);

        return 1;
    }
    
    void __cdecl AudioFile_t::rev_audio_data(ExEdit::FilterProcInfo* efpip) {
        int swap_n = efpip->audio_n / 2;
        if (efpip->audio_ch == 2) {
            int* audio_data = (int*)efpip->audio_data;
            int* audio_data_r = audio_data + efpip->audio_n - 1;
            for (int i = 0; i < swap_n; i++) {
                std::swap(*audio_data, *audio_data_r);
                audio_data++;
                audio_data_r--;
            }
        } else {
            short* audio_data = (short*)efpip->audio_data;
            short* audio_data_r = audio_data + efpip->audio_n - 1;
            for (int i = 0; i < swap_n; i++) {
                std::swap(*audio_data, *audio_data_r);
                audio_data++;
                audio_data_r--;
            }
        }
    }

} // namespace patch
#endif // ifdef PATCH_SWITCH_OBJ_AUDIOFILE
