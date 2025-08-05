// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [CUSD.C      ] - Spandex Map file compresor                            -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <errno.h>

#include "spandex.h"
#include "sdxmacro.h"

/*----------------------------------------------------------------------------*/

extern INT SPDXServicesOpened;

/*----------------------------------------------------------------------------*/

static   INT   CurOff, CurSize;
static   CHR   *FileData;

static   INT   NextByte( INT x, INT y );
static   void  PrintCopyright( void );
static   INT   PreProcessFile( INT Handle, CHR **FileData, INT *MemoryAllocated );
static   INT   StripLTW( CHR *String );
static   void  StripComments( CHR *String );
static   void  MyWrite( void *Buf, INT Count, INT Handle );

/*----------------------------------------------------------------------------*/

INT   main( int argc, char *argv[] )
{
INT   Handle, RetCode, ArrayLength, ArrayCount, MemoryAllocated;
CHR   *FileName, Bits = 9;
char _FUNCTION_[] = "cusd";

   setbuf( stdout, 0 );

   PrintCopyright();

   if (argc <= 1)
   {
      printf( "Usage: CUsd <filename.USD>\n" );
      return 1;
   }

   // Turn on the logging info
   SPDXSetFatalErrors( FALSE );
   SPDXSetLoggedErrors( FALSE );
   SPDXSetLoggedStrings( FALSE );
   SPDXServicesOpened = 1;

   if (SPDXInit( ) != LE_NONE)
   {
      printf( "  ERROR --> Unable to init SPANDEX\n" );
      return 1;
   }

   FileName = argv[1];

   printf( "Reading... %s", FileName );

   Handle = SPDXOpen( FileName, O_RDONLY, 0 );

   if (Handle < 0)
   {
      printf( "\n\nERROR --> Unable to open file \"%s\"\n", FileName );
      return 1;
   }

   MemoryAllocated = USD_FILE_CHUNK;

   FileData = (CHR *) SPDXMalloc( MemoryAllocated );

   if (!FileData)
   {
      printf( "\n\nERROR --> Unable to allocate memory for USD file\n" );
      return 1;
   }

   FileData[0] = '\0';

   RetCode = PreProcessFile( Handle, &FileData, &MemoryAllocated );
   
   if (RetCode != LE_NONE)
   {
      printf( "\n\nERROR --> Unable to preprocess USD file \"%s\"\n", FileName );
      return 1;
   }

   SPDXClose( Handle );
   
   printf( "Writing..." );

   Handle = SPDXOpen( FileName, O_RDWR|O_TRUNC|O_CREAT, 0 );

   if (Handle <= 0)
   {
      printf( "\n\nERROR --> Unable to re-create file \"%s\"\n", FileName );
      return 1;
   }

   CurSize = strlen(FileData);
   ArrayLength = 1000;
   ArrayCount = CurSize / ArrayLength + ((CurSize%ArrayLength) ? 1:0);
   CurSize = ArrayCount * ArrayLength;

   MyWrite( &CurSize,     4, Handle );
   MyWrite( &ArrayLength, 4, Handle );
   MyWrite( &ArrayCount,  4, Handle );
   MyWrite( &Bits, 1, Handle );

   CurOff = 0;
   SPDXSetCompressDataCount( CurSize );

   RetCode = SPDXCompress( Bits+1, Handle, NextByte );

   if (RetCode != LE_NONE)
   {
      printf( "\n\nERROR --> [%d] %s\n", RetCode, SPDXGetErrorString( RetCode ) );
      return 1;
   }

   SPDXClose( Handle );

   printf( "Done.\n" );

   return 0;
}

/*----------------------------------------------------------------------------*/

INT   NextByte( INT x, INT y )
{
   // This is not needed, so we supress the compiler warns
   x = y;

   CurOff++;

   if (!(CurOff & 0x3fff))
      printf( "." );

   // Write blank for leftovers
   if (CurOff >= CurSize)
      return 0;

   return FileData[CurOff-1];
}

/*----------------------------------------------------------------------------*/

void  PrintCopyright( )
{
   printf( "CUSD v1.70 -- USD file compression program.\n" );
   printf( "(c) Copyright 1997 Paul D. Nettle, All Rights Reserved.\n\n" );
}

/*----------------------------------------------------------------------------*/

INT   PreProcessFile( INT Handle, CHR **FileData, INT *MemoryAllocated )
{
INT   RetCode, Len, FDLength;
CHR   CurrentLine[USD_LINE_LENGTH];
CHR   Label[USD_LINE_LENGTH], FileName[USD_LINE_LENGTH];
INT   NewHandle;
char _FUNCTION_[] = "cusd";

   Assert( FileData );
   Assert( MemoryAllocated );

   FDLength = strlen(*FileData);

   FOREVER
   {
      if (!SPDXReadString( Handle, CurrentLine, USD_LINE_LENGTH ))
         break;

      StripComments( CurrentLine );
      Len = StripLTW( CurrentLine );

      if (!Len)
         continue;

      CurrentLine[Len] = ' ';
      Len++;
      CurrentLine[Len] = '\0';

      *Label = '\0';
      *FileName = '\0';

      sscanf( CurrentLine, "%s %s", Label, FileName );

      if (!strcmp( Label, "#INCLUDE" ))
      {
         NewHandle = SPDXOpen( FileName, O_RDONLY, 0 );

         if (NewHandle < 0)
            return LE_NOUSDOPENINC;

         RetCode = PreProcessFile( NewHandle, FileData, MemoryAllocated );

         SPDXClose( NewHandle );

         if (RetCode != LE_NONE )
         {
            SPDXLogError( LE_RANGE, "Unable to #include object file" );
            return RetCode;
         }

         FDLength = strlen(*FileData);
      }
      else
      {
         if (Len + FDLength >= *MemoryAllocated)
         {
            *MemoryAllocated += USD_FILE_CHUNK;
            *FileData = (CHR *) SPDXRealloc( *FileData, *MemoryAllocated );

            if (!*FileData)
            {
               SPDXLogError( LE_NOMEM, "Unable to allocate memory for object file" );
               return LE_NOMEM;
            }
         }

         strcpy( &((*FileData)[FDLength]), CurrentLine );
         FDLength += Len;
      }
   }

   return LE_NONE;
}

/*----------------------------------------------------------------------------*/

#ifndef strrev
void strrev(char *a)
{
   char *b = &a[strlen(a) - 1];
   char temp;
   while(a++ < b--)
   {
      temp = *b;
      *b = *a;
      *a = temp;
   }
}
#endif

/*----------------------------------------------------------------------------*/

INT   StripLTW( CHR *String )
{
INT   front, back, off;
char _FUNCTION_[] = "cusd";

   Assert( String );

   // Find the first non-whitespace
   front = strspn( String, " \f\n\r\t\v" );

   // If the whole string is whitespace, then return...
   if (!String[front])
   {
      *String = '\0';
      return 0;
   }

   // Reverse the string
   strrev(String);

   // Starting at the back, find the last non-whitespace char
   back = strspn( String, " \f\n\r\t\v" );

   // Reverse the string (back to normal)
   strrev(String);

   // Get the offset into the forward string where back lies...
   back = strlen(String) - back;

   // End the string on the last whitespace
   String[back] = '\0';

   // Copy the portion of String into itself starting at 'front'
   memmove( String, &String[front], back-front+1 );

   // Turn these into spaces...
   FOREVER
   {
      off = strcspn( String, "[](),|*:<>" );
      if (String[off] == '\0')
         break;

      String[off] = ' ';
   }

   return back-front;
}

/*----------------------------------------------------------------------------*/

void  StripComments( CHR *String )
{
char _FUNCTION_[] = "cusd";
   Assert( String );

   String[strcspn( String, ";/" )] = '\0';
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
  -   [CUSD.C      ] - End Of File                                           -
  ----------------------------------------------------------------------------*/
