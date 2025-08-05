// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [INT.C       ] - Interrupt handler for VGA                             -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#include "spandex.h"

/*----------------------------------------------------------------------------*/

static   void  (interrupt far *OldInterruptValue)();

static   WRD   IContDefault;
static   UBYT  IContVal, IContReg = 0x11;

/*----------------------------------------------------------------------------*/

void  SPDXInitializeInterrupts( )
{
   FUNC("SPDXInitializeInterrupts");
   _disable();
   outp( SPDXGlobals.Port3x4, IContReg );
   IContVal = (UBYT) inp( SPDXGlobals.Port3x4+1 );
   _enable();

   OldInterruptValue = _dos_getvect( INT_VB + 8);
   _dos_setvect( INT_VB + 8, SPDXInterruptRoutine );

   _disable();
   outp( 0x21, inp( 0x21 ) & 0xFB );
   IContDefault = (WRD) (((IContVal << 8) + IContReg) & 0xCFFF);
   outpw( SPDXGlobals.Port3x4, IContDefault );
   IContDefault = (WRD) (((IContVal << 8) + IContReg) | 0x1000);
   outpw( SPDXGlobals.Port3x4, IContDefault );
   _enable();
}

/*----------------------------------------------------------------------------*/

void  SPDXResetInterrupts( )
{
   FUNC("SPDXResetInterrupts");
   _dos_setvect( INT_VB + 8, OldInterruptValue );
   outp( 0x21, inp( 0x21 ) | 0x04 );
}

/*----------------------------------------------------------------------------*/

void interrupt far SPDXInterruptRoutine( )
{
   FUNC("SPDXInterruptRoutine");
   _disable();

   if (inp(VGA_STATUS_REGISTER) & 0x80)
   {
      if (SPDXGlobals.IRQ_OkToCopy)
      {
         SPDXCopyToScreen(SPDXGlobals.CurrentVScreen.DrawScreen);
         SPDXGlobals.IRQ_OkToCopy = 0;
      }

      IContDefault = (WRD) (((IContVal << 8) + IContReg) & 0xCFFF);
      outpw( SPDXGlobals.Port3x4, IContDefault );
      IContDefault = (WRD) (((IContVal << 8) + IContReg) | 0x1000);
      outpw( SPDXGlobals.Port3x4, IContDefault );
   }
   else
   {
      OldInterruptValue();
   }

   outp( INT_A00, EOI );

   _enable();

   return;
}

/*----------------------------------------------------------------------------
  -   [INT.C       ] - End Of File                                           -
  ----------------------------------------------------------------------------*/
