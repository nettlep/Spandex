// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [MAKEBUM.C   ] - Map fle generator for Spandex                         -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>
#include <math.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>

#include <spandex.h>
#include "makebum.h"

/*----------------------------------------------------------------------------*/

CHR   InputNames[80][80], OutputName[255];
CHR   NameList[MAX_INFILES*2][80];
IMAGE *ImageList[MAX_INFILES];
INT   ActualNamesFound, NamesFound, CurrentFile = 0;
DBL   Multiplier = 1.0;

/*----------------------------------------------------------------------------*/

INT   main( INT argc, CHR *argv[] )
{
INT   i, MapHandle, RetCode;
INT   MapStart, MapEnd;
INT   NameCount = 0;
INT   Dummy;
CHR   TempName[30], *TempPtr;
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
            case 'M':
               Multiplier = (float) atof(&argv[i][2]);
               break;

            case '?':
            case 'H':
               PrintUsage( E_NONE );
               return 0;

            default:
               printf( "Unknown parameter:  \"%s\"\n", argv[i] );
               PrintUsage( E_BADPARMS );
               return 1;
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
      strcpy( OutputName, "BUMFILE.BUM" );

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

      MakeImageMono(ImageList[i]);

      printf( "." );
   }

/* OPEN/CREATE/TRUNC THE BUM OUTPUT FILE */

   MapHandle = SPDXOpen( OutputName, O_BINARY|O_RDWR|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE );

   if (MapHandle <= 0)
      ErrorExit( E_NOCREATE );

/* WRITE OUT THE HEADER FOR THE BUM FILE */

   RetCode = MyWrite( MapHandle, &ActualNamesFound, 2 );

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

/* ADD ALL THE FRAMES TO THE BUM FILE */

   printf( "\nWriting    [%*s]", ActualNamesFound, " " );
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

      SPDXSeek( MapHandle, 2+CurrentFile*(DEF_NAME_LEN+4)+DEF_NAME_LEN, SEEK_SET );

      RetCode = MyWrite( MapHandle, &MapStart, 4 );

      if (RetCode != E_NONE)
         ErrorExit ( RetCode );

      SPDXSeek( MapHandle, MapEnd, SEEK_SET );

      printf( "." );

      i++;
   } while (CurrentFile++ < NamesFound-1);

   printf( "\n" );

/* CLEANUP AFTER YOURSELF */

   SPDXClose( MapHandle );
   for( i = 0; i < ActualNamesFound; i++)
      SPDXDestroyImage( &ImageList[i] );
   SPDXUninit( NULL );
   SPDXCloseServices();

   return 0;
}

/*----------------------------------------------------------------------------*/

void  MakeImageMono(IMAGE *Image)
{
INT   x, y, ResX, ResY, YIndex;
INT   CurIndex, NextXOff, NextYOff;
INT   CurPix,   NextXPix, NextYPix;
INT   CurDiff,  XDiff,    YDiff;
UBYT  *ptr;

   ResX = Image->ResX;
   NextXOff = 3;

   ResY = Image->ResY;
   NextYOff = ResY*3;

   for( y = 0; y < ResY-1; y++ )
   {
      YIndex = y * ResX * 3;

      for( x = 0; x < ResX-1; x++ )
      {
         CurIndex = YIndex + x * 3;

         ptr = &Image->Buffer[CurIndex];
         CurPix   = MAX(ptr[0], MAX(ptr[1], ptr[2]));

         ptr = &Image->Buffer[CurIndex+NextXOff];
         NextXPix = MAX(ptr[0], MAX(ptr[1], ptr[2]));

         ptr = &Image->Buffer[CurIndex+NextYOff];
         NextYPix = MAX(ptr[0], MAX(ptr[1], ptr[2]));

         XDiff = NextXPix - CurPix;
         YDiff = NextYPix - CurPix;
         CurDiff = (XDiff - YDiff) >> 1;

         // Clip it to range...
         CurDiff = MAX(CurDiff, -128);
         CurDiff = MIN(CurDiff,  128);

         //Scale it to user specs...
         CurDiff = (INT) ((DBL) CurDiff * Multiplier);

         //Store this puppy off...
         Image->Buffer[y*ResX+x] = CurDiff;
      }
   }
}

/*----------------------------------------------------------------------------*/

void  ExitSystem( INT ErrorCode )
{
   exit(ErrorCode);
}

/*----------------------------------------------------------------------------*/

void  ErrorExit( INT ErrorCode )
{
   PrintErrorString( ErrorCode );
   ExitSystem( 1 );
}

/*----------------------------------------------------------------------------*/

void  PrintCopyright( )
{
   printf( "MAKEBUM v1.70 -- BUM file generation program.\n" );
   printf( "(c) Copyright 1997 Paul D. Nettle, All Rights Reserved.\n\n" );
}

/*----------------------------------------------------------------------------*/

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

/*----------------------------------------------------------------------------*/

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

   printf( "\nлллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл\n" );
   printf( "лл                                                                        лл\n" );
   printf( "лл FATAL ERROR [%03d]:  %-51sлл\n", ErrorCode, TempStr);
   printf( "лл                                                                        лл\n" );
   printf( "лллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл\n\n%c", 0x7 );
}

/*----------------------------------------------------------------------------*/

void  PrintUsage( INT ErrorCode )
{
   if (ErrorCode != E_NONE )
      PrintErrorString( ErrorCode );

   printf( "Usage:  MAKEBUM [options] <outfile> <infiles>\n" );
   printf( "\n" );
   printf( "        <outfile>  File name of output BUM file (including extension)\n" );
   printf( "                   Default is BUMPFILE\n" );
   printf( "\n" );
   printf( "        <infiles>  Describes input image files to be compiled.\n" );
   printf( "                   Formats supported are IPI/IMG/GIF/TGA/PCX/BMP.\n" );
   printf( "                   Wildcards allowed. Files are compiled in sorted\n" );
   printf( "                   order (maximum of %d files)\n", MAX_INFILES );
   printf( "\n" );
   printf( "        [options]  ?|H -> Get this help information\n" );
   printf( "                   Mnn -> Increase amplitude of bump-map by double-precision\n" );
   printf( "                          multiplier nn\n" );
   printf( "\n" );
   printf( "NOTE:   All files MUST be located in the current directory.\n" );

   ExitSystem( 1 );
}

/*----------------------------------------------------------------------------*/

INT   MyWrite( INT Handle, void *Buffer, INT Length )
{
   if (SPDXWrite( Handle, Buffer, Length ) != Length)
      return E_NOWRITE;

   return E_NONE;
}

/*----------------------------------------------------------------------------
  -   [MAKEBUM.C   ] - End Of File                                           -
  ----------------------------------------------------------------------------*/
