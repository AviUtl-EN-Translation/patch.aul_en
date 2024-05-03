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

#include "patch_fast_divideobject.hpp"
#ifdef PATCH_SWITCH_FAST_DIVIDEOBJECT

#include "patch_shared_cache.hpp"


namespace patch::fast {

    BOOL __cdecl DivideObject_t::func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        if (efpip->xf4 == 0) {
            return func_proc_org(efp, efpip);
        }
        auto dvo = reinterpret_cast<efDivideObject_var*>(GLOBAL::exedit_base + OFS::ExEdit::efDivideObject_var_ptr);
        if (dvo->running != 0) {
            return TRUE;
        }
        void(__cdecl * memcpy_rect)(void* dst, void* src, int w, int h, int dst_line, int src_line) = reinterpret_cast<decltype(memcpy_rect)>(GLOBAL::exedit_base + OFS::ExEdit::memcpy_rect);
        ExEdit::PixelYC* (__cdecl * GetOrCreateCache)(ExEdit::ObjectFilterIndex ofi, int w, int h, int bitcount, int v_func_id, int* old_cache_exists);
        if (SharedCache.is_enabled_i()) {
            (GetOrCreateCache) = reinterpret_cast<decltype(GetOrCreateCache)>((SharedCache_t::GetOrCreateSharedCache));
        } else {
            (GetOrCreateCache) = reinterpret_cast<decltype(GetOrCreateCache)>(GLOBAL::exedit_base + OFS::ExEdit::GetOrCreateCache);
        }

        int obj_h = efpip->obj_h;
        int hor_n = efp->track[0];
        int ver_n = efp->track[1];
        int obj_num = efpip->obj_num;
        auto obj_data = efpip->obj_data;
        int obj_w = efpip->obj_w;
        int obj_index = efpip->obj_index;
        int cache_w = obj_w + (obj_w & 1);
#define BYTE2BIT 8
        ExEdit::PixelYC* cache;
        int old_cache_exists;
        cache = GetOrCreateCache(efp->processing, cache_w, obj_h, sizeof(*cache) * BYTE2BIT, efpip->v_func_idx, &old_cache_exists);
        if (cache == NULL) {
            return TRUE;
        }
        int vram_line_size = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yc_vram_line_size);
        memcpy_rect(cache, efpip->obj_edit, cache_w * sizeof(*cache), obj_h, cache_w * sizeof(*cache), vram_line_size);
        int y = 0;
        int h_idx = obj_h;
        for (int do_y = 0; do_y < ver_n; do_y++) {
            int do_h = h_idx / ver_n - y;
            h_idx += obj_h;
            if (0 < do_h) {
                int x = 0;
                int w_idx = obj_w;
                for (int do_x = 0; do_x < hor_n; do_x++) {
                    int do_w = w_idx / hor_n - x;
                    w_idx += obj_w;
                    if (0 < do_w) {
                        cache = GetOrCreateCache(efp->processing, cache_w, obj_h, sizeof(*cache) * BYTE2BIT, efpip->v_func_idx, &old_cache_exists);
                        if (cache == NULL || old_cache_exists == 0) {
                            return FALSE;
                        }
                        memcpy_rect(efpip->obj_edit, cache + y * cache_w + x, do_w * sizeof(*cache), do_h, vram_line_size, cache_w * sizeof(*cache));
                        efpip->obj_data.ox += (do_w - obj_w) * 0x800 + x * 0x1000;
                        efpip->obj_data.oy += (do_h - obj_h) * 0x800 + y * 0x1000;
                        efpip->obj_num = obj_num * ver_n * hor_n;
                        efpip->obj_w = do_w;
                        efpip->obj_h = do_h;
                        efpip->obj_index = (obj_index * ver_n + do_y) * hor_n + do_x;
                        *(int*)&efpip->object_flag |= 0x2000000;
                        dvo->running = 1;
                        reinterpret_cast<void(__cdecl*)(ExEdit::ObjectFilterIndex, ExEdit::FilterProcInfo*, int)>(GLOBAL::exedit_base + OFS::ExEdit::do_after_filter_effect)(efp->processing, efpip, (int)efp->processing | 0x1000000);
                        dvo->running = 0;
                        efpip->obj_data = obj_data;
                        x += do_w;
                    }
                }
                y += do_h;
            }
        }
        efpip->obj_w = obj_w;
        efpip->obj_h = obj_h;
        reinterpret_cast<void(__cdecl*)(ExEdit::FilterProcInfo*)>(GLOBAL::exedit_base + OFS::ExEdit::set_object_frame_dot_line)(efpip);
        return FALSE;
    }

} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_DIVIDEOBJECT
