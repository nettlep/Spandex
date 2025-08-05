// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [IMAGE.C     ] - IMAGE handling routines                               ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <io.h>
#include <time.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <graph.h>
#include <time.h>

#include "spandex.h"

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXGetFileType( CHR *Name )
{
CHR   TempStr[10];

   FUNC("SPDXGetFileType");
   Assert( Name );

   if (strlen( Name ) < 5)
      return IMAGETYPE_UNKNOWN;

   strcpy( TempStr, &Name[strlen(Name)-4] );

   if (!stricmp( TempStr, ".IMG" ))
      return IMAGETYPE_IMG;

   if (!stricmp( TempStr, ".GIF" ))
      return IMAGETYPE_GIF;

   if (!stricmp( TempStr, ".G24" ))
      return IMAGETYPE_G24;

   if (!stricmp( TempStr, ".TGA" ))
      return IMAGETYPE_TGA;

   if (!stricmp( TempStr, ".BMP" ))
      return IMAGETYPE_BMP;

   if (!stricmp( TempStr, ".IPI" ))
      return IMAGETYPE_IPI;

   if (!stricmp( TempStr, ".PCX" ))
      return IMAGETYPE_PCX;

   return IMAGETYPE_UNKNOWN;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

IMAGE *SPDXCreateImage( INT XRes, INT YRes )
{
IMAGE *Temp;

   FUNC("SPDXCreateImage");
   Temp = (IMAGE *) SPDXMalloc(sizeof(IMAGE));

   if (!Temp)
      return 0;

   Temp->Buffer = (CHR *) SPDXMalloc( XRes * YRes * 3 );

   if (!Temp->Buffer)
      return 0;

   Temp->ResX = XRes;
   Temp->ResY = YRes;
   Temp->Saved = 0;

   return Temp;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXDestroyImage( IMAGE **Image )
{
   FUNC("SPDXDestroyImage");
   Assert( Image );

   SPDXFree( (*Image)->Buffer );
   SPDXFree( *Image );
   *Image = 0;

   return;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [IMAGE.C     ] - End Of File                                           ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
