// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [PALETTE.C   ] - Palette handling routines                             ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#include <i86.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include "spandex.h"

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXSetPalette( INT Start, INT Count, PAL *Palette )
{
union    REGS InRegs, OutRegs;
struct   SREGS SegRegs;

   FUNC("SPDXSetPalette");
   Assert( Palette );

   SPDXMemCopyBYTE( SPDXGlobals.DOSMemPalette, Palette, Count * sizeof(RGB) );

   InRegs.h.ah = 0x10;
   InRegs.h.al = 0x12;
   InRegs.w.bx = (WRD) Start;
   InRegs.w.cx = (WRD) Count;
   InRegs.w.dx = (WRD) PROT_OFF(SPDXGlobals.DOSMemPalette);
   SegRegs.es = (WRD) PROT_SEG(SPDXGlobals.DOSMemPalette);
   SPDXint86x( VIDEO_INT, &InRegs, &OutRegs, &SegRegs );
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXGetPalette( INT Start, INT Count, PAL *Palette )
{
union    REGS InRegs, OutRegs;
struct   SREGS SegRegs;

   FUNC("SPDXGetPalette");
   Assert( Palette );

   InRegs.h.ah = 0x10;
   InRegs.h.al = 0x17;
   InRegs.w.bx = (WRD) Start;
   InRegs.w.cx = (WRD) Count;
   InRegs.w.dx = (WRD) PROT_OFF(SPDXGlobals.DOSMemPalette);
   SegRegs.es = (WRD) PROT_SEG(SPDXGlobals.DOSMemPalette);
   SPDXint86x( VIDEO_INT, &InRegs, &OutRegs, &SegRegs );

   SPDXMemCopyBYTE( Palette, SPDXGlobals.DOSMemPalette, Count * sizeof(RGB) );
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXPushPalette( PAL *Palette )
{
INT   Depth = SPDXGlobals.PaletteStackDepth;
PAL   *Temp1, TempPalette;

   FUNC("SPDXPushPalette");
   Assert( Palette );

   SPDXGetPalette( 0, TOTAL_COLORS, &TempPalette );
   SPDXSetPalette( 0, TOTAL_COLORS, Palette );

   if (!Depth)
   {
      SPDXGlobals.PaletteStack = (PAL *) SPDXMalloc(sizeof(PAL));

      if (!SPDXGlobals.PaletteStack)
         return LE_NOMEM;
   }
   else
   {
      Temp1 = (PAL *) SPDXRealloc( SPDXGlobals.PaletteStack, 
                                   sizeof(PAL) * (Depth+1) );

      if (!Temp1)
         return LE_NOMEM;

      SPDXGlobals.PaletteStack = Temp1;
   }

   SPDXMemCopyBYTE( &SPDXGlobals.PaletteStack[Depth], &TempPalette, sizeof(PAL) );

   SPDXGlobals.PaletteStackDepth++;

   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXPopPalette( PAL *Palette, INT Times )
{
PAL   *Temp1;

   FUNC("SPDXPopPalette");
   Assert( Palette );

   if (Times > SPDXGlobals.PaletteStackDepth)
      return LE_RANGE;

   SPDXGlobals.PaletteStackDepth -= Times;

   SPDXMemCopyBYTE( Palette,
                    &SPDXGlobals.PaletteStack[SPDXGlobals.PaletteStackDepth],
                    sizeof(PAL) );

   if (SPDXGlobals.PaletteStackDepth)
   {
      Temp1 = (PAL *) SPDXRealloc(SPDXGlobals.PaletteStack,
                                  sizeof(PAL) * SPDXGlobals.PaletteStackDepth);

      if (!Temp1)
         return LE_NOMEM;

      SPDXGlobals.PaletteStack = Temp1;
   }
   else
   {
      SPDXFree( SPDXGlobals.PaletteStack );
      SPDXGlobals.PaletteStack = NULL;
   }

   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXFadePalettes( PAL *From, PAL *To, INT Step )
{
INT   i, j;
FLT   f, Dif, fStep;
FLT   DeltaRedBuf[256];
FLT   DeltaGreenBuf[256];
FLT   DeltaBlueBuf[256];
FLT   RedBuf[256];
FLT   GreenBuf[256];
FLT   BlueBuf[256];
FLT   fOneOverStep;
PAL   Temp;

   FUNC("SPDXFadePalettes");
   Assert( From );
   Assert( To );

   fStep = Step;
   fOneOverStep = 1.0 / fStep;

   for( i = 0; i < 256; i++ )
   {
      Dif = To->Colors[i].Red - From->Colors[i].Red;
      DeltaRedBuf[i] = Dif * fOneOverStep;
      RedBuf[i] = From->Colors[i].Red;

      Dif = To->Colors[i].Green - From->Colors[i].Green;
      DeltaGreenBuf[i] = Dif * fOneOverStep;
      GreenBuf[i] = From->Colors[i].Green;

      Dif = To->Colors[i].Blue - From->Colors[i].Blue;
      DeltaBlueBuf[i] = Dif * fOneOverStep;
      BlueBuf[i] = From->Colors[i].Blue;
   }

   for( f = 0; f < fStep; f += 1.0 )
   {
      for( j = 0; j < 256; j++ )
      {
         RedBuf[j]   += DeltaRedBuf[j];
         GreenBuf[j] += DeltaGreenBuf[j];
         BlueBuf[j]  += DeltaBlueBuf[j];

         Temp.Colors[j].Red = RedBuf[j];
         Temp.Colors[j].Green = GreenBuf[j];
         Temp.Colors[j].Blue = BlueBuf[j];
      }

      if (SPDXGlobals.ScrResY == 200) SPDXSetPal( (CHR *) &Temp );
      else                            SPDXSetPalette( 0, 256, &Temp );
   }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXGetClosestColor( UINT Red, UINT Green, UINT Blue, PAL *Palette )
{
INT   i, ClosestDist = 0x7fffffff, r, g, b;
INT   Result;
UINT  Dist;

   Red   >>= 2;
   Green >>= 2;
   Blue  >>= 2;

   for(i = 0; i < 256; i++)
   {
      r = (INT)((UBYT) Palette->Colors[i].Red);
      g = (INT)((UBYT) Palette->Colors[i].Green);
      b = (INT)((UBYT) Palette->Colors[i].Blue);

      Dist = ((INT) Red   - r) * ((INT) Red   - r) +
             ((INT) Green - g) * ((INT) Green - g) +
             ((INT) Blue  - b) * ((INT) Blue  - b);

      if (Dist < (UINT) ClosestDist)
      {
         ClosestDist = Dist;
         Result = i;
      }
   }

   return Result;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [PALETTE.C   ] - End Of File                                           ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
