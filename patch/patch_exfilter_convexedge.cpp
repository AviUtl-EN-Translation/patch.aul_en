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

#include "patch_exfilter_convexedge.hpp"
#ifdef PATCH_SWITCH_FAST_CONVEXEDGE
#ifdef PATCH_SWITCH_FAST_CONVEXEDGE_CL
#ifdef PATCH_SWITCH_EXFILTER_CONVEXEDGE


//#define PATCH_STOPWATCH

namespace patch::exfilter {


    BOOL __cdecl ConvexEdge_t::func_proc(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
        auto flag = efpip->flag;
        auto obj_w = efpip->obj_w;
        auto obj_h = efpip->obj_h;
        auto obj_edit = efpip->obj_edit;
        auto obj_temp = efpip->obj_temp;
        auto no_alpha = efpip->xf4;
        efpip->flag &= ~ExEdit::FilterProcInfo::Flag::fast_preview;
        efpip->obj_w = efpip->scene_w;
        efpip->obj_h = efpip->scene_h;
        efpip->obj_edit = reinterpret_cast<decltype(efpip->obj_edit)>(efpip->frame_edit);
        efpip->obj_temp = reinterpret_cast<decltype(efpip->obj_temp)>(efpip->frame_temp);
        efpip->xf4 = 1;

        BOOL ret = func_proc_org(efp, efpip);

        efpip->flag &= flag;
        efpip->frame_edit = reinterpret_cast<decltype(efpip->frame_edit)>(efpip->obj_edit);
        efpip->frame_temp = reinterpret_cast<decltype(efpip->frame_temp)>(efpip->obj_temp);
        efpip->obj_w = obj_w;
        efpip->obj_h = obj_h;
        efpip->obj_edit = obj_edit;
        efpip->obj_temp = obj_temp;
        efpip->xf4 = no_alpha;

        return ret;
    }

} // namespace patch::exfilter
#endif // ifdef PATCH_SWITCH_EXFILTER_CONVEXEDGE
#endif // ifdef PATCH_SWITCH_FAST_CONVEXEDGE_CL
#endif // ifdef PATCH_SWITCH_FAST_CONVEXEDGE
