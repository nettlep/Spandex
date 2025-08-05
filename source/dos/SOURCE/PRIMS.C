// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [PRIMS.C     ] - Grphics primitive functions                           ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#include <i86.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include "spandex.h"

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static   INT   MCGAMode;
static   int   VESAAvailable;

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXInitScreen( void )
{
   FUNC("SPDXInitScreen");
   MCGAMode = FALSE;
   VESAAvailable = FALSE;

   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXGetVESAInfo( VESA *VESAInfoStruct )
{
union    REGS InRegs, OutRegs;
struct   SREGS SegRegs;

   FUNC("SPDXGetVESAInfo");
   Assert( VESAInfoStruct );

   InRegs.h.ah = 0x4F;
   InRegs.h.al = GET_VESA_INFO;
   InRegs.w.di = (WRD) PROT_OFF(SPDXGlobals.TempVESAInfo);
   SegRegs.es = (WRD) PROT_SEG(SPDXGlobals.TempVESAInfo);
   SPDXint86x( VESA_INT, &InRegs, &OutRegs, &SegRegs );

   if (OutRegs.h.al != 0x4F)
   {
      SPDXLogError( LE_NOTAVAIL, "VESA support not available" );
      return LE_NOTAVAIL;
   }

   if (OutRegs.h.ah != 0x00)
   {
      SPDXLogError( LE_NOTAVAIL, "VESA information not available" );
      return LE_VINOTAVAIL;
   }
   SPDXMemCopyBYTE( VESAInfoStruct, SPDXGlobals.TempVESAInfo, sizeof(VESA) );
   VESAAvailable = TRUE;

   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXGetSVGAInfo( SVGA *SVGAInfoStruct, INT ModeNumber )
{
union    REGS InRegs, OutRegs;
struct   SREGS SegRegs;

   FUNC("SPDXGetSVGAInfo");
   Assert( SVGAInfoStruct );

   if (ModeNumber == VESA_MODE_200_256)
   {
      SPDXGlobals.TempSVGAInfo->ModeAttributes = 31;
      SPDXGlobals.TempSVGAInfo->WinAAttrubutes = 7;
      SPDXGlobals.TempSVGAInfo->WinBAttrubutes = 7;
      SPDXGlobals.TempSVGAInfo->WinGranularity = 64;
      SPDXGlobals.TempSVGAInfo->WinSize = 64;
      SPDXGlobals.TempSVGAInfo->WinASegment = 0xA000;
      SPDXGlobals.TempSVGAInfo->WinBSegment = 0;
      SPDXGlobals.TempSVGAInfo->FunctionPointer = 0;
      SPDXGlobals.TempSVGAInfo->BytesPerScanline = 320;

// Optional fields (now mandatory)

      SPDXGlobals.TempSVGAInfo->XResolution = 320;
      SPDXGlobals.TempSVGAInfo->YResolution = 200;
      SPDXGlobals.TempSVGAInfo->XCharSize = 8;
      SPDXGlobals.TempSVGAInfo->YCharSize = 8;
      SPDXGlobals.TempSVGAInfo->NumberOfPlanes = 1;
      SPDXGlobals.TempSVGAInfo->BitsPerPixel = 8;
      SPDXGlobals.TempSVGAInfo->NumberOfBanks = 1;
      SPDXGlobals.TempSVGAInfo->MemoryModel = 4;
      SPDXGlobals.TempSVGAInfo->BankSize = 0;
      SPDXGlobals.TempSVGAInfo->NumberOfImagePages = 1;
      SPDXGlobals.TempSVGAInfo->reserved01 = 0;

// New direct color fields

      SPDXGlobals.TempSVGAInfo->RedMaskSize = 0;
      SPDXGlobals.TempSVGAInfo->RedFieldPosition = 0;
      SPDXGlobals.TempSVGAInfo->GreenMaskSize = 0;
      SPDXGlobals.TempSVGAInfo->GreenFieldPosition = 0;
      SPDXGlobals.TempSVGAInfo->BlueMaskSize = 0;
      SPDXGlobals.TempSVGAInfo->BlueFieldPosition = 0;
      SPDXGlobals.TempSVGAInfo->ReservedMaskSize = 0;
      SPDXGlobals.TempSVGAInfo->ReservedFieldPosition = 0;
      SPDXGlobals.TempSVGAInfo->DirectColorModeInfo = 0;

      SPDXMemSetDWORD( SPDXGlobals.TempSVGAInfo->reserved02, 0, 216>>2 );
   }
   else
   {
      InRegs.h.ah = 0x4F;
      InRegs.h.al = GET_SVGA_INFO;
      InRegs.w.cx = (WRD) ModeNumber;
      InRegs.w.di = (WRD) PROT_OFF(SPDXGlobals.TempSVGAInfo);
      SegRegs.es = (WRD) PROT_SEG(SPDXGlobals.TempSVGAInfo);
      SPDXint86x( VESA_INT, &InRegs, &OutRegs, &SegRegs );

      if (OutRegs.h.al != 0x4F)
      {
         SPDXLogError( LE_NOTAVAIL, "VESA support not available" );
         return LE_NOTAVAIL;
      }

      if (OutRegs.h.ah != 0x00)
      {
         SPDXLogError( LE_NOTAVAIL, "SVGA information not available" );
         return LE_SINOTAVAIL;
      }
   }

   // Calculate the internal SPANDEX stuff...

   SPDXGlobals.TempSVGAInfo->WinPerScreen = (SPDXGlobals.TempSVGAInfo->XResolution *
                                             SPDXGlobals.TempSVGAInfo->YResolution) /
                                            (SPDXGlobals.TempSVGAInfo->WinSize * 1024);

   SPDXGlobals.TempSVGAInfo->WinPerScreenLeftover = (SPDXGlobals.TempSVGAInfo->XResolution *
                                                     SPDXGlobals.TempSVGAInfo->YResolution) %
                                                    (SPDXGlobals.TempSVGAInfo->WinSize * 1024);

   SPDXGlobals.TempSVGAInfo->WinIncrement = SPDXGlobals.TempSVGAInfo->WinSize /
                                            SPDXGlobals.TempSVGAInfo->WinGranularity;
   SPDXMemCopyBYTE( SVGAInfoStruct, SPDXGlobals.TempSVGAInfo, sizeof(SVGA));

   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXSetMode( INT ModeNumber, INT ClearFlag, INT SquareFlag )
{
INT   RetCode;
union REGS InRegs, OutRegs;

   FUNC("SPDXSetMode");
   Assert(CHECK_INIT);

   // Make sure they don't try to set a VESA mode without VESA capabilities...
   if (VESAAvailable == TRUE)
   {
      RetCode = SPDXGetSVGAInfo(&SPDXGlobals.SVGAInfo, ModeNumber);

      if (RetCode != LE_NONE)
      {
         SPDXLogError( RetCode, "Unable to get local SVGA info" );
         return RetCode;
      }
   }
   else if (ModeNumber != VESA_MODE_200_256)
   {
      SPDXLogError( LE_NOTAVAIL, "Unable to set hi-res mode without VESA" );
      return LE_NOTAVAIL;
   }

   if (ModeNumber == VESA_MODE_200_256)
   {
      InRegs.w.ax = 0x13;
      SPDXint86( VESA_INT, &InRegs, &OutRegs );

      // This'll do the square pixel mode...

      if (SquareFlag)
         outp( 0x3c2, 0xe3 );

      MCGAMode = TRUE;
   }
   else
   {
      InRegs.h.ah = 0x4F;
      InRegs.h.al = SET_SVGA_MODE;
      InRegs.w.bx = (WRD) ModeNumber;

      if (!ClearFlag)
         InRegs.w.bx += 0x8000;

      SPDXint86( VESA_INT, &InRegs, &OutRegs );

      if (OutRegs.h.al != 0x4F)
      {
         SPDXLogError( LE_NOTAVAIL, "VESA support not available" );
         return LE_NOTAVAIL;
      }

      if (OutRegs.h.ah != 0x00)
      {
         SPDXLogError( LE_NOTAVAIL, "VESA video mode not available" );
         return LE_MODENOTAVAIL;
      }

      MCGAMode = FALSE;
   }

   if (RetCode != LE_NONE)
   {
      SPDXLogError( RetCode, "Unable to set video mode" );
      return RetCode;
   }

   SPDXSetGlobalResolution(SPDXGlobals.SVGAInfo.XResolution,
                           SPDXGlobals.SVGAInfo.YResolution);

   SPDXGlobals.AspectX = 1.0;
   SPDXGlobals.AspectY = 1.333333333;

   SPDXGlobals.TotalColors = 1<<SPDXGlobals.SVGAInfo.BitsPerPixel;

   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXGetMode( INT *ModeNumber )
{
union REGS InRegs, OutRegs;

   FUNC("SPDXGetMode");
   Assert( ModeNumber );

   if (MCGAMode == TRUE)
   {
      *ModeNumber = VESA_MODE_200_256;
      return LE_NONE;
   }

   InRegs.h.ah = 0x4F;
   InRegs.h.al = GET_SVGA_MODE;

   SPDXint86( VESA_INT, &InRegs, &OutRegs );

   if (OutRegs.h.al != 0x4F)
   {
      SPDXLogError( LE_NOTAVAIL, "VESA support not available" );
      return LE_NOTAVAIL;
   }

   if (OutRegs.h.ah != 0x00)
   {
      SPDXLogError( LE_NOTAVAIL, "VESA GetSVGAMode support not available" );
      return LE_GMODENOTAVAIL;
   }

   *ModeNumber = OutRegs.w.bx;
   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXCopyToScreen( CHR *CopyFrom )
{
INT   LastWindowPos = 0;
INT   WindowRemaining = SPDXGlobals.SVGAInfo.WinPerScreen;

   FUNC("SPDXCopyToScreen");
   Assert( CopyFrom );

   while( WindowRemaining )
   {
      SPDXSetVideoWindow(0, LastWindowPos );
      SPDXMemCopyDWORD( (void *) (INT) (SPDXGlobals.SVGAInfo.WinASegment << 4), CopyFrom, SPDXGlobals.SVGAInfo.WinSize << 8 ); // * 1024/4
      LastWindowPos += SPDXGlobals.SVGAInfo.WinIncrement;
      CopyFrom += SPDXGlobals.SVGAInfo.WinSize << 10; // * 1024
      WindowRemaining--;
   }

   if (SPDXGlobals.SVGAInfo.WinPerScreenLeftover)
   {
      SPDXSetVideoWindow(0, LastWindowPos );
      SPDXMemCopyDWORD( (void *) (INT) (SPDXGlobals.SVGAInfo.WinASegment << 4), CopyFrom, SPDXGlobals.SVGAInfo.WinPerScreenLeftover>>2 );
   }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXCopyToScreenClipped( CHR *CopyFrom, int Left, int Top, int Right, int Bottom )
{
INT   WinPos, WinOff, iTemp;
INT   ResX, ResY;
INT   WinBankSize = SPDXGlobals.SVGAInfo.WinSize << 10;
CHR   *WinAddress = (CHR *) (SPDXGlobals.SVGAInfo.WinASegment << 4);
INT   RectHeight, RectWidth;

   FUNC("SPDXCopyToScreenClipped");
   Assert( CopyFrom );

   SPDXGetGlobalResolution(&ResX, &ResY);
   RectHeight = Bottom - Top;
   RectWidth = Right - Left;

   // Correct any errors in the Clipping values
   if (Left == Right || Top == Bottom || Left > ResX || Right < 0 || Top > ResY || Bottom < 0)
      return;
   if (Left   > Right)  SWAP(Left, Right, iTemp );
   if (Top    > Bottom) SWAP(Top, Bottom, iTemp );
   if (Left   < 0)      Left = 0;
   if (Top    < 0)      Top = 0;
   if (Right  > ResX)   Right = ResX;
   if (Bottom > ResY)   Bottom = ResY;

   // Set the starting bank
   iTemp = ((Top * ResX) + Left);
   CopyFrom += iTemp;
   WinPos = iTemp / WinBankSize;
   WinOff = iTemp % WinBankSize;
   SPDXSetVideoWindow(0, WinPos);

   // Do the scanlines
   while( RectHeight-- )
   {
      // Does this line split across a bank boundary?
      if (WinOff + RectWidth > WinBankSize)
      {
         // Copy the first part of the scanline within the current window
         iTemp = WinBankSize - WinOff;
         SPDXMemCopyBYTE( WinAddress + WinOff, CopyFrom, iTemp );

         // Next window
         SPDXSetVideoWindow(0, ++WinPos);

         // Copy the rest of the scanline to the beginning of the next window
         SPDXMemCopyBYTE( WinAddress, CopyFrom + iTemp, RectWidth - iTemp );
         WinOff = ResX - iTemp;
      }
      else
      {
         // Copy the whole scanline
         SPDXMemCopyBYTE( WinAddress + WinOff, CopyFrom, RectWidth );
         WinOff += ResX;
      }

      // There is the possiblility that a RectWidth will fit into the window,
      // but the addition of ResX to WinOff may put us past, so let's fix that

      if (WinOff > WinBankSize)
      {
         WinOff -= WinBankSize;
         SPDXSetVideoWindow(0, ++WinPos);
      }

      CopyFrom += ResX;
   }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXCopyFromScreen( CHR *CopyTo )
{
INT   LastWindowPos = 0;
INT   WindowRemaining = SPDXGlobals.SVGAInfo.WinPerScreen;

   FUNC("SPDXCopyFromScreen");
   Assert( CopyTo );

   while( WindowRemaining )
   {
      SPDXSetVideoWindow(0, LastWindowPos );
      SPDXMemCopyDWORD( CopyTo, (void *) (INT) (SPDXGlobals.SVGAInfo.WinASegment << 4), SPDXGlobals.SVGAInfo.WinSize << 8 ); // * 1024/4
      LastWindowPos += SPDXGlobals.SVGAInfo.WinIncrement;
      CopyTo += SPDXGlobals.SVGAInfo.WinSize << 10; // * 1024
      WindowRemaining--;
   }

   if (SPDXGlobals.SVGAInfo.WinPerScreenLeftover)
   {
      SPDXSetVideoWindow(0, LastWindowPos );
      SPDXMemCopyDWORD( CopyTo, (void *) (INT) (SPDXGlobals.SVGAInfo.WinASegment << 4), SPDXGlobals.SVGAInfo.WinPerScreenLeftover>>2 );
   }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXSetVideoWindow( INT WindowFlag, INT WindowPos )
{
union REGS InRegs, OutRegs;

   FUNC("SPDXSetVideoWindow");
   if (MCGAMode == TRUE)
   {
      return LE_NONE;
   }

   InRegs.h.ah = 0x4F;
   InRegs.h.al = SET_VID_WINDOW;
   InRegs.h.bh = 0x00;
   InRegs.h.bl = (CHR) WindowFlag;
   InRegs.w.dx = (WRD) WindowPos;

   SPDXint86( VESA_INT, &InRegs, &OutRegs );

   if (OutRegs.h.al != 0x4F)
   {
      SPDXLogError( LE_NOTAVAIL, "VESA support not available" );
      return LE_NOTAVAIL;
   }

   if (OutRegs.h.ah != 0x00)
   {
      SPDXLogError( LE_NOTAVAIL, "VESA SetVideoWindow support not available" );
      return LE_SWNOTAVAIL;
   }

   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXGetVideoWindow( INT WindowFlag, INT *WindowPos )
{
union REGS InRegs, OutRegs;

   FUNC("SPDXGetVideoWindow");
   Assert( WindowPos );

   if (MCGAMode == TRUE)
   {
      *WindowPos = 0;
      return LE_NONE;
   }

   InRegs.h.ah = 0x4F;
   InRegs.h.al = GET_VID_WINDOW;
   InRegs.h.bh = 0x01;
   InRegs.h.bl = (CHR) WindowFlag;

   SPDXint86( VESA_INT, &InRegs, &OutRegs );

   if (OutRegs.h.al != 0x4F)
   {
      SPDXLogError( LE_NOTAVAIL, "VESA support not available" );
      return LE_NOTAVAIL;
   }

   if (OutRegs.h.ah != 0x00)
   {
      SPDXLogError( LE_NOTAVAIL, "VESA GetVideoWindow support not available" );
      return LE_GWNOTAVAIL;
   }

   *WindowPos = OutRegs.w.dx;

   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXSetTextMode( void )
{
union REGS InRegs, OutRegs;

   FUNC("SPDXSetTextMode");
   InRegs.w.ax = 0x03;
   SPDXint86( VIDEO_INT, &InRegs, &OutRegs );
   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXSetScreenPixel( UINT X, UINT Y, CHR Color )
{
INT   A, B;
CHR   *C;

   FUNC("SPDXSetScreenPixel");
   if (X >= SPDXGlobals.ScrResX || Y >= SPDXGlobals.ScrResY )
      return;

   A = (SPDXGlobals.SVGAInfo.WinGranularity * 1024);
   B = (Y * SPDXGlobals.SVGAInfo.BytesPerScanline + X);
   C = (CHR *) (B % A + (INT) (SPDXGlobals.SVGAInfo.WinASegment << 4));

   SPDXSetVideoWindow( 0, B/A );

   *C = (CHR) Color;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

UINT  SPDXGetScreenPixel( UINT X, UINT Y )
{
INT   A, B;
CHR   *C;

   FUNC("SPDXGetScreenPixel");
   if (X >= SPDXGlobals.ScrResX || Y >= SPDXGlobals.ScrResY )
      return 0;

   A = (SPDXGlobals.SVGAInfo.WinGranularity * 1024);
   B = (Y * SPDXGlobals.SVGAInfo.BytesPerScanline + X);
   C = (CHR *) (B % A + (INT) (SPDXGlobals.SVGAInfo.WinASegment << 4));

   SPDXSetVideoWindow( 0, B/A );

   return (UINT) *C;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXDrawScreenLine( INT X1, INT Y1, INT X2, INT Y2, CHR Color )
{
INT   dx, dy, x, y, jmp = 0;

   FUNC("SPDXDrawScreenLine");
   Assert(CHECK_INIT);

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
            SPDXSetScreenPixel( x, Y1, Color );

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
            SPDXSetScreenPixel( x, Y1, Color );

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
            SPDXSetScreenPixel( X1, y, Color );

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
            SPDXSetScreenPixel( X1, y, Color );

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

VFONT *SPDXRegisterFont( UBYT *Font )
{
INT   Addr = 0, i;
VFONT *Fnt;

   FUNC("SPDXRegisterFont");
   Assert(CHECK_INIT);
   Assert( Font );

   Fnt = (VFONT *) SPDXMalloc(sizeof(VFONT) );
   
   if (!Fnt)
   {
      SPDXLogError( LE_NOMEM, "Unable to allocate memory for font" );
      return 0;
   }

   for( i = 0; i < TOTAL_FONT_CHARS; i++ )
   {
      Fnt->Offsets[i] = Addr;
      Addr += Font[Addr] * Font[Addr+1] + 2;
   }

   Fnt->Font = (CHR *) Font;

   return Fnt;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXDrawScreenText( VFONT *Font, CHR *String, INT X, INT Y )
{
INT   x, y, col, i, addr, height, width;

   FUNC("SPDXDrawScreenText");
   Assert(CHECK_INIT);
   Assert( Font );
   Assert( String );

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
               SPDXSetScreenPixel( x+X, y, (CHR) col );
         }

         addr += width;
      }

      X += width + 1;
   }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXConvertMap( BMAP *Map, INT Intensity )
{
INT   i, Cnt;
CHR   *pTemp1, *pTemp2;

   FUNC("SPDXConvertMap");
   Assert( Intensity < MAP_SHADES );
   Assert( Map );

   pTemp1 = (CHR *) Map->Data;
   pTemp2 = &SPDXGlobals.PalTab[MAP_COLORS*Intensity];
   Cnt = Map->XRes * Map->YRes;

   for (i = 0; i < Cnt; i++, pTemp1++)
      *pTemp1 = pTemp2[*pTemp1];
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [PRIMS.C     ] - End Of File                                           ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
