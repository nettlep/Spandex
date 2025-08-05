// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [SDXMACRO.H  ] - Macros for the SPANDEX library                        ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
// These globals are defined in globals.c (used below...)

extern   INT   _t_fxfl[];
extern   INT   _t_sqrttab[];
extern   INT   _t_rsqrttab[];
extern   INT   _t_exptab[];
extern   INT   _t_rexptab[];

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
// Border color stuff

#define  SPDXSetBordColor(C)        \
{                                   \
   inp(CRT_STATUS_REGISTER);        \
   outp(VGA_ATTR_REGISTER,OVERSCAN);\
   outp(VGA_ATTR_REGISTER,(C));     \
   inp(CRT_STATUS_REGISTER);        \
   outp(VGA_ATTR_REGISTER,PALRESET);\
}

#define  SPDXTimeBorder(C)          \
{                                   \
   WAIT_FOR_VB;                     \
   WAIT_FOR_NO_VB;                  \
   SPDXSetBordColor(C)              \
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#define  SPDXCalcProjection(Relz, Ver, VScreen)                       \
   Relz = Ver->CLoc.z;                                                \
   if (Relz < SPDXGlobals.NearZ)                                      \
   {                                                                  \
      Ver->Scr.x = Ver->Scr.y = 1.0;                                  \
      Ver->Visible = VIS_CLIPZ;                                       \
   }                                                                  \
   else                                                               \
   {                                                                  \
      Relz = 1.0 / Relz;                                              \
      Ver->Scr.x = Ver->CLoc.x * VScreen->ScaleX * Relz;              \
      Ver->Scr.y = Ver->CLoc.y * VScreen->ScaleY * Relz;              \
                                                                      \
      if (Ver->Scr.x >  15000.0 || Ver->Scr.y >  15000.0 ||           \
          Ver->Scr.x < -15000.0 || Ver->Scr.y < -15000.0)             \
      {                                                               \
         Ver->Scr.x  = Ver->Scr.y  = 1.0;                             \
         Ver->Scr.fx = Ver->Scr.fy = FXD_ONE;                         \
         Ver->Visible = VIS_NO;                                       \
      }                                                               \
      else                                                            \
      {                                                               \
         Ver->Scr.x  = VScreen->Center.x + Ver->Scr.x;                \
         Ver->Scr.y  = VScreen->Center.y - Ver->Scr.y;                \
         Ver->Scr.fx = FLT_TO_FXD(Ver->Scr.x);                        \
         Ver->Scr.fy = FLT_TO_FXD(Ver->Scr.y);                        \
      }                                                               \
   }

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
// Misc stuff (typical for speedups)

#define  MAX(A,B)                         ((A) > (B) ? (A) : (B))
#define  MIN(A,B)                         ((A) < (B) ? (A) : (B))
#define  ABS(val)                         (((val) >= 0) ? (val):-(val))
#define  SWAP(A,B,C)                      {(C)=(A);(A)=(B);(B)=(C);}
#define  WAIT_FOR_VB                      while( (inp(SPDXGlobals.Port3x4+6)&0x08))
#define  WAIT_FOR_NO_VB                   while(!(inp(SPDXGlobals.Port3x4+6)&0x08))
#define  RANDOM(r)                        (rand()/(RAND_MAX/(r)))
#define  CEILING(Val,Hi)                  ((Val)>(Hi)?(Hi):(Val))
#define  FLOOR(Val,Lo)                    ((Val)<(Lo)?(Lo):(Val))
#define  ROUND_TO_INT(Val)                ((INT)((Val)+0.5))
#define  SPDXReverseVisibilityTest(a, b)  ((SPDXAngleFromVectors((a),(b))<=0)?VIS_YES:VIS_NO)
#define  SPDXVisibilityTest(a, b)         ((SPDXAngleFromVectors((a),(b))>=0)?VIS_YES:VIS_NO)
#define  SPDXMalloc( Size )               _SPDXMalloc((Size),__FILE__,__LINE__,_FUNCTION_)
#define  SPDXRealloc( Address, Size )     _SPDXRealloc((Address),(Size),__FILE__,__LINE__,_FUNCTION_)
#define  SPDXFree( Address )              _SPDXFree((Address),__FILE__,__LINE__,_FUNCTION_)
#define  FOREVER                          for(;;)
#define  SPDXLogError( code, notes )      _SPDXLogError(__FILE__,__LINE__,_FUNCTION_,(code),(notes))
#define  SPDXLogString( notes )           _SPDXLogString(__FILE__,__LINE__,_FUNCTION_,(notes))
#define  SPDXFatalError( code, notes )    _SPDXFatalError(__FILE__,__LINE__,_FUNCTION_,(code),(notes))
#define  CHECK_INIT                       (SPDXGlobals.InitFlag==INIT_VAL)
#define  DEG_TO_RAD(x)                    ((x)*3.141592654/(360.0/2.0))

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
// Matrix manipulations

#define  SPDXMatrixMulMatrix(d, a, b)                                         \
{                                                                             \
   (d).m00 = ((a).m00 * (b).m00) + ((a).m01 * (b).m10) + ((a).m02 * (b).m20); \
   (d).m01 = ((a).m00 * (b).m01) + ((a).m01 * (b).m11) + ((a).m02 * (b).m21); \
   (d).m02 = ((a).m00 * (b).m02) + ((a).m01 * (b).m12) + ((a).m02 * (b).m22); \
   (d).m10 = ((a).m10 * (b).m00) + ((a).m11 * (b).m10) + ((a).m12 * (b).m20); \
   (d).m11 = ((a).m10 * (b).m01) + ((a).m11 * (b).m11) + ((a).m12 * (b).m21); \
   (d).m12 = ((a).m10 * (b).m02) + ((a).m11 * (b).m12) + ((a).m12 * (b).m22); \
   (d).m20 = ((a).m20 * (b).m00) + ((a).m21 * (b).m10) + ((a).m22 * (b).m20); \
   (d).m21 = ((a).m20 * (b).m01) + ((a).m21 * (b).m11) + ((a).m22 * (b).m21); \
   (d).m22 = ((a).m20 * (b).m02) + ((a).m21 * (b).m12) + ((a).m22 * (b).m22); \
}

#define  SPDXMatrixMulVector(d, a, b)                                         \
{                                                                             \
   (d).dx = ((a).m00 * (b).dx) + ((a).m01 * (b).dy) + ((a).m02 * (b).dz);     \
   (d).dy = ((a).m10 * (b).dx) + ((a).m11 * (b).dy) + ((a).m12 * (b).dz);     \
   (d).dz = ((a).m20 * (b).dx) + ((a).m21 * (b).dy) + ((a).m22 * (b).dz);     \
}

#define  SPDXMatrixMulVertex(d, a, b)                                         \
{                                                                             \
   (d).x = ((a).m00 * (b).x) + ((a).m01 * (b).y) + ((a).m02 * (b).z);         \
   (d).y = ((a).m10 * (b).x) + ((a).m11 * (b).y) + ((a).m12 * (b).z);         \
   (d).z = ((a).m20 * (b).x) + ((a).m21 * (b).y) + ((a).m22 * (b).z);         \
}

#define  SPDXMakeXMatrix(m, x)                                                \
{                                                                             \
   SPDXMemSetDWORD(&m, 0, sizeof( MAT ) >> 2 );                               \
   (m).m00 =  1.0;                                                            \
   (m).m11 =  Cosine(x); (m).m12 =  Sine(x);                                  \
   (m).m21 = -Sine(x);   (m).m22 =  Cosine(x);                                \
}

#define  SPDXMakeYMatrix(m, y)                                                \
{                                                                             \
   SPDXMemSetDWORD(&m, 0, sizeof( MAT ) >> 2 );                               \
   (m).m00 =  Cosine(y); (m).m02 = -Sine(y);                                  \
   (m).m11 =  1.0;                                                            \
   (m).m20 =  Sine(y);   (m).m22 =  Cosine(y);                                \
}

#define  SPDXMakeZMatrix(m, z)                                                \
{                                                                             \
   SPDXMemSetDWORD(&m, 0, sizeof( MAT ) >> 2 );                               \
   (m).m00 =  Cosine(z); (m).m01 =  Sine(z);                                  \
   (m).m10 = -Sine(z);   (m).m11 =  Cosine(z);                                \
   (m).m22 =  1.0;                                                            \
}

#define  SPDXMakeMatrixIdentity(m)                                            \
{                                                                             \
   SPDXMemSetDWORD( &m, 0, sizeof(MAT)>>2 );                                  \
   (m).m00 = (m).m11 = (m).m22 = 1.0;                                         \
}

#define  SPDXInvertOrthoMatrix(m)                                             \
{                                                                             \
FLT a;                                                                        \
   SWAP((m).m02, (m).m20, a);                                                 \
   SWAP((m).m01, (m).m10, a);                                                 \
   SWAP((m).m12, (m).m21, a);                                                 \
}

#define  SPDXOrthoNormalizeMatrix(m)                                 \
{                                                                    \
FLT dot1,dot2,vlen;                                                  \
   dot1 = (m).m00 * (m).m10 + (m).m01 * (m).m11 + (m).m02 * (m).m12; \
   dot2 = (m).m00 * (m).m20 + (m).m01 * (m).m21 + (m).m02 * (m).m22; \
                                                                     \
   (m).m00 -= dot1 * (m).m10;                                        \
   (m).m00 -= dot2 * (m).m20;                                        \
   (m).m01 -= dot1 * (m).m11;                                        \
   (m).m01 -= dot2 * (m).m21;                                        \
   (m).m02 -= dot1 * (m).m12;                                        \
   (m).m02 -= dot2 * (m).m22;                                        \
                                                                     \
   vlen = 1.0 / sqrt((m).m00 * (m).m00 +                             \
                     (m).m01 * (m).m01 +                             \
                     (m).m02 * (m).m02);                             \
                                                                     \
   (m).m00 *= vlen;                                                  \
   (m).m01 *= vlen;                                                  \
   (m).m02 *= vlen;                                                  \
                                                                     \
   dot1 = (m).m10 * (m).m00 + (m).m11 * (m).m01 + (m).m12 * (m).m02; \
   dot2 = (m).m10 * (m).m20 + (m).m11 * (m).m21 + (m).m12 * (m).m22; \
                                                                     \
   (m).m10 -= dot1 * (m).m00;                                        \
   (m).m10 -= dot2 * (m).m20;                                        \
   (m).m11 -= dot1 * (m).m01;                                        \
   (m).m11 -= dot2 * (m).m21;                                        \
   (m).m12 -= dot1 * (m).m02;                                        \
   (m).m12 -= dot2 * (m).m22;                                        \
                                                                     \
   vlen = 1.0 / sqrt((m).m10 * (m).m10 +                             \
                     (m).m11 * (m).m11 +                             \
                     (m).m12 * (m).m12);                             \
                                                                     \
   (m).m10 *= vlen;                                                  \
   (m).m11 *= vlen;                                                  \
   (m).m12 *= vlen;                                                  \
                                                                     \
   (m).m20 = (m).m01 * (m).m12 - (m).m02 * (m).m11;                  \
   (m).m21 = (m).m02 * (m).m10 - (m).m00 * (m).m12;                  \
   (m).m22 = (m).m00 * (m).m11 - (m).m01 * (m).m10;                  \
}


/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
// Vector manipulations

#define  SPDXVectorFromPoints(V, P1, P2 )                            \
{                                                                    \
   (V).dx = (P2).x - (P1).x;                                         \
   (V).dy = (P2).y - (P1).y;                                         \
   (V).dz = (P2).z - (P1).z;                                         \
}

#define  SPDXInvertVector(v)                                         \
{                                                                    \
   (v).dx = -(v).dx;                                                 \
   (v).dy = -(v).dy;                                                 \
   (v).dz = -(v).dz;                                                 \
}

#define  SPDXAngleFromVectors(V1, V2 )                               \
   SPDXDotProductOfVectors( (V1), (V2) )

#define  SPDXDotProductOfVector(V)                                   \
   ((V)->dx * (V)->dx + (V)->dy * (V)->dy + (V)->dz * (V)->dz)

#define  SPDXDotProductOfVectors(V1, V2)                             \
   ((V1)->dx * (V2)->dx + (V1)->dy * (V2)->dy + (V1)->dz * (V2)->dz)

#define  SPDXVectorToUnitVector( V, VLen )                           \
{                                                                    \
   VLen = 1.0 / sqrt((V).dx * (V).dx +                               \
                     (V).dy * (V).dy +                               \
                     (V).dz * (V).dz);                               \
   (V).dx *= VLen;                                                   \
   (V).dy *= VLen;                                                   \
   (V).dz *= VLen;                                                   \
}

#define  SPDXVectorLength(V)  FLT_SQRT(SPDXDotProductOfVector(V))

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
// Asserts and Verifys...

#ifdef U_R
#define  Verify(x)   if (x)  {}
#else
#define  Verify(x)   if (!(x)) {BREAK(); SPDXFatalError( LE_VFAIL, "An internal `Verify' has failed" );}
#endif

#ifdef U_R
#define  Assert(x)
#else
#define  Assert(x)   if (!(x)) {BREAK(); SPDXFatalError( LE_AFAIL, "An internal `Assert' has failed" );}
#endif

#ifdef U_R
#define FUNC(x)
#define _FUNCTION_ ""
#else
#define FUNC(x)      static char _FUNCTION_[] = {x};_FUNCTION_;
#endif

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
// Inline palette manipulation

INT SPDXSetPal( BYT *Palette );                 // For changing the palette

#pragma aux SPDXSetPal =  \
   "     mov   esi,eax"   \
   "     mov   bh,0"      \
   "     mov   bl,2"      \
   "     mov   ecx,384"   \
   "s:"                   \
   "     mov   dx,03DAh"  \
   "v1:"                  \
   "     in    al,dx"     \
   "     test  al,08h"    \
   "     jnz   v1"        \
   "v2:"                  \
   "     in    al,dx"     \
   "     test  al,08h"    \
   "     jz    v2"        \
   "     mov   al,bh"     \
   "     mov   dx,03c8h"  \
   "     push  ecx"       \
   "     out   dx,al"     \
   "     inc   dx"        \
   "     rep   outsb"     \
   "     pop   ecx"       \
   "     add   bh,128"    \
   "     dec   bl"        \
   "     jnz   s"         \
    parm caller [eax]     \
    modify [eax ebx ecx edx esi];

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXBlastBytes( CHR *Dest, INT Val, INT Len );

#pragma aux SPDXBlastBytes =     \
   "or  ecx,ecx"                 \
   "js  skip"                    \
   "mov ebx,ecx"                 \
   "and ecx,3"                   \
   "rep stosb"                   \
   "mov ecx,ebx"                 \
   "shr ecx,2"                   \
   "rep stosd"                   \
   "skip:"                       \
   parm [edi] [eax] [ecx]        \
   modify [ebx ecx edi];

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXMemSetDWORD( void *Addr, INT Val, INT Size );

#pragma aux SPDXMemSetDWORD =      \
   "     or    ecx,ecx"            \
   "     jz    exit1"              \
   "     rep   stosd"              \
   "exit1:"                        \
    parm caller [edi] [eax] [ecx]  \
    modify [ecx edi];

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXMemSetWORD( void *Addr, WRD Val, INT Size );

#pragma aux SPDXMemSetWORD =       \
   "     or    ecx,ecx"            \
   "     jz    exit2"              \
   "     rep   stosw"              \
   "exit2:"                        \
    parm caller [edi] [eax] [ecx]  \
    modify [ecx edi];

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXMemSetBYTE( void *Addr, BYT Val, INT Size );

#pragma aux SPDXMemSetBYTE =       \
   "     or    ecx,ecx"            \
   "     jz    exit3"              \
   "     rep   stosb"              \
   "exit3:"                        \
    parm caller [edi] [eax] [ecx]  \
    modify [ecx edi];

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXMemCopyDWORD( void *Dest, void *Src, INT Size );

#pragma aux SPDXMemCopyDWORD =     \
   "     or    ecx,ecx"            \
   "     jz    exit4"              \
   "     rep   movsd"              \
   "exit4:"                        \
    parm caller [edi] [esi] [ecx]  \
    modify [ecx esi edi];

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXMemCopyWORD( void *Dest, void *Src, INT Size );

#pragma aux SPDXMemCopyWORD =      \
   "     or    ecx,ecx"            \
   "     jz    exit5"              \
   "     rep   movsw"              \
   "exit5:"                        \
    parm caller [edi] [esi] [ecx]  \
    modify [ecx esi edi];

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXMemCopyBYTE( void *Dest, void *Src, INT Size );

#pragma aux SPDXMemCopyBYTE =      \
   "     or    ecx,ecx"            \
   "     jz    exit6"              \
   "     rep   movsb"              \
   "exit6:"                        \
    parm caller [edi] [esi] [ecx]  \
    modify [ecx esi edi];

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
// This one's handy for the Asserts and Verifys

void  BREAK( void );

#pragma aux BREAK = \
   "     int   3"

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  RDTSC_Start( INT *High, INT *Low );

#pragma aux RDTSC_Start =   \
   "     db    0x0f, 0x31"  \
   "     mov   [esi], edx"  \
   "     mov   [edi], eax"  \
    parm caller [esi] [edi]

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   RDTSC_End( INT High, INT Low );

#pragma aux RDTSC_End =     \
   "     db    0x0f, 0x31"  \
   "     sub   eax,edi"     \
   "     sbb   edx,esi"     \
    value [eax]             \
    parm caller [esi] [edi]

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
// Fast square root (accurate to within 5 digits past decimal point)

FLT   FLT_SQRT( FLT Value );

#pragma aux FLT_SQRT =                           \
   "     mov   ecx,eax"                          \
   "     and   eax,0xffffff"                     \
   "     shr   ecx,23"                           \
   "     xor   eax,0x800000"                     \
   "     shr   eax,12"                           \
   "     mov   edx,[eax*4+_t_sqrttab]"           \
   "     mov   ecx,[ecx*4+_t_exptab]"            \
   "     or    ecx,edx"                          \
   "     mov   _t_fxfl,ecx"                      \
   "     fld   _t_fxfl"                          \
    parm caller [eax]                            \
    modify [eax ecx edx];

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
// 1.0 / sqrt - fast reciprocal square root (to within 5 digits)

FLT   FLT_RSQRT( FLT Value );

#pragma aux FLT_RSQRT =                           \
   "     mov   ecx,eax"                           \
   "     and   eax,0xffffff"                      \
   "     shr   ecx,23"                            \
   "     xor   eax,0x800000"                      \
   "     shr   eax,12"                            \
   "     mov   edx,[eax*4+_t_rsqrttab]"           \
   "     mov   ecx,[ecx*4+_t_rexptab]"            \
   "     test  edx,edx"                           \
   "     jz    rsqrt_psqr"                        \
   "     or    ecx,edx"                           \
   "     mov   _t_fxfl,ecx"                       \
   "     fld   _t_fxfl"                           \
   "     jmp   rsqrt_exit"                        \
   "rsqrt_psqr:"                                  \
   "     add   ecx,0x800000"                      \
   "     or    ecx,edx"                           \
   "     mov   _t_fxfl,ecx"                       \
   "     fld   _t_fxfl"                           \
   "rsqrt_exit:"                                  \
    parm caller [eax]                             \
    modify [eax ecx edx];

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [SDXMACRO.H  ] - End Of File                                           ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
