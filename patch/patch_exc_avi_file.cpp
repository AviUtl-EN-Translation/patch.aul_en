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

#include "patch_exc_avi_file.hpp"


#ifdef PATCH_SWITCH_EXC_AVI_FILE
namespace patch {
	BOOL __cdecl exc_avi_file_t::read_object_file_data_wrap(LPSTR data, ExEdit::Object* eop, int filter_idx) {
		BOOL ret = reinterpret_cast<BOOL(__cdecl*)(LPSTR, ExEdit::Object*, int)>(GLOBAL::exedit_base + OFS::ExEdit::read_object_file_data)(data, eop, filter_idx);

		auto LoadedFilterTable = reinterpret_cast<ExEdit::Filter**>(GLOBAL::exedit_base + OFS::ExEdit::LoadedFilterTable);
		switch ((int)LoadedFilterTable[eop->filter_param[filter_idx].id] - GLOBAL::exedit_base) {
		case OFS::ExEdit::efMovieFile_ptr:
		case OFS::ExEdit::efMovieSynthesis_ptr:
		case OFS::ExEdit::efAudioFile_ptr:
		case OFS::ExEdit::efWaveForm_ptr:
			auto exdata_file = reinterpret_cast<char*>(*reinterpret_cast<int*>(GLOBAL::exedit_base + OFS::ExEdit::ExdataPointer) + 4 + eop->exdata_offset + eop->filter_param[filter_idx].exdata_offset);
			if (exdata_file[0] != '\0') {
				if (!PathFileExistsA(exdata_file)) {
					exdata_file[0] = '\0';
				}
			}
		}
		return ret;
	}
} // namespace patch
#endif // ifdef PATCH_SWITCH_EXC_AVI_FILE
