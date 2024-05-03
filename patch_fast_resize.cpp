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

#include "patch_fast_resize.hpp"
#ifdef PATCH_SWITCH_FAST_RESIZE


namespace patch::fast {

    BOOL __cdecl Resize_t::func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        struct {
            int x, y, z, u, v;
        } poly[4];

        if (efpip->obj_w <= 0 || efpip->obj_h <= 0) {
            return TRUE;
        }
        auto resize = reinterpret_cast<efResize_var*>(GLOBAL::exedit_base + OFS::ExEdit::efResize_var_ptr);
        int yca_max_w = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yca_max_w);
        int yca_max_h = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yca_max_h);

        int obj_w = resize->w = efpip->obj_w;
        int obj_h = resize->h = efpip->obj_h;
        if (efp->check[1]) {
            obj_w = min(efp->track[1] / 100, yca_max_w);
            obj_h = min(efp->track[2] / 100, yca_max_h);
        } else {
            if (efp->track[0] != 10000) {
                obj_w = (int)round((double)obj_w * efp->track[0] * 0.0001);
                obj_h = (int)round((double)obj_h * efp->track[0] * 0.0001);
            }
            if (yca_max_h * resize->w <= yca_max_w * resize->h) {
                if (yca_max_h < obj_h) {
                    obj_w = (int)((double)yca_max_h * (double)obj_w / (double)obj_h);
                    obj_h = yca_max_h;
                }
            } else {
                if (yca_max_w < obj_w) {
                    obj_h = (int)((double)yca_max_w * (double)obj_h / (double)obj_w);
                    obj_w = yca_max_w;
                }
            }
            if (efp->track[1] != 10000) {
                obj_w = min((int)round((double)efp->track[1] * (double)obj_w * 0.0001), yca_max_w);
            }
            if (efp->track[2] != 10000) {
                obj_h = min((int)round((double)efp->track[2] * (double)obj_h * 0.0001), yca_max_h);
            }
        }
        if (obj_w <= 0 || obj_h <= 0) {
            return FALSE;
        }
        if (obj_w == resize->w && obj_h == resize->h) {
            return TRUE;
        }
        int flag = 3;
        if (efpip->xf4) {
            int yc_max_w = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yc_max_w);
            int yc_max_h = *reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::yc_max_h);
            if (yc_max_w < obj_w || yc_max_h < obj_h) {
                flag = 2;
                efpip->xf4 = 0;
            } else {
                flag = 0;
            }
        }
        efpip->obj_w = obj_w;
        efpip->obj_h = obj_h;
        if (efp->check[0]) {
            void(__cdecl* mt)(int, int, efResize_var*, ExEdit::FilterProcInfo*);
            if (flag == 3) { // yca2yca
                (mt) = reinterpret_cast<decltype(mt)>(GLOBAL::exedit_base + 0x74c0);
                resize = reinterpret_cast<efResize_var*>(efp); // 元の関数では使われないけど競合対策で念のため
            } else if (flag == 2) {
                (mt) = (mt_yc2yca);
            } else {
                (mt) = (mt_yc2yc);
            }
            efp->aviutl_exfunc->exec_multi_thread_func((AviUtl::MultiThreadFunc)mt, resize, efpip);
        } else {
            if (flag == 3) {
                efp->exfunc->fill(efpip->obj_temp, 0, 0, obj_w, obj_h, 0, 0, 0, 0, 2);
            }
            flag |= 0x13000000;

            poly[0].x = 0;
            poly[0].y = 0;
            poly[0].u = 0;
            poly[0].v = 0;
            poly[1].x = obj_w << 12;
            poly[1].y = 0;
            poly[1].u = resize->w << 12;
            poly[1].v = 0;
            poly[2].x = obj_w << 12;
            poly[2].y = obj_h << 12;
            poly[2].u = resize->w << 12;
            poly[2].v = resize->h << 12;
            poly[3].x = 0;
            poly[3].y = obj_h << 12;
            poly[3].u = 0;
            poly[3].v = resize->h << 12;
            efp->exfunc->x58(efpip->obj_temp, (int*)poly, 4, efpip->obj_edit, 0, flag);
        }
        std::swap(efpip->obj_temp, efpip->obj_edit);
        return TRUE;
    }

    void __cdecl Resize_t::mt_yc2yc(int thread_id, int thread_num, efResize_var* resize, ExEdit::FilterProcInfo* efpip) {
        int y = thread_id * efpip->obj_h / thread_num;
        int ye = (thread_id + 1) * efpip->obj_h / thread_num;
        for (; y < ye; y++) {
            auto src = (ExEdit::PixelYC*)efpip->obj_edit + resize->h * y / efpip->obj_h * efpip->scene_line;
            auto dst = (ExEdit::PixelYC*)efpip->obj_temp + y * efpip->scene_line;
            for (int x = 0; x < efpip->obj_w; x++) {
                int u = resize->w * x / efpip->obj_w;
                dst->y = src[u].y;
                dst->cb = src[u].cb;
                dst->cr = src[u].cr;
                dst++;
            }
        }
    }
    void __cdecl Resize_t::mt_yc2yca(int thread_id, int thread_num, efResize_var* resize, ExEdit::FilterProcInfo* efpip) {
        int y = thread_id * efpip->obj_h / thread_num;
        int ye = (thread_id + 1) * efpip->obj_h / thread_num;
        for (; y < ye; y++) {
            auto src = (ExEdit::PixelYC*)efpip->obj_edit + resize->h * y / efpip->obj_h * efpip->scene_line;
            auto dst = (ExEdit::PixelYCA*)efpip->obj_temp + y * efpip->obj_line;
            for (int x = 0; x < efpip->obj_w; x++) {
                int u = resize->w * x / efpip->obj_w;
                dst->y = src[u].y;
                dst->cb = src[u].cb;
                dst->cr = src[u].cr;
                dst->a = 0x1000;
                dst++;
            }
        }
    }


} // namespace patch::fast
#endif // ifdef PATCH_SWITCH_FAST_RESIZE
