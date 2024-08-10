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

#include "patch_object_copy.hpp"


#ifdef PATCH_SWITCH_OBJECT_COPY
namespace patch {

	void __cdecl object_copy_t::next_index() {
		index++;
		index &= 1;
	}
	void __cdecl object_copy_t::prev_index() {
		next_index();
	}

	void __cdecl object_copy_t::set_buffer_ptr() {
		auto oc = reinterpret_cast<object_copy_var*>(GLOBAL::exedit_base + 0x11edc0);
		if (smi[index] != nullptr) {
			auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);

			auto smip = &smi[index];
			oc->buffer_ptr = a_exfunc->get_shared_mem((int)smip, (int)smip, *smip);
		} else {
			oc->buffer_ptr = nullptr;
		}
		if (oc->buffer_ptr == nullptr) {
			oc->buffer_size = oc->object_num = oc->offset = 0;
		}
	}
	ExEdit::Object* __cdecl object_copy_t::mov_eax_ObjectArrayPointer_wrap() {
		set_buffer_ptr();
		return *reinterpret_cast<ExEdit::Object**>(GLOBAL::exedit_base + OFS::ExEdit::ObjectArrayPointer);
	}
	int __cdecl object_copy_t::mov_eax_offset_wrap() {
		set_buffer_ptr();
		auto oc = reinterpret_cast<object_copy_var*>(GLOBAL::exedit_base + 0x11edc0);
		return oc->offset;
	}

	BOOL __cdecl object_copy_t::realloc_smem(void** ptr, size_t size) {
		auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);

		auto old_smip = &smi[index];
		if (smi[index] == nullptr) {
			*ptr = a_exfunc->create_shared_mem((int)old_smip, (int)old_smip, size, old_smip);
			auto oc = reinterpret_cast<object_copy_var*>(GLOBAL::exedit_base + 0x11edc0);
			oc->object_num = oc->offset = 0;
			return TRUE;
		}
		void* oldptr = a_exfunc->get_shared_mem((int)old_smip, (int)old_smip, *old_smip);
		next_index();
		auto new_smip = &smi[index];
		*ptr = a_exfunc->create_shared_mem((int)new_smip, (int)new_smip, size, new_smip);
		if (*ptr == nullptr) {
			*ptr = oldptr;
			prev_index();
			return FALSE;
		}
		memcpy(*ptr, oldptr, (*old_smip)->size);
		a_exfunc->delete_shared_mem((int)old_smip, *old_smip);
		return TRUE;
	}

	void __cdecl object_copy_t::free_smem() {
		if (smi[index] == nullptr) return;

		auto a_exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
		auto oc = reinterpret_cast<object_copy_var*>(GLOBAL::exedit_base + 0x11edc0);

		a_exfunc->delete_shared_mem((int)&smi[index], smi[index]);
		oc->buffer_ptr = nullptr;

	}

} // namespace patch
#endif // ifdef PATCH_SWITCH_OBJECT_COPY
