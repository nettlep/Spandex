// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [MAKEMAP.C   ] - Map fle generator for Spandex                         ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>

#include <spandex.h>
#include "makemap.h"

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

CHR   InputNames[80][80], OutputName[255];
CHR   NameList[MAX_INFILES*2][80];
IMAGE *ImageList[MAX_INFILES], *ImageList2[2];
INT   ActualNamesFound, NamesFound, CurrentFile = 0;

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   main( INT argc, CHR *argv[] )
{
INT   i, j, MapHandle, RetCode, temp;
INT   MapStart, MapEnd, PalImageStart, PalStart, MapFlags = 0;
INT   NameCount = 0;
INT   Dummy;
UBYT  MapPalette[MAX_COLORS*3];
CHR   TempName[30], *TempPtr;
FLT   red, green, blue, intensity;
DBL   Shine = -1.0, ShineIndex = 0.0, BrightestShade = 1.0, NoShade = 0, Opacity = 1.0;
DBL   RedShade = 1.0, GreenShade = 1.0, BlueShade = 1.0;
extern INT SPDXServicesOpened;

   setbuf( stdout, 0 );

   PrintCopyright();

/* CHECK ARGUMENTS */

   for( i = 1; i < argc; i++ )
   {
      if (argv[i][0] == '/' || argv[i][0] == '-')
      {
         switch( toupper (argv[i][1]) )
         {
            case 'I':
               BrightestShade = atof( &argv[i][2] );
               BrightestShade = MAX(BrightestShade, 0.0);
               BrightestShade = MIN(BrightestShade, 1.0);
               break;

            case 'N':
               NoShade = 1;
               break;

            case 'O':
               MapFlags |= MAP_FLAGS_TRANS;
               Opacity = atof( &argv[i][2] );
               Opacity = MAX(Opacity, 0.0);
               Opacity = MIN(Opacity, 1.0);
               break;

            case 'R':
               RedShade = atof( &argv[i][2] );
               RedShade = MAX(RedShade, 0.0);
               RedShade = MIN(RedShade, 1.0);
               break;

            case 'G':
               BlueShade = atof( &argv[i][2] );
               BlueShade = MAX(BlueShade, 0.0);
               BlueShade = MIN(BlueShade, 1.0);
               break;

            case 'B':
               GreenShade = atof( &argv[i][2] );
               GreenShade = MAX(GreenShade, 0.0);
               GreenShade = MIN(GreenShade, 1.0);
               break;

            case 'S':
               Shine = atof( &argv[i][2] );
               Shine = MAX(Shine, 0.0);
               Shine = MIN(Shine, 0.92);
               break;

            case '?':
            case 'H':
               PrintUsage( E_NONE );

            default:
               printf( "Unknown parameter:  \"%s\"\n", argv[i] );
               PrintUsage( E_BADPARMS );
         }
      }
      else
      {
         if (!OutputName[0])
         {
            strcpy( OutputName, argv[i] );
         }
         else if (!InputNames[NameCount][0])
         {
            strcpy( InputNames[NameCount], argv[i] );
            NameCount++;
         }
         else
         {
            printf( "Unknown parameter:  \"%s\"\n", argv[i] );
            PrintUsage( E_NONE );
         }
      }
   }

   if (!OutputName[0])
      strcpy( OutputName, "MAPFILE.MAP" );

   if (argc < 2 || !NameCount)
      PrintUsage( E_BADPARMS );

/* INITIALIZE THE SPANDEX LIBRARY */

   // Turn on the logging info
   SPDXSetFatalErrors( TRUE );
   SPDXSetLoggedErrors( TRUE );
   SPDXSetLoggedStrings( TRUE );

   // PRE-INITIALIZATION SERVICES
   SPDXServicesOpened = 1;

   // INITIALIZE THE SPANDEX ENVIRONMENT
   RetCode = SPDXInit( );

   if (RetCode != E_NONE)
      ErrorExit( RetCode );

/* GET THE NAMES IN SORTED ORDER */

   GetNames( NameCount );

/* CREATE DUMMY IMAGES SINCE LOADER EXPECTS AN EXISTING IMAGE */

   for( i = 0; i < ActualNamesFound; i++)
   {
      ImageList[i] = SPDXCreateImage( 2, 2 );

      if (!ImageList[i])
         ErrorExit( LE_NOMEMIMG );
   }

/* CREATE THE PALETTE LISTS */

   printf( "Loading    [%*s]", ActualNamesFound, " " );
   printf( "\rLoading    [" );

   for( i = 0; i < ActualNamesFound; i++ )
   {
      RetCode = SPDXReadImage( NameList[i], &ImageList[i] );

      if (RetCode != LE_NONE)
         ErrorExit( RetCode );

      printf( "." );
   }

   if (MapFlags & MAP_FLAGS_TRANS)
      printf( "\nQuantizing [   ]\b\b\b\b" );
   else
      printf( "\nQuantizing [  ]\b\b\b" );

   SPDXQuantizeImages( ImageList, ActualNamesFound, ActualNamesFound, MAP_COLORS, 8, MapPalette );

   printf( "." );
   
/* CREATE PALETTE/TANSPARENCY IMAGES */

   ImageList2[0] = SPDXCreateImage( MAX_SHADES, MAX_COLORS );

   if (!ImageList2[0])
      ErrorExit( LE_NOMEMIMG );

   if (MapFlags & MAP_FLAGS_TRANS)
   {
      ImageList2[1] = SPDXCreateImage( MAX_COLORS, MAX_COLORS );

      if (!ImageList2[1])
         ErrorExit( LE_NOMEMIMG );
   }

   if (NoShade)
   {
      for (i = 0; i < MAX_COLORS; i++)
      {
         for( j = 0; j < MAX_SHADES; j++)
         {
            ImageList2[0]->Buffer[i*MAX_SHADES*3+j*3+0] = MapPalette[i*3+0] >> 2;
            ImageList2[0]->Buffer[i*MAX_SHADES*3+j*3+1] = MapPalette[i*3+1] >> 2;
            ImageList2[0]->Buffer[i*MAX_SHADES*3+j*3+2] = MapPalette[i*3+2] >> 2;
         }
      }
   }
   else
   {
/* SHADE PALETTE IMAGE */
      ShineIndex = Shine * (DBL) MAX_SHADES;

      for (i = 0; i < MAX_COLORS; i++)
      {
         for( j = 0; j < MAX_SHADES; j++)
         {
            red   = MapPalette[i*3+0];
            green = MapPalette[i*3+1];
            blue  = MapPalette[i*3+2];

            intensity = 1.0 / (DBL) MAX_SHADES * (FLT) j;

            red *= intensity;
            green *= intensity;
            blue *= intensity;

            ImageList2[0]->Buffer[i*MAX_SHADES*3+j*3+0] = ((UBYT) (red)) >> 2;
            ImageList2[0]->Buffer[i*MAX_SHADES*3+j*3+1] = ((UBYT) (green)) >> 2;
            ImageList2[0]->Buffer[i*MAX_SHADES*3+j*3+2] = ((UBYT) (blue)) >> 2;
         }
      }

      if (Shine >= 0.0)
      {
         for (i = 0; i < MAX_COLORS; i++)
         {
            for( j = (INT) ShineIndex; j < MAX_SHADES; j++)
            {
               intensity = 1.0 / (DBL) ((DBL) MAX_SHADES - ShineIndex) *
                                 (DBL) ((INT) j          - (INT) ShineIndex) *
                                 (DBL) (BrightestShade*RedShade) * 63.0;
               temp = (INT) intensity;
               if (temp > 63) temp = 63;
               ImageList2[0]->Buffer[i*MAX_SHADES*3+j*3+0] += temp;


               intensity = 1.0 / (DBL) ((DBL) MAX_SHADES - ShineIndex) *
                                 (DBL) ((INT) j          - (INT) ShineIndex) *
                                 (DBL) (BrightestShade*GreenShade) * 63.0;
               temp = (INT) intensity;
               if (temp > 63) temp = 63;
               ImageList2[0]->Buffer[i*MAX_SHADES*3+j*3+1] += temp;


               intensity = 1.0 / (DBL) ((DBL) MAX_SHADES - ShineIndex) *
                                 (DBL) ((INT) j          - (INT) ShineIndex) *
                                 (DBL) (BrightestShade*BlueShade) * 63.0;
               temp = (INT) intensity;
               if (temp > 63) temp = 63;
               ImageList2[0]->Buffer[i*MAX_SHADES*3+j*3+2] += temp;

               if (ImageList2[0]->Buffer[i*MAX_SHADES*3+j*3+0] > 63)
                   ImageList2[0]->Buffer[i*MAX_SHADES*3+j*3+0] = 63;
               if (ImageList2[0]->Buffer[i*MAX_SHADES*3+j*3+1] > 63)
                   ImageList2[0]->Buffer[i*MAX_SHADES*3+j*3+1] = 63;
               if (ImageList2[0]->Buffer[i*MAX_SHADES*3+j*3+2] > 63)
                   ImageList2[0]->Buffer[i*MAX_SHADES*3+j*3+2] = 63;
            }
         }
      }
   }

/* CREATE THE TRANSPARENCY MAP & REDUCE COLORS */

   if (MapFlags & MAP_FLAGS_TRANS)
   {
      IMAGE *TempImage;

      TempPtr = ImageList2[1]->Buffer;
      TempImage = SPDXCreateImage( ImageList2[0]->ResX, ImageList2[0]->ResY );
      SPDXMemCopyBYTE( TempImage->Buffer, ImageList2[0]->Buffer, (ImageList2[0]->ResX*ImageList2[0]->ResY*3) );
      SPDXQuantizeImage( TempImage, MAP_COLORS, 8, MapPalette );

      for (i = 0; i < MAX_COLORS; i++)
      {
         for( j = 0; j < MAX_COLORS; j++)
         {
            red   = (float) MapPalette[j*3+0]*(1.0-Opacity) +
                    (float) MapPalette[i*3+0]*Opacity;
            green = (float) MapPalette[j*3+1]*(1.0-Opacity) +
                    (float) MapPalette[i*3+1]*Opacity;
            blue  = (float) MapPalette[j*3+2]*(1.0-Opacity) +
                    (float) MapPalette[i*3+2]*Opacity;

            if ((UBYT) red > 63)
               TempPtr[i*MAX_COLORS*3+j*3+0] = 63;
            else
               TempPtr[i*MAX_COLORS*3+j*3+0] = ((UBYT) (red+0.5));

            if ((UBYT) red > 63)
               TempPtr[i*MAX_COLORS*3+j*3+1] = 63;
            else
               TempPtr[i*MAX_COLORS*3+j*3+1] = ((UBYT) (green+0.5));

            if ((UBYT) red > 63)
               TempPtr[i*MAX_COLORS*3+j*3+2] = 63;
            else
               TempPtr[i*MAX_COLORS*3+j*3+2] = ((UBYT) (blue+0.5));
         }
      }

      printf( "." );

      _SPDXMakeTransTable( ImageList2[0], ImageList2[1], MAP_COLORS, 8, MapPalette, Opacity );
   }
   else
      SPDXQuantizeImage( ImageList2[0], MAP_COLORS, 8, MapPalette );

   printf( ".\n" );

/* OPEN/CREATE/TRUNC THE MAP OUTPUT FILE */

   MapHandle = SPDXOpen( OutputName, O_BINARY|O_RDWR|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE );

   if (MapHandle <= 0)
      ErrorExit( E_NOCREATE );

/* WRITE OUT THE HEADER FOR THE MAP FILE */

   RetCode = MyWrite( MapHandle, &MapFlags, sizeof(MapFlags) );

   if (RetCode != E_NONE)
      ErrorExit( RetCode );

   RetCode = MyWrite( MapHandle, &ActualNamesFound, 2 );

   if (RetCode != E_NONE)
      ErrorExit( RetCode );

   Dummy = MAX_COLORS;
   RetCode = MyWrite( MapHandle, &Dummy, 2 );

   if (RetCode != E_NONE)
      ErrorExit( RetCode );

   Dummy = MAX_SHADES;
   RetCode = MyWrite( MapHandle, &Dummy, 2 );

   if (RetCode != E_NONE)
      ErrorExit( RetCode );

   for( i = 0; i < ActualNamesFound; i++ )
   {
      memset( TempName, 0, DEF_NAME_LEN );
      strcpy( TempName, NameList[i] );

      TempPtr = strchr(TempName, '.');

      if (TempPtr)
         *TempPtr = '\0';

      RetCode = MyWrite( MapHandle, TempName, DEF_NAME_LEN );

      if (RetCode != E_NONE)
         ErrorExit( RetCode );

      Dummy = 0;
      RetCode = MyWrite( MapHandle, &Dummy, 2 );
      RetCode = MyWrite( MapHandle, &Dummy, 2 );

      if (RetCode != E_NONE)
         ErrorExit( RetCode );
   }

   PalStart = SPDXTell( MapHandle );

   RetCode = MyWrite( MapHandle, MapPalette, MAX_COLORS*3 );

   if (RetCode != E_NONE)
      ErrorExit( RetCode );

/* ADD THE PALETTE PLACEHOLDER TO THE MAP FILE */

   PalImageStart = SPDXTell( MapHandle );

   RetCode = MyWrite( MapHandle, ImageList2[0]->Buffer, ImageList2[0]->ResX*ImageList2[0]->ResY );

   if (RetCode != E_NONE)
      ErrorExit ( RetCode );

   if (MapFlags & MAP_FLAGS_TRANS)
   {
      RetCode = MyWrite( MapHandle, ImageList2[1]->Buffer, ImageList2[1]->ResX*ImageList2[1]->ResY );

      if (RetCode != E_NONE)
         ErrorExit ( RetCode );
   }

/* ADD ALL THE FRAMES TO THE MAP FILE */

   printf( "Writing    [%*s]", ActualNamesFound, " " );
   printf( "\rWriting    [" );

   i = 0;

   do
   {
      MapStart = SPDXTell( MapHandle );

      RetCode = MyWrite( MapHandle, &ImageList[i]->ResX, 2 );

      if (RetCode != E_NONE)
         ErrorExit ( RetCode );

      RetCode = MyWrite( MapHandle, &ImageList[i]->ResY, 2 );

      if (RetCode != E_NONE)
         ErrorExit ( RetCode );

      RetCode = MyWrite( MapHandle, ImageList[i]->Buffer,
                         ImageList[i]->ResX*ImageList[i]->ResY );

      if (RetCode != E_NONE)
         ErrorExit ( RetCode );

      MapEnd = SPDXTell( MapHandle );

      SPDXSeek( MapHandle, 10+CurrentFile*(DEF_NAME_LEN+4)+DEF_NAME_LEN, SEEK_SET );

      RetCode = MyWrite( MapHandle, &MapStart, 4 );

      if (RetCode != E_NONE)
         ErrorExit ( RetCode );

      SPDXSeek( MapHandle, MapEnd, SEEK_SET );

      printf( "." );

      i++;
   } while (CurrentFile++ < NamesFound-1);

   printf( "\n" );

/* ADD THE PALETTE TO THE MAP FILE */

   printf( "Finishing  [ ]\b\b" );

   SPDXSeek( MapHandle, PalStart, SEEK_SET );

   RetCode = MyWrite( MapHandle, MapPalette, MAX_COLORS*3 );

   if (RetCode != E_NONE)
      ErrorExit ( RetCode );

/* ADD THE PALETTE IMAGE TO THE MAP FILE */

   SPDXSeek( MapHandle, PalImageStart, SEEK_SET );

   RetCode = MyWrite( MapHandle, ImageList2[0]->Buffer, ImageList2[0]->ResX*ImageList2[0]->ResY );

   if (RetCode != E_NONE)
      ErrorExit ( RetCode );

   if (MapFlags & MAP_FLAGS_TRANS)
   {
      RetCode = MyWrite( MapHandle, ImageList2[1]->Buffer, ImageList2[1]->ResX*ImageList2[1]->ResY );

      if (RetCode != E_NONE)
         ErrorExit ( RetCode );
   }

   printf( ".\n" );

/* CLEANUP AFTER YOURSELF */

   SPDXClose( MapHandle );
   for( i = 0; i < ActualNamesFound; i++)
      SPDXDestroyImage( &ImageList[i] );
   SPDXDestroyImage( &ImageList2[0] );
   if (MapFlags & MAP_FLAGS_TRANS)
      SPDXDestroyImage( &ImageList2[1] );
   SPDXUninit( NULL );
   SPDXCloseServices();

   return 0;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  ExitSystem( INT ErrorCode )
{
   exit(ErrorCode);
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  ErrorExit( INT ErrorCode )
{
   PrintErrorString( ErrorCode );
   ExitSystem( 1 );
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  PrintCopyright( )
{
   printf( "MAKEMAP v1.70 -- MAP file generation program.\n" );
   printf( "(c) Copyright 1997 Paul D. Nettle, All Rights Reserved.\n\n" );
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  GetNames( INT NameCount )
{
INT   Done = 0, i, j;
CHR  TempStr[80];
struct find_t Block;

/* GET THE LIST */

   for( i = 0; i < NameCount; i++ )
   {
      Done = _dos_findfirst( InputNames[i], _A_NORMAL, &Block );

      while( !Done )
      {
         strcpy( NameList[NamesFound], Block.name );

         NamesFound++;

         if (NamesFound == MAX_INFILES)
         {
            printf( "WARNING:  Max files reached, ignoring extra files\n" );
            break;
         }

         Done = _dos_findnext( &Block );
      }
   }

   if (!NamesFound)
      ErrorExit( E_NONAMES );

/* SORT THE LIST */

   for( i = 0; i < NamesFound - 1; i++ )
   {
      for( j = 0; j < NamesFound - 1; j++ )
      {   
         if (strcmp( NameList[j], NameList[j+1] ) > 0)
         {
            strcpy( TempStr, NameList[j] );
            strcpy( NameList[j], NameList[j+1] );
            strcpy( NameList[j+1], TempStr );
         }
      }
   }

   ActualNamesFound = NamesFound;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  PrintErrorString( INT ErrorCode )
{
CHR   *TempStr;

   switch( ErrorCode )
   {
      case E_BADPARMS:
         TempStr = "Bad Parms";
         break;

      case E_NOCREATE:
         TempStr = "Unable to create file";
         break;

      case E_NOWRITE:
         TempStr = "Unable to write file";
         break;

      case E_NONAMES:
         TempStr = "No names given";
         break;

      default:
         TempStr = SPDXGetErrorString( ErrorCode );
   }

   printf( "\n€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€\n" );
   printf( "€€                                                                        €€\n" );
   printf( "€€ FATAL ERROR [%03d]:  %-51s€€\n", ErrorCode, TempStr);
   printf( "€€                                                                        €€\n" );
   printf( "€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€€\n\n%c", 0x7 );
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  PrintUsage( INT ErrorCode )
{
   if (ErrorCode != E_NONE )
      PrintErrorString( ErrorCode );

   printf( "Usage:  MAKEMAP [options] <outfile> <infiles>\n" );
   printf( "\n" );
   printf( "        <outfile>  File name of output MAP file (including extension)\n" );
   printf( "                   Default is MAPFILE\n" );
   printf( "\n" );
   printf( "        <filespec> Describes input image files to be compiled.\n" );
   printf( "                   Formats supported are IPI/IMG/GIF/TGA/PCX/BMP.\n" );
   printf( "                   Wildcards allowed. Files are compiled in sorted\n" );
   printf( "                   order (maximum of %d files)\n", MAX_INFILES );
   printf( "\n" );
   printf( "        [options]  ?|H -> Get this help information\n" );
   printf( "                   Ixx -> xx = Intensity of specular highlight (0.0 - 1.0)\n" );
   printf( "                     N -> No shading of the palette\n" );
   printf( "                   Rxx -> xx = Specular Highlight Color (Red) (0.0 - 1.0)\n" );
   printf( "                   Gxx -> xx = Specular Highlight Color (Green) (0.0 - 1.0)\n" );
   printf( "                   Bxx -> xx = Specular Highlight Color (Blue) (0.0 - 1.0)\n" );
   printf( "                   Sxx -> xx = Specular value (0.0 - 0.92)\n" );
   printf( "                   Oxx -> Create opacity map with xx opacity (0.0 - 1.0)\n" );
   printf( "\n" );
   printf( "NOTE:   All files MUST be located in the current directory.\n" );

   ExitSystem( 1 );
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   MyWrite( INT Handle, void *Buffer, INT Length )
{
   if (SPDXWrite( Handle, Buffer, Length ) != Length)
      return E_NOWRITE;

   return E_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [MAKEMAP.C   ] - End Of File                                           ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
