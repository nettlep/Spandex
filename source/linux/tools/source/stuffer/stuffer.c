// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [STUFFER.C   ] - Turbo flic routines                                   -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <glob.h>

#include <spandex.h>
#include "stuffer.h"

/*----------------------------------------------------------------------------*/

static   CHR   InputNames[80][80], OutputName[255];
static   CHR   NameList[MAX_INFILES*2][80];
static   INT   ActualNamesFound, NamesFound;
static   STFH  *HeaderList;

/*----------------------------------------------------------------------------*/

INT   main( INT argc, CHR *argv[] )
{
INT   i, Handle, TempHandle, RetCode, CurrentOffset = 0, Length;
INT   NameCount = 0;
CHR   *TempBuffer = NULL;
extern INT SPDXServicesOpened;
	FUNC("main");

   setbuf( stdout, 0 );

   PrintCopyright();

/* CHECK ARGUMENTS */

   if (argc < 2)
      PrintUsage( E_BADPARMS );

   for( i = 1; i < argc; i++ )
   {
      if (argv[i][0] == '/' || argv[i][0] == '-')
      {
         switch( toupper (argv[i][1]) )
         {
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
      strcpy( OutputName, "SPANDEX.STF" );

   if (!NameCount)
      PrintUsage( E_NONAMES );

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

   HeaderList = (STFH *) SPDXMalloc( sizeof(STFH)*ActualNamesFound );

   if (!HeaderList)
      ErrorExit( LE_NOMEM );

/* OPEN/CREATE/TRUNC THE STF OUTPUT FILE */

   Handle = SPDXOpen( OutputName, O_RDWR|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE );

   if (Handle <= 0)
      ErrorExit( E_NOCREATE );

/* WRITE OUT THE HEADER FOR THE STF FILE */
   if (SPDXWrite(Handle, (CHR *) &ActualNamesFound, 2) != 2 )
      ErrorExit( E_NOWRITE );

   if (SPDXWrite(Handle, (CHR *) HeaderList, (int)(sizeof(STFH)*ActualNamesFound)) != (int)(sizeof(STFH)*ActualNamesFound) )
      ErrorExit( E_NOWRITE );

   CurrentOffset = sizeof(STFH)*ActualNamesFound + 2;

/* COPY THE FILES INTO THE STF FILE */

   for( i = 0; i < ActualNamesFound; i++ )
   {
      TempHandle = SPDXOpen(NameList[i], O_RDONLY, 0 );

      if (TempHandle < 0)
         ErrorExit( E_NOOPEN );

      Length = SPDXSeek( TempHandle, 0L, SEEK_END );
      SPDXSeek( TempHandle, 0L, SEEK_SET );

      TempBuffer = (CHR *) SPDXMalloc( Length );

      if (!TempBuffer)
         ErrorExit( LE_NOMEM );

      if (SPDXRead( TempHandle, TempBuffer, Length ) != Length )
         ErrorExit( E_NOREAD );

      SPDXClose( TempHandle );

      if (SPDXWrite( Handle, TempBuffer, Length ) != Length )
         ErrorExit( E_NOWRITE );

      SPDXFree(TempBuffer);

      strncpy( HeaderList[i].Name, NameList[i], DEF_NAME_LEN-1 );
      HeaderList[i].Offset = CurrentOffset;
      HeaderList[i].Length = Length;

      CurrentOffset += Length;
   }

/* WRITE OUT THE TAIL FOR THE STF FILE */

   Length = SPDXSeek( Handle, 0L, SEEK_END ) + 4;

   if (SPDXWrite(Handle, (CHR *) &Length, 4) != 4 )
      ErrorExit( E_NOWRITE );

/* RE-WRITE OUT THE HEADER FOR THE STF FILE */

   SPDXSeek( Handle, 2L, SEEK_SET );

   if (SPDXWrite(Handle, (CHR *) HeaderList, (int)(sizeof(STFH)*ActualNamesFound)) != (int)(sizeof(STFH)*ActualNamesFound) )
      ErrorExit( E_NOWRITE );

   SPDXClose( Handle );

/* CLEANUP AFTER YOURSELF */

   SPDXClose(Handle);
   SPDXUninit( NULL );
   SPDXCloseServices();
   return 0;
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
   printf( "STUFFER v1.70 -- STF file generation program.\n" );
   printf( "(c) Copyright 1997 Paul D. Nettle, All Rights Reserved.\n\n" );
}

/*----------------------------------------------------------------------------*/

void  GetNames( INT NameCount )
{
INT   i, j;
CHR  TempStr[80];
glob_t List;

/* GET THE LIST */

   for( i = 0; i < NameCount; i++ )
   {
     if(!glob( InputNames[i], 0, NULL, &List)) {
       for(j=0; j<List.gl_pathc; j++) {
	 strcpy( NameList[NamesFound], List.gl_pathv[j]);
	 NamesFound++;
	 if (NamesFound == MAX_INFILES) {
	   printf( "WARNING:  Max files reached, ignoring extra files\n" );
	   break;
	 }
       }
       globfree(&List);
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

      case E_NOREAD:
         TempStr = "Unable to read file";
         break;

      case E_NOWRITE:
         TempStr = "Unable to write file";
         break;

      case E_NOOPEN:
         TempStr = "Unable to open input file";
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

   printf( "Usage:  STUFFER [options] <outfile> <infiles>\n" );
   printf( "\n" );
   printf( "        <outfile>  File name of output STF file (including extension)\n" );
   printf( "\n" );
   printf( "        <filespec> Describes input files to be compiled.\n" );
   printf( "                   Wildcards allowed -- files are compiled in sorted\n" );
   printf( "                   order (maximum of %d files)\n", MAX_INFILES );
   printf( "\n" );
   printf( "        [options]  ?|H -> Get this help information\n" );
   printf( "\n" );
   printf( "NOTE:   All files MUST be located in the current directory.\n" );

   ExitSystem( ErrorCode );
}

/*----------------------------------------------------------------------------
  -   [STUFFER.C   ] - End Of File                                           -
  ----------------------------------------------------------------------------*/
