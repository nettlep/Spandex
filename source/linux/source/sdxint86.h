// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [SDXINT86.H     ] - INT86x library header file                         -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#define  MK_REAL(Seg, Off)    ((LNG)   (((UWRD)Seg << 16) + (UWRD)Off & 0xFFFF))
#define  MK_PROT(Seg, Off)    ((void *) ((LNG)(((UWRD)Seg <<  4) + ((UWRD)Off & 0xFFFF))))
#define  REAL_SEG(Real)       ((ULNG)Real >> 16)
#define  REAL_OFF(Real)       ((ULNG)Real &  0xFFFF)
#define  PROT_SEG(Prot)       ((ULNG)Prot>>4)
#define  PROT_OFF(Prot)       ((ULNG)Prot&0xf)
#define  RTOP(Real)           (MK_PROT( REAL_SEG(Real), REAL_OFF(Real)))
#define  PTOR(Prot)           (MK_REAL( PROT_SEG(Prot), PROT_OFF(Prot)))

/*----------------------------------------------------------------------------*/

#define  DPMI_INT             0x31

/*----------------------------------------------------------------------------*/
// INT86.C

//void  SPDXint86( INT interrupt, union REGS *InputRegs, union REGS *OutputRegs );
//void  SPDXint86x( INT interrupt, union REGS *InputRegs, union REGS *OutputRegs, struct SREGS *sregs );
void  *SPDXDMalloc( INT Size );

/*----------------------------------------------------------------------------
  -   [SDXINT86.h     ] - End Of File                                        -
  ----------------------------------------------------------------------------*/
