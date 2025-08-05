// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [POINTS.C    ] - Point based routines                                  -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#include "spandex.h"

/*----------------------------------------------------------------------------*/

INT   SPDXAddObjectPoint(OBJ *Object)
{
VER   **PointList;

   FUNC("SPDXAddObjectPoint");
   Assert( Object );

   if (!Object->PointList.Count)
   {
      PointList = (VER **) SPDXMalloc(sizeof(VER *));
   }
   else
   {
      PointList = (VER **) SPDXRealloc(Object->PointList.Points,
                                      sizeof(VER *) * (Object->PointList.Count + 1));
   }

   if (!PointList)
   {
      SPDXLogError( LE_NOMEM, "Unable to allocate point list" );
      return -1;
   }

   Object->PointList.Points = PointList;

   // This returns the next to the last index value
   return Object->PointList.Count++;
}

/*----------------------------------------------------------------------------*/

INT   SPDXGetPointCount( PLS *PList)
{
   FUNC("SPDXGetPointCount");
   Assert( PList );

   return PList->Count;
}

/*----------------------------------------------------------------------------*/

void  SPDXSetPointVertex( OBJ *Object, INT Index, VER *Ver )
{
   FUNC("SPDXSetPointVertex");
   Assert(Object);
   Assert(Ver);

   if (Index < Object->PointList.Count && Index >= 0)
      Object->PointList.Points[Index] = Ver;
}

/*----------------------------------------------------------------------------*/

VER   *SPDXGetPointVertex( OBJ *Object, INT Index)
{
   FUNC("SPDXGetPointVertex");
   Assert(Object);

   if (Index < Object->PointList.Count && Index >= 0)
      return Object->PointList.Points[Index];
   else
      return NULL;
}

/*----------------------------------------------------------------------------
  -   [POINTS.C    ] - End Of File                                           -
  ----------------------------------------------------------------------------*/
