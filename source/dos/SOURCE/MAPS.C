// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [MAPS.C      ] - Map-based routines                                    ±
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

static   INT   StoreMapLine( CHR *Pixels, INT LineLength, CHR *Buffer, INT ImgWidth, INT Y );

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXInitMaps( )
{
   FUNC("SPDXInitMaps");
   SPDXCleanupMaps( );

   SPDXGlobals.PalTab = (CHR *) SPDXMalloc( MAP_COLORS * MAP_SHADES );

   if (!SPDXGlobals.PalTab)
   {
      SPDXLogError( LE_NOMEM, "Unable to allocate memory for PalTab" );
      return LE_NOMEM;
   }

   SPDXGlobals.TransTab = (CHR *) SPDXMalloc( MAP_COLORS * MAP_COLORS );

   if (!SPDXGlobals.TransTab)
   {
      SPDXLogError( LE_NOMEM, "Unable to allocate memory for TransTab" );
      return LE_NOMEM;
   }

   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXSelectRawMapFile( CHR *MapFileName, PAL *MapPalette )
{
INT   RetCode, Handle, Offset, i, j;
UWRD  XRes, YRes;
WRD   MapCount, ColorCount, ShadeCount;
CHR   *TempPtr, *pTemp, Name[DEF_NAME_LEN+1];

   FUNC("SPDXSelectRawMapFile");
   Assert(CHECK_INIT);
   Assert( MapFileName );
   Assert( MapPalette );
   Assert( SPDXGlobals.PalTab );
   Assert( SPDXGlobals.TransTab );

   Handle = SPDXOpen( MapFileName, O_BINARY|O_RDONLY, 0 );

   if (Handle <= 0)
      return LE_NOMAPFILE;

   if (SPDXRead( Handle, &SPDXGlobals.MapFlags, 4 ) != 4)
   {
      SPDXClose( Handle );
      return LE_NOMAPREAD;
   }

   if (SPDXGlobals.MapFlags & MAP_FLAGS_COMP)
   {
      SPDXClose( Handle );
      return LE_WRONGMAP;
   }

   if (SPDXRead( Handle, &MapCount, 2 ) != 2)
   {
      SPDXClose( Handle );
      return LE_NOMAPREAD;
   }

   if (SPDXRead( Handle, &ColorCount, 2 ) != 2)
   {
      SPDXClose( Handle );
      return LE_NOMAPREAD;
   }

   if (SPDXRead( Handle, &ShadeCount, 2 ) != 2)
   {
      SPDXClose( Handle );
      return LE_NOMAPREAD;
   }

   for( i = 0; i < MapCount; i++)
   {
      if (SPDXSeek( Handle, 10+i*(DEF_NAME_LEN+4), SEEK_SET ) != 10+i*(DEF_NAME_LEN+4))
      {
         SPDXClose( Handle );
         return LE_NOMAPSEEK;
      }

      if (SPDXRead( Handle, Name, DEF_NAME_LEN ) != DEF_NAME_LEN )
      {
         SPDXClose( Handle );
         return LE_NOMAPREAD;
      }

      if (SPDXRead( Handle, &Offset, sizeof(Offset) ) != sizeof(Offset) )
      {
         SPDXClose( Handle );
         return LE_NOMAPREAD;
      }

      if (SPDXSeek( Handle, Offset, SEEK_SET ) != Offset )
      {
         SPDXClose( Handle );
         return LE_NOMAPSEEK;
      }

      if (SPDXRead( Handle, &XRes, 2 ) != 2)
      {
         SPDXClose( Handle );
         return LE_NOMAPREAD;
      }

      if (SPDXRead( Handle, &YRes, 2 ) != 2)
      {
         SPDXClose( Handle );
         return LE_NOMAPREAD;
      }

      pTemp = (CHR *) SPDXMalloc( XRes*YRes );

      if (!pTemp)
      {
         SPDXClose( Handle );
         return LE_NOMEM;
      }

      if (SPDXRead( Handle, pTemp, XRes * YRes ) != XRes * YRes)
      {
         SPDXClose( Handle );
         return LE_NOMAPREAD;
      }

      RetCode = SPDXAddMap( Name, (UBYT *) pTemp, XRes, YRes );

      if (RetCode != LE_NONE)
         return RetCode;
   }

   if (SPDXSeek( Handle, MapCount*(DEF_NAME_LEN+4)+10, SEEK_SET ) != MapCount*(DEF_NAME_LEN+4)+10 )
   {
      SPDXClose( Handle );
      return LE_NOMAPSEEK;
   }

   if (SPDXRead( Handle, MapPalette, MAP_COLORS*sizeof(RGB)) != MAP_COLORS*sizeof(RGB) )
   {
      SPDXClose( Handle );
      return LE_NOMAPREAD;
   }

   TempPtr = (CHR *) SPDXMalloc( MAP_COLORS * MAP_SHADES );

   if (!TempPtr)
   {
      SPDXClose( Handle );
      return LE_NOMEM;
   }

   if (SPDXRead( Handle, TempPtr, MAP_COLORS*MAP_SHADES ) != MAP_COLORS*MAP_SHADES )
   {
      SPDXClose( Handle );
      return LE_NOMAPREAD;
   }

   // Now re-organize the map palette
   Offset = MAP_SHADES/MAP_SHADES;

   for( i = 0; i < MAP_SHADES; i++ )
      for ( j = 0; j < MAP_COLORS; j++ )
         SPDXGlobals.PalTab[i*MAP_COLORS+j] = TempPtr[j*MAP_SHADES+i];

   SPDXFree( TempPtr );

   if (SPDXGlobals.MapFlags & MAP_FLAGS_TRANS)
      if (SPDXRead( Handle, SPDXGlobals.TransTab, MAP_COLORS*MAP_COLORS ) != MAP_COLORS*MAP_COLORS )
      {
         SPDXClose( Handle );
         return LE_NOMAPREAD;
      }

   SPDXClose( Handle );
   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXSelectCompressedMapFile( PAL *MapPalette, INT Handle )
{
INT   i, j, RetCode, NextOffset, SavePos;
WRD   XRes, YRes;
UWRD  MapCount, ColorCount, ShadeCount;
CHR   *TempPtr, *pTemp, Name[DEF_NAME_LEN+1];

   FUNC("SPDXSelectCompressedMapFile");
   Assert(CHECK_INIT);
   Assert( MapPalette );
   Assert( Handle );
   Assert( SPDXGlobals.PalTab );
   Assert( SPDXGlobals.TransTab );

   if (SPDXRead( Handle, &SPDXGlobals.MapFlags, 4 ) != 4)
      return LE_NOMAPREAD;

   if (!(SPDXGlobals.MapFlags & MAP_FLAGS_COMP))
   {
      SPDXClose( Handle );
      return LE_WRONGMAP;
   }

   if (SPDXRead( Handle, (void *) &MapCount, 2 ) != 2)
      return LE_NOMAPREAD;

   if (SPDXRead( Handle, (void *) &ColorCount, 2 ) != 2)
      return LE_NOMAPREAD;

   if (SPDXRead( Handle, (void *) &ShadeCount, 2 ) != 2)
      return LE_NOMAPREAD;

   if (SPDXRead( Handle, (void *) MapPalette, MAP_COLORS*sizeof(RGB)) != MAP_COLORS*sizeof(RGB) )
      return LE_NOMAPREAD;

   TempPtr = (CHR *) SPDXMalloc( MAP_COLORS * MAP_SHADES );

   if (!TempPtr)
      return LE_NOMEM;

   if (SPDXRead( Handle, TempPtr, MAP_COLORS*MAP_SHADES ) != MAP_COLORS*MAP_SHADES )
      return LE_NOMAPREAD;

   // Now rotate the Map by 90 degrees...
   for( i = 0; i < MAP_SHADES; i++ )
      for ( j = 0; j < MAP_COLORS; j++ )
         SPDXGlobals.PalTab[i*MAP_COLORS+j] = TempPtr[j*MAP_SHADES+i];

   SPDXFree( TempPtr );

   if (SPDXGlobals.MapFlags & MAP_FLAGS_TRANS)
      if (SPDXRead( Handle, SPDXGlobals.TransTab, MAP_COLORS*MAP_COLORS ) != MAP_COLORS*MAP_COLORS )
         return LE_NOMAPREAD;

   for( i = 0; i < MapCount; i++)
   {
      if (SPDXRead( Handle, (void *) &XRes, 2 ) != 2)
         return LE_NOMAPREAD;

      if (SPDXRead( Handle, (void *) &YRes, 2 ) != 2)
         return LE_NOMAPREAD;

      if (SPDXRead( Handle, Name, DEF_NAME_LEN ) != DEF_NAME_LEN )
         return LE_NOMAPREAD;

      SavePos = SPDXTell(Handle);

      if (SPDXRead( Handle, (void *) &NextOffset, 4 ) != 4)
         return LE_NOMAPREAD;

      NextOffset += SavePos;

      pTemp = (CHR *) SPDXMalloc( XRes * YRes );

      if (!pTemp)
         return LE_NOMEM;

      RetCode = SPDXDecompress(Handle, XRes, pTemp, XRes, StoreMapLine );

      if (RetCode != LE_NONE)
         return RetCode;
      
      RetCode = SPDXAddMap( Name, (UBYT *) pTemp, XRes, YRes );

      if (RetCode != LE_NONE)
         return RetCode;

      SPDXSeek( Handle, NextOffset, SEEK_SET );
   }

   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXSelectRawBumFile( CHR *BumFileName )
{
INT   RetCode, Handle, Offset, i;
UWRD  XRes, YRes;
WRD   BumCount;
CHR   *pTemp, Name[DEF_NAME_LEN+1];

   FUNC("SPDXSelectRawBumFile");
   Assert(CHECK_INIT);
   Assert( BumFileName );
   Assert( SPDXGlobals.PalTab );
   Assert( SPDXGlobals.TransTab );

   Handle = SPDXOpen( BumFileName, O_BINARY|O_RDONLY, 0 );

   if (Handle <= 0)
      return LE_NOMAPFILE;

   if (SPDXRead( Handle, &BumCount, 2 ) != 2)
   {
      SPDXClose( Handle );
      return LE_NOMAPREAD;
   }

   for( i = 0; i < BumCount; i++)
   {
      if (SPDXSeek( Handle, 2+i*(DEF_NAME_LEN+4), SEEK_SET ) != 2+i*(DEF_NAME_LEN+4))
      {
         SPDXClose( Handle );
         return LE_NOMAPSEEK;
      }

      if (SPDXRead( Handle, Name, DEF_NAME_LEN ) != DEF_NAME_LEN )
      {
         SPDXClose( Handle );
         return LE_NOMAPREAD;
      }

      if (SPDXRead( Handle, &Offset, sizeof(Offset) ) != sizeof(Offset) )
      {
         SPDXClose( Handle );
         return LE_NOMAPREAD;
      }

      if (SPDXSeek( Handle, Offset, SEEK_SET ) != Offset )
      {
         SPDXClose( Handle );
         return LE_NOMAPSEEK;
      }

      if (SPDXRead( Handle, &XRes, 2 ) != 2)
      {
         SPDXClose( Handle );
         return LE_NOMAPREAD;
      }

      if (SPDXRead( Handle, &YRes, 2 ) != 2)
      {
         SPDXClose( Handle );
         return LE_NOMAPREAD;
      }

      pTemp = (CHR *) SPDXMalloc( XRes*YRes );

      if (!pTemp)
      {
         SPDXClose( Handle );
         return LE_NOMEM;
      }

      if (SPDXRead( Handle, pTemp, XRes * YRes ) != XRes * YRes)
      {
         SPDXClose( Handle );
         return LE_NOMAPREAD;
      }

      RetCode = SPDXAddMap( Name, (UBYT *) pTemp, XRes, YRes );

      if (RetCode != LE_NONE)
         return RetCode;
   }

   SPDXClose( Handle );
   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXSelectCompressedBumFile( INT Handle )
{
INT   i, RetCode, NextOffset, SavePos;
WRD   XRes, YRes;
UWRD  BumCount;
CHR   *pTemp, Name[DEF_NAME_LEN+1];

   FUNC("SPDXSelectCompressedBumFile");
   Assert(CHECK_INIT);
   Assert( Handle );
   Assert( SPDXGlobals.PalTab );
   Assert( SPDXGlobals.TransTab );

   if (SPDXRead( Handle, (void *) &BumCount, 2 ) != 2)
      return LE_NOMAPREAD;

   for( i = 0; i < BumCount; i++)
   {
      if (SPDXRead( Handle, (void *) &XRes, 2 ) != 2)
         return LE_NOMAPREAD;

      if (SPDXRead( Handle, (void *) &YRes, 2 ) != 2)
         return LE_NOMAPREAD;

      if (SPDXRead( Handle, Name, DEF_NAME_LEN ) != DEF_NAME_LEN )
         return LE_NOMAPREAD;

      SavePos = SPDXTell(Handle);

      if (SPDXRead( Handle, (void *) &NextOffset, 4 ) != 4)
         return LE_NOMAPREAD;

      NextOffset += SavePos;

      pTemp = (CHR *) SPDXMalloc( XRes * YRes );

      if (!pTemp)
         return LE_NOMEM;

      RetCode = SPDXDecompress(Handle, XRes, pTemp, XRes, StoreMapLine );

      if (RetCode != LE_NONE)
         return RetCode;
      
      RetCode = SPDXAddMap( Name, (UBYT *) pTemp, XRes, YRes );

      if (RetCode != LE_NONE)
         return RetCode;

      SPDXSeek( Handle, NextOffset, SEEK_SET );
   }

   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXAddMap(CHR *Name, UBYT *Data, INT XRes, INT YRes )
{
INT   MapCount, i;
WRD   UMask, VMask;
CHR   TempStr[90];
BMAP  *pTemp;

   FUNC("SPDXAddMap");
   Assert( Name );
   Assert( Data );
   Assert( XRes > 0 );
   Assert( YRes > 0 );

   MapCount = SPDXGlobals.MapCount++;

   if (SPDXGlobals.MapList)
      pTemp = (BMAP *) SPDXRealloc(SPDXGlobals.MapList, sizeof(BMAP) * (MapCount+1) );
   else
      pTemp = (BMAP *) SPDXMalloc(sizeof(BMAP));

   if (!pTemp)
   {
      SPDXLogError(LE_NOMEM, "Not enough memory for Map data" );
      return LE_NOMEM;
   }

   SPDXGlobals.MapList = pTemp;

   SPDXGlobals.MapList[MapCount].STab = (INT *) SPDXMalloc( YRes * sizeof(INT) );

   if (!SPDXGlobals.MapList[MapCount].STab)
   {   
      SPDXLogError(LE_NOMEM, "Not enough memory for Map STab" );
      return LE_NOMEM;
   }

   strcpy( SPDXGlobals.MapList[MapCount].Name, Name );
   SPDXGlobals.MapList[MapCount].XRes  = (WRD) XRes;
   SPDXGlobals.MapList[MapCount].YRes  = (WRD) YRes;
   SPDXGlobals.MapList[MapCount].Data  = Data;
   SPDXGlobals.MapList[MapCount].UMask = (WRD) XRes - 1;
   SPDXGlobals.MapList[MapCount].VMask = (WRD) YRes - 1;

   UMask = SPDXGlobals.MapList[MapCount].UMask;
   VMask = SPDXGlobals.MapList[MapCount].VMask;

   // Verify the integrity of the Masks

//   if (UMask != 0x0000 && UMask != 0x0001 && UMask != 0x0003 && UMask != 0x0007 && UMask != 0x000E &&
//       UMask != 0x000F && UMask != 0x001F && UMask != 0x003F && UMask != 0x007F && UMask != 0x00EF &&
//       UMask != 0x00FF && UMask != 0x01FF && UMask != 0x03FF && UMask != 0x07FF && UMask != 0x0EFF &&
//       UMask != 0x0FFF && UMask != 0x1FFF && UMask != 0x3FFF && UMask != 0x7FFF && UMask != 0xEFFF)
//   {
//      sprintf( TempStr, "Texture %s has an Invalid Width:  %dx%d", Name, XRes, YRes );
//      SPDXLogError( LE_RANGE, TempStr );
//   }
//
//   if (VMask != 0x0000 && VMask != 0x0001 && VMask != 0x0003 && VMask != 0x0007 && VMask != 0x000E &&
//       VMask != 0x000F && VMask != 0x001F && VMask != 0x003F && VMask != 0x007F && VMask != 0x00EF &&
//       VMask != 0x00FF && VMask != 0x01FF && VMask != 0x03FF && VMask != 0x07FF && VMask != 0x0EFF &&
//       VMask != 0x0FFF && VMask != 0x1FFF && VMask != 0x3FFF && VMask != 0x7FFF && VMask != 0xEFFF)
//   {
//      sprintf( TempStr, "Texture %s has an Invalid Height:  %dx%d", Name, XRes, YRes );
//      SPDXLogError( LE_RANGE, TempStr );
//   }

   if (UMask != 0x00FF)
   {
      sprintf( TempStr, "Texture %s has an Invalid Width:  %dx%d", Name, XRes, YRes );
      SPDXLogError( LE_RANGE, TempStr );
   }

   if (VMask != 0x00FF)
   {
      sprintf( TempStr, "Texture %s has an Invalid Height:  %dx%d", Name, XRes, YRes );
      SPDXLogError( LE_RANGE, TempStr );
   }

   for( i = 0; i < YRes; i++ )
      SPDXGlobals.MapList[MapCount].STab[i] = (INT) &SPDXGlobals.MapList[MapCount].Data[i * XRes];

   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   StoreMapLine( CHR *Pixels, INT LineLength, CHR *Buffer, INT ImgWidth, INT Y )
{
   FUNC("StoreMapLine");
   Assert( Pixels );
   Assert( Buffer );

   SPDXMemCopyBYTE( &Buffer[Y * ImgWidth], Pixels, ImgWidth );
   return LineLength * 3;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXCleanupMaps()
{
INT   i;

   FUNC("SPDXCleanupMaps");

   if (SPDXGlobals.PalTab)
   {
      SPDXFree(SPDXGlobals.PalTab);
      SPDXGlobals.PalTab = NULL;
   }

   if (SPDXGlobals.TransTab)
   {
      SPDXFree(SPDXGlobals.TransTab);
      SPDXGlobals.TransTab = NULL;
   }

   if (SPDXGlobals.MatList)
   {
      SPDXFree(SPDXGlobals.MatList);
      SPDXGlobals.MatList = NULL;
   }

   if (SPDXGlobals.MapList)
   {
      for( i = 0; i < SPDXGlobals.MapCount; i++ )
      {
         if (SPDXGlobals.MapList[i].STab)
         {
            SPDXFree(SPDXGlobals.MapList[i].STab);
            SPDXGlobals.MapList[i].STab = NULL;
         }

         if (SPDXGlobals.MapList[i].Data)
         {
            SPDXFree(SPDXGlobals.MapList[i].Data);
            SPDXGlobals.MapList[i].Data = NULL;
         }
      }

      SPDXFree(SPDXGlobals.MapList);
      SPDXGlobals.MapList = NULL;
      SPDXGlobals.MapCount = 0;
   }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

BMAP  *SPDXFindMapByName( CHR *Name )
{
INT   i;

   FUNC("SPDXFindMapByName");
   Assert( Name );

   for( i = 0; i < SPDXGlobals.MapCount; i++)
   {
      if (!stricmp(SPDXGlobals.MapList[i].Name, Name))
         return &SPDXGlobals.MapList[i];
   }

   return NULL;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [MAPS.C      ] - End Of File                                           ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
