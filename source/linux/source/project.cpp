// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [PROJECT.C   ] - 2D projection routines                                -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#include "spandex.h"

/*----------------------------------------------------------------------------*/

void  SPDXProjectHierarchy( OBJ *Object, OBJ *Cam, VSCR *VScreen )
{
INT   i, Count;
OBJ   *Obj;

   FUNC("SPDXProjectHierarchy");
   Assert(CHECK_INIT);
   Assert( Object );
   Assert( Cam );
   Assert( VScreen );

   Count = Object->Children.Count;
   Obj   = Object->Children.Objs;

   // First, project my children
   for (i = 0; i < Count; i++, Obj++ )
      SPDXProjectHierarchy( Obj, Cam, VScreen );

   // Next, project myself
   SPDXCalcObjectProjection( Object, VScreen );

   return;
}
  
/*----------------------------------------------------------------------------*/

void  SPDXCalcObjectProjection( OBJ *Object, VSCR *VScreen )
{
INT   i;
INT   Count;
FLT   Relz = 0.0f;
VER   *Ver;
 
   FUNC("SPDXCalcObjectProjection");  
   Assert(CHECK_INIT);
   Assert( Object );
   Assert( VScreen );

   Count = Object->VertexList.Count;
   Ver   = Object->VertexList.Vers;

   for( i = 0; i < Count; i++, Ver++)
   {
      if (Ver->Visible == VIS_YES)
         SPDXCalcProjection(Relz, Ver, VScreen); // MACRO
   }

   return;
}
  
/*----------------------------------------------------------------------------*/

FLT   SPDXCalcFOV( FLT FocalLength )
{
   FUNC("SPDXCalcFOV");
   return (15.0 / FocalLength) * 160.0;
}

/*----------------------------------------------------------------------------
  -   [PROJECT.C   ] - End Of File                                           -
  ----------------------------------------------------------------------------*/


