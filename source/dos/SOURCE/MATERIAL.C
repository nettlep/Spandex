// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [MATERIAL.C  ] - Material-based routines                               ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#include <mem.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include <math.h>

#include "spandex.h"

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXInitMaterials( )
{
   FUNC("SPDXInitMaterials");
   SPDXCleanupMaterials();
   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXAddMaterial( CHR *Name )
{
INT   MatCount;
MTRL  *pTemp;

   FUNC("SPDXAddMaterial");

   MatCount = SPDXGlobals.MatCount++;

   if (SPDXGlobals.MatList)
      pTemp = (MTRL *) SPDXRealloc(SPDXGlobals.MatList, sizeof(MTRL) * (MatCount+1) );
   else
      pTemp = (MTRL *) SPDXMalloc(sizeof(MTRL));

   if (!pTemp)
   {
      SPDXLogError(LE_NOMEM, "Not enough memory for new material" );
      return LE_NOMEM;
   }

   SPDXGlobals.MatList = pTemp;

   pTemp = &pTemp[MatCount];

   strcpy( pTemp->Name, Name );
   pTemp->Ambient = 0.0;
   pTemp->BumpMap = NULL;
   pTemp->Map = NULL;
   pTemp->Color = 0;
   pTemp->Surface = 0;
   pTemp->Transparency = 0;
   pTemp->Ka = 1.0;
   pTemp->Kd = 1.0;
   pTemp->Ks = 1.0;
   pTemp->Shininess = 1.0;

   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXAssignTriMaterial( CHR *MatName, OBJ *Object, INT Index, P2D *Map1, P2D *Map2, P2D *Map3 )
{
TRI   *Tri;
CHR   TempStr[90];
MTRL  *Mat;
BMAP  *Map;

   FUNC("SPDXAssignTriMaterial");
   Assert(CHECK_INIT);
   Assert( MatName );
   Assert( Object );
   Assert( Map1 );
   Assert( Map2 );
   Assert( Map3 );

   Tri = &Object->TriList.Tris[Index];

   Mat = SPDXFindMaterialByName( MatName );
   Map = Mat->Map;

   if (Mat)
   {
      Tri->Material = Mat;
      Tri->M1.x = Map1->x * (Map->XRes-1);
      Tri->M1.y = Map1->y * (Map->YRes-1);
      Tri->M2.x = Map2->x * (Map->XRes-1);
      Tri->M2.y = Map2->y * (Map->YRes-1);
      Tri->M3.x = Map3->x * (Map->XRes-1);
      Tri->M3.y = Map3->y * (Map->YRes-1);

      Tri->M1.fx = FLT_TO_FXD(Tri->M1.x);
      Tri->M1.fy = FLT_TO_FXD(Tri->M1.y);
      Tri->M2.fx = FLT_TO_FXD(Tri->M2.x);
      Tri->M2.fy = FLT_TO_FXD(Tri->M2.y);
      Tri->M3.fx = FLT_TO_FXD(Tri->M3.x);
      Tri->M3.fy = FLT_TO_FXD(Tri->M3.y);

      Tri->V1->Material = Mat;
      Tri->V2->Material = Mat;
      Tri->V3->Material = Mat;

      return LE_NONE;
   }

   sprintf( TempStr, "Unable to locate material based on name \"%s\"", MatName );
   SPDXLogError( LE_MATNOTFOUND, TempStr );
   return LE_MATNOTFOUND;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXSetMaterialName( MTRL *Material, CHR *Name )
{
   FUNC("SPDXSetMaterialName");
   Assert( Material );
   Assert( Name );

   strcpy( Material->Name, Name );
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

CHR   *SPDXGetMaterialName( MTRL *Material )
{
   FUNC("SPDXGetMaterialName");
   Assert( Material );

   return Material->Name;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXSetMaterialTexture( MTRL *Material, CHR *Name )
{
BMAP  *bmTemp;

   FUNC("SPDXSetMaterialTexture");
   Assert( Material );
   Assert( Name );

   bmTemp = SPDXFindMapByName(Name);

   if (!bmTemp)
      return LE_NOMAPFILE;

   Material->Map = bmTemp;
   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

BMAP  *SPDXGetMaterialTexture( MTRL *Material )
{
   FUNC("SPDXGetMaterialTexture");
   Assert( Material );

   return Material->Map;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXSetMaterialSurface( MTRL *Material, BYT Surface )
{
   FUNC("SPDXSetMaterialSurface");
   Assert( Material );

   Material->Surface = Surface;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

BYT   SPDXGetMaterialSurface( MTRL *Material )
{
   FUNC("SPDXGetMaterialSurface");
   Assert( Material );

   return Material->Surface;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXSetMaterialBumpMap( MTRL *Material, CHR *Name )
{
BMAP  *bmTemp;

   FUNC("SPDXSetMaterialBumpMap");
   Assert( Material );
   Assert( Name );

   bmTemp = SPDXFindMapByName(Name);

   if (!bmTemp)
      return LE_NOMAPFILE;

   Material->BumpMap = bmTemp;
   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

BMAP  *SPDXGetMaterialBumpMap( MTRL *Material )
{
   FUNC("SPDXGetMaterialBumpMap");
   Assert( Material );

   return Material->BumpMap;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXSetMaterialTransparency( MTRL *Material, BYT Transparency )
{
   FUNC("SPDXSetMaterialTransparency");
   Assert( Material );

   Material->Transparency = Transparency;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

BYT   SPDXGetMaterialTransparency( MTRL *Material )
{
   FUNC("SPDXGetMaterialTransparency");
   Assert( Material );

   return Material->Transparency;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXSetMaterialAmbientReflectCoefficient( MTRL *Material, FLT Ka )
{
   FUNC("SPDXSetMaterialAmbientReflectCoefficient");
   Assert( Material );

   Material->Ka = Ka;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

FLT   SPDXGetMaterialAmbientReflectCoefficient( MTRL *Material )
{
   FUNC("SPDXGetMaterialAmbientReflectCoefficient");
   Assert( Material );

   return Material->Ka;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXSetMaterialDiffuseReflectCoefficient( MTRL *Material, FLT Kd )
{
   FUNC("SPDXSetMaterialDiffuseReflectCoefficient");
   Assert( Material );

   Material->Kd = Kd;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

FLT   SPDXGetMaterialDiffuseReflectCoefficient( MTRL *Material )
{
   FUNC("SPDXGetMaterialDiffuseReflectCoefficient");
   Assert( Material );

   return Material->Kd;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXSetMaterialSpecularReflectCoefficient( MTRL *Material, FLT Ks )
{
   FUNC("SPDXSetMaterialSpecularReflectCoefficient");
   Assert( Material );

   Material->Ks = Ks;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

FLT   SPDXGetMaterialSpecularReflectCoefficient( MTRL *Material )
{
   FUNC("SPDXGetMaterialSpecularReflectCoefficient");
   Assert( Material );

   return Material->Ks;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXSetMaterialShineCoefficient( MTRL *Material, FLT Shininess )
{
   FUNC("SPDXSetMaterialShineCoefficient");
   Assert( Material );

   Material->Shininess = Shininess;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

FLT   SPDXGetMaterialShineCoefficient( MTRL *Material )
{
   FUNC("SPDXGetMaterialShineCoefficient");
   Assert( Material );

   return Material->Shininess;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXSetMaterialColor( MTRL *Material, RGB *Color, FLT Intensity, PAL *Palette )
{
INT   i;
PAL   NewPalette;
CHR   *Map;

   FUNC("SPDXSetMaterialColor");
   Assert( Material );
   Assert( Color );
   Assert( Palette );

   Map = &SPDXGlobals.PalTab[FLT_TO_INT(Intensity*(FLT)(MAP_SHADES-1)) * MAP_COLORS];

   for( i = 0; i < 256; i++)
   {
      NewPalette.Colors[i].Red   = Palette->Colors[Map[i]].Red;
      NewPalette.Colors[i].Green = Palette->Colors[Map[i]].Green;
      NewPalette.Colors[i].Blue  = Palette->Colors[Map[i]].Blue;
   }

   Material->Color = SPDXGetClosestColor((UINT)Color->Red,
                                         (UINT)Color->Green,
                                         (UINT)Color->Blue,
                                         &NewPalette);
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

UBYT  SPDXGetMaterialColor( MTRL *Material )
{
   FUNC("SPDXGetMaterialColor");
   Assert( Material );

   return Material->Color;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXSetMaterialAmbientLight( MTRL *Material, RGB *Ambient )
{
   FUNC("SPDXSetMaterialAmbientLight");
   Assert( Material );
   Assert( Ambient );

   Material->Ambient = (FLT) (MAX(Ambient->Red, MAX(Ambient->Green, Ambient->Blue))) / 255.0;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

FLT   SPDXGetMaterialAmbientLight( MTRL *Material )
{
   FUNC("SPDXGetMaterialAmbientLight");
   Assert( Material );

   return Material->Ambient;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

MTRL  *SPDXFindMaterialByName( CHR *Name )
{
INT   i;

   FUNC("SPDXFindMaterialByName");
   Assert( Name );

   for( i = 0; i < SPDXGlobals.MatCount; i++)
   {
      if (!stricmp(SPDXGlobals.MatList[i].Name, Name))
         return &SPDXGlobals.MatList[i];
   }

   return NULL;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXUpdateMaterialsLighting()
{
INT   i;
MTRL  *Material;

   for( i = 0; i < SPDXGlobals.MatCount; i++ )
   {
      Material = &SPDXGlobals.MatList[i];
      Material->_AM = Material->Ambient * Material->Ka;
      Material->_IR = ((MAP_SHADES-1.0) * Material->Shininess) +
                      ((1.0-Material->Shininess) *
                       Material->Ks) * (MAP_SHADES-1.0);
   }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXCleanupMaterials()
{
   FUNC("SPDXCleanupMaterials");

   if (SPDXGlobals.MatList)
   {
      SPDXFree(SPDXGlobals.MatList);
      SPDXGlobals.MatList = NULL;
   }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [MATERIAL.C  ] - End Of File                                           ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
