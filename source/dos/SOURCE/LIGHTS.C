// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [LIGHTS.C    ] - Light based routines                                  ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#include <mem.h>

#include "spandex.h"

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

LGT   *SPDXAddLight(OBJ *Object)
{
LLS   *LightList;
LGT   *Light;

   FUNC("SPDXAddLight");
   Assert( Object );

   if (SPDXGlobals.LightCount > MAX_LIGHTS)
   {
      SPDXLogError( LE_RANGE, "Too many light sources" );
      return 0;
   }

   // Do it for the object
   LightList = &Object->LightList;

   if (!LightList->Count)
      Light = (LGT *) SPDXMalloc(sizeof(LGT));
   else
      Light = (LGT *) SPDXRealloc(LightList->Lgts, sizeof(LGT) * (LightList->Count+1));

   if (!Light)
   {
      SPDXLogError( LE_NOMEM, "Out of memory allocating a light list" );
      return 0;
   }

   LightList->Lgts = Light;

   Light = &LightList->Lgts[LightList->Count];
   SPDXMemSetBYTE(Light, 0, sizeof(LGT));

   LightList->Count++;

   return Light;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXGetLightCount( LLS *Light )
{
   FUNC("SPDXGetLightCount");
   Assert( Light );

   return( Light->Count );
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXSetLightName( LGT *Light, CHR *Name )
{
   FUNC("SPDXSetLightName");
   Assert( Light );
   Assert( Name );

   strcpy( Light->Name, Name );
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

CHR   *SPDXGetLightName( LGT *Light )
{
   FUNC("SPDXGetLightName");
   Assert( Light );

   return Light->Name;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXSetLightLocal( LGT *Light, P3D *Location )
{
   FUNC("SPDXSetLightLocal");
   Assert( Light );
   Assert( Location );

   Light->LLoc = *Location;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

P3D   *SPDXGetLightLocal( LGT *Light )
{
   FUNC("SPDXGetLightLocal");
   Assert( Light );

   return &Light->LLoc;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXSetLightIntensity( LGT *Light, RGB *Color )
{
   FUNC("SPDXSetLightIntensity");
   Assert( Light );
   Assert( Color );

   Light->Int = (FLT) (MAX(Color->Red, MAX(Color->Green, Color->Blue))) / 255.0;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

FLT   SPDXGetLightIntensity( LGT *Light )
{
   FUNC("SPDXGetLightIntensity");
   Assert( Light );

   return Light->Int;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXCleanupLights( )
{
   FUNC("SPDXCleanupLights");
   SPDXGlobals.LightCount = 0;

   SPDXMemSetDWORD( SPDXGlobals.LightList, 0, sizeof( SPDXGlobals.LightList ) >>2 );
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

LGT   *SPDXFindLightByName( CHR *Name, BYT CaseSense )
{
INT   i;

   FUNC("SPDXFindLightByName");
   Assert( Name );

   for(i = 0; i < SPDXGlobals.LightCount; i++ )
   {
      if (CaseSense == TRUE)
      {
         if (!strcmp( SPDXGlobals.LightList[i]->Name, Name ))
            return SPDXGlobals.LightList[i];
      }
      else
      {
         if (!stricmp( SPDXGlobals.LightList[i]->Name, Name ))
            return SPDXGlobals.LightList[i];
      }
   }

   return NULL;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

LGT   *SPDXFindLightByNameLen( CHR *Name, INT Len, BYT CaseSense )
{
INT   i;

   FUNC("SPDXFindLightByNameLen");
   Assert( Name );

   for(i = 0; i < SPDXGlobals.LightCount; i++ )
   {
      if (CaseSense == TRUE)
      {
         if (!strncmp( SPDXGlobals.LightList[i]->Name, Name, Len ))
            return SPDXGlobals.LightList[i];
      }
      else
      {
         if (!strnicmp( SPDXGlobals.LightList[i]->Name, Name, Len ))            return SPDXGlobals.LightList[i];
      }
   }

   return NULL;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXTranslateLight( LGT *Light, VEC *Vector )
{
P3D   *Location;

   FUNC("SPDXTranslateLight");
   Assert( Light );
   Assert( Vector );

   Location = SPDXGetLightLocal( Light );
   Location->x += Vector->dx;
   Location->y += Vector->dy;
   Location->z += Vector->dz;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [LIGHTS.C    ] - End Of File                                           ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
