// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [MOUSE.C     ] - Mouse routines                                        -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#ifdef _OS_DOS
#include "spandex.h"

/*----------------------------------------------------------------------------*/

INT   SPDXInitMouse( )
{
union REGS InRegs, OutRegs;

   FUNC("SPDXInitMouse");
   InRegs.w.ax = MOUSE_RESET;
   SPDXint86( MOUSE_INT, &InRegs, &OutRegs );

   if (OutRegs.w.ax != 0xffff)
   {
      SPDXLogError( LE_NOMOUSE, "Mouse driver not found" );
      return LE_NOMOUSE;
   }

   InRegs.w.ax = MOUSE_INIT;
   SPDXint86( MOUSE_INT, &InRegs, &OutRegs );

   if (OutRegs.w.ax != 0xffff)
   {
      SPDXLogError( LE_NOMOUSE, "Mouse driver not found" );
      return LE_NOMOUSE;
   }

   // SET THE RANGE
   SPDXSetMouseRange( 0, ROT_POINTS, 0, ROT_POINTS );
   SPDXSetMousePosition( ROT_POINTS/2, ROT_POINTS/2 );

   return LE_NONE;
}

/*----------------------------------------------------------------------------*/

void  SPDXSetMousePosition( INT X, INT Y )
{
union    REGS InRegs, OutRegs;

   FUNC("SPDXSetMousePosition");
   InRegs.w.ax = MOUSE_SET_POS;
   InRegs.w.cx = (UWRD) X;
   InRegs.w.dx = (UWRD) Y;
   SPDXint86( MOUSE_INT, &InRegs, &OutRegs );

   return;
}

/*----------------------------------------------------------------------------*/

void  SPDXSetMouseRange( INT X1, INT X2, INT Y1, INT Y2 )
{
union    REGS InRegs, OutRegs;

   FUNC("SPDXSetMouseRange");
   InRegs.w.ax = MOUSE_SET_COL;
   InRegs.w.cx = (UWRD) Y1;
   InRegs.w.dx = (UWRD) Y2;
   SPDXint86( MOUSE_INT, &InRegs, &OutRegs );

   InRegs.w.ax = MOUSE_SET_ROW;
   InRegs.w.cx = (UWRD) X1;
   InRegs.w.dx = (UWRD) X2;
   SPDXint86( MOUSE_INT, &InRegs, &OutRegs );

   return;
}

/*----------------------------------------------------------------------------*/

void  SPDXReadMouse( INT *Col, INT *Row, INT *BLeft, INT *BRight )
{
union    REGS InRegs, OutRegs;

   FUNC("SPDXReadMouse");
   Assert( Col );
   Assert( Row );
   Assert( BLeft );
   Assert( BRight );

   InRegs.w.ax = MOUSE_READ_INF;
   SPDXint86( MOUSE_INT, &InRegs, &OutRegs );

   *Col    = OutRegs.w.cx;
   *Row    = OutRegs.w.dx;
   *BLeft  = OutRegs.w.bx & 1;
   *BRight = (OutRegs.w.bx >> 1) & 1;

   return;
}

/*----------------------------------------------------------------------------*/

void  SPDXUninitMouse( )
{
   FUNC("SPDXUninitMouse");
   SPDXInitMouse( );
   return;
}
#endif // _OS_DOS

/*----------------------------------------------------------------------------
  -   [MOUSE.C     ] - End Of File                                           -
  ----------------------------------------------------------------------------*/
