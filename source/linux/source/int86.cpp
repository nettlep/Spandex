// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [INT86.C     ] - Interrupt routines                                    -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#include "spandex.h"
               
/*----------------------------------------------------------------------------*/

static struct rminfo {
    LNG EDI;
    LNG ESI;
    LNG EBP;
    LNG reserved_by_system;
    LNG EBX;
    LNG EDX;
    LNG ECX;
    LNG EAX;
    WRD flags;
    WRD ES,DS,FS,GS,IP,CS,SP,SS;
} RMI;

/*----------------------------------------------------------------------------*/

void  SPDXint86( INT intno, union REGS *InputRegs, union REGS *OutputRegs )
{
struct   SREGS sregs;

   FUNC("SPDXint86");
   Assert( InputRegs );
   Assert( OutputRegs );

   SPDXMemSetBYTE(&RMI, 0, sizeof(RMI));

   RMI.EAX = InputRegs->x.eax;
   RMI.EBX = InputRegs->x.ebx;
   RMI.ECX = InputRegs->x.ecx;
   RMI.EDX = InputRegs->x.edx;
   RMI.ESI = InputRegs->x.esi;
   RMI.EDI = InputRegs->x.edi;
   RMI.EBP = 0;

   RMI.ES = 0;
   RMI.DS = 0;
   RMI.FS = 0;
   RMI.GS = 0;

   InputRegs->w.ax = 0x0300;
   InputRegs->h.bl = (UBYT) intno;
   InputRegs->h.bh = 0;
   InputRegs->w.cx = 0;
   InputRegs->x.edi = FP_OFF(&RMI);
   sregs.es = FP_SEG(&RMI);
   sregs.ds = 0;
   sregs.fs = 0;
   sregs.gs = 0;
   int386x( DPMI_INT, InputRegs, OutputRegs, &sregs );

   OutputRegs->x.eax = RMI.EAX;
   OutputRegs->x.ebx = RMI.EBX;
   OutputRegs->x.ecx = RMI.ECX;
   OutputRegs->x.edx = RMI.EDX;
   OutputRegs->x.esi = RMI.ESI;
   OutputRegs->x.edi = RMI.EDI;

   return;
}

/*----------------------------------------------------------------------------*/

void  SPDXint86x( INT intno, union REGS *InputRegs, union REGS *OutputRegs, struct SREGS *sregs )
{
   FUNC("SPDXint86x");
   Assert( InputRegs );
   Assert( OutputRegs );
   Assert( sregs );

   SPDXMemSetBYTE(&RMI, 0, sizeof(RMI));

   RMI.EAX = InputRegs->x.eax;
   RMI.EBX = InputRegs->x.ebx;
   RMI.ECX = InputRegs->x.ecx;
   RMI.EDX = InputRegs->x.edx;
   RMI.ESI = InputRegs->x.esi;
   RMI.EDI = InputRegs->x.edi;
   RMI.EBP = 0;

   RMI.ES = sregs->es;
   RMI.DS = sregs->ds;
   RMI.FS = sregs->fs;
   RMI.GS = sregs->gs;

   InputRegs->w.ax = 0x0300;
   InputRegs->h.bl = (UBYT) intno;
   InputRegs->h.bh = 0;
   InputRegs->w.cx = 0;
   InputRegs->x.edi = FP_OFF(&RMI);
   sregs->es = FP_SEG(&RMI);
   sregs->ds = 0;
   sregs->fs = 0;
   sregs->gs = 0;

   int386x( DPMI_INT, InputRegs, OutputRegs, sregs );

   OutputRegs->x.eax = RMI.EAX;
   OutputRegs->x.ebx = RMI.EBX;
   OutputRegs->x.ecx = RMI.ECX;
   OutputRegs->x.edx = RMI.EDX;
   OutputRegs->x.esi = RMI.ESI;
   OutputRegs->x.edi = RMI.EDI;

   sregs->es = RMI.ES;
   sregs->ds = RMI.DS;
   sregs->fs = RMI.FS;
   sregs->gs = RMI.GS;

   return;
}

/*----------------------------------------------------------------------------*/

void  *SPDXDMalloc( INT Size )
{
union    REGS  inregs, outregs;
struct   SREGS sregs;

   FUNC("SPDXDMalloc");
   inregs.w.ax = 0x0100;
   inregs.w.bx = (UWRD) (Size/16+1);
   sregs.es = 0;
   sregs.ds = 0;
   sregs.fs = 0;
   sregs.gs = 0;

   int386x( DPMI_INT, &inregs, &outregs, &sregs );

   return(MK_PROT( outregs.w.ax, 0 ));
}

/*----------------------------------------------------------------------------
  -   [INT86.C     ] - End Of File                                           -
  ----------------------------------------------------------------------------*/
