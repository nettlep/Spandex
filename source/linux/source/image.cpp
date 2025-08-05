// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [IMAGE.C     ] - IMAGE handling routines                               -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#include "spandex.h"

/*----------------------------------------------------------------------------*/

INT   SPDXGetFileType( CHR *Name )
{
CHR   TempStr[10];

   FUNC("SPDXGetFileType");
   Assert( Name );

   if (strlen( Name ) < 5)
      return IMAGETYPE_UNKNOWN;

   strcpy( TempStr, &Name[strlen(Name)-4] );

   if (!strcasecmp( TempStr, ".IMG" ))
      return IMAGETYPE_IMG;

   if (!strcasecmp( TempStr, ".GIF" ))
      return IMAGETYPE_GIF;

   if (!strcasecmp( TempStr, ".G24" ))
      return IMAGETYPE_G24;

   if (!strcasecmp( TempStr, ".TGA" ))
      return IMAGETYPE_TGA;

   if (!strcasecmp( TempStr, ".BMP" ))
      return IMAGETYPE_BMP;

   if (!strcasecmp( TempStr, ".IPI" ))
      return IMAGETYPE_IPI;

   if (!strcasecmp( TempStr, ".PCX" ))
      return IMAGETYPE_PCX;

   return IMAGETYPE_UNKNOWN;
}

/*----------------------------------------------------------------------------*/

IMAGE *SPDXCreateImage( INT XRes, INT YRes )
{
IMAGE *Temp;

   FUNC("SPDXCreateImage");
   Temp = (IMAGE *) SPDXMalloc(sizeof(IMAGE));

   if (!Temp)
      return 0;

   Temp->Buffer = (UCHR *) SPDXMalloc( XRes * YRes * 3 );

   if (!Temp->Buffer)
      return 0;

   Temp->ResX = XRes;
   Temp->ResY = YRes;
   Temp->Saved = 0;

   return Temp;
}

/*----------------------------------------------------------------------------*/

void  SPDXDestroyImage( IMAGE **Image )
{
   FUNC("SPDXDestroyImage");
   Assert( Image );

   SPDXFree( (*Image)->Buffer );
   SPDXFree( *Image );
   *Image = 0;

   return;
}

/*----------------------------------------------------------------------------
  -   [IMAGE.C     ] - End Of File                                           -
  ----------------------------------------------------------------------------*/
