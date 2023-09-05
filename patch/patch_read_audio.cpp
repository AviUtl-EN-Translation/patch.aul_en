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

#define AUDIO_SMEM_SHL 19
#define AUDIO_SMEM_SIZE (1 << AUDIO_SMEM_SHL)
#define AUDIO_SMEM_HIGH(ofs) ((-1 << AUDIO_SMEM_SHL)&ofs)
#define AUDIO_SMEM_LOW(ofs) ((AUDIO_SMEM_SIZE - 1)&ofs)

#define AUDIO_SMEM_PART_SHL 10
#define AUDIO_SMEM_PART_SIZE (1 << AUDIO_SMEM_PART_SHL)

#define AVI_FILE_HANDLE_WAVEFORMATEX 0x38
#define AVI_FILE_HANDLE_INPUT_INFO 0x4c
#define AVI_FILE_HANDLE_AUDIO_N 0xc3b8
#define AVI_FILE_HANDLE_AUDIO_CURRENT_POS 0xc3c0
#define AVI_FILE_HANDLE_WAVEFORMATEX2 8624

	int __fastcall read_audio_t::update_waveformat_wrap(AviUtl::AviFileHandle* afh, WAVEFORMATEX* wfe) {
		if (memcmp((WAVEFORMATEX*)((int)afh + AVI_FILE_HANDLE_WAVEFORMATEX), wfe, 16)) {
			*(int*)((int)afh + AVI_FILE_HANDLE_AUDIO_CURRENT_POS) = -1;
		}
		return reinterpret_cast<int(__fastcall*)(AviUtl::AviFileHandle*, WAVEFORMATEX*)>(GLOBAL::aviutl_base + 0x2550)(afh, wfe);
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
		return exfunc->create_shared_mem((int)min_priority_aci->input_handle_info, ~n, AUDIO_SMEM_SIZE * wfe->nBlockAlign, &min_priority_aci->smi); // key2の0～は動画に使われるためマイナスにして-1
	}


	void* get_or_create_shared_mem() {
		auto exfunc = (AviUtl::ExFunc*)(GLOBAL::aviutl_base + OFS::AviUtl::exfunc);
		void* ptr = exfunc->get_shared_mem((int)&get_or_create_shared_mem, (int)&get_or_create_shared_mem, nullptr);
		if (ptr != nullptr) return ptr;
		return exfunc->create_shared_mem((int)&get_or_create_shared_mem, (int)&get_or_create_shared_mem, 0x40000, nullptr);
	}
	
	int __cdecl read_audio_t::exfunc_avi_file_read_audio_sample_wrap(AviUtl::AviFileHandle* afh, int start, int length, short* buf) {
		if (afh == NULL) return 0;
		//printf("%d,%d\n", ((WAVEFORMATEX*)((int)afh + AVI_FILE_HANDLE_WAVEFORMATEX))->nSamplesPerSec, ((WAVEFORMATEX*)((int)afh + AVI_FILE_HANDLE_WAVEFORMATEX2))->nSamplesPerSec);
		auto buf0 = buf;
		int rev = 0;
		if (length < 0) {
			rev++;
			start += length - 1;
			length = -length;
		}
		short audio_blocksize = ((WAVEFORMATEX*)((int)afh + AVI_FILE_HANDLE_WAVEFORMATEX))->nBlockAlign;
		if (start < 0) {
			length = min(length + start, *(int*)((int)afh + AVI_FILE_HANDLE_AUDIO_N));
			if (length <= 0) return 0;
			int ofs = -start * audio_blocksize;
			memset(buf, 0, ofs);
			buf = (short*)((int)buf + ofs);
			start = 0;
		} else {
			length = min(length, *(int*)((int)afh + AVI_FILE_HANDLE_AUDIO_N) - start);
		}
		int length0 = length;
		while (0 < length) {
			short* ptr = (short*)get_audio_shared_mem(afh, start >> AUDIO_SMEM_SHL);
			if (ptr == NULL) {
				ptr = (short*)create_audio_shared_mem(afh, start >> AUDIO_SMEM_SHL);
				if (ptr == NULL) {
					exfunc_avi_file_read_audio_sample_org(afh, start, length, buf);
					break;
				}

				if (*(int*)((int)afh + AVI_FILE_HANDLE_AUDIO_CURRENT_POS) != AUDIO_SMEM_HIGH(start) && 0 < AUDIO_SMEM_HIGH(start)) {
					for (int i = min(min(4096, AUDIO_SMEM_HIGH(start) >> 8), (((WAVEFORMATEX*)((int)afh + AVI_FILE_HANDLE_WAVEFORMATEX))->nSamplesPerSec >> 7) + 1); 0 < i; i--) {
						exfunc_avi_file_read_audio_sample_org(afh, AUDIO_SMEM_HIGH(start) - 256 * i, 256, ptr);
					}
				}

				int i2 = (0x40000 / audio_blocksize + AUDIO_SMEM_PART_SIZE - 1) >> AUDIO_SMEM_PART_SHL;

				int s = AUDIO_SMEM_HIGH(start);
				short* p = ptr;
				for (int i = (1 << (AUDIO_SMEM_SHL - AUDIO_SMEM_PART_SHL)) - i2; 0 < i; i--) {
					exfunc_avi_file_read_audio_sample_org(afh, s, AUDIO_SMEM_PART_SIZE, p);
					s += AUDIO_SMEM_PART_SIZE;
					p = (short*)((int)p + (audio_blocksize << AUDIO_SMEM_PART_SHL));
				}
				short* p2 = (short*)get_or_create_shared_mem();
				for (int i = i2; 0 < i; i--) {
					exfunc_avi_file_read_audio_sample_org(afh, s, AUDIO_SMEM_PART_SIZE, p2);
					s += AUDIO_SMEM_PART_SIZE;
					memcpy(p, p2, audio_blocksize << AUDIO_SMEM_PART_SHL);
					p = (short*)((int)p + (audio_blocksize << AUDIO_SMEM_PART_SHL));
				}
			}
			ptr = (short*)((int)ptr + AUDIO_SMEM_LOW(start) * audio_blocksize);
			int smem_right = min(length, AUDIO_SMEM_SIZE - AUDIO_SMEM_LOW(start));
			memcpy(buf, ptr, smem_right * audio_blocksize);
			buf = (short*)((int)buf + smem_right * audio_blocksize);
			start += smem_right;
			length -= smem_right;

		}
		if (rev) {
			short audio_ch = ((WAVEFORMATEX*)((int)afh + AVI_FILE_HANDLE_WAVEFORMATEX))->nChannels;
			int p1 = 0;
			int p2 = length0 - 1;
			if (audio_ch == 1) {
				while (p1 < p2) {
					std::swap(buf0[p1], buf0[p2]);
					p1++; p2--;
				}
			} else if (audio_ch == 2) {
				int* bufi = (int*)buf0;
				while (p1 < p2) {
					std::swap(bufi[p1], bufi[p2]);
					p1++; p2--;
				}
			} else {
				p2 *= audio_ch;
				while (p1 < p2) {
					for (int i = 0; i < audio_ch; i++) {
						std::swap(buf0[p1 + i], buf0[p2 + i]);
					}
					p1 += audio_ch; p2 -= audio_ch;
				}
			}
		}
		return length0;
	}
	
	/*
#define READ_AUDIO_PRE_SH 8
#define READ_AUDIO_PRE_SIZE (1 << READ_AUDIO_PRE_SH)
#define READ_AUDIO_PART_SH 10
#define READ_AUDIO_PART_SIZE (1 << READ_AUDIO_PART_SH)
#define READ_AUDIO_PART_HIGH(ofs) (ofs >> READ_AUDIO_PART_SH)
#define READ_AUDIO_PART_LOW(ofs) ((READ_AUDIO_PART_SIZE - 1)&ofs)

	int __cdecl read_audio_t::exfunc_avi_file_read_audio_sample_wrap(AviUtl::AviFileHandle* afh, int start, int length, short* buf) {
		if (afh == NULL) return 0;
		auto buf0 = buf;
		int rev = 0;
		if (length < 0) {
			rev++;
			start += length - 1;
			length = -length;
		}
		int length0 = length;
		short audio_blocksize = ((WAVEFORMATEX*)((int)afh + AVI_FILE_HANDLE_WAVEFORMATEX))->nBlockAlign;
		int ofs = 0;
		if (start < 0) {
			length = min(length + start, *(int*)((int)afh + AVI_FILE_HANDLE_AUDIO_N));
			if (length <= 0) return 0;
			ofs = -start * audio_blocksize;
			start = 0;
		} else {
			length = min(length, *(int*)((int)afh + AVI_FILE_HANDLE_AUDIO_N) - start);
		}

		if (*(int*)((int)afh + AVI_FILE_HANDLE_AUDIO_CURRENT_POS) != start && READ_AUDIO_PRE_SIZE <= start) {
			for (int i = min(min(1024, start >> READ_AUDIO_PRE_SH), (((WAVEFORMATEX*)((int)afh + AVI_FILE_HANDLE_WAVEFORMATEX))->nSamplesPerSec >> 7) + 1); 0 < i; i--) {
				exfunc_avi_file_read_audio_sample_org(afh, start - READ_AUDIO_PRE_SIZE * i, READ_AUDIO_PRE_SIZE, buf);
			}
		}

		if (0 < ofs) {
			memset(buf, 0, ofs);
			buf = (short*)((int)buf + ofs);
		}
		if (READ_AUDIO_PART_LOW(length)) {
			exfunc_avi_file_read_audio_sample_org(afh, start, READ_AUDIO_PART_LOW(length), buf);
			start += READ_AUDIO_PART_LOW(length);
			buf = (short*)((int)buf + READ_AUDIO_PART_LOW(length) * audio_blocksize);
		}
		int size = audio_blocksize << READ_AUDIO_PART_SH;
		for (int i = READ_AUDIO_PART_HIGH(length); 0 < i; i--) {
			exfunc_avi_file_read_audio_sample_org(afh, start, READ_AUDIO_PART_SIZE, buf);
			start += READ_AUDIO_PART_SIZE;
			buf = (short*)((int)buf + size);
		}

		if (rev) {
			short audio_ch = ((WAVEFORMATEX*)((int)afh + AVI_FILE_HANDLE_WAVEFORMATEX))->nChannels;
			int p1 = 0;
			int p2 = length0 - 1;
			if (audio_ch == 1) {
				while (p1 < p2) {
					std::swap(buf0[p1], buf0[p2]);
					p1++; p2--;
				}
			} else if (audio_ch == 2) {
				int* bufi = (int*)buf0;
				while (p1 < p2) {
					std::swap(bufi[p1], bufi[p2]);
					p1++; p2--;
				}
			} else {
				p2 *= audio_ch;
				while (p1 < p2) {
					for (int i = 0; i < audio_ch; i++) {
						std::swap(buf0[p1 + i], buf0[p2 + i]);
					}
					p1 += audio_ch; p2 -= audio_ch;
				}
			}
		}
		return length0;
	}
	*/
} // namespace patch
#endif // ifdef PATCH_SWITCH_READ_AUDIO
