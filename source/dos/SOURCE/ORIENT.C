// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [ORIENT.C    ] - Hierarchy orientation                                 ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#include "spandex.h"

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXOrientHierarchy( OBJ *Object, OBJ *Camera )
{
MAT   TempMat;
P3D   TempLoc;

   FUNC("SPDXOrientHierarchy");
   Assert( Object );
   Assert( Camera );

   // Setup 'Absolute World' object locaion
   SPDXMemSetDWORD( &TempLoc, 0, sizeof( TempLoc ) >>2 );
   SPDXMakeMatrixIdentity( TempMat );

   // Go off and build Phase 1 matrices
   SPDXBuildPhaseOne( Object, &TempLoc, &TempMat );

   // Invert camera's Phase 1 matrix into a temp matrix
   TempMat = Camera->PhaseOne;
   SPDXInvertOrthoMatrix(TempMat);

   // Go off and build Phase 2 matrices
   SPDXBuildPhaseTwo( Object, &TempMat, &Camera->WLoc );
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXBuildPhaseOne( OBJ *Object, P3D *ParentLoc, MAT *ParentMat )
{
INT   i, Count;
P3D   TempVer;
OBJ   *Obj;

   FUNC("SPDXBuildPhaseOne");
   Assert( Object );
   Assert( ParentLoc );
   Assert( ParentMat );

   // Add in the steady rotation
   SPDXRotateObject( Object, &Object->SteadyRot );

   // Multiply Parent's matrix by the new rotation matrix
   SPDXMatrixMulMatrix(Object->PhaseOne, *ParentMat, Object->RotMat);

   // Transform location (relative parent)
   SPDXMatrixMulVertex(TempVer, *ParentMat, Object->LLoc);
   Object->WLoc.x = TempVer.x + ParentLoc->x;
   Object->WLoc.y = TempVer.y + ParentLoc->y;
   Object->WLoc.z = TempVer.z + ParentLoc->z;

   // Transform direction (relative Parent)
   SPDXMatrixMulVector(Object->WDir, Object->PhaseOne, Object->LDir);

   Count = Object->Children.Count;
   Obj   = Object->Children.Objs;

   // Rotate my children
   for( i = 0; i < Count; i++, Obj++ )
      SPDXBuildPhaseOne( Obj, &Object->WLoc, &Object->PhaseOne );
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXBuildPhaseTwo( OBJ *Object, MAT *CamMat, P3D *CamLoc )
{
INT   i, Count;
P3D   TempVer;
OBJ   *Obj;

   FUNC("SPDXBuildPhaseTwo");
   Assert( Object );
   Assert( CamMat );
   Assert( CamLoc );

   // Transform location (relative camera)
   TempVer.x = Object->WLoc.x - CamLoc->x;
   TempVer.y = Object->WLoc.y - CamLoc->y;
   TempVer.z = Object->WLoc.z - CamLoc->z;
   SPDXMatrixMulVertex(Object->CLoc, *CamMat, TempVer);

   SPDXMatrixMulMatrix(Object->PhaseTwo, *CamMat, Object->PhaseOne);

   Count = Object->Children.Count;
   Obj   = Object->Children.Objs;

   for( i = 0; i < Count; i++, Obj++ )
      SPDXBuildPhaseTwo( Obj, CamMat, CamLoc );
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [ORIENT.C    ] - End Of File                                           ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
