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

#include "patch_setting_new_project.hpp"

#include <numeric>

#ifdef PATCH_SWITCH_SETTING_NEW_PROJECT
namespace patch {

#define SCALE_MAX 2147483
	BOOL __cdecl setting_new_project_t::exedit_edit_open_wrap(int w, int h, int video_rate, int video_scale, int audio_rate, HWND hwnd, AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp) {
		int gcd = std::gcd(video_rate, video_scale);
		video_rate /= gcd;
		video_scale /= gcd;

		if (SCALE_MAX < video_scale) { // ファレイ数列のアルゴリズムでscaleを減らす
			int ratei = video_rate / video_scale;
			int ratef = video_rate % video_scale;

			int rate0 = 0;
			int scale0 = 1;
			int rate1 = 1;
			int scale1 = 1;
			while (true) {
				int scale = scale0 + scale1;
				if (SCALE_MAX < scale) {
					int64_t cmp0 = (int64_t)ratef * (int64_t)scale0 - (int64_t)rate0 * (int64_t)video_scale;
					int64_t cmp1 = (int64_t)rate1 * (int64_t)video_scale - (int64_t)ratef * (int64_t)scale1;
					if (cmp0 < cmp1) {
						ratef = rate0;
						video_scale = scale0;
					} else if (cmp0 > cmp1 || scale0 > scale1) {
						ratef = rate1;
						video_scale = scale1;
					} else {
						ratef = rate0;
						video_scale = scale0;
					}
					break;
				}
				int rate = rate0 + rate1;
				int64_t cmp = (int64_t)rate * (int64_t)video_scale - (int64_t)ratef * (int64_t)scale;
				if (cmp > 0) { // rate / scale > video_rate / video_scale
					rate1 = rate;
					scale1 = scale;
				} else if (cmp < 0) { // rate / scale < video_rate / video_scale
					rate0 = rate;
					scale0 = scale;
				} else { //  // rate / scale == video_rate / video_scale
					ratef = rate;
					video_scale = scale;
					break;
				}
			}
			video_rate = ratei * video_scale + ratef;
		}

		if (reinterpret_cast<BOOL(__cdecl*)(int, int, int, int, int, HWND, AviUtl::EditHandle*, AviUtl::FilterPlugin*)>(GLOBAL::exedit_base + OFS::ExEdit::edit_open)(w, h, video_rate, video_scale, audio_rate, hwnd, editp, fp)) {
			HWND* aviutl_hwnd = (HWND*)(GLOBAL::exedit_base + OFS::ExEdit::aviutl_hwnd);
			auto clipping_and_resize_fp = (AviUtl::FilterPlugin*)(GLOBAL::aviutl_base + OFS::AviUtl::filter_clipping_and_resize_ptr);
			
			if (clipping_and_resize_fp->exfunc->is_filter_active(clipping_and_resize_fp)) {
				SendMessageA(*aviutl_hwnd, WM_COMMAND, 10001 + clipping_and_resize_fp->menu_index, 0); // フィルタ>クリッピング&リサイズ
			}
			/*
			以下2つに関して参考として
			track_array[0] : 選択されているもの
			track_array[1] : メニューの開始ID
			track_array[2] : メニューの最終ID
			*/
			auto change_size_fp = (AviUtl::FilterPlugin*)(GLOBAL::aviutl_base + OFS::AviUtl::filter_change_size_ptr);
			if (change_size_fp->track_array[0] != 0) { // なし以外
				SendMessageA(*aviutl_hwnd, WM_COMMAND, change_size_fp->track_array[1], 0);
			}
			
			auto change_framerate_fp = (AviUtl::FilterPlugin*)(GLOBAL::aviutl_base + OFS::AviUtl::filter_change_framerate_ptr);
			if (change_framerate_fp->track_array[0] != 0) { // なし以外
				SendMessageA(*aviutl_hwnd, WM_COMMAND, change_framerate_fp->track_array[1], 0);
			}
			
			return TRUE;
		}
		return FALSE;
	}

} // namespace patch
#endif // ifdef PATCH_SWITCH_SETTING_NEW_PROJECT