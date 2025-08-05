// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [SDXFIXED.H  ] - Fixed-point macros                                    ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
// These globals are defined in globals.c (used below...)

extern   INT   _t_bign[];
extern   INT   _t_fxfl[];
extern   INT   _t_frac[];
extern   FLT   _t_half;
extern   INT   _t_fxdflt;

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
// Fixed point conversion & Misc.

#define  FXD_HALF             (0x8000)
#define  FXD_ONE              (0x10000)

#define  INT_TO_FXD(Val)      ((FXD)((Val) << 16))
#define  WRD_TO_FXD(Val)      ((FXD)((Val) << 16))
#define  UWRD_TO_FXD(Val)     ((FXD)((Val) << 16))

#define  FXD_TO_FLT(Val)      (((FLT)(Val)) / 65536.0)
#define  FXD_TO_INT(Val)      ((INT)((Val) >> 16))
#define  FXD_TO_WRD(Val)      ((WRD)((Val) >> 16))
#define  FXD_TO_UWRD(Val)     ((UWRD)((UINT)(Val) >> 16))

#define  FXD_WHOLE_BITS       (0xFFFF0000)
#define  FXD_FRAC_BITS        (0x0000FFFF)

#define  ROUND_FXD_TO_INT(x)  (((x) + 0x8000) >> 16)
#define  FixedFrac(x)         ((x) & FXD_FRAC_BITS)
#define  FixedWhole(x)        ((x) & FXD_WHOLE_BITS)

#define  FXD_RANDOM(r)        (INT_TO_FXD((rand()/(RAND_MAX/(FXD_TO_INT(r))))))

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

FXD FixedMul(FXD Val1, FXD Val2);               // With rounding...

#pragma aux FixedMul =     \
   "imul edx"              \
   "add eax, 8000h"        \
   "adc edx, 0"            \
   "shrd eax, edx, 16"     \
   parm caller [eax] [edx] \
   value [eax]             \
   modify [eax edx];

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

FXD FixedDiv(FXD Val1, FXD Val2);               // No rounding...

#pragma aux FixedDiv =     \
   "xor eax, eax"          \
   "shrd eax, edx, 16"     \
   "sar edx, 16"           \
   "idiv ebx"              \
   parm caller [edx] [ebx] \
   value [eax]             \
   modify [eax ebx edx];

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

FXD FixedSquare(FXD Value);                     // x*x (faster than FixedMul)

#pragma aux FixedSquare =  \
   "imul eax"              \
   "add eax, 8000h"        \
   "adc edx, 0"            \
   "shrd eax, edx, 16"     \
   parm caller [eax]       \
   value [eax]             \
   modify [eax edx];

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

FXD OneOver(FXD Value);                         // 1/x (faster than FixedDiv)

#pragma aux OneOver =      \
   "xor eax, eax"          \
   "mov edx, 1"            \
   "idiv ebx"              \
   parm caller [ebx]       \
   value [eax]             \
   modify [eax ebx edx];

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

FXD FixedSqrt(FXD Value );                      // sqrt(x)

#pragma aux FixedSqrt =             \
   "         xor eax, eax"          \
   "         mov ebx, 40000000h"    \
   "sqrtHP1: mov edx, ecx"          \
   "         sub edx, ebx"          \
   "         jb  sqrtHP2"           \
   "         sub edx, eax"          \
   "         jb  sqrtHP2"           \
   "         mov ecx,edx"           \
   "         shr eax, 1"            \
   "         or  eax, ebx"          \
   "         shr ebx, 2"            \
   "         jnz sqrtHP1"           \
   "         jz  sqrtHP5"           \
   "sqrtHP2: shr eax, 1"            \
   "         shr ebx, 2"            \
   "         jnz sqrtHP1"           \
   "sqrtHP5: mov ebx, 00004000h"    \
   "         shl eax, 16"           \
   "         shl ecx, 16"           \
   "sqrtHP3: mov edx, ecx"          \
   "         sub edx, ebx"          \
   "         jb  sqrtHP4"           \
   "         sub edx, eax"          \
   "         jb  sqrtHP4"           \
   "         mov ecx, edx"          \
   "         shr eax, 1"            \
   "         or  eax, ebx"          \
   "         shr ebx, 2"            \
   "         jnz sqrtHP3"           \
   "         jmp sqrtHP6"           \
   "sqrtHP4: shr eax, 1"            \
   "         shr ebx, 2"            \
   "         jnz sqrtHP3"           \
   "sqrtHP6: nop"                   \
   parm caller [ecx]                \
   value [eax]                      \
   modify [eax ebx ecx edx];

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT IntegerSqrt( INT Value );                   // Integer sqrt()

#pragma aux IntegerSqrt =           \
   "         mov ecx,16"            \
   "         xor ebx,ebx"           \
   "         xor edx,edx"           \
   "SqrtLoop:"                      \
   "         adc eax,eax"           \
   "         adc ebx,ebx"           \
   "         adc eax,eax"           \
   "         adc ebx,ebx"           \
   "         shl edx,2"             \
   "         inc edx"               \
   "         cmp edx,ebx"           \
   "         jg  short Sqrt_00"     \
   "         sub ebx,edx"           \
   "         inc edx"               \
   "Sqrt_00:"                       \
   "         shr edx,1"             \
   "         dec ecx"               \
   "         jne short SqrtLoop"    \
   "         mov eax,edx"           \
   "         and eax,0ffffh"        \
    parm caller [eax]               \
    value [eax]                     \
    modify [eax ebx ecx edx];

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

FXD   FLT_TO_FXD( float start );                // Faster than a cast/FMUL

#pragma aux FLT_TO_FXD =             \
   "     mov   _t_fxfl, eax"         \
   "     fld   dword ptr _t_fxfl"    \
   "     fadd  qword ptr _t_bign"    \
   "     fstp  qword ptr _t_fxfl"    \
   "     mov   eax,_t_fxfl"          \
    value [eax]                      \
    parm caller [eax]

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   FLT_TO_INT( float start );                // Faster than a cast (no round)

#pragma aux FLT_TO_INT =             \
   "     mov   _t_fxfl, eax"         \
   "     fld   dword ptr _t_fxfl"    \
   "     fadd  qword ptr _t_bign"    \
   "     fstp  qword ptr _t_fxfl"    \
   "     mov   eax,_t_fxfl"          \
   "     sar   eax,16"               \
    value [eax]                      \
    parm caller [eax]

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

FLT   FLT_FRAC( float start );                  // Returns fractional component

#pragma aux FLT_FRAC =               \
   "     mov   _t_fxfl,eax"          \
   "     fld   dword ptr _t_fxfl"    \
   "     fsub  dword ptr _t_half"    \
   "     fistp dword ptr _t_frac"    \
   "     fld   dword ptr _t_fxfl"    \
   "     fisub dword ptr _t_frac"    \
    parm caller [eax]

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

FLT   FLT_1MINUSFRAC( float start );            // Returns 1 - frac component

#pragma aux FLT_1MINUSFRAC =         \
   "     mov   _t_fxfl,eax"          \
   "     fld   dword ptr [_t_fxfl]"  \
   "     fadd  dword ptr [_t_half]"  \
   "     frndint"                    \
   "     fsub  dword ptr [_t_fxfl]"  \
    parm caller [eax]

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

FLT FXD_FLTFRAC(INT a);

#pragma aux FXD_FLTFRAC =            \
   "     And   eax,0xffff"           \
   "     Mov   ebx,65536"            \
   "     Sub   ebx,eax"              \
   "     Mov   [_t_fxfl],ebx"        \
   "     Fild  dword ptr [_t_fxfl]"  \
   "     FMul  dword ptr [_t_fxdflt]"\
    parm [eax]                       \
    modify [ebx];

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

int  FIXED_DELTA(INT dx, INT dy);

#pragma aux FIXED_DELTA =            \
   "    Mov    edx,eax"              \
   "    Sar    edx,31"               \
   "    Shld   edx,eax,16"           \
   "    Shl    eax,16"               \
   "    IDiv   ebx"                  \
    parm [eax] [ebx]                 \
    modify [edx]                     \
    value  [eax];

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [SDXFIXED.H  ] - End Of File                                           ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
