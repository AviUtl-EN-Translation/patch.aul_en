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
#include <cstdint>
#include <bit>
#include <algorithm>

#include <Windows.h>

#include "global.hpp"

#include "util_int.hpp"
#include "util_pe.hpp"


inline i32 CalcNearJmp(i32 address, i32 jmp_address) {
	return jmp_address - (address + 4);
}

class OverWriteOnProtectHelper {
	uintptr_t m_address, m_size;
	DWORD m_oldProtect;
public:
	template<class T>
	OverWriteOnProtectHelper(T address, uintptr_t size) noexcept : m_address((uintptr_t)address), m_size(size) {
		VirtualProtect(reinterpret_cast<LPVOID>(m_address), m_size, PAGE_EXECUTE_READWRITE, &m_oldProtect);
	}
	~OverWriteOnProtectHelper() noexcept {
		VirtualProtect(reinterpret_cast<LPVOID>(m_address), m_size, m_oldProtect, &m_oldProtect);
	}

	template<class T1, class T2>
	void store_i8(T1 address, T2 value) const {
		::store_i8(m_address + address, value);
	}
	template<class T1, class T2>
	void store_i16(T1 address, T2 value) const {
		::store_i16(m_address + address, value);
	}
	template<class T1, class T2>
	void store_i32(T1 address, T2 value) const {
		::store_i32(m_address + address, value);
	}
	template<class T1, class T2>
	void store_i64(T1 address, T2 value) const {
		::store_i64(m_address + address, value);
	}

	template<class T0 = uint8_t, class T1>
	T0 load_i8(T1 address) const {
		return ::load_i8<T0>(m_address + address);
	}
	template<class T0 = uint16_t, class T1>
	T0 load_i16(T1 address) const {
		return ::load_i16<T0>(m_address + address);
	}
	template<class T0 = uint32_t, class T1>
	T0 load_i32(T1 address) const {
		return ::load_i32<T0>(m_address + address);
	}
	template<class T0 = uint64_t, class T1>
	T0 load_i64(T1 address) const {
		return ::load_i64<T0>(m_address + address);
	}

	void replaceNearJmp(i32 offset, void* jmp_address) {
		store_i32(offset, CalcNearJmp(m_address + offset, reinterpret_cast<i32>(jmp_address)));
	}

	auto address() const {
		return m_address;
	}
	auto address(uintptr_t ofs) const {
		return m_address + ofs;
	}
};

/// <summary>
/// ニアージャンプ・コールを書き換える
/// </summary>
/// <param name="address">書き換える対象のアドレス</param>
/// <param name="jmp_address">代わりに飛ばして欲しいアドレス</param>
inline void ReplaceNearJmp(i32 address, void* jmp_address) {
	OverWriteOnProtectHelper(address, 4).replaceNearJmp(0, jmp_address);
}

// 既存の関数を破壊して，自分の関数を実行する
inline class ReplaceFunction_t {
	static const int asm_size = 5;

public:
	// 乗っ取りたい関数があるアドレス,ジャンプさせる関数のポインタ,元の関数の内容が返る場所
	template<class T, size_t N = asm_size, std::enable_if_t<N >= asm_size, std::nullptr_t> = nullptr>
	void operator()(T address, const void* function, std::byte(&original)[N]) noexcept {
		auto adr = std::bit_cast<i32>(address);
		OverWriteOnProtectHelper h(adr, asm_size);
		std::copy(adr, adr + N, original);
		store_i8(adr, '\xe9'); // jmp rel32
		store_i32(adr + 1, CalcNearJmp(address + 1, (i32)function));
	}

	template<class T>
	void operator()(T address, const void* function) noexcept {
		auto adr = std::bit_cast<i32>(address);
		OverWriteOnProtectHelper h(adr, asm_size);
		store_i8(adr, '\xe9'); // jmp rel32
		store_i32(adr + 1, CalcNearJmp(adr + 1, (i32)function));
	}
} ReplaceFunction;


// 乗っ取りたいモジュール, 乗っ取る関数があるDLLのファイル名, 乗っ取る関数の名前, 新しい関数へのポインタ
inline BOOL ExchangeFunction(HMODULE hModule, std::string_view modname, std::string_view funcname, void* function) noexcept {
	auto ptr = search_import(hModule, modname, funcname);
	if (!ptr)return FALSE;
	DWORD flOldProtect;
	if (VirtualProtect(ptr, 4, PAGE_EXECUTE_READWRITE, &flOldProtect) == FALSE) return FALSE;
	store_i32(ptr, function);
	return VirtualProtect(ptr, 4, flOldProtect, &flOldProtect);
}

/// <summary>
/// 指定したアドレスの関数の直前に、自分の関数を実行する
/// 実行後元の関数に戻る
/// __stdcall,__cdecl専用
/// </summary>
/// <param name="address"> 中断したい関数のアドレス </param>
/// <param name="function"> 挿入する関数 </param>
/// <param name="asm_word_n"> 命令単位に合った数(7以上) </param>
/// <returns> TRUE </returns>
inline bool InjectFunction_stdcall(uint32_t address, const void* function, size_t asm_word_n) noexcept {
	std::byte* cursor = GLOBAL::executable_memory_cursor;

	store_i8(cursor, '\xb8'); // mov eax, (i32)
	store_i32(cursor + 1, function);
	store_i16(cursor + 5, '\xff\xd0'); // call eax
	
	std::copy((std::byte*)address, (std::byte*)address + asm_word_n, cursor + 7);
	store_i16(cursor + asm_word_n + 7, '\xff\x25'); // jmp [(i32)]
	store_i32(cursor + asm_word_n + 9, cursor + asm_word_n + 13);
	store_i32(cursor + asm_word_n + 13, address + asm_word_n);
	GLOBAL::executable_memory_cursor += asm_word_n + 17;

	{
		OverWriteOnProtectHelper protect(address, 7);
		store_i8(address, '\xb8'); // mov eax, (i32)
		store_i32(address + 1, cursor);
		store_i16(address + 5, '\xff\xe0'); // call eax
	}
	return TRUE;
}

/// <summary>
/// 指定したアドレスの関数の直前に、自分の関数を実行する
/// 実行後元の関数に戻る
/// __cdecl専用 (__stdcallと一緒だけど)
/// </summary>
/// <param name="address"> 中断したい関数のアドレス </param>
/// <param name="function"> 挿入する関数 </param>
/// <param name="asm_word_n"> 命令単位に合った数(7以上) </param>
/// <returns> TRUE </returns>
inline bool InjectFunction_cdecl(uint32_t address, const void* function, size_t asm_word_n) noexcept {
	InjectFunction_stdcall(address, function, asm_word_n);
}

/// <summary>
/// 指定したアドレスの関数の直前に、自分の関数を実行する
/// 実行後元の関数に戻る
/// __fastcall専用
/// </summary>
/// <param name="address"> 中断したい関数のアドレス </param>
/// <param name="function"> 挿入する関数 </param>
/// <param name="asm_word_n"> 命令単位に合った数(7以上) </param>
/// <returns> TRUE </returns>
inline bool InjectFunction_fastcall(uint32_t address, void(*func)(), size_t asm_word_n) {
	if (asm_word_n < 7)return false;
	OverWriteOnProtectHelper helper(address, asm_word_n);

	auto bridge = GLOBAL::executable_memory_cursor;

	store_i16(bridge, '\x51\x52'); // PUSH ECX; PUSH EDX
	store_i8(bridge + 2, '\xb8'); // MOV EAX, (i32)
	store_i32(bridge + 3, func);
	store_i16(bridge + 7, '\xff\xd0'); // CALL EAX
	store_i16(bridge + 9, '\x5a\x59'); // POP EDX; POP ECX
	std::copy((std::byte*)address, (std::byte*)address + asm_word_n, bridge + 11);
	store_i16(bridge + asm_word_n + 11, '\xff\x25'); // JMP (i32)
	store_i32(bridge + asm_word_n + 13, bridge + asm_word_n + 17);
	store_i32(bridge + asm_word_n + 17, address + asm_word_n);
	GLOBAL::executable_memory_cursor += asm_word_n + 21;

	store_i8(address, '\xb8'); // MOV EAX,
	store_i32(address + 1, bridge);
	store_i16(address + 5, '\xff\xe0'); // JMP EAX

	return true;
}



inline static void(__cdecl* push_new_args)(uint32_t function, int arg_ofs, int arg_n);

/// <summary>
/// 指定したアドレスの関数の直前に、自分の関数を実行する
/// 実行後元の関数に戻る
/// __cdecl専用
/// </summary>
/// <param name="address"> 中断したい関数のアドレス </param>
/// <param name="function"> 挿入する関数 </param>
/// <param name="asm_word_n"> 命令単位に合った数(5以上) </param>
/// <param name="stack_s"> 何番目のスタックから引数にするか(4byte単位) </param>
/// <param name="arg_n"> 引数の数(4byte単位) </param>
/// <param name="flag"> EAX,ECX,EDXを退避するかを設定 </param>
/// <returns> TRUE </returns>
#define FLAG_PUSH_POP_EAX 0x00000001 // 戻り値が元のEAXに上書きされる注意
#define FLAG_PUSH_POP_ECX 0x00000002
#define FLAG_PUSH_POP_EDX 0x00000004
// それ以外は呼出規約として関数側が行っている
inline void InjectionFunction_push_args_cdecl(uint32_t address, const uint32_t function, size_t asm_word_n, uint16_t stack_s, uint8_t arg_n, uint32_t flag) {
	auto& cursor = GLOBAL::executable_memory_cursor;
	auto cursor0 = GLOBAL::executable_memory_cursor;
	stack_s++; // バイナリ関数retアドレス分
	// push
	if (flag & FLAG_PUSH_POP_EAX) {
		store_i8(cursor, 0x50); cursor++;
		stack_s++;
	}
	if (flag & FLAG_PUSH_POP_ECX) {
		store_i8(cursor, 0x51); cursor++;
		stack_s++;
	}
	if (flag & FLAG_PUSH_POP_EDX) {
		store_i8(cursor, 0x52); cursor++;
		stack_s++;
	}

	// args eax,ecx,edx
	store_i8(cursor, 0xb8); cursor++;
	store_i32(cursor, function); cursor += 4;
	store_i8(cursor, 0xb9); cursor++;
	store_i32(cursor, stack_s); cursor += 4;
	store_i8(cursor, 0xba); cursor++;
	store_i32(cursor, arg_n); cursor += 4;
	store_i8(cursor, 0xe8); cursor++;
	store_i32(cursor, CalcNearJmp((uint32_t)cursor, (uint32_t)push_new_args)); cursor += 4;

	// pop
	if (flag & FLAG_PUSH_POP_EDX) {
		store_i8(cursor, 0x5a); cursor++;
	}
	if (flag & FLAG_PUSH_POP_ECX) {
		store_i8(cursor, 0x59); cursor++;
	}
	if (flag & FLAG_PUSH_POP_EAX) {
		store_i8(cursor, 0x58); cursor++;
	}

	memcpy(cursor, (void*)address, asm_word_n); cursor += asm_word_n;

	store_i8(cursor, 0xe9); cursor++;
	store_i32(cursor, CalcNearJmp((uint32_t)cursor, address + asm_word_n)); cursor += 4;


	OverWriteOnProtectHelper h(address, 5);
	h.store_i8(0, 0xe9);
	h.replaceNearJmp(1, cursor0);
}

inline void init_util_magic() {
	auto& cursor = GLOBAL::executable_memory_cursor;

	static const char bin_push_new_args[] = {
		"\x53"                     // push    ebx
		"\x03\xca"                 // add     ecx,edx
		"\x33\xdb"                 // mov     ebx,00
		"\x8d\x0c\x8c"             // mov     ecx,esp+ecx*4
		"\x85\xd2"                 // test    edx
		"\x7e\x0a"                 // jng     skip,0a
		"\x8b\xda"                 // mov     ebx,edx
		"\xff\x31"                 // push    dword ptr [ecx]
		"\x83\xe9\x04"             // sub     ecx,+04
		"\x4a"                     // dec     edx
		"\x7f\xf8"                 // jg      back,08
		"\xc1\xe3\x02"             // shl     ebx,02
		"\xff\xd0"                 // call    eax
		"\x03\xe3"                 // add     esp,ebx
		"\x5b"                     // pop     ebx
		"\xc3"                     // ret
	};
	(push_new_args) = reinterpret_cast<decltype(push_new_args)>(cursor);

	memcpy(cursor, bin_push_new_args, sizeof(bin_push_new_args) - 1);
	cursor += sizeof(bin_push_new_args) - 1;
}
