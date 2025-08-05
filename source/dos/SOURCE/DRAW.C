// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [DRAW.C      ] - Routines to causes things to be drawn                 ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#include "spandex.h"

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXDrawRenderList( RLS *RList, OBJ *Cam, rendfunptr *SolidRenderers, rendfunptr *TransRenderers, VSCR *VScreen )
{
INT   i, Count;
VER   *Ver, *Ver1, *Ver2, *Ver3;
TRI   *Tri;
UBYT  Material, Trans;
CHR   *Intensity;

   FUNC("SPDXDrawRenderList");
   Assert( RList );
   Assert( Cam );
   Assert( VScreen );

   Count = RList->PointCount;

   // Draw the points...
   for( i = 0; i < Count; i++ )
   {
      Ver = RList->Points[i];

      if (Ver->ShadeModel & (GOURAUD | LAMBERT))
      {
         Intensity = &SPDXGlobals.PalTab[Ver->Material->Color];
         SPDXSetVirtualPixel( Ver->Scr.x, Ver->Scr.y, Intensity[FLT_TO_INT(Ver->Int) * 256], VScreen);
      }
      else
      {
         SPDXSetVirtualPixel( Ver->Scr.x, Ver->Scr.y, SPDXGlobals.PalTab[FLT_TO_INT(Ver->Material->Color) *
                              MAP_SHADES + MAP_SHADES - 1], VScreen);
      }
   }

   // Draw the tris...

   if (VScreen->MaxPolysToRender && RList->TriCount > VScreen->MaxPolysToRender)
      i = RList->TriCount - VScreen->MaxPolysToRender;
   else
      i = 0;

   Count = RList->TriCount;

   for( ;i < Count; i++ )
   {
      Tri      = RList->Tris[i];
      Material = Tri->Object->ShadeModel + Tri->Material->Surface;
      Trans    = Tri->Material->Transparency;
      Ver1     = Tri->V1;
      Ver2     = Tri->V2;
      Ver3     = Tri->V3;

      if (Ver1->Visible != VIS_YES ||
          Ver2->Visible != VIS_YES ||
          Ver3->Visible != VIS_YES)
         continue;

      if (Material & (TEXTURED | BUMP))
      {
         #ifdef U_P
         if (!(Material & SOLID))
         {
            Tri->T1.u  = Tri->M1.x * Ver1->OneOverZ;
            Tri->T1.v  = Tri->M1.y * Ver1->OneOverZ;
            Tri->T2.u  = Tri->M2.x * Ver2->OneOverZ;
            Tri->T2.v  = Tri->M2.y * Ver2->OneOverZ;
            Tri->T3.u  = Tri->M3.x * Ver3->OneOverZ;
            Tri->T3.v  = Tri->M3.y * Ver3->OneOverZ;
            Tri->T1.fu = FLT_TO_FXD(Tri->T1.u);
            Tri->T1.fv = FLT_TO_FXD(Tri->T1.v);
            Tri->T2.fu = FLT_TO_FXD(Tri->T2.u);
            Tri->T2.fv = FLT_TO_FXD(Tri->T2.v);
            Tri->T3.fu = FLT_TO_FXD(Tri->T3.u);
            Tri->T3.fv = FLT_TO_FXD(Tri->T3.v);
         }
         else
         {
            Tri->T1 = Tri->M1;
            Tri->T2 = Tri->M2;
            Tri->T3 = Tri->M3;
         }
         #else
         Tri->T1 = Tri->M1;
         Tri->T2 = Tri->M2;
         Tri->T3 = Tri->M3;
         #endif
      }
      else if (Material & ENVMAP)
      {
         #ifdef U_P
         Tri->T1.u  = Ver1->Env.x * Ver1->OneOverZ;
         Tri->T1.v  = Ver1->Env.y * Ver1->OneOverZ;
         Tri->T2.u  = Ver2->Env.x * Ver2->OneOverZ;
         Tri->T2.v  = Ver2->Env.y * Ver2->OneOverZ;
         Tri->T3.u  = Ver3->Env.x * Ver3->OneOverZ;
         Tri->T3.v  = Ver3->Env.y * Ver3->OneOverZ;
         Tri->T1.fu = FLT_TO_FXD(Tri->T1.u);
         Tri->T1.fv = FLT_TO_FXD(Tri->T1.v);
         Tri->T2.fu = FLT_TO_FXD(Tri->T2.u);
         Tri->T2.fv = FLT_TO_FXD(Tri->T2.v);
         Tri->T3.fu = FLT_TO_FXD(Tri->T3.u);
         Tri->T3.fv = FLT_TO_FXD(Tri->T3.v);
         #else
         Tri->T1 = Ver1->Env;
         Tri->T2 = Ver2->Env;
         Tri->T3 = Ver3->Env;
         #endif
      }

      if (Trans == TRUE)
      {
          (TransRenderers[Material])( Tri );
      }
      else
      {
          (SolidRenderers[Material])( Tri );
      }
   }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [DRAW.C      ] - End Of File                                           ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
