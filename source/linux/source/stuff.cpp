// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [STUFF.C     ] - Routines for handling .STF files                      -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#include "spandex.h"

/*----------------------------------------------------------------------------*/

INT   SPDXSelectSTF( CHR *FileName, STFP *StuffPack )
{
INT   Handle, Length, Offset;

   FUNC("SPDXSelectSTF");
   Assert( FileName );
   Assert( StuffPack );

   Handle = SPDXOpen( FileName, O_BINARY|O_RDONLY, 0 );

   if (Handle < 0)
   {
      char disp[80];
      sprintf(disp, "Unable to open file %s", FileName);
      SPDXLogError(LE_NOSTFOPEN, disp);
      return LE_NOSTFOPEN;
   }

   Offset = SPDXSeek( Handle, 0L, SEEK_END );
   SPDXSeek( Handle, Offset-4, SEEK_SET );

   if (SPDXRead( Handle, (CHR *) &Length, 4 ) != 4)
   {
      SPDXClose( Handle );
      return LE_NOSTFREAD;
   }

   Offset = SPDXSeek( Handle, Offset-Length, SEEK_SET );

   StuffPack->Handle = Handle;
   StuffPack->FileStart = Offset;

   return LE_NONE;
}

/*----------------------------------------------------------------------------*/

void  SPDXReleaseSTF( STFP *StuffPack )
{
   FUNC("SPDXReleaseSTF");
   Assert( StuffPack );

   SPDXClose( StuffPack->Handle );
   StuffPack->Handle = 0;
   StuffPack->FileStart = -1;
}

/*----------------------------------------------------------------------------*/

INT   SPDXOpenSTF( CHR *FileName, STFP *StuffPack )
{
INT   i, Count = 0;
STFH  TempHeader;

   FUNC("SPDXOpenSTF");
   Assert( FileName );
   Assert( StuffPack );

   SPDXSeek( StuffPack->Handle, StuffPack->FileStart, SEEK_SET );

   if (SPDXRead( StuffPack->Handle, (CHR *) &Count, 2 ) != 2)
      return -1;

   for (i = 0; i < Count; i++)
   {
      if (SPDXRead( StuffPack->Handle, (CHR *) &TempHeader, sizeof(STFH) ) != sizeof(STFH))
         return -1;

      if (!strcasecmp( FileName, TempHeader.Name ))
      {
         SPDXSeek( StuffPack->Handle, StuffPack->FileStart + TempHeader.Offset, SEEK_SET );
         return StuffPack->Handle;
      }
   }

   return -1;
}

/*----------------------------------------------------------------------------*/

void  SPDXCloseSTF( INT Handle )
{
   FUNC("SPDXCloseSTF");
   Handle = Handle; // Supress compiler warnings.
   return;
}

/*----------------------------------------------------------------------------
  -   [STUFF.C     ] - End Of File                                           -
  ----------------------------------------------------------------------------*/
