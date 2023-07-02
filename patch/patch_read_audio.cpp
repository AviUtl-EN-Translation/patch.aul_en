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

#include "patch_read_audio.hpp"

#ifdef PATCH_SWITCH_READ_AUDIO
namespace patch {

#define AUDIO_SMEM_SHL 20
#define AUDIO_SMEM_SIZE (1 << AUDIO_SMEM_SHL)
#define AUDIO_SMEM_HIGH(ofs) ((-1 << AUDIO_SMEM_SHL)&ofs)
#define AUDIO_SMEM_LOW(ofs) ((AUDIO_SMEM_SIZE - 1)&ofs)

#define AVI_FILE_HANDLE_WAVEFORMATEX 0x38
#define AVI_FILE_HANDLE_INPUT_INFO 0x4c
#define AVI_FILE_HANDLE_AUDIO_N 0xc3b8
#define AVI_FILE_HANDLE_AUDIO_CURRENT_POS 0xc3c0

	int __fastcall read_audio_t::update_waveformat_wrap(AviUtl::AviFileHandle* afh, WAVEFORMATEX* wfe) {
		if (memcmp((WAVEFORMATEX*)((int)afh + AVI_FILE_HANDLE_WAVEFORMATEX), wfe, 16)) {
			*(int*)((int)afh + AVI_FILE_HANDLE_AUDIO_CURRENT_POS) = -1;
		}
		return  reinterpret_cast<int(__fastcall*)(AviUtl::AviFileHandle*, WAVEFORMATEX*)>(GLOBAL::aviutl_base + 0x2550)(afh, wfe);
	}


	void* read_audio_t::get_audio_shared_mem(AviUtl::AviFileHandle* afh, int n) {
		WAVEFORMATEX* wfe = (WAVEFORMATEX*)((int)afh + AVI_FILE_HANDLE_WAVEFORMATEX);
		AudioCacheInfo* aci = cache_info;
		for (int i = CACHEINFO_N; 0 < i; i--) {
			if (aci->smi != NULL) {
				if (aci->input_handle_info == *(void**)((int)afh + AVI_FILE_HANDLE_INPUT_INFO) && aci->audio_rate == wfe->nSamplesPerSec && aci->audio_ch == wfe->nChannels && aci->n == n) {
					auto exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
					return exfunc->get_shared_mem((int)aci->input_handle_info, ~n, aci->smi); // key2の0～は動画に使われるためマイナスにして-1
				}
			}
			aci++;
		}
		return NULL;
	}
	void* read_audio_t::create_audio_shared_mem(AviUtl::AviFileHandle* afh, int n) {
		auto exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
		AudioCacheInfo* min_priority_aci = cache_info;
		int min_priority = INT_MAX;

		AudioCacheInfo* aci = cache_info;
		for (int i = CACHEINFO_N; 0 < i; i--) {
			if (aci->smi == NULL) {
				min_priority_aci = aci;
				break;
			} else if (aci->smi->id < min_priority) {
				min_priority = aci->smi->id;
				min_priority_aci = aci;
			}
			aci++;
		}
		if (min_priority_aci->smi != NULL) {
			exfunc->delete_shared_mem(*(int*)((int)afh + AVI_FILE_HANDLE_INPUT_INFO), min_priority_aci->smi);
		}
		WAVEFORMATEX* wfe = (WAVEFORMATEX*)((int)afh + AVI_FILE_HANDLE_WAVEFORMATEX);

		min_priority_aci->input_handle_info = *(void**)((int)afh + AVI_FILE_HANDLE_INPUT_INFO);
		min_priority_aci->audio_rate = wfe->nSamplesPerSec;
		min_priority_aci->audio_ch = wfe->nChannels;
		min_priority_aci->n = n;
		return exfunc->create_shared_mem((int)min_priority_aci->input_handle_info, ~n, (256 + AUDIO_SMEM_SIZE) * wfe->nBlockAlign, &min_priority_aci->smi); // key2の0～は動画に使われるためマイナスにして-1
	}



	int __cdecl read_audio_t::exfunc_avi_file_read_audio_sample_wrap(AviUtl::AviFileHandle* afh, int start, int length, short* buf) {
		if (afh == NULL) return 0;

		if (start < 0) {
			length = min(length - start, *(int*)((int)afh + AVI_FILE_HANDLE_AUDIO_N));
			start = 0;
		} else {
			length = min(length, *(int*)((int)afh + AVI_FILE_HANDLE_AUDIO_N) - start);
		}
		short audio_blocksize = ((WAVEFORMATEX*)((int)afh + AVI_FILE_HANDLE_WAVEFORMATEX))->nBlockAlign;
		int len = length;
		while (0 < len) {
			short* ptr = (short*)get_audio_shared_mem(afh, start >> AUDIO_SMEM_SHL);
			if (ptr == NULL) {
				ptr = (short*)create_audio_shared_mem(afh, start >> AUDIO_SMEM_SHL);
				if (ptr == NULL) {
					exfunc_avi_file_read_audio_sample_org(afh, start, len, buf);
					break;
				}
				
				if (*(int*)((int)afh + AVI_FILE_HANDLE_AUDIO_CURRENT_POS) != AUDIO_SMEM_HIGH(start) && 0 < AUDIO_SMEM_HIGH(start)) {
					//printf("%d,%d\n", *(int*)((int)afh + AVI_FILE_HANDLE_AUDIO_CURRENT_POS) , AUDIO_SMEM_HIGH(start));
					//for (int i = min(2, start >> AUDIO_SMEM_SHL); 0 < i; i--) {
					//	exfunc_avi_file_read_audio_sample_org(afh, AUDIO_SMEM_HIGH(start) - AUDIO_SMEM_SIZE * i, AUDIO_SMEM_SIZE, ptr);
					//}
					for (int i = min(1023, ((WAVEFORMATEX*)((int)afh + AVI_FILE_HANDLE_WAVEFORMATEX))->nSamplesPerSec >> 7) + 1; 0 < i; i--) {
						exfunc_avi_file_read_audio_sample_org(afh, AUDIO_SMEM_HIGH(start) - 1024 * i, 1024, ptr);
					}
				}
				exfunc_avi_file_read_audio_sample_org(afh, AUDIO_SMEM_HIGH(start), AUDIO_SMEM_SIZE, ptr);
			}
			
			ptr = (short*)((int)ptr + AUDIO_SMEM_LOW(start) * audio_blocksize);
			int smem_right = min(len, AUDIO_SMEM_SIZE - AUDIO_SMEM_LOW(start));
			memcpy(buf, ptr, smem_right * audio_blocksize);
			buf = (short*)((int)buf + smem_right * audio_blocksize);
			start += smem_right;
			len -= smem_right;
			
		}
		return length;
	}
	
	/*
#define AVI_FILE_HANDLE_AUDIO_SAMPLING_RATE 0x3c
#define AVI_FILE_HANDLE_AUDIO_N 0xc3b8
#define AVI_FILE_HANDLE_AUDIO_CURRENT_POS 0xc3c0

	int __cdecl read_audio_t::exfunc_avi_file_read_audio_sample_wrap(AviUtl::AviFileHandle* afh, int start, int length, short* buf) {
		//auto a = *(short**)(GLOBAL::exedit_base + OFS::ExEdit::memory_ptr);

		if (afh == NULL) return 0;

		if (start < 0) {
			length = min(length - start, *(int*)((int)afh + AVI_FILE_HANDLE_AUDIO_N));
			start = 0;
		} else {
			length = min(length, *(int*)((int)afh + AVI_FILE_HANDLE_AUDIO_N) - start);
		}
		printf("%d,", *(int*)((int)afh + 0xc3b8));
		if (*(int*)((int)afh + AVI_FILE_HANDLE_AUDIO_CURRENT_POS) != start && 0 < start) {
			int l = min(start, *(int*)((int)afh + AVI_FILE_HANDLE_AUDIO_SAMPLING_RATE) / 16);
			exfunc_avi_file_read_audio_sample_org(afh, start - l, l, buf);
		}
		return exfunc_avi_file_read_audio_sample_org(afh, start, length, buf);

	}
	*/
} // namespace patch
#endif // ifdef PATCH_SWITCH_READ_AUDIO