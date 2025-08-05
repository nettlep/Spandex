// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [OBJECTS.C   ] - Object based routines                                 -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#include "spandex.h"

/*----------------------------------------------------------------------------*/

INT   SPDXCreateObject( OBJ **Object )
{
   FUNC("SPDXCreateObject");
   Assert( Object );

   *Object = (OBJ *) SPDXMalloc( sizeof(OBJ) );

   if (!*Object)
      return LE_NOMEM;

   SPDXMakeObjectDefault( *Object );

   return LE_NONE;
}

/*----------------------------------------------------------------------------*/

INT   SPDXAddChild( OBJ *Parent )
{
OBL   *Children;
OBJ   *oTmp;

   FUNC("SPDXAddChild");
   Assert( Parent );

   Children = &Parent->Children;

   if (Children->Count)
   {
      oTmp = (OBJ *) SPDXRealloc( Children->Objs, (Children->Count+1) * sizeof(OBJ) );

      if (!oTmp)
         return LE_NOMEM;

      Children->Objs = oTmp;
   }
   else
   {
      Children->Objs = (OBJ *) SPDXMalloc( sizeof(OBJ) );

      if (!Children->Objs)
         return LE_NOMEM;
   }

   SPDXInheritObject( Parent, &Children->Objs[Children->Count] );

   Children->Count++;

   return LE_NONE;
}

/*----------------------------------------------------------------------------*/

INT   SPDXRemoveChild( OBJ *Parent, INT WhichChild )
{
OBL   *Children;
OBJ   *oTmp;

   FUNC("SPDXRemoveChild");
   Assert( Parent );

   Children = &Parent->Children;

   if (Children->Count < WhichChild)
      return LE_RANGE;

   SPDXCleanupObject( &Children->Objs[WhichChild] );

   if (Children->Count != WhichChild)
      memmove( &Children->Objs[WhichChild],
               &Children->Objs[WhichChild+1],
               (Children->Count - WhichChild - 1) * sizeof( OBJ ) );

   Children->Count--;

   oTmp = (OBJ *) SPDXRealloc( Children->Objs, (Children->Count-1) * sizeof(OBJ) );

   if (!oTmp)
      return LE_NOMEM;

   Children->Objs = oTmp;

   return LE_NONE;
}

/*----------------------------------------------------------------------------*/

OBJ   *SPDXGetChild( OBJ *Parent, INT Index )
{
   FUNC("SPDXGetChild");
   Assert( Parent );

   if (Index > Parent->Children.Count || Index < 0)
      return NULL;

   return &Parent->Children.Objs[Index];
}

/*----------------------------------------------------------------------------*/

void  SPDXInheritObject( OBJ *Parent, OBJ *Child )
{
   FUNC("SPDXInheritObject");
   Assert( Parent );
   Assert( Child );

   SPDXMemCopyBYTE( Child, Parent, sizeof(OBJ) );
   SPDXMemSetBYTE( &Child->LightList,  0, sizeof(LLS));
   SPDXMemSetBYTE( &Child->TriList,    0, sizeof(TLS));
   SPDXMemSetBYTE( &Child->PointList,  0, sizeof(PLS));
   SPDXMemSetBYTE( &Child->VertexList, 0, sizeof(VLS));
   SPDXMemSetBYTE( &Child->Children,   0, sizeof(OBL));
}

/*----------------------------------------------------------------------------*/

INT   SPDXGetChildCount( OBJ *Parent )
{
   FUNC("SPDXGetChildCount");
   Assert( Parent );

   return Parent->Children.Count;
}

/*----------------------------------------------------------------------------*/

void  SPDXMakeObjectDefault( OBJ *Object )
{
   FUNC("SPDXMakeObjectDefault");
   Assert( Object );

   SPDXMemSetBYTE(Object, 0, sizeof(OBJ));
   Object->DrawFlags = DRAW_ALL;
   Object->LDir.dx = 0;
   Object->LDir.dy = 0;
   Object->LDir.dz = 1.0;
   Object->Lense = 48.23529;
   SPDXMakeMatrixIdentity( Object->RotMat );
}

/*----------------------------------------------------------------------------*/

INT   SPDXGetObjectCount( OBL *Object )
{
   FUNC("SPDXGetObjectCount");
   Assert( Object );

   return Object->Count;
}

/*----------------------------------------------------------------------------*/

void  SPDXSetObjectName( OBJ *Object, CHR *Name )
{
   FUNC("SPDXSetObjectName");
   Assert( Object );
   Assert( Name );

   strcpy( Object->Name, Name );
}

/*----------------------------------------------------------------------------*/

CHR   *SPDXGetObjectName( OBJ *Object )
{
   FUNC("SPDXGetObjectName");
   Assert( Object );

   return Object->Name;
}

/*----------------------------------------------------------------------------*/

void  SPDXSetObjectLense( OBJ *Object, FLT Lense )
{
   FUNC("SPDXSetObjectLense");
   Assert( Object );

   Object->Lense = Lense;
}

/*----------------------------------------------------------------------------*/

FLT   SPDXGetObjectLense( OBJ *Object )
{
   FUNC("SPDXGetObjectLense");
   Assert( Object );

   return Object->Lense;
}

/*----------------------------------------------------------------------------*/

void  SPDXSetObjectShowBacks( OBJ *Object, BYT ShowBacks )
{
   FUNC("SPDXSetObjectShowBacks");
   Assert( Object );

   Object->BackFace = ShowBacks;
}

/*----------------------------------------------------------------------------*/

BYT   SPDXGetObjectShowBacks( OBJ *Object )
{
   FUNC("SPDXGetObjectShowBacks");
   Assert( Object );

   return Object->BackFace;
}

/*----------------------------------------------------------------------------*/

void  SPDXSetObjectShadeModel( OBJ *Object, BYT Model )
{
   FUNC("SPDXSetObjectShadeModel");
   Assert( Object );

   Object->ShadeModel = Model;
}

/*----------------------------------------------------------------------------*/

BYT   SPDXGetObjectShadeModel( OBJ *Object )
{
   FUNC("SPDXGetObjectShadeModel");
   Assert( Object );

   return Object->ShadeModel;
}

/*----------------------------------------------------------------------------*/

void  SPDXSetObjectDrawFlags( OBJ *Object, BYT DrawFlags )
{
   FUNC("SPDXSetObjectDrawFlags");
   Assert( Object );

   Object->DrawFlags = DrawFlags;
}

/*----------------------------------------------------------------------------*/

BYT   SPDXGetObjectDrawFlags( OBJ *Object )
{
   FUNC("SPDXGetObjectDrawFlags");
   Assert( Object );

   return Object->DrawFlags;
}

/*----------------------------------------------------------------------------*/

void  SPDXSetObjectSteadyOrientation( OBJ *Object, ROT *Rotation )
{
   FUNC("SPDXSetObjectSteadyOrientation");
   Assert( Object );
   Assert( Rotation );

   while( Rotation->x >= ROT_POINTS )
      Rotation->x -= ROT_POINTS;

   while( Rotation->y >= ROT_POINTS )
      Rotation->y -= ROT_POINTS;

   while( Rotation->z >= ROT_POINTS )
      Rotation->z -= ROT_POINTS;

   while( Rotation->x < 0 )
      Rotation->x += ROT_POINTS;

   while( Rotation->y < 0 )
      Rotation->y += ROT_POINTS;

   while( Rotation->z < 0 )
      Rotation->z += ROT_POINTS;

   Object->SteadyRot = *Rotation;
}

/*----------------------------------------------------------------------------*/

ROT   *SPDXGetObjectSteadyOrientation( OBJ *Object )
{
   FUNC("SPDXGetObjectSteadyOrientation");
   Assert( Object );

   return &Object->SteadyRot;
}

/*----------------------------------------------------------------------------*/

void  SPDXSetObjectDirection( OBJ *Object, VEC *Direction )
{
   FUNC("SPDXSetObjectDirection");
   Assert( Object );
   Assert( Direction );

   Object->LDir = *Direction;
}

/*----------------------------------------------------------------------------*/

VEC   *SPDXGetObjectDirection( OBJ *Object )
{
   FUNC("SPDXGetObjectDirection");
   Assert( Object );

   return &Object->LDir;
}

/*----------------------------------------------------------------------------*/

void  SPDXSetObjectLocal( OBJ *Object, P3D *Location )
{
   FUNC("SPDXSetObjectLocal");
   Assert( Object );
   Assert( Location );

   Object->LLoc = *Location;
}

/*----------------------------------------------------------------------------*/

P3D   *SPDXGetObjectLocal( OBJ *Object)
{
   FUNC("SPDXGetObjectLocal");
   Assert( Object );

   return &Object->LLoc;
}

/*----------------------------------------------------------------------------*/

void  SPDXSetObjectWorld( OBJ *Object, P3D *Location )
{
   FUNC("SPDXSetObjectWorld");
   Assert( Object );
   Assert( Location );

   Object->WLoc = *Location;
}

/*----------------------------------------------------------------------------*/

P3D   *SPDXGetObjectWorld( OBJ *Object)
{
   FUNC("SPDXGetObjectWorld");
   Assert( Object );

   return &Object->WLoc;
}

/*----------------------------------------------------------------------------*/

void  SPDXSetObjectType( OBJ *Object, INT Type )
{
   FUNC("SPDXSetObjectType");
   Assert( Object );

   Object->Type = Type;
}

/*----------------------------------------------------------------------------*/

INT   SPDXGetObjectType( OBJ *Object )
{
   FUNC("SPDXGetObjectType");
   Assert( Object );

   return Object->Type;
}

/*----------------------------------------------------------------------------*/

void  SPDXSetObjectDevUse( OBJ *Object, INT Index, INT Value )
{
   FUNC("SPDXSetObjectDevUse");
   Assert( Object );

   if (Index < DEV_USE_COUNT && Index >= 0)
      Object->DevUse[Index] = Value;
}

/*----------------------------------------------------------------------------*/

INT   SPDXGetObjectDevUse( OBJ *Object, INT Index )
{
   FUNC("SPDXGetObjectDevUse");
   Assert( Object );

   if (Index < DEV_USE_COUNT && Index >= 0)
      return Object->DevUse[Index];
   else
      return 0;
}

/*----------------------------------------------------------------------------*/

void  SPDXTranslateObject( OBJ *Object, VEC *Vector )
{
P3D   *Location;

   FUNC("SPDXTranslateObject");
   Assert( Object );
   Assert( Vector );

   Location = SPDXGetObjectLocal( Object );
   Location->x += Vector->dx;
   Location->y += Vector->dy;
   Location->z += Vector->dz;
}

/*----------------------------------------------------------------------------*/

void  SPDXTranslateObjectFacing( OBJ *Object, FLT Amplitude )
{
P3D   *Location;
VEC   *Direction;

   FUNC("SPDXTranslateObjectFacing");
   Assert( Object );

   Location = SPDXGetObjectLocal( Object );
   Direction = &Object->WDir;

   Location->x += Direction->dx * Amplitude;
   Location->y += Direction->dy * Amplitude;
   Location->z += Direction->dz * Amplitude;
}

/*----------------------------------------------------------------------------*/

void  SPDXRotateObject( OBJ *Object, ROT *Rotation )
{
MAT   MatA, MatB, MatX, MatY, MatZ;

   FUNC("SPDXRotateObject");
   Assert( Object );
   Assert( Rotation );

   while( Rotation->x >= ROT_POINTS )
      Rotation->x -= ROT_POINTS;

   while( Rotation->y >= ROT_POINTS )
      Rotation->y -= ROT_POINTS;

   while( Rotation->z >= ROT_POINTS )
      Rotation->z -= ROT_POINTS;

   while( Rotation->x < 0 )
      Rotation->x += ROT_POINTS;

   while( Rotation->y < 0 )
      Rotation->y += ROT_POINTS;

   while( Rotation->z < 0 )
      Rotation->z += ROT_POINTS;

   // Add this rotation to the rotation matrix for the object
   SPDXMakeXMatrix(MatX, (INT) Rotation->x);
   SPDXMakeYMatrix(MatY, (INT) Rotation->y);
   SPDXMakeZMatrix(MatZ, (INT) Rotation->z);

   // Concatinate the rotation matrix into RotMat
   SPDXMatrixMulMatrix(MatA, Object->RotMat, MatX);
   SPDXMatrixMulMatrix(MatB, MatA, MatY);
   SPDXMatrixMulMatrix(Object->RotMat, MatB, MatZ);

   // Correct for precision errors by re-orthonormalizing
   SPDXOrthoNormalizeMatrix(Object->RotMat);
}

/*----------------------------------------------------------------------------*/

OBJ   *SPDXFindObjectByName( CHR *Name, OBJ *Universe, BYT CaseSense )
{
INT   i;
OBJ   *Object;

   FUNC("SPDXFindObjectByName");
   Assert( Name );
   Assert( Universe );

   if (CaseSense == TRUE)
   {
      if (!strcmp( Universe->Name, Name ))
         return Universe;
   }
   else
   {
      if (!strcasecmp( Universe->Name, Name ))
         return Universe;
   }

   for( i = 0; i < Universe->Children.Count; i++ )
   {
      Object = SPDXFindObjectByName( Name, &Universe->Children.Objs[i], CaseSense );

      if (Object)
         return Object;
   }

   return 0;
}

/*----------------------------------------------------------------------------*/

OBJ   *SPDXFindObjectByNameLen( CHR *Name, OBJ *Universe, INT Len, BYT CaseSense )
{
INT   i;
OBJ   *Object;

   FUNC("SPDXFindObjectByNameLen");
   Assert( Name );
   Assert( Universe );

   if (CaseSense == TRUE)
   {
      if (!strncmp( Universe->Name, Name, Len ))
         return Universe;
   }
   else
   {
      if (!strncasecmp( Universe->Name, Name, Len ))
         return Universe;
   }

   for( i = 0; i < Universe->Children.Count; i++ )
   {
      Object = SPDXFindObjectByNameLen( Name, &Universe->Children.Objs[i], Len, CaseSense );

      if (Object)
         return Object;
   }

   return 0;
}

/*----------------------------------------------------------------------------*/

void  SPDXUnifyObjectNormals( OBJ *Object )
{
INT   CurVer, CurTri, VCount, TCount;
FLT   TriCount, VLen;
VEC   VerVec;
VER   *Ver;
TRI   *Tri;

   VCount = Object->VertexList.Count;

   for( CurVer = 0; CurVer < VCount; CurVer++ )
   {
      Ver = &Object->VertexList.Vers[CurVer];

      VerVec.dx = 0.0;
      VerVec.dy = 0.0;
      VerVec.dz = 0.0;
      TriCount  = 0.0;

      TCount = Object->TriList.Count;

      for( CurTri = 0; CurTri < TCount; CurTri++ )
      {
         Tri = &Object->TriList.Tris[CurTri];

         if (Tri->V1 == Ver || Tri->V2 == Ver || Tri->V3 == Ver)
         {
            VerVec.dx += Tri->LN.dx;
            VerVec.dy += Tri->LN.dy;
            VerVec.dz += Tri->LN.dz;
            TriCount  += 1.0;
         }
      }

      if (TriCount)
      {
         VLen = 1.0 / TriCount;
         VerVec.dx *= VLen;
         VerVec.dy *= VLen;
         VerVec.dz *= VLen;

         SPDXVectorToUnitVector( VerVec, VLen );

         Ver->LN.dx = VerVec.dx;
         Ver->LN.dy = VerVec.dy;
         Ver->LN.dz = VerVec.dz;
      }
   }
}

/*----------------------------------------------------------------------------*/

void  SPDXCleanupObject( OBJ *Object )
{
INT   i;

   FUNC("SPDXCleanupObject");
   Assert( Object );

   // Recursively clean up the children
   for( i = 0; i < Object->Children.Count; i++ )
      SPDXCleanupObject( &Object->Children.Objs[i] );

   // Free the Children
   if (Object->Children.Objs)
   {
      SPDXFree( Object->Children.Objs );
      Object->Children.Objs = NULL;
      Object->Children.Count = 0;
   }

   // Free the points
   if (Object->PointList.Count)
   {
      SPDXFree( Object->PointList.Points );
      Object->PointList.Points = NULL;
      Object->PointList.Count = 0;
   }

   // Free the tris
   if (Object->TriList.Tris)
   {
      SPDXFree( Object->TriList.Tris );
      Object->TriList.Tris = NULL;
      Object->TriList.Count = 0;
   }

   // Free the vertices
   if (Object->VertexList.Vers)
   {
      SPDXFree( Object->VertexList.Vers );
      Object->VertexList.Vers = NULL;
      Object->VertexList.Count = 0;
   }

   // Free the lights
   if (Object->LightList.Lgts)
   {
      SPDXFree( Object->LightList.Lgts );
      Object->LightList.Lgts = NULL;
      Object->LightList.Count = 0;
   }
}

/*----------------------------------------------------------------------------
  -   [OBJECTS.C   ] - End Of File                                           -
  ----------------------------------------------------------------------------*/
