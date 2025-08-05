// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [VSCREEN.C   ] - Virtual Screen handling                               ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "spandex.h"

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

//#define  DIRECT_SCREEN

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXInitVirtualScreen( INT ResX, INT ResY, VSCR *VScreen )
{
INT   i;
CHR   *TempScreen;

   FUNC("SPDXInitVirtualScreen");
   Assert( VScreen );

#ifdef DIRECT_SCREEN
   TempScreen = (CHR *) 0xa0000;
   SPDXLogError( LE_NONE, "Using direct screen access" );
#else
   if (!SPDXGetVirtualBuffer(VScreen))
      TempScreen = (CHR *) SPDXMalloc(ResX * ResY);
   else
      TempScreen = (CHR *) SPDXRealloc(VScreen->DrawScreen, ResX * ResY);
#endif

   if (!TempScreen)
      return LE_NOMEMOFF;

   SPDXSetVirtualBuffer(VScreen, TempScreen);

#ifdef U_Z
   if (!SPDXGetVirtualZBuffer(VScreen))
      TempScreen = SPDXMalloc(ResX * ResY * sizeof(ZBUF));
   else
      TempScreen = SPDXRealloc( VScreen->ZBuffer, ResX*ResY * sizeof(ZBUF));

   if (!TempScreen)
      return LE_NOMEMOFF;

   SPDXSetVirtualZBuffer(VScreen, (ZBUF *) TempScreen);
#endif

   for( i = 0; i < ResY; i++ )
      VScreen->STab[i] = i * ResX;

   SPDXSetVirtualResolution( ResX, ResY, VScreen );
   SPDXSetVirtualClipping( 0, ResX - 1, 0, ResY - 1, VScreen );
   SPDXSetVirtualScaleFactors( ResY, ResY, VScreen );

   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXCleanupVirtualScreen(VSCR *VScreen)
{
   FUNC("SPDXCleanupVirtualScreen");
   Assert( VScreen );

   if (VScreen->DrawScreen)
   {
      SPDXFree(VScreen->DrawScreen);
      VScreen->DrawScreen = NULL;
   }
#ifdef U_Z
   if (VScreen->ZBuffer)
   {
      SPDXFree(VScreen->ZBuffer);
      VScreen->ZBuffer = NULL;
   }
#endif
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXSetVirtualClipping(INT Left, INT Right, INT Top, INT Bottom, VSCR *VScreen )
{
   FUNC("SPDXSetVirtualClipping");
   Assert( VScreen );

   VScreen->ClipMinX = Left;
   VScreen->ClipMinY = Top;
   VScreen->ClipMaxX = Right + 1;
   VScreen->ClipMaxY = Bottom + 1;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXGetVirtualClipping(INT *Left, INT *Right, INT *Top, INT *Bottom, VSCR *VScreen )
{
   FUNC("SPDXGetVirtualClipping");
   Assert( Left );
   Assert( Right );
   Assert( Top );
   Assert( Bottom );
   Assert( VScreen );

   *Left   = VScreen->ClipMinX;
   *Top    = VScreen->ClipMinY;
   *Right  = VScreen->ClipMaxX - 1;
   *Bottom = VScreen->ClipMaxY - 1;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXGetVirtualCenter( INT *CenterX, INT *CenterY, VSCR *VScreen )
{
   FUNC("SPDXGetVirtualCenter");
   Assert( CenterX );
   Assert( CenterY );
   Assert( VScreen );

   *CenterX = VScreen->Center.x;
   *CenterY = VScreen->Center.y;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXSetVirtualCenter(INT CenterX, INT CenterY, VSCR *VScreen )
{
   FUNC("SPDXSetVirtualCenter");
   Assert( VScreen );

   VScreen->Center.x = CenterX;
   VScreen->Center.y = CenterY;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXSetVirtualResolution(INT ResX, INT ResY, VSCR *VScreen )
{
   FUNC("SPDXSetVirtualResolution");
   Assert( VScreen );

   VScreen->iResX = ResX;
   VScreen->iResY = ResY;
   VScreen->fResX = ResX;
   VScreen->fResY = ResY;
   SPDXSetVirtualCenter(ResX / 2, ResY / 2, VScreen);
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXGetVirtualiResolution(INT *ResX, INT *ResY, VSCR *VScreen )
{
   FUNC("SPDXGetVirtualiResolution");
   Assert( VScreen );

   *ResX = VScreen->iResX;
   *ResY = VScreen->iResY;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXGetVirtualfResolution(FLT *ResX, FLT *ResY, VSCR *VScreen )
{
   FUNC("SPDXGetVirtualfResolution");
   Assert( VScreen );

   *ResX = VScreen->fResX;
   *ResY = VScreen->fResY;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

CHR   *SPDXGetVirtualBuffer( VSCR *VScreen )
{
   FUNC("SPDXGetVirtualBuffer");
   Assert( VScreen );

   return VScreen->DrawScreen;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXSetVirtualBuffer( VSCR *VScreen, CHR *Buffer )
{
   FUNC("SPDXSetVirtualBuffer");
   Assert( VScreen );
   Assert( Buffer );

   VScreen->DrawScreen = Buffer;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

ZBUF  *SPDXGetVirtualZBuffer( VSCR *VScreen )
{
   FUNC("SPDXGetVirtualZBuffer");
   Assert( VScreen );

#ifdef U_Z
   return VScreen->ZBuffer;
#else
   return NULL;
#endif
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXSetVirtualZBuffer( VSCR *VScreen, ZBUF *ZBuffer )
{
   FUNC("SPDXSetVirtualZBuffer");
   Assert( VScreen );
   Assert( ZBuffer );

#ifdef U_Z
   VScreen->ZBuffer = ZBuffer;
#endif
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXResetVirtualZBuffer( VSCR *VScreen )
{
   FUNC("SPDXResetVirtualZBuffer");
   Assert( VScreen );

#ifdef U_Z
   if (VScreen->ZBuffer)
      SPDXMemSetDWORD(VScreen->ZBuffer, 0xFFFFFFFF, (VScreen->iResX*VScreen->iResY*sizeof(ZBUF)) >> 2);
#endif
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXClearVirtualScreen( CHR Color, VSCR *VScreen )
{
   FUNC("SPDXClearVirtualScreen");
   Assert( VScreen );

   SPDXMemSetDWORD( VScreen->DrawScreen,
                    (Color << 24) + (Color << 16) + (Color << 8) + Color,
                    (VScreen->iResX * VScreen->iResY) >> 2 );
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXResetVirtualScreen( CHR Color, VSCR *VScreen )
{
   FUNC("SPDXResetVirtualScreen");
   Assert( VScreen );

   SPDXClearVirtualScreen( Color, VScreen );

#ifdef U_Z
   SPDXResetVirtualZBuffer( VScreen );
#endif
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXSetVirtualPixel( UINT X, UINT Y, CHR Color, VSCR *VScreen )
{
   FUNC("SPDXSetVirtualPixel");
   Assert( VScreen );

   if (X >= VScreen->iResX || Y >= VScreen->iResY)
      return;

   VScreen->DrawScreen[VScreen->STab[Y]+X] = Color;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

UINT  SPDXGetVirtualPixel( UINT X, UINT Y, VSCR *VScreen )
{
   FUNC("SPDXGetVirtualPixel");
   Assert( VScreen );

   if (X >= VScreen->iResX || Y >= VScreen->iResY)
      return 0;

   return VScreen->DrawScreen[VScreen->STab[Y]+X];
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXDrawVirtualLine( INT X1, INT Y1, INT X2, INT Y2, CHR Color, VSCR *VScreen )
{
INT   dx, dy, x, y, jmp = 0;

   FUNC("SPDXDrawVirtualLine");
   Assert( VScreen );

   dx = ABS(X1 - X2);
   dy = ABS(Y1 - Y2);

   // Primarily horizontal?
   if (dx >= dy)
   {
      jmp = dx >> 1;

      // Sort from left to right
      if (X1 > X2)
      {
         x = X1; X1 = X2; X2 = x;
         y = Y1; Y1 = Y2; Y2 = y;
      }

      // Upward direction?
      if (Y1 > Y2)
      {
         for (x = X1; x <= X2; x++)
         {
            SPDXSetVirtualPixel( x, Y1, Color, VScreen );

            jmp += dy;

            if (jmp > dx)
            {
               jmp -= dx;
               Y1--;
            }
         }
      }
      // Downward direction
      else
      {
         for (x = X1; x <= X2; x++)
         {
            SPDXSetVirtualPixel( x, Y1, Color, VScreen );

            jmp += dy;

            if (jmp > dx)
            {
               jmp -= dx;
               Y1++;
            }
         }
      }
   }
   // Primarily vertical
   else
   {
      jmp = dy >> 1;

      // Sort from top to bottom
      if (Y1 > Y2)
      {
         y = Y1; Y1 = Y2; Y2 = y;
         x = X1; X1 = X2; X2 = x;
      }

      // Right to left?
      if (X1 > X2)
      {
         for (y = Y1; y <= Y2; y++)
         {
            SPDXSetVirtualPixel( X1, y, Color, VScreen );

            jmp += dx;

            if (jmp > dy)
            {
               jmp -= dy;
               X1--;
            }
         }
      }
      // Left to right
      else
      {
         for (y = Y1; y <= Y2; y++)
         {
            SPDXSetVirtualPixel( X1, y, Color, VScreen );

            jmp += dx;

            if (jmp > dy)
            {
               jmp -= dy;
               X1++;
            }
         }
      }
   }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXDrawVirtualText( VFONT *Font, CHR *String, INT X, INT Y, VSCR *VScreen )
{
INT   x, y, col, i, addr, height, width;

   FUNC("SPDXDrawVirtualText");
   Assert(CHECK_INIT);
   Assert( Font );
   Assert( String );
   Assert( VScreen );

   for( i = 0; i < strlen(String); i++ )
   {
      if (String[i] < FONT_CHAR_OFFSET || String[i] > 127)
         continue;

      addr = Font->Offsets[String[i]-FONT_CHAR_OFFSET];

      height = Font->Font[addr+1];
      width = Font->Font[addr];
      addr+=2;

      for( y = Y; y < Y+height; y++ )
      {
         for( x = 0; x < width; x++ )
         {
            col = Font->Font[addr+x];

            if (col)
               SPDXSetVirtualPixel( x+X, y, (CHR) col, VScreen );
         }

         addr += width;
      }

      X += width + 1;
   }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXSetVirtualScaleFactors( FLT ScaleX, FLT ScaleY, VSCR *VScreen )
{
   FUNC("SPDXSetVirtualScaleFactors");
   Assert( VScreen );

   if (ScaleX < 10.0)
      ScaleX = 10.0;

   if (ScaleY < 10.0)
      ScaleY = 10.0;

   VScreen->ScaleX = ScaleX;
   VScreen->ScaleY = ScaleY;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXGetVirtualScaleFactors( FLT *ScaleX, FLT *ScaleY, VSCR *VScreen )
{
   FUNC("SPDXGetVirtualScaleFactors");
   Assert( VScreen );
   Assert( ScaleX );
   Assert( ScaleY );

   *ScaleX = VScreen->ScaleX;
   *ScaleY = VScreen->ScaleY;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXSetVirtualMaxPolysToRender( INT Max, VSCR *VScreen )
{
   FUNC("SPDXSetVirtualMaxPolysToRender");
   Assert( VScreen );

   if (Max < 0) Max = 0;

   VScreen->MaxPolysToRender = Max;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXGetVirtualMaxPolysToRender( VSCR *VScreen )
{
   FUNC("SPDXGetVirtualMaxPolysToRender");
   Assert( VScreen );

   return VScreen->MaxPolysToRender;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXSetVirtualLense( FLT Lense, VSCR *VScreen )
{
FLT   ScaleX, ScaleY, fTemp;

   FUNC("SPDXSetVirtualLense");
   Assert( VScreen );

   fTemp = (2.0 * tan(DEG_TO_RAD(SPDXCalcFOV(Lense)) / 2.0));
   ScaleX = VScreen->fResX / fTemp;
   ScaleY = VScreen->fResY / fTemp;

   SPDXSetVirtualScaleFactors( ScaleX, ScaleY, VScreen );
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXCorrectVirtualAspect( FLT XAspect, FLT YAspect, VSCR *VScreen )
{
FLT   ScaleX, ScaleY;

   FUNC("SPDXCorrectVirtualAspect");
   Assert( VScreen );

   SPDXGetVirtualScaleFactors( &ScaleX, &ScaleY, VScreen );

   ScaleX *= XAspect;
   ScaleY *= YAspect;

   SPDXSetVirtualScaleFactors( ScaleX, ScaleY, VScreen );
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXVirtualAntiAlias( VSCR *Dest, VSCR *Src )
{
INT   Width, Height, x, y;
CHR   *pOMap, *pDest, *pSrc;

   FUNC("SPDXVirtualAntiAlias");

   Assert( Dest );
   Assert( Src );

   // Make sure that the sizes are correct
   Assert( Dest->iResX == Src->iResX >> 1 );
   Assert( Dest->iResY == Src->iResY >> 1 );

   Width = Src->iResX;
   Height = Src->iResY;

   pDest = Dest->DrawScreen;
   pSrc = Src->DrawScreen;

   pOMap = SPDXGlobals.TransTab;
   Assert( pOMap );

   for( y = 0; y < Height; y+=2, pSrc += Width )
   {
      for( x = 0; x < Width; x+=2 )
      {
         *(pDest++) = pOMap[(pOMap[(pOMap[(*(pSrc++) << 8) + *pSrc] << 8) + *(pSrc+Width-1)] << 8) + *((pSrc++)+Width)];
      }
   }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXVirtualSmoothFrame( VSCR *VScreen )
{
INT   x, y, YOffset;
INT   ResX, ResY;
CHR   *MixTab, *ImageTop, *ImageBot, CurrMix, NextMix, *Image;

   FUNC("SPDXVirtualSmoothFrame");
   Assert( VScreen );

   SPDXGetVirtualiResolution( &ResX, &ResY, VScreen );
   Image = SPDXGetVirtualBuffer( VScreen );

   MixTab = SPDXGlobals.TransTab;
   Assert( MixTab );
   ImageTop = Image;
   ImageBot = &Image[ResX];

   for( y = 1; y < ResY; y++ )
   {
      YOffset = ResX;
      NextMix = MixTab[((*ImageTop)<<8)+(*ImageBot)];

      for( x = 1; x < ResX; x++ )
      {
         CurrMix = NextMix;
         NextMix = MixTab[((*(ImageTop+1))<<8)+(*(ImageBot+1))];
         *(Image++) = MixTab[(NextMix<<8)+CurrMix];
         ImageTop++;
         ImageBot++;
      }

      Image++;
      ImageTop++;
      ImageBot++;
   }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [VSCREEN.C   ] - End Of File                                           ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
