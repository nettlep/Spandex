// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [CULL.C      ] - Pre-rotation smart culling routines                   ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#include "Spandex.h"

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static   void  CullHierarchy( OBJ *Object );
static   void  CullObject( OBJ *Object );

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXCullHierarchy( OBJ *Object )
{
   FUNC("SPDXCullHierarchy");
   Assert(Object);

   CullHierarchy( Object );
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  CullHierarchy( OBJ *Object )
{
INT   i;

   FUNC("CullHierarchy");
   Assert(Object);

   // Cull me
   CullObject( Object );

   // Cull my children
   for( i = 0; i < Object->Children.Count; i++ )
      CullHierarchy( &Object->Children.Objs[i] );
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  CullObject( OBJ *Object )
{
INT   i, Count;
TRI   *Tri;
VER   *Ver;
VEC   CullDir;
P3D   CullLoc, TempLoc;
MAT   InverseMat;
FLT   fTemp;

   FUNC("CullObject");
   Assert(Object);

   TempLoc.x = -Object->CLoc.x;
   TempLoc.y = -Object->CLoc.y;
   TempLoc.z = -Object->CLoc.z;
   InverseMat = Object->PhaseTwo;
   SPDXInvertOrthoMatrix( InverseMat );
   SPDXMatrixMulVertex( CullLoc, InverseMat, TempLoc );

   if (Object->BackFace == TRUE)
   {
      // Set all the vertices for this object...
      Ver   = Object->VertexList.Vers;
      Count = Object->VertexList.Count;

      for( i = 0; i < Count; i++, Ver++ )
         Ver->Visible = VIS_YES;

      // Now go thru the polys and set them visible
      if (Object->DrawFlags & DRAW_TRIS)
      {
         Tri   = Object->TriList.Tris;
         Count = Object->TriList.Count;

         for (i = 0; i < Count; i++, Tri++ )
            Tri->Visible = VIS_YES;
      }

      return;
   }

   if (Object->ShadeModel & (LAMBERT | AMBIENT))
   {
      // Reset all the vertices for this object...
      Ver   = Object->VertexList.Vers;
      Count = Object->VertexList.Count;

      for( i = 0; i < Count; i++, Ver++ )
         Ver->Visible = VIS_NO;

      // Cull the tris
      if (Object->DrawFlags & DRAW_TRIS)
      {
         Tri   = Object->TriList.Tris;
         Count = Object->TriList.Count;

         fTemp = 1.0 / 3.0;

         for (i = 0; i < Count; i++, Tri++)
         {
            TempLoc.x = (Tri->V1->LLoc.x + Tri->V2->LLoc.x + Tri->V3->LLoc.x) * fTemp;
            TempLoc.y = (Tri->V1->LLoc.y + Tri->V2->LLoc.y + Tri->V3->LLoc.y) * fTemp;
            TempLoc.z = (Tri->V1->LLoc.z + Tri->V2->LLoc.z + Tri->V3->LLoc.z) * fTemp;

            SPDXVectorFromPoints( CullDir, CullLoc, TempLoc );

            Tri->Visible = SPDXReverseVisibilityTest(&Tri->LN, &CullDir);

            if (Tri->Visible == VIS_YES)
            {
               Tri->V1->Visible = VIS_YES;
               Tri->V2->Visible = VIS_YES;
               Tri->V3->Visible = VIS_YES;
            }
         }
      }

      // Cull the points
      if (Object->DrawFlags & DRAW_POINTS)
      {
         Ver   = Object->VertexList.Vers;
         Count = Object->VertexList.Count;

         for (i = 0; i < Count; i++, Ver++ )
         {
            SPDXVectorFromPoints( CullDir, CullLoc, Ver->LLoc );

            if (SPDXReverseVisibilityTest(&Ver->LN, &CullDir) == VIS_YES)
               Ver->Visible = VIS_YES;
         }
      }
   }
   else
   {
      if (Object->DrawFlags != DRAW_NONE)
      {
         Ver = Object->VertexList.Vers;
         Count = Object->VertexList.Count;

         // Cull all the vertices
         for (i = 0; i < Count; i++, Ver++ )
         {
            SPDXVectorFromPoints( CullDir, CullLoc, Ver->LLoc );
            Ver->Visible = SPDXReverseVisibilityTest(&Ver->LN, &CullDir);
         }

         Tri = Object->TriList.Tris;
         Count = Object->TriList.Count;

         // Now go thru the polys and set their culling according to the vertices
         for (i = 0; i < Count; i++, Tri++ )
         {
            if (Tri->V1->Visible == VIS_YES || Tri->V2->Visible == VIS_YES || Tri->V3->Visible == VIS_YES)
               Tri->Visible = VIS_YES;
            else
               Tri->Visible = VIS_NO;
         }

         Tri = Object->TriList.Tris;
         Count = Object->TriList.Count;

         // Now go thru and make sure that all needed vertices are visible
         for (i = 0; i < Count; i++, Tri++ )
         {
            if (Tri->Visible == VIS_YES)
            {
               Tri->V1->Visible = VIS_YES;
               Tri->V2->Visible = VIS_YES;
               Tri->V3->Visible = VIS_YES;
            }
         }
      }
   }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [CULL.C    ] - End Of File                                             ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
