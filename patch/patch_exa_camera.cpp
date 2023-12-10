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

#include "patch_exa_camera.hpp"

#ifdef PATCH_SWITCH_EXA_CAMERA
namespace patch {
	int __cdecl exa_camera_t::get_obj_camera_flag(int object_idx) {
		if (object_idx < 0) return -1;
		auto obj = *(ExEdit::Object**)(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
		if (has_flag(obj[object_idx].flag, ExEdit::Object::Flag::Camera)) return 1;
		return 0;
	}
} // namespace patch
#endif // ifdef PATCH_SWITCH_EXA_CAMERA
