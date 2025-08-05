// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [SHADE.C     ] - Performs all shading on the given render list         ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#include <math.h>

#include "spandex.h"

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXShadeRenderList( RLS *RList )
{
INT   i, j, Count, LCount;
FLT   Int, Diff, VLen;
TRI   **Tris, *Tri;
VER   **Vers, *Ver;
VER   **Points, *Point;
LGT   **Lights, *Light;
MTRL  *Material;
VEC   LightVector;

   FUNC("SPDXShadeRenderList");
   Assert( RList );

   Vers = RList->Vers;
   Tris = RList->Tris;
   Points = RList->Points;
   Lights = SPDXGlobals.LightList;

   Count = RList->VertexCount;

   // Shade the vertices
   for( i = 0; i < Count; i++ )
   {
      Ver = Vers[i];
      Material = Ver->Material;

      // No need to do anything, we've already done it. :>
      if (!(Ver->ShadeModel & GOURAUD))
         continue;

      LCount = SPDXGlobals.LightCount;

      for(Int = Material->_AM, j = 0; j < LCount; j++)
      {
         Light = Lights[j];

         SPDXVectorFromPoints( LightVector, Ver->CLoc, Light->CLoc );
         SPDXVectorToUnitVector( LightVector, VLen );
         Diff = FLOOR(SPDXAngleFromVectors(&LightVector, &Ver->CN), 0);
         Int += (Material->Kd * Light->Int) * Diff;
      }

      Int = FLOOR(CEILING(Int, 1.0), 0);
      Ver->Int = Int * Material->_IR;
   }

   Count = RList->TriCount;

   // Shade the tris
   for( i = 0; i < Count; i++ )
   {
      Tri = Tris[i];
      Material = Tri->Material;

      if (Tri->Object->ShadeModel & LAMBERT)
      {
         LCount = SPDXGlobals.LightCount;

         for(Int = Material->_AM, j = 0; j < LCount; j++)
         {
            Light = Lights[j];

            SPDXVectorFromPoints( LightVector, Tri->V1->CLoc, Light->CLoc );
            SPDXVectorToUnitVector( LightVector, VLen );
            Diff = FLOOR(SPDXAngleFromVectors(&LightVector, &Tri->CN), 0);
            Int += (Material->Kd * Light->Int) * Diff;
         }

         Int = FLOOR(CEILING(Int, 1.0), 0);
         Tri->Int = Int * Material->_IR;
      }
      else if (Tri->Object->ShadeModel & AMBIENT)
      {
         Tri->Int = Material->_AM * Material->_IR;
      }
   }

   Count = RList->PointCount;

   // Shade the points
   for( i = 0; i < Count; i++ )
   {
      Point = Points[i];
      Material = Point->Material;

      // No need to do anything, we've already done it. :)
      if (Point->ShadeModel & LAMBERT)
      {
         LCount = SPDXGlobals.LightCount;

         for( Int = Material->_AM, j = 0; j < LCount; j++)
         {
            Light = Lights[j];

            SPDXVectorFromPoints( LightVector, Point->CLoc, Light->CLoc );
            SPDXVectorToUnitVector( LightVector, VLen );
            Diff = FLOOR(SPDXAngleFromVectors(&LightVector, &Point->CN), 0);
            Int += (Material->Kd * Light->Int) * Diff;
         }

         Int = FLOOR(CEILING(Int, 1.0), 0);
         Point->Int = Int * Material->_IR;
      }
      else if (Point->ShadeModel & AMBIENT)
      {
         Point->Int = Material->_AM * Material->_IR;
      }
   }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [SHADE.C     ] - End Of File                                           ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
