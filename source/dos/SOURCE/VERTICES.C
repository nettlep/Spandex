// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [VERTICES.C  ] - Vertex based routines                                 ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#include <mem.h>

#include "spandex.h"

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXAddObjectVertex(OBJ *Object)
{
INT   i, Diff;
TRI   *Tri;
VER   *VertexList;

   FUNC("SPDXAddObjectVertex");
   Assert( Object );

   if (!Object->VertexList.Count)
   {
      VertexList = (VER *) SPDXMalloc(sizeof(VER) );
   }
   else
   {
      VertexList = (VER *) SPDXRealloc(Object->VertexList.Vers,
                                       sizeof(VER) * (Object->VertexList.Count + 1));
   }

   if (!VertexList)
   {
      SPDXLogError( LE_NOMEM, "Unable to allocate polygon list" );
      return -1;
   }

   // The array moved.. we need to adjust all the polys in the object (ugh!)
   if (VertexList != Object->VertexList.Vers)
   {
      Diff = VertexList - Object->VertexList.Vers;
      Tri = Object->TriList.Tris;

      for( i = 0; i < Object->TriList.Count; i++ )
         Tri[i].V1 += Diff, Tri[i].V2 += Diff, Tri[i].V3 += Diff;

      for( i = 0; i < Object->PointList.Count; i++ )
         Object->PointList.Points[i] += Diff;
   }

   Object->VertexList.Vers = VertexList;

   // This returns the next index value (end + 1)
   return Object->VertexList.Count++;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXGetObjectVertexCount(OBJ *Object)
{
   FUNC("SPDXGetObjectVertexCount");
   Assert( Object );

   return Object->VertexList.Count;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXSetVertex( OBJ *Object, INT Index, VER *Vertex )
{
   FUNC("SPDXSetVertex");
   Assert(Object);
   Assert( Vertex );

   if (Index < Object->VertexList.Count && Index >= 0)
      Object->VertexList.Vers[Index] = *Vertex;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

VER   *SPDXGetVertex( OBJ *Object, INT Index )
{
   FUNC("SPDXGetVertex");
   Assert(Object);

   if (Index < Object->VertexList.Count && Index >= 0)
      return &Object->VertexList.Vers[Index];
   else
      return NULL;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [VERTICES.C  ] - End Of File                                           ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
