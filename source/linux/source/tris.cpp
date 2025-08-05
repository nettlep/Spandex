// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [TRIS.C      ] - Tri based routines                                    -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#include "spandex.h"

/*----------------------------------------------------------------------------*/

INT   SPDXAddObjectTri(OBJ *Object)
{
TRI  *TriList;

   FUNC("SPDXAddObjectTri");
   Assert( Object );

   if (!Object->TriList.Count)
   {
      TriList = (TRI *) SPDXMalloc(sizeof(TRI) );
   }
   else
   {
      TriList = (TRI *) SPDXRealloc(Object->TriList.Tris,
                                      sizeof(TRI) * (Object->TriList.Count + 1));
   }

   if (!TriList)
   {
      SPDXLogError( LE_NOMEM, "Unable to allocate polygon list" );
      return -1;
   }

   Object->TriList.Tris = TriList;

   // This returns the next index value (end + 1)
   return Object->TriList.Count++;
}

/*----------------------------------------------------------------------------*/

INT   SPDXGetObjectTriCount( OBJ *Object )
{
   FUNC("SPDXGetObjectTriCount");
   Assert( Object );

   return Object->TriList.Count;
}

/*----------------------------------------------------------------------------*/

void  SPDXSetTriVertices( OBJ *Object, INT Index, VER *V1, VER *V2, VER *V3)
{
   FUNC("SPDXSetTriVertices");
   Assert( Object );
   Assert( V1 );
   Assert( V2 );
   Assert( V3 );

   if (Index < Object->TriList.Count && Index >= 0)
   {
      Object->TriList.Tris[Index].V1 = V1;
      Object->TriList.Tris[Index].V2 = V2;
      Object->TriList.Tris[Index].V3 = V3;

      SPDXSetTriNormal( Object, Index );
   }
}

/*----------------------------------------------------------------------------*/

void  SPDXGetTriVertices( OBJ *Object, INT Index, VER *V1, VER *V2, VER *V3 )
{
   FUNC("SPDXGetTriVertices");
   Assert( Object );
   Assert( V1 );
   Assert( V2 );
   Assert( V3 );

   if (Index < Object->TriList.Count && Index >= 0)
   {
      V1 = Object->TriList.Tris[Index].V1;
      V2 = Object->TriList.Tris[Index].V2;
      V3 = Object->TriList.Tris[Index].V3;
   }
}

/*----------------------------------------------------------------------------*/

void  SPDXSetTriNormal( OBJ *Object, INT Index )
{
TLS   *TriList;

   FUNC("SPDXSetTriNormal");
   Assert( Object );

   TriList = &Object->TriList;

   if (Index >= TriList->Count || Index < 0 )
      return;

   SPDXGetTriNormalFromOrientation( &TriList->Tris[Index], &TriList->Tris[Index].LN );
}

/*----------------------------------------------------------------------------*/

void  SPDXGetTriNormal( OBJ *Object, INT Index, VEC *Normal )
{
TLS   *TriList;

   FUNC("SPDXGetTriNormal");
   Assert( Object );
   Assert( Normal );

   TriList = &Object->TriList;

   if (Index >= TriList->Count || Index < 0 )
      Normal->dx = Normal->dy = Normal->dz = 0;
   else
      *Normal = TriList->Tris[Index].LN;

   return;
}

/*----------------------------------------------------------------------------*/

void  SPDXSetTri( OBJ *Object, INT Index, TRI *Triangle )
{
   FUNC("SPDXSetTri");
   Assert(Object);
   Assert( Triangle );

   if (Index < Object->TriList.Count && Index >= 0)
      Object->TriList.Tris[Index] = *Triangle;
}

/*----------------------------------------------------------------------------*/

TRI   *SPDXGetTri( OBJ *Object, INT Index )
{
   FUNC("SPDXGetTri");
   Assert(Object);

   if (Index < Object->TriList.Count && Index >= 0)
      return &Object->TriList.Tris[Index];
   else
      return NULL;
}

/*----------------------------------------------------------------------------
  -   [TRIS.C     ] - End Of File                                            -
  ----------------------------------------------------------------------------*/
