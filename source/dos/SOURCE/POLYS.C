// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [POLYS.C     ] - Combines all renderers                                ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#include <stdio.h>
#include <math.h>

#include "spandex.h"

#ifndef _FUNCTION_
#define _FUNCTION_ "Internal Renderer"
#endif

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#ifdef U_P
#define  TEXT_INCLUDE      "PERSP.TRI"
#else
#define  TEXT_INCLUDE      "AFFINE.TRI"
#endif

#define  SOLID_INCLUDE     "AFFINE.TRI"

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

//  *************************************************************************
//
//  The algorithm changes slightly [there is no else for the if(iTopHeight)]
//  for the solid color, non-textured polygons... so for maximum speed,
//  this routine gets its own code set.
//
//  *************************************************************************

void SPDXSolFTri( TRI *Tri )
{
INT   iColor, iPitch, iTopHeight, iBotHeight;
CHR   *pScanStart;
FXD   fShortDX, fLongDX, fShortX, fLongX;
P2D   *PA, *PB, *PC, *pTemp;

   Assert( Tri );

   PA = &Tri->V1->Scr;
   PB = &Tri->V2->Scr;
   PC = &Tri->V3->Scr;

   if (PA->fy > PB->fy) SWAP(PA, PB, pTemp);
   if (PB->fy > PC->fy) SWAP(PB, PC, pTemp);
   if (PA->fy > PB->fy) SWAP(PA, PB, pTemp);

   if(FixedWhole(PC->fy) == FixedWhole(PA->fy)) return;

   iTopHeight = FXD_TO_INT(PB->fy) - FXD_TO_INT(PA->fy);
   iBotHeight = FXD_TO_INT(PC->fy) - FXD_TO_INT(PB->fy);
   pScanStart = &SPDXGlobals.CurrentVScreen.DrawScreen[SPDXGlobals.CurrentVScreen.STab[FXD_TO_INT(PA->fy)]];
   iPitch     = SPDXGlobals.CurrentVScreen.iResX;
   iColor     = SPDXGlobals.PalTab[(FLT_TO_INT(Tri->Int)<<8)+FLT_TO_INT(Tri->Material->Color)];
   iColor    += (iColor << 8);
   iColor    += (iColor << 16);
   fLongDX    = FLT_TO_FXD((PC->x - PA->x) / (PC->y - PA->y));
   fLongX     = PA->fx + FixedMul(fLongDX, FXD_ONE - FixedFrac(PA->fy));

   if (iTopHeight)
   {
      fShortDX = FLT_TO_FXD((PB->x - PA->x) / (PB->y - PA->y));
      fShortX = PA->fx + FixedMul(fShortDX, FXD_ONE - FixedFrac(PA->fy));

      if (fShortDX < fLongDX)
         while( iTopHeight-- )
         {
            SPDXBlastBytes(pScanStart+FXD_TO_INT(fShortX), iColor, ROUND_FXD_TO_INT(fLongX) - FXD_TO_INT(fShortX) );
            pScanStart += iPitch; fShortX += fShortDX; fLongX += fLongDX;
         }
      else
         while( iTopHeight-- )
         {
            SPDXBlastBytes(pScanStart+FXD_TO_INT(fLongX), iColor, ROUND_FXD_TO_INT(fShortX) - FXD_TO_INT(fLongX) );
            pScanStart += iPitch; fShortX += fShortDX; fLongX += fLongDX;
         }
   }

   if (!iBotHeight) return;

   fShortDX = FLT_TO_FXD((PC->x - PB->x) / (PC->y - PB->y));
   fShortX = PB->fx + FixedMul(fShortDX, FXD_ONE - FixedFrac(PB->fy));

   if (fShortX < fLongX)
      while( iBotHeight-- )
      {
         SPDXBlastBytes(pScanStart+FXD_TO_INT(fShortX), iColor, ROUND_FXD_TO_INT(fLongX) - FXD_TO_INT(fShortX) );
         pScanStart += iPitch; fShortX += fShortDX; fLongX += fLongDX;
      }
   else
      while( iBotHeight-- )
      {
         SPDXBlastBytes(pScanStart+FXD_TO_INT(fLongX), iColor, ROUND_FXD_TO_INT(fShortX) - FXD_TO_INT(fLongX) );
         pScanStart += iPitch; fShortX += fShortDX; fLongX += fLongDX;
      }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void SPDXSolGTri( TRI *Tri )
{
   #define  INNER_LOOP(height, start, end)                                 \
   while( height-- )                                                       \
   {                                                                       \
      pScr   = pScanStart + FXD_TO_INT(start##X);                          \
      CurG.d = start##G + FixedMul(fDeltaG, FXD_ONE - FixedFrac(start##X));\
      iCount = FXD_TO_INT(end##X) - FXD_TO_INT(start##X);                  \
                                                                           \
      while (iCount-- > 0)                                                 \
      {                                                                    \
         *(pScr++)= pPal[CurG.b.h<<8];                                     \
         CurG.d += fDeltaG;                                                \
      }                                                                    \
                                                                           \
      pScanStart += iPitch;                                                \
      start##G   += start##DG;                                             \
      start##X   += start##DX;                                             \
      end  ##X   += end  ##DX;                                             \
   }

   #define __INTERP_G
   #define __GOURAUD
   #include SOLID_INCLUDE
   #undef  __GOURAUD
   #undef  __INTERP_G
   #undef  INNER_LOOP
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void SPDXSolPTri( TRI *Tri )
{
   #define  INNER_LOOP(height, start, end)                                 \
   while( height-- )                                                       \
   {                                                                       \
      pScr   = pScanStart + FXD_TO_INT(start##X);                          \
      fxTemp = FXD_ONE - FixedFrac(start##X);                              \
      CurS.d = start##S + FixedMul(fDeltaS, fxTemp);                       \
      CurT.d = start##T + FixedMul(fDeltaT, fxTemp);                       \
      iCount = FXD_TO_INT(end##X) - FXD_TO_INT(start##X);                  \
                                                                           \
      while (iCount-- > 0)                                                 \
      {                                                                    \
         *(pScr++) = ((UBYT *) pPhong[(CurT.b.h<<8)+CurS.b.h])[iColor];    \
         CurS.d += fDeltaS;                                                \
         CurT.d += fDeltaT;                                                \
      }                                                                    \
                                                                           \
      pScanStart += iPitch;                                                \
      start##S   += start##DS;                                             \
      start##T   += start##DT;                                             \
      start##X   += start##DX;                                             \
      end  ##X   += end  ##DX;                                             \
   }

   #define __INTERP_ST
   #define __PHONG
   #define __SOLID
   #include SOLID_INCLUDE
   #undef  __SOLID
   #undef  __PHONG
   #undef  __INTERP_ST
   #undef  INNER_LOOP
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void SPDXSolBTri( TRI *Tri )
{
   #define  INNER_LOOP(height, start, end)                                 \
   while( height-- )                                                       \
   {                                                                       \
      pScr   = pScanStart + FXD_TO_INT(start##X);                          \
      fxTemp = FXD_ONE - FixedFrac(start##X);                              \
      CurS.d = start##S + FixedMul(fDeltaS, fxTemp);                       \
      CurT.d = start##T + FixedMul(fDeltaT, fxTemp);                       \
      CurU.d = start##U + FixedMul(fDeltaU, fxTemp);                       \
      CurV.d = start##V + FixedMul(fDeltaV, fxTemp);                       \
      iCount = FXD_TO_INT(end##X) - FXD_TO_INT(start##X);                  \
                                                                           \
      while (iCount-- > 0)                                                 \
      {                                                                    \
         cTemp    = *((CHR *) pBump[CurV.w.h&VMask] + (CurU.w.h&UMask));   \
         *(pScr++)= ((UBYT *) pPhong[(((CurT.b.h+cTemp)<<8)&0xff00)+       \
                                    ((CurS.b.h+cTemp)&0x00ff)])[iColor];   \
         CurS.d += fDeltaS;                                                \
         CurT.d += fDeltaT;                                                \
         CurU.d += fDeltaU;                                                \
         CurV.d += fDeltaV;                                                \
      }                                                                    \
                                                                           \
      pScanStart += iPitch;                                                \
      start##S   += start##DS;                                             \
      start##T   += start##DT;                                             \
      start##U   += start##DU;                                             \
      start##V   += start##DV;                                             \
      start##X   += start##DX;                                             \
      end  ##X   += end  ##DX;                                             \
   }

   #define __INTERP_ST
   #define __INTERP_UV
   #define __PHONG
   #define __BUMP
   #define __SOLID
   #include SOLID_INCLUDE
   #undef  __SOLID
   #undef  __BUMP
   #undef  __PHONG
   #undef  __INTERP_UV
   #undef  __INTERP_ST
   #undef  INNER_LOOP
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXTexFTri( TRI *Tri )
{
   #ifdef   U_P
   #define  INNER_LOOP( height, start, end )                           \
   {                                                                   \
      fDeltaUaff = fDeltaU * F_PERSP_SPANS;                            \
      fDeltaVaff = fDeltaV * F_PERSP_SPANS;                            \
      fDeltaZaff = fDeltaZ * F_PERSP_SPANS;                            \
                                                                       \
      while( height-- )                                                \
      {                                                                \
         pScr    = pScanStart + FLT_TO_INT(start##X);                  \
         fTemp   = 1.0 - FLT_FRAC(start##X);                           \
         fCurU   = start##U + (fDeltaU * fTemp);                       \
         fCurV   = start##V + (fDeltaV * fTemp);                       \
         fCurZ   = start##Z + (fDeltaZ * fTemp);                       \
         fTemp   = 1.0 / fCurZ;                                        \
         ULeft.d = URight = FLT_TO_FXD(fCurU * fTemp);                 \
         VLeft.d = VRight = FLT_TO_FXD(fCurV * fTemp);                 \
         iCount  = FLT_TO_INT(end##X) - FLT_TO_INT(start##X);          \
                                                                       \
         while( iCount > I_PERSP_SPANS )                               \
         {                                                             \
            fCurU += fDeltaUaff;                                       \
            fCurV += fDeltaVaff;                                       \
            fCurZ += fDeltaZaff;                                       \
            fTemp  = 1.0 / fCurZ;                                      \
            URight = FLT_TO_FXD(fCurU * fTemp);                        \
            VRight = FLT_TO_FXD(fCurV * fTemp);                        \
            DeltaU = (URight - ULeft.d) >> PERSP_SHIFT;                \
            DeltaV = (VRight - VLeft.d) >> PERSP_SHIFT;                \
            iTemp  = I_PERSP_SPANS;                                    \
                                                                       \
            while(iTemp--)                                             \
            {                                                          \
               *(pScr++)=pPal[*((UBYT *)pMap[VLeft.w.h&VMask]+         \
                                    (ULeft.b.h&UMask))];               \
               ULeft.d+=DeltaU;                                        \
               VLeft.d+=DeltaV;                                        \
            }                                                          \
                                                                       \
            ULeft.d  = URight;                                         \
            VLeft.d  = VRight;                                         \
            iCount  -= I_PERSP_SPANS;                                  \
         }                                                             \
                                                                       \
         if (iCount > 0)                                               \
         {                                                             \
            fCurU += fDeltaUaff;                                       \
            fCurV += fDeltaVaff;                                       \
            fCurZ += fDeltaZaff;                                       \
            fTemp  = 1.0 / fCurZ;                                      \
            URight = FLT_TO_FXD(fCurU * fTemp);                        \
            VRight = FLT_TO_FXD(fCurV * fTemp);                        \
            DeltaU = (URight - ULeft.d) >> PERSP_SHIFT;                \
            DeltaV = (VRight - VLeft.d) >> PERSP_SHIFT;                \
                                                                       \
            while( iCount-- > 0 )                                      \
            {                                                          \
               *(pScr++)=pPal[*((UBYT *)pMap[VLeft.w.h&VMask]+         \
                                    (ULeft.b.h&UMask))];               \
               ULeft.d+=DeltaU;                                        \
               VLeft.d+=DeltaV;                                        \
            }                                                          \
         }                                                             \
                                                                       \
         pScanStart += iPitch;                                         \
         start##X   += start##DX; end##X += end##DX;                   \
         start##U   += start##DU; end##U += end##DU;                   \
         start##V   += start##DV; end##V += end##DV;                   \
         start##Z   += start##DZ; end##Z += end##DZ;                   \
      }                                                                \
   }
   #else
   #define  INNER_LOOP(height, start, end)                      \
   while( height-- )                                            \
   {                                                            \
      pScr    = pScanStart + FXD_TO_INT(start##X);              \
      fxTemp  = FXD_ONE - FixedFrac(start##X);                  \
      CurU.d  = start##U + FixedMul(fDeltaU, fxTemp);           \
      CurV.d  = start##V + FixedMul(fDeltaV, fxTemp);           \
      iCount  = FXD_TO_INT(end##X) - FXD_TO_INT(start##X);      \
                                                                \
      while (iCount-- > 0)                                      \
      {                                                         \
         *(pScr++)= pPal[*((UBYT *)pMap[CurV.b.h]+              \
                                 CurU.b.h)];                    \
         CurU.d += fDeltaU;                                     \
         CurV.d += fDeltaV;                                     \
      }                                                         \
                                                                \
      pScanStart += iPitch;                                     \
      start##U   += start##DU;                                  \
      start##V   += start##DV;                                  \
      start##X   += start##DX;                                  \
      end  ##X   += end  ##DX;                                  \
   }
   #endif

   #define __INTERP_UV
   #define __LAMBERT
   #include TEXT_INCLUDE
   #undef  __LAMBERT
   #undef  __INTERP_UV
   #undef  INNER_LOOP
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXTexGTri( TRI *Tri )
{
   #ifdef   U_P
   #define  INNER_LOOP( height, start, end )                           \
   {                                                                   \
      fDeltaUaff = fDeltaU * F_PERSP_SPANS;                            \
      fDeltaVaff = fDeltaV * F_PERSP_SPANS;                            \
      fDeltaZaff = fDeltaZ * F_PERSP_SPANS;                            \
                                                                       \
      while( height-- )                                                \
      {                                                                \
         pScr    = pScanStart + FLT_TO_INT(start##X);                  \
         fTemp   = 1.0 - FLT_FRAC(start##X);                           \
         fCurU   = start##U + (fDeltaU * fTemp);                       \
         fCurV   = start##V + (fDeltaV * fTemp);                       \
         fCurZ   = start##Z + (fDeltaZ * fTemp);                       \
         CurG.d  = FLT_TO_FXD(start##G);                               \
         fTemp   = 1.0 / fCurZ;                                        \
         ULeft.d = URight = FLT_TO_FXD(fCurU * fTemp);                 \
         VLeft.d = VRight = FLT_TO_FXD(fCurV * fTemp);                 \
         iCount  = FLT_TO_INT(end##X) - FLT_TO_INT(start##X);          \
                                                                       \
         while( iCount > I_PERSP_SPANS )                               \
         {                                                             \
            fCurU += fDeltaUaff;                                       \
            fCurV += fDeltaVaff;                                       \
            fCurZ += fDeltaZaff;                                       \
            fTemp  = 1.0 / fCurZ;                                      \
            URight = FLT_TO_FXD(fCurU * fTemp);                        \
            VRight = FLT_TO_FXD(fCurV * fTemp);                        \
            DeltaU = (URight - ULeft.d) >> PERSP_SHIFT;                \
            DeltaV = (VRight - VLeft.d) >> PERSP_SHIFT;                \
            iTemp  = I_PERSP_SPANS;                                    \
                                                                       \
            while(iTemp--)                                             \
            {                                                          \
               *(pScr++)=pPal[(CurG.b.h<<8)+*((UBYT *)pMap[VLeft.w.h&  \
                                           VMask]+(ULeft.w.h&UMask))]; \
               ULeft.d+=DeltaU;                                        \
               VLeft.d+=DeltaV;                                        \
               CurG.d +=DeltaG;                                        \
            }                                                          \
                                                                       \
            ULeft.d  = URight;                                         \
            VLeft.d  = VRight;                                         \
            iCount  -= I_PERSP_SPANS;                                  \
         }                                                             \
                                                                       \
         if (iCount > 0)                                               \
         {                                                             \
            fCurU += fDeltaUaff;                                       \
            fCurV += fDeltaVaff;                                       \
            fCurZ += fDeltaZaff;                                       \
            fTemp  = 1.0 / fCurZ;                                      \
            URight = FLT_TO_FXD(fCurU * fTemp);                        \
            VRight = FLT_TO_FXD(fCurV * fTemp);                        \
            DeltaU = (URight - ULeft.d) >> PERSP_SHIFT;                \
            DeltaV = (VRight - VLeft.d) >> PERSP_SHIFT;                \
                                                                       \
            while( iCount-- > 0 )                                      \
            {                                                          \
               *(pScr++)=pPal[(CurG.b.h<<8)+*((UBYT *)pMap[VLeft.w.h&  \
                                           VMask]+(ULeft.w.h&UMask))]; \
               ULeft.d+=DeltaU;                                        \
               VLeft.d+=DeltaV;                                        \
               CurG.d +=DeltaG;                                        \
            }                                                          \
         }                                                             \
                                                                       \
         pScanStart += iPitch;                                         \
         start##X   += start##DX; end##X += end##DX;                   \
         start##U   += start##DU; end##U += end##DU;                   \
         start##V   += start##DV; end##V += end##DV;                   \
         start##Z   += start##DZ; end##Z += end##DZ;                   \
         start##G   += start##DG;                                      \
      }                                                                \
   }
   #else
   #define  INNER_LOOP(height, start, end)                                 \
                                                                           \
   while( height-- )                                                       \
   {                                                                       \
      pScr   = pScanStart + FXD_TO_INT(start##X);                          \
      fxTemp = FXD_ONE - FixedFrac(start##X);                              \
      CurG.d = start##G + FixedMul(fDeltaG, fxTemp);                       \
      CurU.d = start##U + FixedMul(fDeltaU, fxTemp);                       \
      CurV.d = start##V + FixedMul(fDeltaV, fxTemp);                       \
      iCount = FXD_TO_INT(end##X) - FXD_TO_INT(start##X);                  \
                                                                           \
      while (iCount-- > 0)                                                 \
      {                                                                    \
         *(pScr++)=pPal[(CurG.b.h<<8)+*((UBYT *)pMap[CurV.w.h&             \
                                     VMask]+(CurU.w.h&UMask))];            \
         CurG.d += fDeltaG;                                                \
         CurU.d += fDeltaU;                                                \
         CurV.d += fDeltaV;                                                \
      }                                                                    \
                                                                           \
      pScanStart += iPitch;                                                \
      start##G   += start##DG;                                             \
      start##U   += start##DU;                                             \
      start##V   += start##DV;                                             \
      start##X   += start##DX;                                             \
      end  ##X   += end  ##DX;                                             \
   }
   #endif

   #define __INTERP_UV
   #define __INTERP_G
   #define __GOURAUD
   #include TEXT_INCLUDE
   #undef  __GOURAUD
   #undef  __INTERP_G
   #undef  __INTERP_UV
   #undef  INNER_LOOP
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXTexPTri( TRI *Tri )
{
   #ifdef   U_P
   #define  INNER_LOOP( height, start, end )                           \
   {                                                                   \
      fDeltaUaff = fDeltaU * F_PERSP_SPANS;                            \
      fDeltaVaff = fDeltaV * F_PERSP_SPANS;                            \
      fDeltaZaff = fDeltaZ * F_PERSP_SPANS;                            \
      DeltaS     = FLT_TO_FXD(fDeltaS);                                \
      DeltaT     = FLT_TO_FXD(fDeltaT);                                \
                                                                       \
      while( height-- )                                                \
      {                                                                \
         pScr    = pScanStart + FLT_TO_INT(start##X);                  \
         CurS.d  = FLT_TO_FXD(start##S);                               \
         CurT.d  = FLT_TO_FXD(start##T);                               \
         fTemp   = 1.0 - FLT_FRAC(start##X);                           \
         fCurU   = start##U + (fDeltaU * fTemp);                       \
         fCurV   = start##V + (fDeltaV * fTemp);                       \
         fCurZ   = start##Z + (fDeltaZ * fTemp);                       \
         fTemp   = 1.0 / fCurZ;                                        \
         ULeft.d = URight = FLT_TO_FXD(fCurU * fTemp);                 \
         VLeft.d = VRight = FLT_TO_FXD(fCurV * fTemp);                 \
         iCount  = FLT_TO_INT(end##X) - FLT_TO_INT(start##X);          \
                                                                       \
         while( iCount > I_PERSP_SPANS )                               \
         {                                                             \
            fCurU += fDeltaUaff;                                       \
            fCurV += fDeltaVaff;                                       \
            fCurZ += fDeltaZaff;                                       \
            fTemp  = 1.0 / fCurZ;                                      \
            URight = FLT_TO_FXD(fCurU * fTemp);                        \
            VRight = FLT_TO_FXD(fCurV * fTemp);                        \
            DeltaU = (URight - ULeft.d) >> PERSP_SHIFT;                \
            DeltaV = (VRight - VLeft.d) >> PERSP_SHIFT;                \
            iTemp  = I_PERSP_SPANS;                                    \
                                                                       \
            while(iTemp--)                                             \
            {                                                          \
               *(pScr++) = ((UBYT *) pPhong[(CurT.b.h<<8)+CurS.b.h])   \
                           [(*((UBYT *)pMap[VLeft.w.h&VMask]+          \
                               (ULeft.w.h&UMask)))];                   \
               CurS.d +=DeltaS;                                        \
               CurT.d +=DeltaT;                                        \
               ULeft.d+=DeltaU;                                        \
               VLeft.d+=DeltaV;                                        \
            }                                                          \
                                                                       \
            ULeft.d  = URight;                                         \
            VLeft.d  = VRight;                                         \
            iCount  -= I_PERSP_SPANS;                                  \
         }                                                             \
                                                                       \
         if (iCount > 0)                                               \
         {                                                             \
            fCurU += fDeltaUaff;                                       \
            fCurV += fDeltaVaff;                                       \
            fCurZ += fDeltaZaff;                                       \
            fTemp  = 1.0 / fCurZ;                                      \
            URight = FLT_TO_FXD(fCurU * fTemp);                        \
            VRight = FLT_TO_FXD(fCurV * fTemp);                        \
            DeltaU = (URight - ULeft.d) >> PERSP_SHIFT;                \
            DeltaV = (VRight - VLeft.d) >> PERSP_SHIFT;                \
                                                                       \
            while( iCount-- > 0 )                                      \
            {                                                          \
               *(pScr++) = ((UBYT *) pPhong[(CurT.b.h<<8)+CurS.b.h])   \
                           [(*((UBYT *)pMap[VLeft.w.h&VMask]+          \
                               (ULeft.w.h&UMask)))];                   \
               CurS.d +=DeltaS;                                        \
               CurT.d +=DeltaT;                                        \
               ULeft.d+=DeltaU;                                        \
               VLeft.d+=DeltaV;                                        \
            }                                                          \
         }                                                             \
                                                                       \
         pScanStart += iPitch;                                         \
         start##S   += start##DS;                                      \
         start##T   += start##DT;                                      \
         start##U   += start##DU; end##U += end##DU;                   \
         start##V   += start##DV; end##V += end##DV;                   \
         start##X   += start##DX; end##X += end##DX;                   \
         start##Z   += start##DZ; end##Z += end##DZ;                   \
      }                                                                \
   }
   #else
   #define  INNER_LOOP(height, start, end)                                 \
   while( height-- )                                                       \
   {                                                                       \
      pScr   = pScanStart + FXD_TO_INT(start##X);                          \
      fxTemp = FXD_ONE - FixedFrac(start##X);                              \
      CurS.d = start##S + FixedMul(fDeltaS, fxTemp);                       \
      CurT.d = start##T + FixedMul(fDeltaT, fxTemp);                       \
      CurU.d = start##U + FixedMul(fDeltaU, fxTemp);                       \
      CurV.d = start##V + FixedMul(fDeltaV, fxTemp);                       \
      iCount = FXD_TO_INT(end##X) - FXD_TO_INT(start##X);                  \
                                                                           \
      while (iCount-- > 0)                                                 \
      {                                                                    \
         *(pScr++) = ((UBYT *) pPhong[(CurT.b.h<<8)+CurS.b.h])             \
                           [(*((UBYT *)pMap[CurV.w.h&VMask]+               \
                               (CurU.w.h&UMask)))];                        \
         CurS.d += fDeltaS;                                                \
         CurT.d += fDeltaT;                                                \
         CurU.d += fDeltaU;                                                \
         CurV.d += fDeltaV;                                                \
      }                                                                    \
                                                                           \
      pScanStart += iPitch;                                                \
      start##S   += start##DS;                                             \
      start##T   += start##DT;                                             \
      start##U   += start##DU;                                             \
      start##V   += start##DV;                                             \
      start##X   += start##DX;                                             \
      end  ##X   += end  ##DX;                                             \
   }
   #endif

   #define __INTERP_ST
   #define __INTERP_UV
   #define __PHONG
   #include TEXT_INCLUDE
   #undef  __PHONG
   #undef  __INTERP_UV
   #undef  __INTERP_ST
   #undef  INNER_LOOP
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXTexBTri( TRI *Tri )
{
   #ifdef   U_P
   #define  INNER_LOOP( height, start, end )                                 \
   {                                                                         \
      fDeltaUaff = fDeltaU * F_PERSP_SPANS;                                  \
      fDeltaVaff = fDeltaV * F_PERSP_SPANS;                                  \
      fDeltaZaff = fDeltaZ * F_PERSP_SPANS;                                  \
      DeltaS     = FLT_TO_FXD(fDeltaS);                                      \
      DeltaT     = FLT_TO_FXD(fDeltaT);                                      \
                                                                             \
      while( height-- )                                                      \
      {                                                                      \
         pScr    = pScanStart + FLT_TO_INT(start##X);                        \
         CurS.d  = FLT_TO_FXD(start##S);                                     \
         CurT.d  = FLT_TO_FXD(start##T);                                     \
         fTemp   = 1.0 - FLT_FRAC(start##X);                                 \
         fCurU   = start##U + (fDeltaU * fTemp);                             \
         fCurV   = start##V + (fDeltaV * fTemp);                             \
         fCurZ   = start##Z + (fDeltaZ * fTemp);                             \
         fTemp   = 1.0 / fCurZ;                                              \
         ULeft.d = URight = FLT_TO_FXD(fCurU * fTemp);                       \
         VLeft.d = VRight = FLT_TO_FXD(fCurV * fTemp);                       \
         iCount  = FLT_TO_INT(end##X) - FLT_TO_INT(start##X);                \
                                                                             \
         while( iCount > I_PERSP_SPANS )                                     \
         {                                                                   \
            fCurU += fDeltaUaff;                                             \
            fCurV += fDeltaVaff;                                             \
            fCurZ += fDeltaZaff;                                             \
            fTemp  = 1.0 / fCurZ;                                            \
            URight = FLT_TO_FXD(fCurU * fTemp);                              \
            VRight = FLT_TO_FXD(fCurV * fTemp);                              \
            DeltaU = (URight - ULeft.d) >> PERSP_SHIFT;                      \
            DeltaV = (VRight - VLeft.d) >> PERSP_SHIFT;                      \
            iTemp  = I_PERSP_SPANS;                                          \
                                                                             \
            while(iTemp--)                                                   \
            {                                                                \
               cTemp    = *((CHR *)pBump[VLeft.w.h&VMask]+(ULeft.w.h&UMask));\
               *(pScr++)= ((UBYT *)pPhong[(((CurT.b.h+cTemp)<<8)&0xff00)+    \
                          ((CurS.b.h+cTemp)&0x00ff)])                        \
                        [*((UBYT *)pMap[VLeft.w.h&VMask]+(ULeft.w.h&UMask))];\
               CurS.d +=DeltaS;                                              \
               CurT.d +=DeltaT;                                              \
               ULeft.d+=DeltaU;                                              \
               VLeft.d+=DeltaV;                                              \
            }                                                                \
                                                                             \
            ULeft.d  = URight;                                               \
            VLeft.d  = VRight;                                               \
            iCount  -= I_PERSP_SPANS;                                        \
         }                                                                   \
                                                                             \
         if (iCount > 0)                                                     \
         {                                                                   \
            fCurU += fDeltaUaff;                                             \
            fCurV += fDeltaVaff;                                             \
            fCurZ += fDeltaZaff;                                             \
            fTemp  = 1.0 / fCurZ;                                            \
            URight = FLT_TO_FXD(fCurU * fTemp);                              \
            VRight = FLT_TO_FXD(fCurV * fTemp);                              \
            DeltaU = (URight - ULeft.d) >> PERSP_SHIFT;                      \
            DeltaV = (VRight - VLeft.d) >> PERSP_SHIFT;                      \
                                                                             \
            while( iCount-- > 0 )                                            \
            {                                                                \
               cTemp    = *((CHR *)pBump[VLeft.w.h&VMask]+(ULeft.w.h&UMask));\
               *(pScr++)= ((UBYT *)pPhong[(((CurT.b.h+cTemp)<<8)&0xff00)+    \
                          ((CurS.b.h+cTemp)&0x00ff)])                        \
                        [*((UBYT *)pMap[VLeft.w.h&VMask]+(ULeft.w.h&UMask))];\
               CurS.d +=DeltaS;                                              \
               CurT.d +=DeltaT;                                              \
               ULeft.d+=DeltaU;                                              \
               VLeft.d+=DeltaV;                                              \
            }                                                                \
         }                                                                   \
                                                                             \
         pScanStart += iPitch;                                               \
         start##S   += start##DS;                                            \
         start##T   += start##DT;                                            \
         start##U   += start##DU; end##U += end##DU;                         \
         start##V   += start##DV; end##V += end##DV;                         \
         start##X   += start##DX; end##X += end##DX;                         \
         start##Z   += start##DZ; end##Z += end##DZ;                         \
      }                                                                      \
   }
   #else
   #define  INNER_LOOP(height, start, end)                                 \
   while( height-- )                                                       \
   {                                                                       \
      pScr   = pScanStart + FXD_TO_INT(start##X);                          \
      fxTemp = FXD_ONE - FixedFrac(start##X);                              \
      CurS.d = start##S + FixedMul(fDeltaS, fxTemp);                       \
      CurT.d = start##T + FixedMul(fDeltaT, fxTemp);                       \
      CurU.d = start##U + FixedMul(fDeltaU, fxTemp);                       \
      CurV.d = start##V + FixedMul(fDeltaV, fxTemp);                       \
      iCount = FXD_TO_INT(end##X) - FXD_TO_INT(start##X);                  \
                                                                           \
      while (iCount-- > 0)                                                 \
      {                                                                    \
         cTemp    = *((CHR *)pBump[CurV.w.h&VMask]+(CurU.w.h&UMask));      \
         *(pScr++)= ((UBYT *)pPhong[(((CurT.b.h+cTemp)<<8)&0xff00)+        \
                    ((CurS.b.h+cTemp)&0x00ff)])                            \
                  [*((UBYT *)pMap[CurV.w.h&VMask]+(CurU.w.h&UMask))];      \
         CurS.d += fDeltaS;                                                \
         CurT.d += fDeltaT;                                                \
         CurU.d += fDeltaU;                                                \
         CurV.d += fDeltaV;                                                \
      }                                                                    \
                                                                           \
      pScanStart += iPitch;                                                \
      start##S   += start##DS;                                             \
      start##T   += start##DT;                                             \
      start##U   += start##DU;                                             \
      start##V   += start##DV;                                             \
      start##X   += start##DX;                                             \
      end  ##X   += end  ##DX;                                             \
   }
   #endif

   #define __INTERP_ST
   #define __INTERP_UV
   #define __PHONG
   #define __BUMP
   #include TEXT_INCLUDE
   #undef  __BUMP
   #undef  __PHONG
   #undef  __INTERP_UV
   #undef  __INTERP_ST
   #undef  INNER_LOOP
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXEnvBTri( TRI *Tri )
{
   #ifdef   U_P
   #define  INNER_LOOP( height, start, end )                                 \
   {                                                                         \
      fDeltaUaff = fDeltaU * F_PERSP_SPANS;                                  \
      fDeltaVaff = fDeltaV * F_PERSP_SPANS;                                  \
      fDeltaZaff = fDeltaZ * F_PERSP_SPANS;                                  \
      DeltaS     = FLT_TO_FXD(fDeltaS);                                      \
      DeltaT     = FLT_TO_FXD(fDeltaT);                                      \
                                                                             \
      while( height-- )                                                      \
      {                                                                      \
         pScr    = pScanStart + FLT_TO_INT(start##X);                        \
         CurS.d  = FLT_TO_FXD(start##S);                                     \
         CurT.d  = FLT_TO_FXD(start##T);                                     \
         fTemp   = 1.0 - FLT_FRAC(start##X);                                 \
         fCurU   = start##U + (fDeltaU * fTemp);                             \
         fCurV   = start##V + (fDeltaV * fTemp);                             \
         fCurZ   = start##Z + (fDeltaZ * fTemp);                             \
         fTemp   = 1.0 / fCurZ;                                              \
         ULeft.d = URight = FLT_TO_FXD(fCurU * fTemp);                       \
         VLeft.d = VRight = FLT_TO_FXD(fCurV * fTemp);                       \
         iCount  = FLT_TO_INT(end##X) - FLT_TO_INT(start##X);                \
                                                                             \
         while( iCount > I_PERSP_SPANS )                                     \
         {                                                                   \
            fCurU += fDeltaUaff;                                             \
            fCurV += fDeltaVaff;                                             \
            fCurZ += fDeltaZaff;                                             \
            fTemp  = 1.0 / fCurZ;                                            \
            URight = FLT_TO_FXD(fCurU * fTemp);                              \
            VRight = FLT_TO_FXD(fCurV * fTemp);                              \
            DeltaU = (URight - ULeft.d) >> PERSP_SHIFT;                      \
            DeltaV = (VRight - VLeft.d) >> PERSP_SHIFT;                      \
            iTemp  = I_PERSP_SPANS;                                          \
                                                                             \
            while(iTemp--)                                                   \
            {                                                                \
               cTemp    = *((CHR *) pBump[VLeft.w.h&VMask] + (ULeft.b.h&UMask));           \
               *(pScr++)= ((UBYT *) pPhong[(((CurT.b.h+cTemp)<<8)&0xff00)+   \
                          ((CurS.b.h+cTemp)&0x00ff)])                        \
                          [*((UBYT *)pMap[(CurT.b.h+cTemp)&0xff]+            \
                            ((CurS.b.h+cTemp)&0xff))];                       \
               CurS.d +=DeltaS;                                              \
               CurT.d +=DeltaT;                                              \
               ULeft.d+=DeltaU;                                              \
               VLeft.d+=DeltaV;                                              \
            }                                                                \
                                                                             \
            ULeft.d  = URight;                                               \
            VLeft.d  = VRight;                                               \
            iCount  -= I_PERSP_SPANS;                                        \
         }                                                                   \
                                                                             \
         if (iCount > 0)                                                     \
         {                                                                   \
            fCurU += fDeltaUaff;                                             \
            fCurV += fDeltaVaff;                                             \
            fCurZ += fDeltaZaff;                                             \
            fTemp  = 1.0 / fCurZ;                                            \
            URight = FLT_TO_FXD(fCurU * fTemp);                              \
            VRight = FLT_TO_FXD(fCurV * fTemp);                              \
            DeltaU = (URight - ULeft.d) >> PERSP_SHIFT;                      \
            DeltaV = (VRight - VLeft.d) >> PERSP_SHIFT;                      \
                                                                             \
            while( iCount-- > 0 )                                            \
            {                                                                \
               cTemp    = *((CHR *) pBump[VLeft.w.h&VMask] + (ULeft.b.h&UMask));           \
               *(pScr++)= ((UBYT *) pPhong[(((CurT.b.h+cTemp)<<8)&0xff00)+   \
                          ((CurS.b.h+cTemp)&0x00ff)])                        \
                          [*((UBYT *)pMap[(CurT.b.h+cTemp)&0xff]+            \
                            ((CurS.b.h+cTemp)&0xff))];                       \
               CurS.d +=DeltaS;                                              \
               CurT.d +=DeltaT;                                              \
               ULeft.d+=DeltaU;                                              \
               VLeft.d+=DeltaV;                                              \
            }                                                                \
         }                                                                   \
                                                                             \
         pScanStart += iPitch;                                               \
         start##S   += start##DS;                                            \
         start##T   += start##DT;                                            \
         start##U   += start##DU; end##U += end##DU;                         \
         start##V   += start##DV; end##V += end##DV;                         \
         start##X   += start##DX; end##X += end##DX;                         \
         start##Z   += start##DZ; end##Z += end##DZ;                         \
      }                                                                      \
   }
   #else
   #define  INNER_LOOP(height, start, end)                                 \
   while( height-- )                                                       \
   {                                                                       \
      pScr   = pScanStart + FXD_TO_INT(start##X);                          \
      fxTemp = FXD_ONE - FixedFrac(start##X);                              \
      CurS.d = start##S + FixedMul(fDeltaS, fxTemp);                       \
      CurT.d = start##T + FixedMul(fDeltaT, fxTemp);                       \
      CurU.d = start##U + FixedMul(fDeltaU, fxTemp);                       \
      CurV.d = start##V + FixedMul(fDeltaV, fxTemp);                       \
      iCount = FXD_TO_INT(end##X) - FXD_TO_INT(start##X);                  \
                                                                           \
      while (iCount-- > 0)                                                 \
      {                                                                    \
         cTemp    = *((CHR *) pBump[CurV.b.h] + CurU.b.h);                 \
         *(pScr++)= ((UBYT *) pPhong[(((CurT.b.h+cTemp)<<8)&0xff00)+       \
                    ((CurS.b.h+cTemp)&0x00ff)])                            \
                    [*((UBYT *)pMap[(CurT.b.h+cTemp)&0xff]+                \
                      ((CurS.b.h+cTemp)&0xff))];                           \
         CurS.d += fDeltaS;                                                \
         CurT.d += fDeltaT;                                                \
         CurU.d += fDeltaU;                                                \
         CurV.d += fDeltaV;                                                \
      }                                                                    \
                                                                           \
      pScanStart += iPitch;                                                \
      start##S   += start##DS;                                             \
      start##T   += start##DT;                                             \
      start##U   += start##DU;                                             \
      start##V   += start##DV;                                             \
      start##X   += start##DX;                                             \
      end  ##X   += end  ##DX;                                             \
   }
   #endif

   #define __INTERP_ST
   #define __INTERP_UV
   #define __PHONG
   #define __BUMP
   #include TEXT_INCLUDE
   #undef  __BUMP
   #undef  __PHONG
   #undef  __INTERP_UV
   #undef  __INTERP_ST
   #undef  INNER_LOOP
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

//  *************************************************************************
//
//  The algorithm changes slightly [there is no else for the if(iTopHeight)]
//  for the solid color, non-textured polygons... so for maximum speed,
//  this routine gets its own code set.
//
//  *************************************************************************

void  SPDXTransparentSolFTri( TRI *Tri )
{
INT   iColor, iPitch, iCount;
CHR   *pScanStart, *pScr, *OMap;
FXD   iTopHeight, iBotHeight, fShortDX, fLongDX, fShortX, fLongX;
P2D   *PA, *PB, *PC, *pTemp;

   Assert( Tri );
   Assert( SPDXGlobals.MapFlags & MAP_FLAGS_TRANS );

   PA = &Tri->V1->Scr;
   PB = &Tri->V2->Scr;
   PC = &Tri->V3->Scr;

   if (PA->fy > PB->fy) SWAP(PA, PB, pTemp);
   if (PB->fy > PC->fy) SWAP(PB, PC, pTemp);
   if (PA->fy > PB->fy) SWAP(PA, PB, pTemp);

   if(FixedWhole(PC->fy) == FixedWhole(PA->fy)) return;

   iTopHeight = FXD_TO_INT(PB->fy) - FXD_TO_INT(PA->fy);
   iBotHeight = FXD_TO_INT(PC->fy) - FXD_TO_INT(PB->fy);
   pScanStart = &SPDXGlobals.CurrentVScreen.DrawScreen[SPDXGlobals.CurrentVScreen.STab[FXD_TO_INT(PA->fy)]];
   iPitch     = SPDXGlobals.CurrentVScreen.iResX;
   iColor     = SPDXGlobals.PalTab[(FLT_TO_INT(Tri->Int)<<8)+FLT_TO_INT(Tri->Material->Color)] << 8;
   OMap       = SPDXGlobals.TransTab;
   fLongDX    = FLT_TO_FXD((PC->x - PA->x) / (PC->y - PA->y));
   fLongX     = PA->fx + FixedMul(fLongDX, FXD_ONE - FixedFrac(PA->fy));

   if (iTopHeight)
   {
      fShortDX = FLT_TO_FXD((PB->x - PA->x) / (PB->y - PA->y));
      fShortX = PA->fx + FixedMul(fShortDX, FXD_ONE - FixedFrac(PA->fy));

      if (fShortDX < fLongDX)
         while( iTopHeight-- )
         {
            pScr   = pScanStart + FXD_TO_INT(fShortX);
            iCount = FXD_TO_INT(fLongX) - FXD_TO_INT(fShortX);

            while( iCount-- > 0 ) *(pScr++) = OMap[iColor + *pScr];
            pScanStart += iPitch; fShortX += fShortDX; fLongX += fLongDX;
         }
      else
         while( iTopHeight-- )
         {
            pScr   = pScanStart + FXD_TO_INT(fLongX);
            iCount = FXD_TO_INT(fShortX) - FXD_TO_INT(fLongX);

            while( iCount-- > 0 ) *(pScr++) = OMap[iColor + *pScr];
            pScanStart += iPitch; fShortX += fShortDX; fLongX += fLongDX;
         }
   }

   if (!iBotHeight) return;

   fShortDX = FLT_TO_FXD((PC->x - PB->x) / (PC->y - PB->y));
   fShortX = PB->fx + FixedMul(fShortDX, FXD_ONE - FixedFrac(PB->fy));

   if (fShortX < fLongX)
      while( iBotHeight-- )
      {
         pScr   = pScanStart + FXD_TO_INT(fShortX);
         iCount = FXD_TO_INT(fLongX) - FXD_TO_INT(fShortX);

         while( iCount-- > 0 ) *(pScr++) = OMap[iColor + *pScr];
         pScanStart += iPitch; fShortX += fShortDX; fLongX += fLongDX;
      }
   else
      while( iBotHeight-- )
      {
         pScr   = pScanStart + FXD_TO_INT(fLongX);
         iCount = FXD_TO_INT(fShortX) - FXD_TO_INT(fLongX);

         while( iCount-- > 0 ) *(pScr++) = OMap[iColor + *pScr];
         pScanStart += iPitch; fShortX += fShortDX; fLongX += fLongDX;
      }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void SPDXTransparentSolGTri( TRI *Tri )
{
   #define  INNER_LOOP(height, start, end)                                 \
   while( height-- )                                                       \
   {                                                                       \
      pScr   = pScanStart + FXD_TO_INT(start##X);                          \
      CurG.d = start##G + FixedMul(fDeltaG, FXD_ONE - FixedFrac(start##X));\
      iCount = FXD_TO_INT(end##X) - FXD_TO_INT(start##X);                  \
                                                                           \
      while (iCount-- > 0)                                                 \
      {                                                                    \
         *(pScr++)= OMap[(pPal[CurG.b.h<<8]<<8)+*pScr];                    \
         CurG.d += fDeltaG;                                                \
      }                                                                    \
                                                                           \
      pScanStart += iPitch;                                                \
      start##G   += start##DG;                                             \
      start##X   += start##DX;                                             \
      end  ##X   += end  ##DX;                                             \
   }

   #define __INTERP_G
   #define __GOURAUD
   #define __TRANSPARENT
   #include SOLID_INCLUDE
   #undef  __TRANSPARENT
   #undef  __GOURAUD
   #undef  __INTERP_G
   #undef  INNER_LOOP
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void SPDXTransparentSolPTri( TRI *Tri )
{
   #define  INNER_LOOP(height, start, end)                                 \
   while( height-- )                                                       \
   {                                                                       \
      pScr   = pScanStart + FXD_TO_INT(start##X);                          \
      fxTemp = FXD_ONE - FixedFrac(start##X);                              \
      CurS.d = start##S + FixedMul(fDeltaS, fxTemp);                       \
      CurT.d = start##T + FixedMul(fDeltaT, fxTemp);                       \
      iCount = FXD_TO_INT(end##X) - FXD_TO_INT(start##X);                  \
                                                                           \
      while (iCount-- > 0)                                                 \
      {                                                                    \
         *(pScr++) = OMap[(((UBYT *) pPhong[(CurT.b.h<<8)+                 \
                            CurS.b.h])[iColor]<<8)+*pScr];                 \
         CurS.d += fDeltaS;                                                \
         CurT.d += fDeltaT;                                                \
      }                                                                    \
                                                                           \
      pScanStart += iPitch;                                                \
      start##S   += start##DS;                                             \
      start##T   += start##DT;                                             \
      start##X   += start##DX;                                             \
      end  ##X   += end  ##DX;                                             \
   }

   #define __INTERP_ST
   #define __PHONG
   #define __SOLID
   #define __TRANSPARENT
   #include SOLID_INCLUDE
   #undef  __TRANSPARENT
   #undef  __SOLID
   #undef  __PHONG
   #undef  __INTERP_ST
   #undef  INNER_LOOP
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void SPDXTransparentSolBTri( TRI *Tri )
{
   #define  INNER_LOOP(height, start, end)                                 \
   while( height-- )                                                       \
   {                                                                       \
      pScr   = pScanStart + FXD_TO_INT(start##X);                          \
      fxTemp = FXD_ONE - FixedFrac(start##X);                              \
      CurS.d = start##S + FixedMul(fDeltaS, fxTemp);                       \
      CurT.d = start##T + FixedMul(fDeltaT, fxTemp);                       \
      CurU.d = start##U + FixedMul(fDeltaU, fxTemp);                       \
      CurV.d = start##V + FixedMul(fDeltaV, fxTemp);                       \
      iCount = FXD_TO_INT(end##X) - FXD_TO_INT(start##X);                  \
                                                                           \
      while (iCount-- > 0)                                                 \
      {                                                                    \
         cTemp    = *((CHR *) pBump[CurV.b.h] + CurU.b.h);                 \
         *(pScr++)= OMap[(((UBYT *)pPhong[(((CurT.b.h+cTemp)<<8)&0xff00)+  \
                         ((CurS.b.h+cTemp)&0x00ff)])[iColor]<<8)+*pScr];   \
         CurS.d += fDeltaS;                                                \
         CurT.d += fDeltaT;                                                \
         CurU.d += fDeltaU;                                                \
         CurV.d += fDeltaV;                                                \
      }                                                                    \
                                                                           \
      pScanStart += iPitch;                                                \
      start##S   += start##DS;                                             \
      start##T   += start##DT;                                             \
      start##U   += start##DU;                                             \
      start##V   += start##DV;                                             \
      start##X   += start##DX;                                             \
      end  ##X   += end  ##DX;                                             \
   }

   #define __INTERP_ST
   #define __INTERP_UV
   #define __PHONG
   #define __BUMP
   #define __SOLID
   #define __TRANSPARENT
   #include SOLID_INCLUDE
   #undef  __TRANSPARENT
   #undef  __SOLID
   #undef  __BUMP
   #undef  __PHONG
   #undef  __INTERP_UV
   #undef  __INTERP_ST
   #undef  INNER_LOOP
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXTransparentTexFTri( TRI *Tri )
{
   #ifdef   U_P
   #define  INNER_LOOP( height, start, end )                           \
   {                                                                   \
      fDeltaUaff = fDeltaU * F_PERSP_SPANS;                            \
      fDeltaVaff = fDeltaV * F_PERSP_SPANS;                            \
      fDeltaZaff = fDeltaZ * F_PERSP_SPANS;                            \
                                                                       \
      while( height-- )                                                \
      {                                                                \
         pScr    = pScanStart + FLT_TO_INT(start##X);                  \
         fTemp   = 1.0 - FLT_FRAC(start##X);                           \
         fCurU   = start##U + (fDeltaU * fTemp);                       \
         fCurV   = start##V + (fDeltaV * fTemp);                       \
         fCurZ   = start##Z + (fDeltaZ * fTemp);                       \
         fTemp   = 1.0 / fCurZ;                                        \
         ULeft.d = URight = FLT_TO_FXD(fCurU * fTemp);                 \
         VLeft.d = VRight = FLT_TO_FXD(fCurV * fTemp);                 \
         iCount  = FLT_TO_INT(end##X) - FLT_TO_INT(start##X);          \
                                                                       \
         while( iCount > I_PERSP_SPANS )                               \
         {                                                             \
            fCurU += fDeltaUaff;                                       \
            fCurV += fDeltaVaff;                                       \
            fCurZ += fDeltaZaff;                                       \
            fTemp  = 1.0 / fCurZ;                                      \
            URight = FLT_TO_FXD(fCurU * fTemp);                        \
            VRight = FLT_TO_FXD(fCurV * fTemp);                        \
            DeltaU = (URight - ULeft.d) >> PERSP_SHIFT;                \
            DeltaV = (VRight - VLeft.d) >> PERSP_SHIFT;                \
            iTemp  = I_PERSP_SPANS;                                    \
                                                                       \
            while(iTemp--)                                             \
            {                                                          \
               *(pScr++)=OMap[(pPal[*((UBYT *)pMap[VLeft.w.h&VMask]+   \
                                    (ULeft.b.h&UMask))]<<8)+*pScr];    \
               ULeft.d+=DeltaU;                                        \
               VLeft.d+=DeltaV;                                        \
            }                                                          \
                                                                       \
            ULeft.d  = URight;                                         \
            VLeft.d  = VRight;                                         \
            iCount  -= I_PERSP_SPANS;                                  \
         }                                                             \
                                                                       \
         if (iCount > 0)                                               \
         {                                                             \
            fCurU += fDeltaUaff;                                       \
            fCurV += fDeltaVaff;                                       \
            fCurZ += fDeltaZaff;                                       \
            fTemp  = 1.0 / fCurZ;                                      \
            URight = FLT_TO_FXD(fCurU * fTemp);                        \
            VRight = FLT_TO_FXD(fCurV * fTemp);                        \
            DeltaU = (URight - ULeft.d) >> PERSP_SHIFT;                \
            DeltaV = (VRight - VLeft.d) >> PERSP_SHIFT;                \
                                                                       \
            while( iCount-- > 0 )                                      \
            {                                                          \
               *(pScr++)=OMap[(pPal[*((UBYT *)pMap[VLeft.w.h&VMask]+   \
                                    (ULeft.b.h&UMask))]<<8)+*pScr];    \
               ULeft.d+=DeltaU;                                        \
               VLeft.d+=DeltaV;                                        \
            }                                                          \
         }                                                             \
                                                                       \
         pScanStart += iPitch;                                         \
         start##X   += start##DX; end##X += end##DX;                   \
         start##U   += start##DU; end##U += end##DU;                   \
         start##V   += start##DV; end##V += end##DV;                   \
         start##Z   += start##DZ; end##Z += end##DZ;                   \
      }                                                                \
   }
   #else
   #define  INNER_LOOP(height, start, end)                      \
   while( height-- )                                            \
   {                                                            \
      pScr    = pScanStart + FXD_TO_INT(start##X);              \
      fxTemp  = FXD_ONE - FixedFrac(start##X);                  \
      CurU.d  = start##U + FixedMul(fDeltaU, fxTemp);           \
      CurV.d  = start##V + FixedMul(fDeltaV, fxTemp);           \
      iCount  = FXD_TO_INT(end##X) - FXD_TO_INT(start##X);      \
                                                                \
      while (iCount-- > 0)                                      \
      {                                                         \
         *(pScr++)= OMap[(pPal[*((UBYT *)pMap[CurV.b.h]+        \
                                 CurU.b.h)]<<8)+*pScr];         \
         CurU.d += fDeltaU;                                     \
         CurV.d += fDeltaV;                                     \
      }                                                         \
                                                                \
      pScanStart += iPitch;                                     \
      start##U   += start##DU;                                  \
      start##V   += start##DV;                                  \
      start##X   += start##DX;                                  \
      end  ##X   += end  ##DX;                                  \
   }
   #endif

   #define __INTERP_UV
   #define __LAMBERT
   #define __TRANSPARENT
   #include TEXT_INCLUDE
   #undef  __TRANSPARENT
   #undef  __LAMBERT
   #undef  __INTERP_UV
   #undef  INNER_LOOP
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXTransparentTexGTri( TRI *Tri )
{
   #ifdef   U_P
   #define  INNER_LOOP( height, start, end )                           \
   {                                                                   \
      fDeltaUaff = fDeltaU * F_PERSP_SPANS;                            \
      fDeltaVaff = fDeltaV * F_PERSP_SPANS;                            \
      fDeltaZaff = fDeltaZ * F_PERSP_SPANS;                            \
                                                                       \
      while( height-- )                                                \
      {                                                                \
         pScr    = pScanStart + FLT_TO_INT(start##X);                  \
         fTemp   = 1.0 - FLT_FRAC(start##X);                           \
         fCurU   = start##U + (fDeltaU * fTemp);                       \
         fCurV   = start##V + (fDeltaV * fTemp);                       \
         fCurZ   = start##Z + (fDeltaZ * fTemp);                       \
         CurG.d  = FLT_TO_FXD(start##G);                               \
         fTemp   = 1.0 / fCurZ;                                        \
         ULeft.d = URight = FLT_TO_FXD(fCurU * fTemp);                 \
         VLeft.d = VRight = FLT_TO_FXD(fCurV * fTemp);                 \
         iCount  = FLT_TO_INT(end##X) - FLT_TO_INT(start##X);          \
                                                                       \
         while( iCount > I_PERSP_SPANS )                               \
         {                                                             \
            fCurU += fDeltaUaff;                                       \
            fCurV += fDeltaVaff;                                       \
            fCurZ += fDeltaZaff;                                       \
            fTemp  = 1.0 / fCurZ;                                      \
            URight = FLT_TO_FXD(fCurU * fTemp);                        \
            VRight = FLT_TO_FXD(fCurV * fTemp);                        \
            DeltaU = (URight - ULeft.d) >> PERSP_SHIFT;                \
            DeltaV = (VRight - VLeft.d) >> PERSP_SHIFT;                \
            iTemp  = I_PERSP_SPANS;                                    \
                                                                       \
            while(iTemp--)                                             \
            {                                                          \
               *(pScr++)= OMap[(pPal[(CurG.b.h<<8) +                   \
                     *((UBYT *)pMap[VLeft.w.h&VMask]+(ULeft.b.h&UMask))]<<8)+*pScr]; \
               ULeft.d+=DeltaU;                                        \
               VLeft.d+=DeltaV;                                        \
               CurG.d +=DeltaG;                                        \
            }                                                          \
                                                                       \
            ULeft.d  = URight;                                         \
            VLeft.d  = VRight;                                         \
            iCount  -= I_PERSP_SPANS;                                  \
         }                                                             \
                                                                       \
         if (iCount > 0)                                               \
         {                                                             \
            fCurU += fDeltaUaff;                                       \
            fCurV += fDeltaVaff;                                       \
            fCurZ += fDeltaZaff;                                       \
            fTemp  = 1.0 / fCurZ;                                      \
            URight = FLT_TO_FXD(fCurU * fTemp);                        \
            VRight = FLT_TO_FXD(fCurV * fTemp);                        \
            DeltaU = (URight - ULeft.d) >> PERSP_SHIFT;                \
            DeltaV = (VRight - VLeft.d) >> PERSP_SHIFT;                \
                                                                       \
            while( iCount-- > 0 )                                      \
            {                                                          \
               *(pScr++)= OMap[(pPal[(CurG.b.h<<8) +                   \
                     *((UBYT *)pMap[VLeft.w.h&VMask]+(ULeft.b.h&UMask))]<<8)+*pScr]; \
               ULeft.d+=DeltaU;                                        \
               VLeft.d+=DeltaV;                                        \
               CurG.d +=DeltaG;                                        \
            }                                                          \
         }                                                             \
                                                                       \
         pScanStart += iPitch;                                         \
         start##X   += start##DX; end##X += end##DX;                   \
         start##U   += start##DU; end##U += end##DU;                   \
         start##V   += start##DV; end##V += end##DV;                   \
         start##Z   += start##DZ; end##Z += end##DZ;                   \
         start##G   += start##DG;                                      \
      }                                                                \
   }
   #else
   #define  INNER_LOOP(height, start, end)                                 \
   while( height-- )                                                       \
   {                                                                       \
      pScr   = pScanStart + FXD_TO_INT(start##X);                          \
      fxTemp = FXD_ONE - FixedFrac(start##X);                              \
      CurG.d = start##G + FixedMul(fDeltaG, fxTemp);                       \
      CurU.d = start##U + FixedMul(fDeltaU, fxTemp);                       \
      CurV.d = start##V + FixedMul(fDeltaV, fxTemp);                       \
      iCount = FXD_TO_INT(end##X) - FXD_TO_INT(start##X);                  \
                                                                           \
      while (iCount-- > 0)                                                 \
      {                                                                    \
         *(pScr++)= OMap[(pPal[(CurG.b.h<<8) +                             \
                         *((UBYT *)pMap[CurV.b.h]+CurU.b.h)]<<8)+*pScr];   \
         CurG.d += fDeltaG;                                                \
         CurU.d += fDeltaU;                                                \
         CurV.d += fDeltaV;                                                \
      }                                                                    \
                                                                           \
      pScanStart += iPitch;                                                \
      start##G   += start##DG;                                             \
      start##U   += start##DU;                                             \
      start##V   += start##DV;                                             \
      start##X   += start##DX;                                             \
      end  ##X   += end  ##DX;                                             \
   }
   #endif

   #define __INTERP_UV
   #define __INTERP_G
   #define __GOURAUD
   #define __TRANSPARENT
   #include TEXT_INCLUDE
   #undef  __TRANSPARENT
   #undef  __GOURAUD
   #undef  __INTERP_G
   #undef  __INTERP_UV
   #undef  INNER_LOOP
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXTransparentTexPTri( TRI *Tri )
{
   #ifdef   U_P
   #define  INNER_LOOP( height, start, end )                           \
   {                                                                   \
      fDeltaUaff = fDeltaU * F_PERSP_SPANS;                            \
      fDeltaVaff = fDeltaV * F_PERSP_SPANS;                            \
      fDeltaZaff = fDeltaZ * F_PERSP_SPANS;                            \
      DeltaS     = FLT_TO_FXD(fDeltaS);                                \
      DeltaT     = FLT_TO_FXD(fDeltaT);                                \
                                                                       \
      while( height-- )                                                \
      {                                                                \
         pScr    = pScanStart + FLT_TO_INT(start##X);                  \
         CurS.d  = FLT_TO_FXD(start##S);                               \
         CurT.d  = FLT_TO_FXD(start##T);                               \
         fTemp   = 1.0 - FLT_FRAC(start##X);                           \
         fCurU   = start##U + (fDeltaU * fTemp);                       \
         fCurV   = start##V + (fDeltaV * fTemp);                       \
         fCurZ   = start##Z + (fDeltaZ * fTemp);                       \
         fTemp   = 1.0 / fCurZ;                                        \
         ULeft.d = URight = FLT_TO_FXD(fCurU * fTemp);                 \
         VLeft.d = VRight = FLT_TO_FXD(fCurV * fTemp);                 \
         iCount  = FLT_TO_INT(end##X) - FLT_TO_INT(start##X);          \
                                                                       \
         while( iCount > I_PERSP_SPANS )                               \
         {                                                             \
            fCurU += fDeltaUaff;                                       \
            fCurV += fDeltaVaff;                                       \
            fCurZ += fDeltaZaff;                                       \
            fTemp  = 1.0 / fCurZ;                                      \
            URight = FLT_TO_FXD(fCurU * fTemp);                        \
            VRight = FLT_TO_FXD(fCurV * fTemp);                        \
            DeltaU = (URight - ULeft.d) >> PERSP_SHIFT;                \
            DeltaV = (VRight - VLeft.d) >> PERSP_SHIFT;                \
            iTemp  = I_PERSP_SPANS;                                    \
                                                                       \
            while(iTemp--)                                             \
            {                                                          \
               *(pScr++)=OMap[(((UBYT *)pPhong[(CurT.b.h<<8)+CurS.b.h])\
                   [(*((UBYT *)pMap[VLeft.w.h&VMask]+(ULeft.b.h&UMask)))]<<8)+*pScr];\
               CurS.d +=DeltaS;                                        \
               CurT.d +=DeltaT;                                        \
               ULeft.d+=DeltaU;                                        \
               VLeft.d+=DeltaV;                                        \
            }                                                          \
                                                                       \
            ULeft.d  = URight;                                         \
            VLeft.d  = VRight;                                         \
            iCount  -= I_PERSP_SPANS;                                  \
         }                                                             \
                                                                       \
         if (iCount > 0)                                               \
         {                                                             \
            fCurU += fDeltaUaff;                                       \
            fCurV += fDeltaVaff;                                       \
            fCurZ += fDeltaZaff;                                       \
            fTemp  = 1.0 / fCurZ;                                      \
            URight = FLT_TO_FXD(fCurU * fTemp);                        \
            VRight = FLT_TO_FXD(fCurV * fTemp);                        \
            DeltaU = (URight - ULeft.d) >> PERSP_SHIFT;                \
            DeltaV = (VRight - VLeft.d) >> PERSP_SHIFT;                \
                                                                       \
            while( iCount-- > 0 )                                      \
            {                                                          \
               *(pScr++)=OMap[(((UBYT *)pPhong[(CurT.b.h<<8)+CurS.b.h])\
                   [(*((UBYT *)pMap[VLeft.w.h&VMask]+(ULeft.b.h&UMask)))]<<8)+*pScr];\
               CurS.d +=DeltaS;                                        \
               CurT.d +=DeltaT;                                        \
               ULeft.d+=DeltaU;                                        \
               VLeft.d+=DeltaV;                                        \
            }                                                          \
         }                                                             \
                                                                       \
         pScanStart += iPitch;                                         \
         start##S   += start##DS;                                      \
         start##T   += start##DT;                                      \
         start##U   += start##DU; end##U += end##DU;                   \
         start##V   += start##DV; end##V += end##DV;                   \
         start##X   += start##DX; end##X += end##DX;                   \
         start##Z   += start##DZ; end##Z += end##DZ;                   \
      }                                                                \
   }
   #else
   #define  INNER_LOOP(height, start, end)                                 \
   while( height-- )                                                       \
   {                                                                       \
      pScr   = pScanStart + FXD_TO_INT(start##X);                          \
      fxTemp = FXD_ONE - FixedFrac(start##X);                              \
      CurS.d = start##S + FixedMul(fDeltaS, fxTemp);                       \
      CurT.d = start##T + FixedMul(fDeltaT, fxTemp);                       \
      CurU.d = start##U + FixedMul(fDeltaU, fxTemp);                       \
      CurV.d = start##V + FixedMul(fDeltaV, fxTemp);                       \
      iCount = FXD_TO_INT(end##X) - FXD_TO_INT(start##X);                  \
                                                                           \
      while (iCount-- > 0)                                                 \
      {                                                                    \
         *(pScr++) = OMap[(((UBYT *) pPhong[(CurT.b.h<<8)+CurS.b.h])       \
                         [(*((UBYT *)pMap[CurV.b.h]+CurU.b.h))]<<8)+*pScr];\
         CurS.d += fDeltaS;                                                \
         CurT.d += fDeltaT;                                                \
         CurU.d += fDeltaU;                                                \
         CurV.d += fDeltaV;                                                \
      }                                                                    \
                                                                           \
      pScanStart += iPitch;                                                \
      start##S   += start##DS;                                             \
      start##T   += start##DT;                                             \
      start##U   += start##DU;                                             \
      start##V   += start##DV;                                             \
      start##X   += start##DX;                                             \
      end  ##X   += end  ##DX;                                             \
   }
   #endif

   #define __INTERP_ST
   #define __INTERP_UV
   #define __PHONG
   #define __TRANSPARENT
   #include TEXT_INCLUDE
   #undef  __TRANSPARENT
   #undef  __PHONG
   #undef  __INTERP_UV
   #undef  __INTERP_ST
   #undef  INNER_LOOP
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXTransparentTexBTri( TRI *Tri )
{
   #ifdef   U_P
   #define  INNER_LOOP( height, start, end )                                 \
   {                                                                         \
      fDeltaUaff = fDeltaU * F_PERSP_SPANS;                                  \
      fDeltaVaff = fDeltaV * F_PERSP_SPANS;                                  \
      fDeltaZaff = fDeltaZ * F_PERSP_SPANS;                                  \
      DeltaS     = FLT_TO_FXD(fDeltaS);                                      \
      DeltaT     = FLT_TO_FXD(fDeltaT);                                      \
                                                                             \
      while( height-- )                                                      \
      {                                                                      \
         pScr    = pScanStart + FLT_TO_INT(start##X);                        \
         CurS.d  = FLT_TO_FXD(start##S);                                     \
         CurT.d  = FLT_TO_FXD(start##T);                                     \
         fTemp   = 1.0 - FLT_FRAC(start##X);                                 \
         fCurU   = start##U + (fDeltaU * fTemp);                             \
         fCurV   = start##V + (fDeltaV * fTemp);                             \
         fCurZ   = start##Z + (fDeltaZ * fTemp);                             \
         fTemp   = 1.0 / fCurZ;                                              \
         ULeft.d = URight = FLT_TO_FXD(fCurU * fTemp);                       \
         VLeft.d = VRight = FLT_TO_FXD(fCurV * fTemp);                       \
         iCount  = FLT_TO_INT(end##X) - FLT_TO_INT(start##X);                \
                                                                             \
         while( iCount > I_PERSP_SPANS )                                     \
         {                                                                   \
            fCurU += fDeltaUaff;                                             \
            fCurV += fDeltaVaff;                                             \
            fCurZ += fDeltaZaff;                                             \
            fTemp  = 1.0 / fCurZ;                                            \
            URight = FLT_TO_FXD(fCurU * fTemp);                              \
            VRight = FLT_TO_FXD(fCurV * fTemp);                              \
            DeltaU = (URight - ULeft.d) >> PERSP_SHIFT;                      \
            DeltaV = (VRight - VLeft.d) >> PERSP_SHIFT;                      \
            iTemp  = I_PERSP_SPANS;                                          \
                                                                             \
            while(iTemp--)                                                   \
            {                                                                \
               cTemp    = *((CHR *) pBump[VLeft.w.h&VMask] + (ULeft.b.h&UMask));           \
               *(pScr++)=OMap[(((UBYT *)pPhong[(((CurT.b.h+cTemp)<<8)&       \
                                0xff00)+((CurS.b.h+cTemp)&0x00ff)])          \
                               [*((UBYT *)pMap[VLeft.w.h&VMask]+             \
                                  (ULeft.b.h&UMask))]<<8)+*pScr];            \
               CurS.d +=DeltaS;                                              \
               CurT.d +=DeltaT;                                              \
               ULeft.d+=DeltaU;                                              \
               VLeft.d+=DeltaV;                                              \
            }                                                                \
                                                                             \
            ULeft.d  = URight;                                               \
            VLeft.d  = VRight;                                               \
            iCount  -= I_PERSP_SPANS;                                        \
         }                                                                   \
                                                                             \
         if (iCount > 0)                                                     \
         {                                                                   \
            fCurU += fDeltaUaff;                                             \
            fCurV += fDeltaVaff;                                             \
            fCurZ += fDeltaZaff;                                             \
            fTemp  = 1.0 / fCurZ;                                            \
            URight = FLT_TO_FXD(fCurU * fTemp);                              \
            VRight = FLT_TO_FXD(fCurV * fTemp);                              \
            DeltaU = (URight - ULeft.d) >> PERSP_SHIFT;                      \
            DeltaV = (VRight - VLeft.d) >> PERSP_SHIFT;                      \
                                                                             \
            while( iCount-- > 0 )                                            \
            {                                                                \
               cTemp    = *((CHR *) pBump[VLeft.w.h&VMask] + (ULeft.b.h&UMask));           \
               *(pScr++)=OMap[(((UBYT *)pPhong[(((CurT.b.h+cTemp)<<8)&       \
                                0xff00)+((CurS.b.h+cTemp)&0x00ff)])          \
                               [*((UBYT *)pMap[VLeft.w.h&VMask]+             \
                                  (ULeft.b.h&UMask))]<<8)+*pScr];            \
               CurS.d +=DeltaS;                                              \
               CurT.d +=DeltaT;                                              \
               ULeft.d+=DeltaU;                                              \
               VLeft.d+=DeltaV;                                              \
            }                                                                \
         }                                                                   \
                                                                             \
         pScanStart += iPitch;                                               \
         start##S   += start##DS;                                            \
         start##T   += start##DT;                                            \
         start##U   += start##DU; end##U += end##DU;                         \
         start##V   += start##DV; end##V += end##DV;                         \
         start##X   += start##DX; end##X += end##DX;                         \
         start##Z   += start##DZ; end##Z += end##DZ;                         \
      }                                                                      \
   }
   #else
   #define  INNER_LOOP(height, start, end)                                 \
   while( height-- )                                                       \
   {                                                                       \
      pScr   = pScanStart + FXD_TO_INT(start##X);                          \
      fxTemp = FXD_ONE - FixedFrac(start##X);                              \
      CurS.d = start##S + FixedMul(fDeltaS, fxTemp);                       \
      CurT.d = start##T + FixedMul(fDeltaT, fxTemp);                       \
      CurU.d = start##U + FixedMul(fDeltaU, fxTemp);                       \
      CurV.d = start##V + FixedMul(fDeltaV, fxTemp);                       \
      iCount = FXD_TO_INT(end##X) - FXD_TO_INT(start##X);                  \
                                                                           \
      while (iCount-- > 0)                                                 \
      {                                                                    \
         cTemp    = *((CHR *) pBump[CurV.b.h] + CurU.b.h);                 \
         *(pScr++)= OMap[(((UBYT *)pPhong[(((CurT.b.h+cTemp)<<8)&0xff00)+  \
                          ((CurS.b.h+cTemp)&0x00ff)])                      \
                         [*((UBYT *)pMap[CurV.b.h]+CurU.b.h)]<<8)+*pScr];  \
         CurS.d += fDeltaS;                                                \
         CurT.d += fDeltaT;                                                \
         CurU.d += fDeltaU;                                                \
         CurV.d += fDeltaV;                                                \
      }                                                                    \
                                                                           \
      pScanStart += iPitch;                                                \
      start##S   += start##DS;                                             \
      start##T   += start##DT;                                             \
      start##U   += start##DU;                                             \
      start##V   += start##DV;                                             \
      start##X   += start##DX;                                             \
      end  ##X   += end  ##DX;                                             \
   }
   #endif

   #define __INTERP_ST
   #define __INTERP_UV
   #define __PHONG
   #define __BUMP
   #define __TRANSPARENT
   #include TEXT_INCLUDE
   #undef  __TRANSPARENT
   #undef  __BUMP
   #undef  __PHONG
   #undef  __INTERP_UV
   #undef  __INTERP_ST
   #undef  INNER_LOOP
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXTransparentEnvBTri( TRI *Tri )
{
   #ifdef   U_P
   #define  INNER_LOOP( height, start, end )                                 \
   {                                                                         \
      fDeltaUaff = fDeltaU * F_PERSP_SPANS;                                  \
      fDeltaVaff = fDeltaV * F_PERSP_SPANS;                                  \
      fDeltaZaff = fDeltaZ * F_PERSP_SPANS;                                  \
      DeltaS     = FLT_TO_FXD(fDeltaS);                                      \
      DeltaT     = FLT_TO_FXD(fDeltaT);                                      \
                                                                             \
      while( height-- )                                                      \
      {                                                                      \
         pScr    = pScanStart + FLT_TO_INT(start##X);                        \
         CurS.d  = FLT_TO_FXD(start##S);                                     \
         CurT.d  = FLT_TO_FXD(start##T);                                     \
         fTemp   = 1.0 - FLT_FRAC(start##X);                                 \
         fCurU   = start##U + (fDeltaU * fTemp);                             \
         fCurV   = start##V + (fDeltaV * fTemp);                             \
         fCurZ   = start##Z + (fDeltaZ * fTemp);                             \
         fTemp   = 1.0 / fCurZ;                                              \
         ULeft.d = URight = FLT_TO_FXD(fCurU * fTemp);                       \
         VLeft.d = VRight = FLT_TO_FXD(fCurV * fTemp);                       \
         iCount  = FLT_TO_INT(end##X) - FLT_TO_INT(start##X);                \
                                                                             \
         while( iCount > I_PERSP_SPANS )                                     \
         {                                                                   \
            fCurU += fDeltaUaff;                                             \
            fCurV += fDeltaVaff;                                             \
            fCurZ += fDeltaZaff;                                             \
            fTemp  = 1.0 / fCurZ;                                            \
            URight = FLT_TO_FXD(fCurU * fTemp);                              \
            VRight = FLT_TO_FXD(fCurV * fTemp);                              \
            DeltaU = (URight - ULeft.d) >> PERSP_SHIFT;                      \
            DeltaV = (VRight - VLeft.d) >> PERSP_SHIFT;                      \
            iTemp  = I_PERSP_SPANS;                                          \
                                                                             \
            while(iTemp--)                                                   \
            {                                                                \
               cTemp    = *((CHR *) pBump[VLeft.w.h&VMask] + (ULeft.b.h&UMask));           \
               *(pScr++)= OMap[(((UBYT *) pPhong[(((CurT.b.h+cTemp)<<8)&     \
                                0xff00)+((CurS.b.h+cTemp)&0x00ff)])          \
                                [*((UBYT *)pMap[(CurT.b.h+cTemp)&0xff]+      \
                                ((CurS.b.h+cTemp)&0xff))]<<8)+*pScr];        \
               CurS.d +=DeltaS;                                              \
               CurT.d +=DeltaT;                                              \
               ULeft.d+=DeltaU;                                              \
               VLeft.d+=DeltaV;                                              \
            }                                                                \
                                                                             \
            ULeft.d  = URight;                                               \
            VLeft.d  = VRight;                                               \
            iCount  -= I_PERSP_SPANS;                                        \
         }                                                                   \
                                                                             \
         if (iCount > 0)                                                     \
         {                                                                   \
            fCurU += fDeltaUaff;                                             \
            fCurV += fDeltaVaff;                                             \
            fCurZ += fDeltaZaff;                                             \
            fTemp  = 1.0 / fCurZ;                                            \
            URight = FLT_TO_FXD(fCurU * fTemp);                              \
            VRight = FLT_TO_FXD(fCurV * fTemp);                              \
            DeltaU = (URight - ULeft.d) >> PERSP_SHIFT;                      \
            DeltaV = (VRight - VLeft.d) >> PERSP_SHIFT;                      \
                                                                             \
            while( iCount-- > 0 )                                            \
            {                                                                \
               cTemp    = *((CHR *) pBump[VLeft.w.h&VMask] + (ULeft.b.h&UMask));           \
               *(pScr++)= OMap[(((UBYT *) pPhong[(((CurT.b.h+cTemp)<<8)&     \
                                0xff00)+((CurS.b.h+cTemp)&0x00ff)])          \
                                [*((UBYT *)pMap[(CurT.b.h+cTemp)&0xff]+      \
                                ((CurS.b.h+cTemp)&0xff))]<<8)+*pScr];        \
               CurS.d +=DeltaS;                                              \
               CurT.d +=DeltaT;                                              \
               ULeft.d+=DeltaU;                                              \
               VLeft.d+=DeltaV;                                              \
            }                                                                \
         }                                                                   \
                                                                             \
         pScanStart += iPitch;                                               \
         start##S   += start##DS;                                            \
         start##T   += start##DT;                                            \
         start##U   += start##DU; end##U += end##DU;                         \
         start##V   += start##DV; end##V += end##DV;                         \
         start##X   += start##DX; end##X += end##DX;                         \
         start##Z   += start##DZ; end##Z += end##DZ;                         \
      }                                                                      \
   }
   #else
   #define  INNER_LOOP(height, start, end)                                 \
   while( height-- )                                                       \
   {                                                                       \
      pScr   = pScanStart + FXD_TO_INT(start##X);                          \
      fxTemp = FXD_ONE - FixedFrac(start##X);                              \
      CurS.d = start##S + FixedMul(fDeltaS, fxTemp);                       \
      CurT.d = start##T + FixedMul(fDeltaT, fxTemp);                       \
      CurU.d = start##U + FixedMul(fDeltaU, fxTemp);                       \
      CurV.d = start##V + FixedMul(fDeltaV, fxTemp);                       \
      iCount = FXD_TO_INT(end##X) - FXD_TO_INT(start##X);                  \
                                                                           \
      while (iCount-- > 0)                                                 \
      {                                                                    \
         cTemp    = *((CHR *) pBump[CurV.b.h] + CurU.b.h);                 \
         *(pScr++)= OMap[(((UBYT *) pPhong[(((CurT.b.h+cTemp)<<8)&0xff00)+ \
                          ((CurS.b.h+cTemp)&0x00ff)])                      \
                          [*((UBYT *)pMap[(CurT.b.h+cTemp)&0xff]+          \
                          ((CurS.b.h+cTemp)&0xff))]<<8)+*pScr];            \
         CurS.d += fDeltaS;                                                \
         CurT.d += fDeltaT;                                                \
         CurU.d += fDeltaU;                                                \
         CurV.d += fDeltaV;                                                \
      }                                                                    \
                                                                           \
      pScanStart += iPitch;                                                \
      start##S   += start##DS;                                             \
      start##T   += start##DT;                                             \
      start##U   += start##DU;                                             \
      start##V   += start##DV;                                             \
      start##X   += start##DX;                                             \
      end  ##X   += end  ##DX;                                             \
   }
   #endif

   #define __INTERP_ST
   #define __INTERP_UV
   #define __PHONG
   #define __BUMP
   #define __TRANSPARENT
   #include TEXT_INCLUDE
   #undef  __TRANSPARENT
   #undef  __BUMP
   #undef  __PHONG
   #undef  __INTERP_UV
   #undef  __INTERP_ST
   #undef  INNER_LOOP
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [POLYS.C    ] - End Of File                                            ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

