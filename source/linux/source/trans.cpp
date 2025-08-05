// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [TRANS.C     ] - Transformation routines                               -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#include "spandex.h"

/*----------------------------------------------------------------------------*/

static   void  TransformMatrices(OBJ *Object);

/*----------------------------------------------------------------------------*/

void  SPDXTransformHierarchy( OBJ *Object )
{
INT   i, Count;
OBJ   *Obj;

   FUNC("SPDXTransformHierarchy");
   Assert(Object);

   // Transform me
   TransformMatrices(Object);

   // Transform my children
   Count = Object->Children.Count;
   Obj = Object->Children.Objs;

   for( i = 0; i < Count; i++, Obj++ )
      SPDXTransformHierarchy( Obj );
}

/*----------------------------------------------------------------------------*/

void  TransformMatrices(OBJ *Object)
{
UBYT  ShadeModel;
INT   i, Count;
MAT   RotMat;
VLS   *Vls;
TLS   *Tls;
LLS   *Lls;
VER   *Ver;
LGT   *Lgt;
TRI   *Tri;

   FUNC("TransformMatrices");
   Assert(Object);

   // Gather these for some speed...
   ShadeModel = Object->ShadeModel;
   RotMat = Object->PhaseTwo;
   Vls = &Object->VertexList;
   Tls = &Object->TriList;
   Lls = &Object->LightList;

   Ver = Vls->Vers;
   Count = Vls->Count;

   // Perform transformation on all vertex locations
   for (i = 0; i < Count; i++, Ver++)
   {
      // Only the visible ones...
      if (Ver->Visible != VIS_YES)
         continue;

      // Transform this vertex
      SPDXMatrixMulVertex( Ver->CLoc, RotMat, Ver->LLoc );
      Ver->CLoc.x += Object->CLoc.x;
      Ver->CLoc.y += Object->CLoc.y;
      Ver->CLoc.z += Object->CLoc.z;
      Ver->FixedZ  = FLT_TO_FXD(Ver->CLoc.z);
   }

   Lgt = Lls->Lgts;
   Count = Lls->Count;

   // Perform transformation on all light locations
   for (i = 0; i < Count; i++, Lgt++)
   {
      // Transform this light
      SPDXMatrixMulVertex( Lgt->CLoc, RotMat, Lgt->LLoc );
      Lgt->CLoc.x += Object->CLoc.x;
      Lgt->CLoc.y += Object->CLoc.y;
      Lgt->CLoc.z += Object->CLoc.z;
   }

   Count = Vls->Count;
   Ver = Vls->Vers;

   for (i = 0; i < Count; i++, Ver++)
   {
      if (Ver->Visible == VIS_YES)
      {
         // Transform this Vector
         SPDXMatrixMulVector( Ver->CN, RotMat, Ver->LN );

         // Perform rotation on all vertex normals and Env coords (dx/dy only)
         if (ShadeModel & (PHONG | BUMP) || (Ver->Material->Surface & ENVMAP))
         {   
            Ver->Env.x  = ((Ver->CN.dx + 1.0) / 2.0) * 256;
            Ver->Env.y  = (1.0 - ((Ver->CN.dy + 1.0) / 2.0)) * 256;
            Ver->Env.fx = FLT_TO_FXD(Ver->Env.x);
            Ver->Env.fy = FLT_TO_FXD(Ver->Env.y);
         }
         // Perform rotation on all vertex normals only
         else if (ShadeModel & GOURAUD)
         {
            SPDXMatrixMulVector( Ver->CN, RotMat, Ver->LN );
         }
      }
   }

   // Perform rotation on all poly normals
   if (ShadeModel & LAMBERT)
   {
      Count = Tls->Count;
      Tri = Tls->Tris;

      for (i = 0; i < Count; i++, Tri++)
         if (Tri->Visible == VIS_YES)
            SPDXMatrixMulVector( Tri->CN, RotMat, Tri->LN );
   }

}

/*----------------------------------------------------------------------------
  -   [TRANS.C    ] - End Of File                                            -
  ----------------------------------------------------------------------------*/
