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

#include "patch_exfilter_flash.hpp"
#ifdef PATCH_SWITCH_EXFILTER_FLASH

#include "patch_fast_cl.hpp"
#include "patch_fast_flash.hpp"

namespace patch::exfilter {
    BOOL Flash_t::func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        if (efp->track[0] <= 0 || efpip->scene_w <= 0 || efpip->scene_h <= 0) {
            return TRUE;
        }
        auto flash = reinterpret_cast<efFlash_var*>(GLOBAL::exedit_base + OFS::ExEdit::efFlash_var_ptr);
        flash->intensity = (efp->track[0] << 9) / 125;
        flash->range = efp->track[3]; // default : 750
        flash->r_intensity = max(0, 0x1000 - flash->intensity);
        flash->cx = (efpip->scene_w >> 1) + efp->track[1];
        flash->cy = (efpip->scene_h >> 1) + efp->track[2];
        int pixel_range = (std::max)({ abs(flash->cy), abs(flash->cx), abs(flash->cx - efpip->scene_w), abs(flash->cy - efpip->scene_h) });
        pixel_range = min(pixel_range, (int)sqrt((double)(efpip->scene_h * efpip->scene_h + efpip->scene_w * efpip->scene_w)));
        if (has_flag(efpip->flag, ExEdit::FilterProcInfo::Flag::fast_preview)) {
            pixel_range = min(50, pixel_range);
        }
        flash->pixel_range = pixel_range * (1500 - flash->range) / 1000;

        int color = *(int*)&(reinterpret_cast<ExEdit::Exdata::efFlash*>(efp->exdata_ptr)->color);
        if (color & 0x1000000) {
			if (fast::Flash.is_enabled_i()) {
				try {
					const auto src_size = efpip->scene_line * efpip->scene_h * sizeof(ExEdit::PixelYC);
						cl::Buffer clmem_src(patch::fast::cl.context, CL_MEM_READ_ONLY, src_size);
						patch::fast::cl.queue.enqueueWriteBuffer(clmem_src, CL_TRUE, 0, src_size, efpip->frame_edit);

						cl::Buffer clmem_dst(patch::fast::cl.context, CL_MEM_WRITE_ONLY, src_size);

						cl::Kernel kernel;
						kernel = patch::fast::cl.readyKernel(
							"FlashFilter",
							clmem_dst,
							clmem_src,
							efpip->scene_w,
							efpip->scene_h,
							efpip->scene_line,
							flash->cx,
							flash->cy,
							flash->range,
							flash->pixel_range,
							flash->r_intensity
						);
					patch::fast::cl.queue.enqueueNDRangeKernel(kernel, { 0,0 }, { (size_t)efpip->scene_w, (size_t)efpip->scene_h });

					patch::fast::cl.queue.enqueueReadBuffer(clmem_dst, CL_TRUE, 0, src_size, efpip->frame_temp);
				} catch (const cl::Error& err) {
					debug_log("OpenCL Error\n({}) {}", err.err(), err.what());
					efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&mt, efp, efpip);
				}
			} else {
				efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&mt, efp, efpip);
			}
		} else {
            reinterpret_cast<void(__cdecl*)(short*, short*, short*, int)>(GLOBAL::exedit_base + OFS::ExEdit::rgb2yc)(&flash->color_y, &flash->color_cb, &flash->color_cr, color & 0xffffff);
            efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)&mt_color, efp, efpip);
        }
        std::swap(efpip->frame_temp, efpip->frame_edit);
		reinterpret_cast<BOOL(__cdecl*)(ExEdit::Filter*, ExEdit::FilterProcInfo*, int)>(GLOBAL::exedit_base + OFS::ExEdit::drawfilter_func_proc)(efp, efpip, 0x80d);
        return TRUE;
    }
	void __cdecl Flash_t::mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
		auto flash = reinterpret_cast<efFlash_var*>(GLOBAL::exedit_base + OFS::ExEdit::efFlash_var_ptr);
		for (int y = thread_id; y < efpip->scene_h; y += thread_num) {
			auto dst = (ExEdit::PixelYC*)efpip->frame_temp + y * efpip->scene_line;
			auto src = (ExEdit::PixelYC*)efpip->frame_edit + y * efpip->scene_line;
			for (int x = 0; x < efpip->scene_w; x++) {
				int cx = flash->cx - x, cy = flash->cy - y;
				int c_dist_times8 = (int)round(sqrt((double)(cx * cx + cy * cy)) * 8.0);
				int range = flash->range * c_dist_times8 / 1000;
				if (flash->pixel_range < c_dist_times8) {
					range = flash->pixel_range * flash->range / 1000;
					c_dist_times8 = flash->pixel_range;
				} else if (8 < c_dist_times8) {
					c_dist_times8 *= 8;
					range *= 8;
				} else if (4 < c_dist_times8) {
					c_dist_times8 *= 4;
					range *= 4;
				} else if (2 < c_dist_times8) {
					c_dist_times8 *= 2;
					range *= 2;
				}
				if (c_dist_times8 < 2 || range < 2) {
					c_dist_times8 = range = 1;
				}
				int sum_y = 0, sum_cb = 0, sum_cr = 0;
				for (int i = 0; i < range; i++) {
					int u = x + i * cx / c_dist_times8;
					int v = y + i * cy / c_dist_times8;

					if (0 <= u && 0 <= v && u < efpip->scene_w && v < efpip->scene_h) {
						auto pix = (ExEdit::PixelYC*)efpip->frame_edit + u + v * efpip->scene_line;
						sum_y += pix->y;
						sum_cb += pix->cb;
						sum_cr += pix->cr;
					}
				}
				sum_y /= range;
				sum_cb /= range;
				sum_cr /= range;

				short ya = sum_y - flash->r_intensity;
				if (0 < ya) {
					dst->y = src->y + ya;
					dst->cb = src->cb + sum_cb - flash->intensity * sum_cb / sum_y;
					dst->cr = src->cr + sum_cr - flash->intensity * sum_cr / sum_y;
				} else {
					dst->y = src->y;
					dst->cb = src->cb;
					dst->cr = src->cr;
				}
				src++;
				dst++;
			}
		}
	}
    void __cdecl Flash_t::mt_color(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
		auto flash = reinterpret_cast<efFlash_var*>(GLOBAL::exedit_base + OFS::ExEdit::efFlash_var_ptr);
		int ye = (thread_id + 1) * efpip->scene_h / thread_num;
		for (int y = thread_id * efpip->scene_h / thread_num; y < ye; y++) {
			auto dst = (ExEdit::PixelYC*)efpip->frame_temp + y * efpip->scene_line;
			auto src = (ExEdit::PixelYC*)efpip->frame_edit + y * efpip->scene_line;
			for (int x = 0; x < efpip->scene_w; x++) {
				int cx = flash->cx - x, cy = flash->cy - y;
				int c_dist_times8 = (int)round(sqrt((double)(cx * cx + cy * cy)) * 8.0);
				int range = flash->range * c_dist_times8 / 1000;
				if (flash->pixel_range < c_dist_times8) {
					range = flash->pixel_range * flash->range / 1000;
					c_dist_times8 = flash->pixel_range;
				} else if (8 < c_dist_times8) {
					c_dist_times8 *= 8;
					range *= 8;
				} else if (4 < c_dist_times8) {
					c_dist_times8 *= 4;
					range *= 4;
				} else if (2 < c_dist_times8) {
					c_dist_times8 *= 2;
					range *= 2;
				}
				if (c_dist_times8 < 2 || range < 2) {
					c_dist_times8 = range = 1;
				}
				int xr;
				if (cx < 0) {
					xr = (x + 1) * c_dist_times8 / (-cx) + 1;
				} else if (0 < cx) {
					xr = ((efpip->scene_w - x) * c_dist_times8 - 1) / cx + 1;
				} else {
					xr = range;
				}
				int yr;
				if (cy < 0) {
					yr = (y + 1) * c_dist_times8 / (-cy) + 1;
				} else if (0 < cy) {
					yr = ((efpip->scene_h - y) * c_dist_times8 - 1) / cy + 1;
				} else {
					yr = range;
				}
				int sum_a = (std::min)({ xr, yr, range });
				int col_y = flash->color_y * sum_a / range;

				short ya = col_y - flash->r_intensity;
				if (0 < ya) {
					int col_cb = flash->color_cb * sum_a / range;
					int col_cr = flash->color_cr * sum_a / range;
					dst->y = src->y + ya;
					dst->cb = src->cb + col_cb - flash->r_intensity * col_cb / col_y;
					dst->cr = src->cr + col_cr - flash->r_intensity * col_cr / col_y;
				} else {
					dst->y = src->y;
					dst->cb = src->cb;
					dst->cr = src->cr;
				}
				src++;
				dst++;
			}
		}
    }


	BOOL __cdecl Flash_t::func_WndProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, AviUtl::EditHandle* editp, ExEdit::Filter* efp) {
		if (reinterpret_cast<BOOL(__cdecl*)(HWND, UINT, WPARAM, LPARAM, AviUtl::EditHandle*, ExEdit::Filter*)>(GLOBAL::exedit_base + 0x4f3f0)(hwnd, message, wparam, lparam, editp, efp)) { // flash_wndproc
			return TRUE;
		}
		return reinterpret_cast<BOOL(__cdecl*)(HWND, UINT, WPARAM, LPARAM, AviUtl::EditHandle*, ExEdit::Filter*)>(GLOBAL::exedit_base + OFS::ExEdit::drawfilter_func_WndProc)(hwnd, message, wparam, lparam, editp, efp);
	}

} // namespace patch::exfilter
#endif // ifdef PATCH_SWITCH_EXFILTER_FLASH
