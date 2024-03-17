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
#include "patch_fast_wipe.hpp"
#ifdef PATCH_SWITCH_FAST_WIPE

#include <numbers>

#include "global.hpp"
#include "offset_address.hpp"
#include "util_int.hpp"



namespace patch::fast {

    void __cdecl Wipe_t::fan_mt(int thread_id, int thread_num, ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
    }

}

#endif // ifdef PATCH_SWITCH_FAST_WIPE
