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

#include "patch_playback_pos.hpp"

#ifdef PATCH_SWITCH_PLAYBACK_POS
namespace patch {
	int __cdecl playback_pos_t::calc_scene_frame(ExEdit::Filter* efp, ExEdit::FilterProcInfo* efpip) {
		int subframe = (efpip->frame - efp->frame_start) * 100 + efpip->subframe;
		
		return (int)((int64_t)subframe * (int64_t)efp->track_value_left[1] / 1000);
	}
} // namespace patch
#endif // ifdef PATCH_SWITCH_PLAYBACK_POS