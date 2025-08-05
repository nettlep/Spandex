// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [LATMACRO.H  ] - Macros for the SPANDEX library                        -
  -                                                                          -
  ----------------------------------------------------------------------------*/

inline	void *SPDXDMalloc(INT size)
{
   return malloc(size);
}

/*----------------------------------------------------------------------------*/

inline	void  SPDXBlastBytes( CHR *Dest, INT Val, INT Len )
{
   while(Len--) *(Dest++) = Val;
}

/*----------------------------------------------------------------------------*/

inline	void  SPDXMemSetDWORD( void *Addr, INT Val, INT Size )
{
   INT *addr = (INT *) Addr;
   while(Size--) *(addr++) = Val;
}

/*----------------------------------------------------------------------------*/

inline	void  SPDXMemSetWORD( void *Addr, WRD Val, INT Size )
{
   WRD *addr = (WRD *) Addr;
   while(Size--) *(addr++) = Val;
}

/*----------------------------------------------------------------------------*/

inline	void  SPDXMemSetBYTE( void *Addr, BYT Val, INT Size )
{
   memset(Addr, Val, Size);
}

/*----------------------------------------------------------------------------*/

inline	void  SPDXMemCopyDWORD( void *Dest, void *Src, INT Size )
{
   INT *src = (INT *) Src, *dst = (INT *) Dest;
   while(Size--) *(dst++) = *(src++);
}

/*----------------------------------------------------------------------------*/

inline	void  SPDXMemCopyWORD( void *Dest, void *Src, INT Size )
{
   WRD *src = (WRD *) Src, *dst = (WRD *) Dest;
   while(Size--) *(dst++) = *(src++);
}

/*----------------------------------------------------------------------------*/

inline	void  SPDXMemCopyBYTE( void *Dest, void *Src, INT Size )
{
   BYT *src = (BYT *) Src, *dst = (BYT *) Dest;
   while(Size--) *(dst++) = *(src++);
}

/*----------------------------------------------------------------------------*/

inline	void  BREAK(void)
{
   abort();
}

/*----------------------------------------------------------------------------*/

inline	FLT FLT_SQRT( FLT Value )
{
   return sqrt(Value);
}

/*----------------------------------------------------------------------------*/

inline	FLT   FLT_RSQRT( FLT Value )
{
   return 1.0f / (FLT) sqrt(Value);
}

/*----------------------------------------------------------------------------*/

inline	INT   RDTSC_End( INT High, INT Low )
{
   // PDNDEBUG -- do this
   return High-Low;
}

/*----------------------------------------------------------------------------*/

inline	void  RDTSC_Start( INT *High, INT *Low )
{
   // PDNDEBUG -- do this
   *High = *Low;
}
