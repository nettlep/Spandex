// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [GLOBALS.C   ] - Global variables routines                             -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#include "spandex.h"
#include "sqrttab.cpp"

/*----------------------------------------------------------------------------*/

GLOBS    SPDXGlobals;

/*----------------------------------------------------------------------------*/

// These globals are used for the FXD_TO_INT conversion

INT   _t_bign[2] = {0,0x42380000};
INT   _t_fxfl[2], _t_frac[2];
FLT   _t_half = 0.5;
INT   _t_fxdflt = 0x37800000;

/*----------------------------------------------------------------------------*/

static   void  BuildGlobalLightList( OBJ *Object );

/*----------------------------------------------------------------------------*/

INT   SPDXInitGlobals( )
{
   FUNC("SPDXInitGlobals");
   SPDXMemSetBYTE( &SPDXGlobals, 0, sizeof(SPDXGlobals) );

#if _OS_DOS
   SPDXGlobals.TempVESAInfo = (VESA *) SPDXDMalloc( sizeof(VESA) );

   if (!SPDXGlobals.TempVESAInfo)
   {
      SPDXLogError( LE_NOMEM, "Unable to allocate memory for TempVESAInfo structure" );
      return LE_NOMEM;
   }

   SPDXGlobals.TempSVGAInfo = (SVGA *) SPDXDMalloc( sizeof(SVGA) );

   if (!SPDXGlobals.TempSVGAInfo)
   {
      SPDXLogError( LE_NOMEM, "Unable to allocate memory for TempSVGAInfo structure" );
      return LE_NOMEM;
   }

   SPDXGlobals.DOSMemPalette = (PAL *) SPDXDMalloc( sizeof(PAL) );

   if (!SPDXGlobals.DOSMemPalette)
   {
      SPDXLogError( LE_NOMEM, "Unable to allocate memory for DOSMemPalette" );
      return LE_NOMEM;
   }

   SPDXGlobals.DOSMemGeneric = (CHR *) SPDXDMalloc( GENERIC_DOS_RAM_LEN );

   if (!SPDXGlobals.DOSMemGeneric)
   {
      SPDXLogError( LE_NOMEM, "Unable to allocate generic DOS memory" );
      return LE_NOMEM;
   }

   SPDXGlobals.Port3x4 = *(WRD*)MK_PROT(0x40,0x63);
#else
   SPDXGlobals.Port3x4 = 0;
#endif

   SPDXGlobals.IRQ_OkToCopy = 0;
   SPDXGlobals.UseIRQ = 0;

   SPDXGlobals.MapCount = 0;
   SPDXGlobals.MapList = 0;

   SPDXGlobals.BackgroundColor = 0;

   SPDXGlobals.NearZ = DEF_NEAR_Z;

   // Ignore any errors from this call
   SPDXSetDebugStateFromFile();

   return LE_NONE;
}

/*----------------------------------------------------------------------------*/

INT   SPDXInitTables()
{
INT   i, ix, iy, RetCode;
FLT   Theta;
DBL   dTempX, dTempY, dx, dy, Intensity;

   FUNC("SPDXInitTables");

   SPDXGlobals.LightList = (LGT **) SPDXMalloc( MAX_LIGHTS * sizeof(LGT *) );

   if (!SPDXGlobals.LightList)
   {
      SPDXLogError( LE_NOMEM, "Unable to allocate global light list" );
      return LE_NOMEM;
   }

   SPDXGlobals.SineTable = (FLT *) SPDXMalloc( (SIN_TAB_LEN+1) * sizeof(FLT) );

   if (!SPDXGlobals.SineTable)
   {
      SPDXLogError( LE_NOMEM, "Unable to allocate global special table 1" );
      return LE_NOMEM;
   }

   SPDXGlobals.CosTable = (FLT *) SPDXMalloc( (SIN_TAB_LEN+1) * sizeof(FLT) );

   if (!SPDXGlobals.CosTable)
   {
      SPDXLogError( LE_NOMEM, "Unable to allocate global special table 2" );
      return LE_NOMEM;
   }

   SPDXGlobals.PhongTab = (INT *) SPDXMalloc( PHONG_TAB_LEN * PHONG_TAB_LEN * sizeof(INT) );

   if (!SPDXGlobals.PhongTab)
   {
      SPDXLogError( LE_NOMEM, "Unable to allocate global special table 3" );
      return LE_NOMEM;
   }

   RetCode = SPDXInitMaps( );

   if (RetCode != LE_NONE)
      return RetCode;

   for( i = 0, Theta = 0.0; i < SIN_TAB_LEN; Theta += 6.283185307 / (FLT) SIN_TAB_LEN, i++)
   {
      SPDXGlobals.SineTable[i] = sin(Theta);
      SPDXGlobals.CosTable[i] = cos(Theta);
   }

   // Go one past the end
   SPDXGlobals.SineTable[i] = sin(Theta);
   SPDXGlobals.CosTable[i] = cos(Theta);

   for( iy = 0, dy = -1.0; dy < 1.0; dy+=2.0/(DBL) PHONG_TAB_LEN, iy++ )
   {
      dTempY = dy*dy;

      for (ix = 0, dx = -1.0; dx < 1.0; dx+=2.0/(DBL) PHONG_TAB_LEN, ix++)
      {
         dTempX = dx*dx;

         Intensity = 1.0 - dTempX - dTempY;

         if (Intensity > 0.0)
            Intensity = sqrt(Intensity) * (DBL) (MAP_SHADES-1);
         else
            Intensity = 0.0;

         SPDXGlobals.PhongTab[iy*PHONG_TAB_LEN+ix] = (INT) &SPDXGlobals.PalTab[FLT_TO_INT(Intensity) * MAP_COLORS];
      }
   }

   return LE_NONE;
}

/*----------------------------------------------------------------------------*/

void  SPDXReplacePhongMap( BMAP *Map )
{
INT   x, y;

   FUNC("SPDXReplacePhongMap");
   Assert( Map );

   for( y = 0; y < 256; y++ )
      for( x = 0; x < 256; x++ )
         SPDXGlobals.PhongTab[y*PHONG_TAB_LEN+x] = (INT) &SPDXGlobals.PalTab[(INT) (Map->Data[(y<<8)+x]>>2) * MAP_COLORS];
}

/*----------------------------------------------------------------------------*/

INT   SPDXSetDebugStateFromFile( )
{
INT   Handle, State = 0, i;
CHR   DebugInfo[32];

   FUNC("SPDXSetDebugStateFromFile");
   SPDXMemSetDWORD(DebugInfo, 0, 32>>2);

   Handle = open( SPANDEX_DEBUG_FILE, O_RDONLY|O_BINARY );

   if (Handle < 0)
      return LE_NOOPEN;

   read( Handle, DebugInfo, 32 );
   close(Handle);

   for( i = 0; i < 32; i++)
      if (DebugInfo[i] == '1')
         State += 1 << i;

   SPDXSetDebugState(State);

   return LE_NONE;
}

/*----------------------------------------------------------------------------*/

INT   SPDXGetDebugState( )
{
   FUNC("SPDXGetDebugState");
   return SPDXGlobals.DebugState;
}

/*----------------------------------------------------------------------------*/

void  SPDXSetDebugState( INT State )
{
CHR  TempStr[80];

   FUNC("SPDXSetDebugState");
   if (State != SPDXGlobals.DebugState)
   {
      SPDXGlobals.DebugState = State;
      sprintf( TempStr, "Debug state set to 0x%08X", (INT) SPDXGlobals.DebugState );
      SPDXLogString( TempStr );
   }
}

/*----------------------------------------------------------------------------*/

void  SPDXSetGlobalResolution(INT ResX, INT ResY )
{
   FUNC("SPDXSetGlobalResolution");
   SPDXGlobals.ScrResX = ResX;
   SPDXGlobals.ScrResY = ResY;
}

/*----------------------------------------------------------------------------*/

void  SPDXGetGlobalResolution(INT *ResX, INT *ResY )
{
   FUNC("SPDXGetGlobalResolution");
   Assert( ResX );
   Assert( ResY );

   *ResX = SPDXGlobals.ScrResX;
   *ResY = SPDXGlobals.ScrResY;
}

/*----------------------------------------------------------------------------*/

void  SPDXCleanupGlobals()
{
   FUNC("SPDXCleanupGlobals");
   SPDXCleanupMaps();
   SPDXCleanupMaterials();

   if (SPDXGlobals.LightList)
   {
      SPDXFree(SPDXGlobals.LightList);
      SPDXGlobals.LightList = NULL;
   }

   if (SPDXGlobals.SineTable)
   {
      SPDXFree(SPDXGlobals.SineTable);
      SPDXGlobals.SineTable = NULL;
   }

   if (SPDXGlobals.CosTable)
   {
      SPDXFree(SPDXGlobals.CosTable);
      SPDXGlobals.CosTable = NULL;
   }

   if (SPDXGlobals.PhongTab)
   {
      SPDXFree(SPDXGlobals.PhongTab);
      SPDXGlobals.PhongTab = NULL;
   }
}

/*----------------------------------------------------------------------------*/

void  SPDXRebuildGlobalLightList( OBJ *Universe )
{
   FUNC("SPDXRebuildGlobalLightList");
   Assert( Universe );

   SPDXCleanupLights();
   BuildGlobalLightList( Universe );
}

/*----------------------------------------------------------------------------*/

void  BuildGlobalLightList( OBJ *Object )
{
INT   i;

   FUNC("BuildGlobalLightList");
   Assert( Object );

   for( i = 0; i < Object->LightList.Count; i++ )
      SPDXGlobals.LightList[SPDXGlobals.LightCount++] = &Object->LightList.Lgts[i];

   for( i = 0; i < Object->Children.Count; i++ )
      BuildGlobalLightList( &Object->Children.Objs[i] );
}

/*----------------------------------------------------------------------------*/

void  SPDXSetCurrentVScreen( VSCR *VScreen )
{
   FUNC("SPDXSetCurrentVScreen");
   Assert( VScreen );

   SPDXGlobals.CurrentVScreen = *VScreen;
}

/*----------------------------------------------------------------------------*/

FLT   SPDXGetNearZ( )
{
   FUNC("SPDXGetNearZ");
   return SPDXGlobals.NearZ;
}

/*----------------------------------------------------------------------------*/

void  SPDXSetNearZ( FLT NearZ )
{
   FUNC("SPDXSetNearZ");
   SPDXGlobals.NearZ = MAX(NearZ, 0.0);
}

/*----------------------------------------------------------------------------*/

INT   SPDXGetBackgroundColor( )
{
   FUNC("SPDXGetBackgroundColor");
   return SPDXGlobals.BackgroundColor;
}

/*----------------------------------------------------------------------------*/

void  SPDXSetBackgroundColor( RGB *Color )
{
PAL   Palette;

   FUNC("SPDXSetBackgroundColor");

   SPDXGetPalette(0, TOTAL_COLORS, &Palette);

   SPDXGlobals.BackgroundColor = SPDXGetClosestColor(Color->Red,
                                                     Color->Blue,
                                                     Color->Green,
                                                     &Palette);
}

/*----------------------------------------------------------------------------
  -   [GLOBALS.C   ] - End Of File                                           -
  ----------------------------------------------------------------------------*/
