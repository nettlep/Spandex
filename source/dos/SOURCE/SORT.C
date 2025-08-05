// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [SORT.C      ] - Sorting stuffs                                        ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#ifndef U_Z

#include "spandex.h"

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static   TRI   *tpTemp, **tppTemp;
static   TRI   *List0[256];
static   TRI   *List1[256];

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXInitByteSortStack( void )
{
   FUNC("SPDXInitByteSortStack");
   // Null out the lists...
   SPDXMemSetDWORD( List0, 0, sizeof(List0) >> 2 );
   SPDXMemSetDWORD( List1, 0, sizeof(List1) >> 2 );
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXUninitByteSortStack( void )
{
   FUNC("SPDXUninitByteSortStack");
   return;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXByteSortRenderList( RLS *RList )
{
INT   i, Index, TriCount;
TRI   **TriList;

   FUNC("SPDXByteSortRenderList");
   Assert( RList );

   // Init these
   TriList = RList->Tris;
   TriCount = RList->TriCount;

   // Sort all of them by their highest 8 bits into List0...
   for(i = 0; i < TriCount; i++, TriList++)
   {
      Index = ((*TriList)->MinZ >> 8) & 0xff;
      (*TriList)->Next = List0[Index];
      List0[Index] = *TriList;
   }

   TriList = RList->Tris;
   tppTemp = &List0[0];

   // Sort the List0 into List1 according to upper-middle 8 bits
   for( i = 0; i < 256; i++, tppTemp++ )
      while( *tppTemp )
      {
         Index = ((*tppTemp)->MinZ >> 16) & 0xff;
         tpTemp = (*tppTemp)->Next;
         (*tppTemp)->Next = List1[Index];
         List1[Index] = (*tppTemp);
         (*tppTemp) = tpTemp;
      }

   tppTemp = &List1[0];

   // Sort the List1 back into List0 according to lower-middle 8 bits
   for( i = 0; i < 256; i++, tppTemp++ )
      while( *tppTemp )
      {
         Index = ((*tppTemp)->MinZ >> 24) & 0xff;
         tpTemp = (*tppTemp)->Next;
         (*tppTemp)->Next = List0[Index];
         List0[Index] = *tppTemp;
         (*tppTemp) = tpTemp;
      }

   tppTemp = &List0[255];

   // Rebuild the render list from List0
   for( i = 0; i < 256; i++, tppTemp-- )
      while( *tppTemp )
      {
         *(TriList++) = *tppTemp;
         *tppTemp = (*tppTemp)->Next;
      }

}
#endif
/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [SORT.C      ] - End Of File                                           ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
