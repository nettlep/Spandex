// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [CMAP.C      ] - Spandex Map file compresor                            -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include "spandex.h"

/*----------------------------------------------------------------------------*/

extern INT SPDXServicesOpened;

/*----------------------------------------------------------------------------*/

typedef struct   mapinfo
{
   CHR   Name[DEF_NAME_LEN];
   INT   Offset;
} MI;

/*----------------------------------------------------------------------------*/

static   void  MyRead( void *Buf, INT Count, INT Handle );
static   void  MyWrite( void *Buf, INT Count, INT Handle );
static   INT   NextByte( INT x, INT y );
static   void  PrintCopyright( void );

static   INT   CurOff, CurSize, CurMap;

/*----------------------------------------------------------------------------*/

int   main( int argc, char *argv[] )
{
INT   i, Handle, RetCode, CompressedLength, SavePos, MapFlags;
WRD   MapCount, ColorCount, ShadeCount;
CHR   *ColorTable, *TransTable, *FileName, Bits = 8;
MI    *MInfo;
PAL   MapPalette;
char _FUNCTION_[] = "cmap";

   setbuf( stdout, 0 );

   PrintCopyright();

   if (argc <= 1)
   {
      printf( "Usage: CMap <filename.MAP>\n" );
      return 1;
   }

   // Turn on the logging info
   SPDXSetFatalErrors( FALSE );
   SPDXSetLoggedErrors( FALSE );
   SPDXSetLoggedStrings( FALSE );
   SPDXServicesOpened = 1;

   if (SPDXInit( ) != LE_NONE)
   {
      printf( "  ERROR --> Unable to init Spandex\n" );
      return 1;
   }

   FileName = argv[1];

   printf( "Reading... %s", FileName );

   Handle = open( FileName, O_RDONLY );

   if (Handle <= 0)
   {
      printf( "\n\nERROR --> Unable to open file \"%s\"\n", FileName );
      return 1;
   }

   MyRead( &MapFlags,   4, Handle );
   MapFlags |= MAP_FLAGS_COMP;
   MyRead( &MapCount,   2, Handle );
   MyRead( &ColorCount, 2, Handle );
   MyRead( &ShadeCount, 2, Handle );

   SPDXGlobals.MapList = (BMAP *) SPDXMalloc( sizeof(BMAP)*MapCount );

   if (!SPDXGlobals.MapList)
   {
      printf( "\n\nERROR --> Unable to allocate RAM for global map list\n" );
      return 1;
   }

   MInfo = (MI *) SPDXMalloc( sizeof(MI) * MapCount );

   if (!MInfo)
   {
      printf( "\n\nERROR --> Unable to allocate RAM for map negotiation information\n" );
      return 1;
   }

   MyRead( MInfo, sizeof(MI) * MapCount, Handle );

   MyRead( &MapPalette, MAP_COLORS*sizeof(RGB), Handle );

   ColorTable = (CHR *) SPDXMalloc( MAP_COLORS * MAP_SHADES );

   if (!ColorTable)
   {
      printf( "\n\nERROR --> Unable to allocate RAM for color table\n" );
      return 1;
   }

   MyRead( ColorTable, MAP_COLORS*MAP_SHADES, Handle );

   if (MapFlags & MAP_FLAGS_TRANS)
   {
      TransTable = (CHR *) SPDXMalloc( MAP_COLORS * MAP_COLORS );

      if (!TransTable)
      {
         printf( "\n\nERROR --> Unable to allocate RAM for transparency table\n" );
         return 1;
      }

      MyRead( TransTable, MAP_COLORS*MAP_COLORS, Handle );
   }

   for( i = 0; i < MapCount; i++)
   {
      strcpy( SPDXGlobals.MapList[i].Name, MInfo[i].Name );

      MyRead( &SPDXGlobals.MapList[i].XRes, 2, Handle );
      MyRead( &SPDXGlobals.MapList[i].YRes, 2, Handle );

      SPDXGlobals.MapList[i].Data = (unsigned char *) SPDXMalloc( SPDXGlobals.MapList[i].XRes*SPDXGlobals.MapList[i].YRes );

      if (!SPDXGlobals.MapList[i].Data)
      {
         printf( "\n\nERROR --> Unable to allocate RAM for map data\n" );
         return 1;
      }

      MyRead( SPDXGlobals.MapList[i].Data, SPDXGlobals.MapList[i].XRes*SPDXGlobals.MapList[i].YRes, Handle); 
   }

   close( Handle );

   printf( "Writing..." );

   Handle = SPDXOpen( FileName, O_RDWR|O_TRUNC|O_CREAT, 0 );

   if (Handle <= 0)
   {
      printf( "\n\nERROR --> Unable to re-create file \"%s\"\n", FileName );
      return 1;
   }

   MyWrite( &MapFlags,   4, Handle );
   MyWrite( &MapCount,   2, Handle );
   MyWrite( &ColorCount, 2, Handle );
   MyWrite( &ShadeCount, 2, Handle );
   MyWrite( &MapPalette, MAP_COLORS*sizeof(RGB), Handle );
   MyWrite( ColorTable, MAP_COLORS*MAP_SHADES, Handle );
   if (MapFlags & MAP_FLAGS_TRANS)
      MyWrite( TransTable, MAP_COLORS*MAP_COLORS, Handle );

   for( i = 0; i < MapCount; i++ )
   {
      CompressedLength = 0;
      MyWrite( &SPDXGlobals.MapList[i].XRes, 2, Handle );
      MyWrite( &SPDXGlobals.MapList[i].YRes, 2, Handle );
      MyWrite( SPDXGlobals.MapList[i].Name, DEF_NAME_LEN, Handle );
      SavePos = SPDXTell( Handle );
      MyWrite( &CompressedLength, 4, Handle );
      MyWrite( &Bits, 1, Handle );
      CurOff = 0;
      CurMap = i;
      CurSize = SPDXGlobals.MapList[i].XRes * SPDXGlobals.MapList[i].YRes;
      SPDXSetCompressDataCount( CurSize );

      RetCode = SPDXCompress( Bits+1, Handle, NextByte );

      if (RetCode != LE_NONE)
      {
         printf( "\n\nERROR --> [%d] %s\n", RetCode, SPDXGetErrorString( RetCode ) );
         return 1;
      }

      CompressedLength = SPDXTell( Handle ) - SavePos;

      SPDXSeek( Handle, SavePos, SEEK_SET );

      MyWrite( &CompressedLength, 4, Handle );

      SPDXSeek( Handle, 0, SEEK_END );
   }

   SPDXClose( Handle );


   SPDXFree(SPDXGlobals.MapList);
   SPDXFree(ColorTable);

   if (MapFlags & MAP_FLAGS_TRANS)
      SPDXFree(TransTable);

   SPDXFree(SPDXGlobals.MapList[i].Data);

   printf( "Done.\n" );

   return 0;
}

/*----------------------------------------------------------------------------*/

void  PrintCopyright( )
{
   printf( "CMAP v1.70 -- MAP file compression program.\n" );
   printf( "(c) Copyright 1997 Paul D. Nettle, All Rights Reserved.\n\n" );
}

/*----------------------------------------------------------------------------*/

INT   NextByte( INT x, INT y )
{
   // This is not needed, so we supress the compiler warns
   x = y;

   if (!(CurOff & 0x3fff))
      printf( "." );

   // Write blank for leftovers
   if (CurOff >= CurSize)
      return 0;

   return SPDXGlobals.MapList[CurMap].Data[CurOff++];
}

/*----------------------------------------------------------------------------*/

void  MyRead( void *Buf, INT Count, INT Handle )
{
INT   ret;

   ret = read( Handle, Buf, Count );

   if (ret != Count)
   {
      printf( "\n\nERROR --> Unable to read from file (%d/%d)\n",
	      ret, Count );
      printf( "            error[%d]:  %s\n", errno, sys_errlist[errno] );
      exit(1);
   }
}

/*----------------------------------------------------------------------------*/

void  MyWrite( void *Buf, INT Count, INT Handle )
{
INT   ret;

   ret = SPDXWrite( Handle, Buf, Count );

   if (ret != Count)
   {
      printf( "\n\nERROR --> Unable to write to file (%d/%d)\n",
	      ret, Count );
      printf( "            error[%d]:  %s\n", errno, sys_errlist[errno] );
      exit(1);
   }
}

/*----------------------------------------------------------------------------
  -   [CMAP.C      ] - End Of File                                           -
  ----------------------------------------------------------------------------*/
