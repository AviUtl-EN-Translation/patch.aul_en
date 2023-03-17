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
#include "macro.h"
#ifdef PATCH_SWITCH_TEXT_OP_SIZE
#include <memory>

#include "global.hpp"
#include "offset_address.hpp"
#include "util.hpp"

#include "config_rw.hpp"

namespace patch {
	// init at exedit load
	// 制御文字のサイズのみを変えるとフォントの情報が壊れる
    // 
    // 
    // 影付き文字や縁取り文字と制御文字のサイズの組み合わせで描画がバグる
    inline class text_op_size_t {

        bool enabled = true;
        bool enabled_i;
        inline static const char key[] = "text_op_size";
    public:
        void init() {
            enabled_i = enabled;
            if (!enabled_i)return;

            { // 制御文字のサイズのみを変えるとフォントの情報が壊れる
                OverWriteOnProtectHelper h(GLOBAL::exedit_base + OFS::ExEdit::text_op_logfont_size, 1);
                h.store_i8(0, sizeof(LOGFONTW));
            }

#ifdef PATCH_SWITCH_TEXT_OP_FIX
            { // 影付き文字や縁取り文字と制御文字のサイズの組み合わせで描画がバグる

                auto& cursor = GLOBAL::executable_memory_cursor;
                {
                    /*
                    10050382 03f0                 add     esi,eax
                    10050384 03e8                 add     ebp,eax
                    10050386 89b4247c010000       mov     dword ptr [esp+0000017c],esi
                    1005038d 89ac2480010000       mov     dword ptr [esp+00000180],ebp

                    100503ae 03f0                 add     esi,eax
                    100503b0 03e8                 add     ebp,eax

                    100503f1 2bd7                 sub     edx,edi

                    ↓
                    全削除
                    */
                    constexpr int vp_begin = 0x50382;
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x503f3 - vp_begin);
                    h.store_i16(0x50382 - vp_begin, '\xeb\x10');
                    h.store_i16(0x503ae - vp_begin, '\xeb\x02');
                    h.store_i16(0x503f1 - vp_begin, '\x90\x90');
                }
                {
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + 0x5048f, 5);
                    h.store_i8(0, '\xe8');
                    h.replaceNearJmp(1, cursor);
                    /*
                    1005048f 33c0               xor     eax,eax
                    10050491 668b06             mov     ax,[esi]
                    ↓
                    1005048f e8XxXxXxXx         call    cursor

                    10000000 8b4c2420           mov     ecx,dword ptr [esp+20]
                    10000000 2b4c241c           sub     ecx,dword ptr [esp+1c]
                    10000000 014c2418           add     dword ptr [esp+18],ecx ; x160
                    10000000 894c2430           mov     dword ptr [esp+30],ecx ; y148
                    10000000 33c0               xor     eax,eax
                    10000000 668b06             mov     ax,[esi]
                    10000000 c3                 ret
                    */
                    static const char code_put[] =
                        "\x8b\x4c\x24\x20"         // mov     ecx,dword ptr [esp+20]
                        "\x2b\x4c\x24\x1c"         // sub     ecx,dword ptr [esp+1c]
                        "\x01\x4c\x24\x18"         // add     dword ptr [esp+18],ecx ; x160
                        "\x89\x4c\x24\x30"         // mov     dword ptr [esp+30],ecx ; y148
                        "\x33\xc0"                 // xor     eax,eax
                        "\x66\x8b\x06"             // mov     ax,[esi]
                        "\xc3"                     // ret
                        ;

                    memcpy(cursor, code_put, sizeof(code_put) - 1);
                    cursor += sizeof(code_put) - 1;
                }
                {
                    constexpr int vp_begin = 0x504f0;
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x5055c - vp_begin);
                    h.store_i32(0x504f0 - vp_begin, '\x0f\x1f\x44\x00');
                    h.store_i16(0x504f4 - vp_begin, '\x00\xe8');
                    h.replaceNearJmp(0x504f6 - vp_begin, cursor);
                    /*
                    100504f0 81fd00040000       cmp     ebp,00000400
                    100504f6 894c2414           mov     dword ptr [esp+14],ecx
                    ↓
                    100504f0 0f1f440000         nop
                    100504f5 e8XxXxXxXx         call    cursor

                    10000000 8b4c2420           mov     ecx,dword ptr [esp+20]
                    10000000 2b4c241c           sub     ecx,dword ptr [esp+1c]
                    10000000 894c2418           mov     dword ptr [esp+18],ecx ; x160
                    10000000 33c9               xor     ecx,ecx
                    10000000 81fd00040000       cmp     ebp,00000400
                    10000000 c3                 ret
                    */
                
                    static const char code_put[] =
                        "\x8b\x4c\x24\x20"         // mov     ecx,dword ptr [esp+20]
                        "\x2b\x4c\x24\x1c"         // sub     ecx,dword ptr [esp+1c]
                        "\x89\x4c\x24\x18"         // mov     dword ptr [esp+18],ecx ; x160
                        "\x33\xc9"                 // xor     ecx,ecx
                        "\x81\xfd\x00\x04\x00\x00" // cmp     ebp,00000400
                        "\xc3"                     // ret
                        ;

                    memcpy(cursor, code_put, sizeof(code_put) - 1);
                    cursor += sizeof(code_put) - 1;

                    /*
                    10050531 f6c201             test    dl,01
                    10050534 894c2414           mov     dword ptr [esp+14],ecx
                    ↓
                    10050531 014c2414           add     dword ptr [esp+14],ecx
                    10050535 f6c201             test    dl,01
                    */
                    h.store_i32(0x50531 - vp_begin, '\x01\x4c\x24\x14');
                    h.store_i32(0x50534 - vp_begin, '\x14\xf6\xc2\x01');

                    /*
                    1005055a 894c2414           mov     dword ptr [esp+14],ecx
                    ↓
                    1005055a 01442414           add     dword ptr [esp+14],eax
                    */
                    h.store_i16(0x5055a - vp_begin, '\x01\x44');

                }
                {
                /*
                  1005080a 89442418           mov     dword ptr [esp+18],eax
                  1005080e 7d09               jnl     10050819
                  10050810 b802000000         mov     eax,00000002
              
                  1005080a 7d04               jnl     skip,4
                  1005080c 33c0               xor     eax,eax
                  1005080e 40                 inc     eax
                  1005080f 40                 inc     eax
                  10050810 e8XxXxXxXx         call    cursor
              
                    10000000 8b542458           mov     edx,dword ptr [esp+58] ; 一つ下とほぼ同じ
                    10000000 85d2               test    edx,edx
                    10000000 7516               jnz     skip,+16
                    10000000 8bd0               mov     edx,eax
                    10000000 2b54241c           sub     edx,dword ptr [esp+1c]
                    10000000 01542418           add     dword ptr [esp+18],edx ; x160
                    10000000 8b4c242c           mov     ecx,dword ptr [esp+2c] ; text_line_n
                    10000000 85c9               test    ecx,ecx
                    10000000 7504               jnz     skip,+04
                    10000000 01542430           add     dword ptr [esp+30],edx ; y148
                    10000000 8944241c           mov     dword ptr [esp+1c],eax ; apend_size
                    10000000 c3                 ret



                  10050841 c1f805             sar     eax,05
                  10050844 89442418           mov     dword ptr [esp+18],eax
                  ↓
                  10050841 9090               nop
                  10050843 e8XxXxXxXx         call    cursor
              
                    10000000 c1f805             sar     eax,05
                    10000000 8b542458           mov     edx,dword ptr [esp+58] ; text_w
                    10000000 85d2               test    edx,edx
                    10000000 7516               jnz     skip,+16
                    10000000 8bd0               mov     edx,eax
                    10000000 2b54241c           sub     edx,dword ptr [esp+1c]
                    10000000 01542418           add     dword ptr [esp+18],edx ; x160
                    10000000 8b4c242c           mov     ecx,dword ptr [esp+2c] ; text_line_n
                    10000000 85c9               test    ecx,ecx
                    10000000 7504               jnz     skip,+04
                    10000000 01542430           add     dword ptr [esp+30],edx ; y148
                    10000000 8944241c           mov     dword ptr [esp+1c],eax ; apend_size
                    10000000 c3                 ret
                */
                    constexpr int vp_begin = 0x5080a;
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x50848 - vp_begin);
                    h.store_i32(0x5080a - vp_begin, '\x7d\x04\x33\xc0');
                    h.store_i32(0x5080e - vp_begin, '\x40\x40\xe8\x00');
                    h.replaceNearJmp(0x50811 - vp_begin, cursor + 3); // 2行目から

                    h.store_i32(0x50841 - vp_begin, '\x90\x90\xe8\x00');
                    h.replaceNearJmp(0x50844 - vp_begin, cursor); // 1行目から

                    static const char code_put[] =
                        "\xc1\xf8\x05"             // sar     eax,05
                        "\x8b\x54\x24\x58"         // mov     edx,dword ptr [esp+58] ; text_w
                        "\x85\xd2"                 // test    edx,edx
                        "\x75\x16"                 // jnz     skip,+16
                        "\x8b\xd0"                 // mov     edx,eax
                        "\x2b\x54\x24\x1c"         // sub     edx,dword ptr [esp+1c]
                        "\x01\x54\x24\x18"         // add     dword ptr [esp+18],edx ; x160
                        "\x8b\x4c\x24\x2c"         // mov     ecx,dword ptr [esp+2c] ; text_line_n
                        "\x85\xc9"                 // test    ecx,ecx
                        "\x75\x04"                 // jnz     skip,+04
                        "\x01\x54\x24\x30"         // add     dword ptr [esp+30],edx ; y148
                        "\x89\x44\x24\x1c"         // mov     dword ptr [esp+1c],eax ; apend_size
                        "\xc3"                     // ret
                        ;

                    memcpy(cursor, code_put, sizeof(code_put) - 1);
                    cursor += sizeof(code_put) - 1;
                }
                {
                    /*
                        100509ac 81fd00040000       cmp     ebp,00000400
                        100509b2 895c2414           mov     dword ptr [esp+14],ebx
                        100509b6 895c242c           mov     dword ptr [esp+2c],ebx
                        ↓

                        100509ac 90                 nop
                        100509ad e8XxXxXxXx         call    cursor
                        100509b2 89442414           mov     dword ptr [esp+14],eax
                        100509b6 8944242c           mov     dword ptr [esp+2c],eax

                        10000000 8b442420           mov     eax,dword ptr [esp+20] ; apend_range
                        10000000 2b44241c           sub     eax,dword ptr [esp+1c] ; apend_size
                        10000000 81fd00040000       cmp     ebp,00000400
                        10000000 c3                 ret

                    */
                    /*
                        100509c6 7c09               jl      skip,+09
                        ↓
                        100509c6 7c05               jl      skip,+05
                    */
                    /*
                        100509e9 a801               test    al,01
                        100509eb 897c2414           mov     dword ptr [esp+14],edi
                        ↓
                        100509e9 017c2414           add     dword ptr [esp+14],edi
                        100509ed a801               test    al,01
                    */

                    /*
                        10050a05 03f8               add     edi,eax
                        ↓
                        10050a05 8bf8               mov     edi,eax
                    */
                    /*
                        10050a1a 03d1               add     edx,ecx
                        10050a1c 03fa               add     edi,edx
                        ↓
                        10050a1a 03d1               add     edx,ecx
                        10050a1c 8bfa               mov     edi,edx
                    */
                    /*
                        10050a1e 897c2414           mov     dword ptr [esp+14],edi
                        ↓
                        10050a1e 017c2414           add     dword ptr [esp+14],edi
                    */


                    const int vp_begin = 0x509ac;
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x50a1f - vp_begin);

                    h.store_i16(0x509ac - vp_begin, '\x90\xe8');
                    h.replaceNearJmp(0x509ae - vp_begin, cursor);
                    h.store_i8(0x509b3 - vp_begin, '\x44');
                    h.store_i8(0x509b7 - vp_begin, '\x44');

                    static const char code_put[] =
                        "\x8b\x44\x24\x20"         // mov     eax,dword ptr [esp+20] ; apend_range
                        "\x2b\x44\x24\x1c"         // sub     eax,dword ptr [esp+1c] ; apend_size
                        "\x45"                     // inc     ebp
                        "\x81\xfd\x00\x04\x00\x00" // cmp     ebp,00000400
                        "\xc3"                     // ret
                        ;

                    memcpy(cursor, code_put, sizeof(code_put) - 1);
                    cursor += sizeof(code_put) - 1;

                    h.store_i8(0x509c7 - vp_begin, '\x05');
                    
                    h.store_i32(0x509e9 - vp_begin, '\x01\x7c\x24\x14');
                    h.store_i16(0x509ed - vp_begin, '\xa8\x01');

                    h.store_i8(0x50a05 - vp_begin, '\x8b');
                    h.store_i32(0x50a1b - vp_begin, '\xd1\x8b\xfa\x01');

                }

                {

                    /*
                    10050a7e 8d0480             lea     eax,dword ptr [eax+eax*4]
                    10050a81 8d0480             lea     eax,dword ptr [eax+eax*4]

                    10050ab6 8d0480             lea     eax,dword ptr [eax+eax*4]
                    10050ab9 8d0480             lea     eax,dword ptr [eax+eax*4]

                    ↓
                    10050a7e 90                 nop
                    10050a7f e8XxXxXxXx         call    cursor

                    10000000 837c246c00         cmp     dword ptr [esp+6c],+00
                    10000000 740d               jz      skip,+0d
                    10000000 8b6c242c           mov     ebp,dword ptr [esp+2c]
                    10000000 8b14adXxXxXxXx     mov     edx,dword ptr [ebp*4+exedit+1b0e28]
                    10000000 03c2               add     eax,edx
                    10000000 2b442420           sub     eax,dword ptr [esp+20] ; apend_range
                    10000000 0344241c           add     eax,dword ptr [esp+1c] ; apend_size
                    10000000 8d0480             mov     eax,eax*5
                    10000000 8d0480             mov     eax,eax*5
                    10000000 c3                 ret
                    */
                    constexpr int vp_begin = 0x50a7e;
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x50b86 - vp_begin);
                    h.store_i16(0x50a7e - vp_begin, '\x90\xe8');
                    h.replaceNearJmp(0x50a80 - vp_begin, cursor);
                    h.store_i16(0x50ab6 - vp_begin, '\x90\xe8');
                    h.replaceNearJmp(0x50ab8 - vp_begin, cursor);

                    static const char code_put1[] =
                        "\x83\x7c\x24\x6c\x00"     // cmp     dword ptr[esp+6c],+00
                        "\x74\x0d"                 // jz      skip,+0d
                        "\x8b\x6c\x24\x2c"         // mov     ebp,dword ptr [esp+2c]
                        "\x8b\x14\xadXXXX"         // mov     edx,dword ptr[ebp*4+exedit+1b0e28]
                        "\x03\xc2"                 // add     eax,edx
                        "\x2b\x44\x24\x20"         // sub     eax,dword ptr [esp+20] ; apend_range
                        "\x03\x44\x24\x1c"         // add     eax,dword ptr [esp+1c] ; apend_size
                        "\x8d\x04\x80"             // mov     eax,eax*5
                        "\x8d\x04\x80"             // mov     eax,eax*5
                        "\xc3"                     // ret
                        ;

                    memcpy(cursor, code_put1, sizeof(code_put1) - 1);
                    store_i32(cursor + 14, GLOBAL::exedit_base + 0x1b0e28);
                    cursor += sizeof(code_put1) - 1;

                    /*
                    10050aeb c1e81f             shr     eax,1f
                    10050aee 03d0               add     edx,eax

                    10050b00 c1e81f             shr     eax,1f
                    10050b03 03d0               add     edx,eax

                    10050b6c c1e81f             shr     eax,1f
                    10050b6f 03d0               add     edx,eax

                    10050b81 c1e81f             shr     eax,1f
                    10050b84 03d0               add     edx,eax

                    ↓
                    10050aeb e8XxXxXxXx         call    cursor

                    10000000 c1e81f             shr     eax,1f
                    10000000 03d0               add     edx,eax
                    10000000 2b54241c           sub     edx,dword ptr [esp+1c]
                    10000000 03542420           add     edx,dword ptr [esp+20]
                    10000000 837c246c00         cmp     dword ptr [esp+6c],+00
                    10000000 740d               jz      skip,+0d
                    10000000 8b6c242c           mov     ebp,dword ptr [esp+2c]
                    10000000 8b04adXxXxXxXx     mov     eax,dword ptr [ebp*4+exedit+1b0e28]
                    10000000 2bd0               sub     edx,eax
                    10000000 c3                 ret
                    */



                    h.store_i8(0x50aeb - vp_begin, '\xe8');
                    h.replaceNearJmp(0x50aec - vp_begin, cursor);
                    h.store_i8(0x50b00 - vp_begin, '\xe8');
                    h.replaceNearJmp(0x50b01 - vp_begin, cursor);
                    h.store_i8(0x50b6c - vp_begin, '\xe8');
                    h.replaceNearJmp(0x50b6d - vp_begin, cursor);
                    h.store_i8(0x50b81 - vp_begin, '\xe8');
                    h.replaceNearJmp(0x50b82 - vp_begin, cursor);

                    static const char code_put2[] =
                        "\xc1\xe8\x1f"             // shr     eax,1f
                        "\x03\xd0"                 // add     edx,eax
                        "\x2b\x54\x24\x1c"         // sub     edx,dword ptr [esp+1c]
                        "\x03\x54\x24\x20"         // add     edx,dword ptr [esp+20]
                        "\x83\x7c\x24\x6c\x00"     // cmp     dword ptr[esp+6c],+00
                        "\x74\x0d"                 // jz      skip,+0d
                        "\x8b\x6c\x24\x2c"         // mov     ebp,dword ptr [esp+2c]
                        "\x8b\x04\xadXXXX"         // mov     eax,dword ptr [ebp*4+exedit+1b0e28]
                        "\x2b\xd0"                 // sub     edx,eax
                        "\xc3"                     // ret
                        ;

                    memcpy(cursor, code_put2, sizeof(code_put2) - 1);
                    store_i32(cursor + 27, GLOBAL::exedit_base + 0x1b0e28);
                    cursor += sizeof(code_put2) - 1;

                    /*
                    10050b13 e9a3060000         jmp     100511bb

                    10050b9f e917060000         jmp     100511bb
                    ↓
                    cursorへ

                    10000000 8b7c2450           mov     edi,dword ptr [esp+50]
                    10000000 33c9               xor     ecx,ecx
                    10000000 8b6c2428           mov     ebp,dword ptr [esp+28]
                    10000000 45                 inc     ebp
                    10000000 81fd00040000       cmp     ebp,00000400
                    10000000 e9XxXxXxXx         jmp     exedit + 5050a
                    */
                    /*
                    h.replaceNearJmp(0x50b14 - vp_begin, cursor);
                    h.replaceNearJmp(0x50ba0 - vp_begin, cursor);
                    static const char code_put3[] =
                        "\x8b\x7c\x24\x50"         // mov     edi,dword ptr [esp+50]
                        "\x33\xc9"                 // xor     ecx,ecx
                        "\x8b\x6c\x24\x28"         // mov     ebp,dword ptr [esp+28]
                        "\x45"                     // inc     ebp
                        "\x81\xfd\x00\x04\x00\x00" // cmp     ebp,00000400
                        "\xe9"                     // jmp     exedit + 5050a
                        ;

                    memcpy(cursor, code_put3, sizeof(code_put3) - 1);
                    cursor += sizeof(code_put3) - 1 + 4;
                    store_i32(cursor - 4, GLOBAL::exedit_base + 0x5050a - (int)cursor);
                    */
                }

                {
                /*
                  100510e8 8d040a             mov     eax,edx+ecx
                  100510eb 3bf0               cmp     esi,eax
                  ↓
                  100510e8 e8XxXxXxXx         call    cursor

                  10000000 8d040a             mov     eax,edx+ecx
                  10000000 0344241c           add     eax,dword ptr [esp+1c] ; apend_size
                  10000000 2b442420           sub     eax,dword ptr [esp+20] ; apend_range
                  10000000 3bf0               cmp     esi,eax
                  10000000 c3                 ret 

                  10051133 3bd1               cmp     edx,ecx
                  10051135 894c2414           mov     dword ptr [esp+14],ecx
                  ↓
                  10051133 90                 nop
                  10051134 e8XxXxXxXx         call    excute_memory

                    10000000 894c2418           mov     dword ptr [esp+18],ecx
                    10000000 034c241c           add     ecx,dword ptr [esp+1c] ; apend_size
                    10000000 3bd1               cmp     edx,ecx
                    10000000 c3                 ret



                  1005115c 8b7c2430           mov     edi,dword ptr [esp+30]
                  10051160 03c1               add     eax,ecx
                  ↓
                  1005115c 90                 nop
                  1005115d e8XxXxXxXx         call    excute_memory

                    10000000 03c1               add     eax,ecx
                    10000000 0344241c           add     eax,dword ptr [esp+1c] ; apend_size
                    10000000 8b7c2434           mov     edi,dword ptr [esp+34]
                    10000000 c3



                  10051240 03c7               add     eax,edi
                  ↓
                  10051240 9090               nop


                  10051272 03c7               add     eax,edi
                  ↓
                  10051272 9090               nop
                */
                    constexpr int vp_begin = 0x510e8;
                    OverWriteOnProtectHelper h(GLOBAL::exedit_base + vp_begin, 0x51274 - vp_begin);
                    
                    h.store_i8(0x510e8 - vp_begin, '\xe8');
                    h.replaceNearJmp(0x510e9 - vp_begin, cursor);
                    static const char code_put1[] =
                        "\x8d\x04\x0a"             // mov     eax,edx+ecx
                        "\x03\x44\x24\x1c"         // add     eax,dword ptr [esp+1c] ; apend_size
                        "\x2b\x44\x24\x20"         // sub     eax,dword ptr [esp+20] ; apend_range
                        "\x3b\xf0"                 // cmp     esi,eax
                        "\xc3"                     // ret 
                        ;
                    memcpy(cursor, code_put1, sizeof(code_put1) - 1);
                    cursor += sizeof(code_put1) - 1;
                    
                    
                    h.store_i16(0x510ff - vp_begin, '\x90\xe8');
                    h.replaceNearJmp(0x51101 - vp_begin, cursor);
                    //100510ff 8b542454           mov     edx, dword ptr[esp+54]
                    //10051103 03c1               add     eax, ecx
                    static const char code_put2[] =
                        "\x8b\x54\x24\x58"         // mov     edx, dword ptr[esp+58]
                        "\x03\xc1"                 // add     eax, ecx
                        "\x03\x44\x24\x1c"         // add     eax,dword ptr [esp+1c] ; apend_size
                        "\xc3"                     // ret 
                        ;
                    memcpy(cursor, code_put2, sizeof(code_put2) - 1);
                    cursor += sizeof(code_put2) - 1;


                    h.store_i16(0x51133 - vp_begin, '\x90\xe8');
                    h.replaceNearJmp(0x51135 - vp_begin, cursor);
                    static const char code_put3[] =
                        "\x89\x4c\x24\x18"         // mov     dword ptr [esp+18],ecx
                        "\x03\x4c\x24\x1c"         // add     ecx,dword ptr [esp+1c] ; apend_size
                        "\x3b\xd1"                 // cmp     edx,ecx
                        "\xc3"                     // ret
                        ;
                    memcpy(cursor, code_put3, sizeof(code_put3) - 1);
                    cursor += sizeof(code_put3) - 1;

                    h.store_i16(0x5115c - vp_begin, '\x90\xe8');
                    h.replaceNearJmp(0x5115e - vp_begin, cursor);
                    static const char code_put4[] =
                        "\x03\xc1"                 // add     eax,ecx
                        "\x03\x44\x24\x1c"         // add     eax,dword ptr [esp+1c] ; apend_size
                        "\x8b\x7c\x24\x34"         // mov     edi,dword ptr [esp+34]
                        "\xc3"                     // ret
                        ;
                    memcpy(cursor, code_put4, sizeof(code_put4) - 1);
                    cursor += sizeof(code_put4) - 1;

                    h.store_i16(0x51240 - vp_begin, '\x90\x90');
                    h.store_i16(0x51272 - vp_begin, '\x90\x90');
                }
            }
#endif

        }

        void switching(bool flag) {
            enabled = flag;
        }

        bool is_enabled() { return enabled; }
        bool is_enabled_i() { return enabled_i; }

        void switch_load(ConfigReader& cr) {
            cr.regist(key, [this](json_value_s* value) {
                ConfigReader::load_variable(value, enabled);
            });
        }

        void switch_store(ConfigWriter& cw) {
            cw.append(key, enabled);
        }
    } text_op_size;
} // namespace patch
#endif // ifdef PATCH_SWITCH_TEXT_OP_SIZE
