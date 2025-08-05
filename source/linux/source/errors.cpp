// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [ERRORS.C    ] - Error handling routines                               -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#include "spandex.h"

/*----------------------------------------------------------------------------*/

ERRLIST  ErrorList[] = {
   {LE_NONE,             "No error occured"},
   {LE_NOMEM,            "Memory related error (most likely insufficient)"},
   {LE_MEMLEAK,          "Memory leak detected"},
   {LE_NOMEMOFF,         "Not enough memory for Off-screen RAM"},
   {LE_NOTAVAIL,         "Function not supported by your VESA BIOS or TSR"},
   {LE_VINOTAVAIL,       "Information not available"},
   {LE_SINOTAVAIL,       "SVGA information not available"},
   {LE_MODENOTAVAIL,     "VESA/SVGA mode not available"},
   {LE_GMODENOTAVAIL,    "Unable to obtain current mode information"},
   {LE_SWNOTAVAIL,       "Unable to set SVGA window viewport"},
   {LE_GWNOTAVAIL,       "Unable to obtain window viewport information"},
   {LE_NOMAPFILE,        "Unable to open map file"},
   {LE_NOMAPREAD,        "Unable to read map file"},
   {LE_NOMAPSEEK,        "Unable to seek map file"},
   {LE_WRONGMAP,         "Wrong map type (compressed/non-compressed)"},
   {LE_MAPNOTFOUND,      "Map not found in map file"},
   {LE_MATNOTFOUND,      "Material not found in material list"},
   {LE_NOSTFOPEN,        "Unable to open STF file"},
   {LE_NOSTFREAD,        "Unable to read STF file"},
   {LE_NOUSDOPEN,        "Unable to open USD file"},
   {LE_NOUSDOPENINC,     "Unable to open USD include file"},
   {LE_USDSYNTAX,        "Bad syntax in USD file"},
   {LE_RANGE,            "Out of range error"},
   {LE_NOERROPEN,        "Unable to open error file"},
   {LE_NOERRRESET,       "Unable to reset error file"},
   {LE_VFAIL,            "Verify failed"},
   {LE_AFAIL,            "Assert failed"},
   {LE_NOMOUSE,          "Mouse driver not found"},
   {LE_NOCREATE,         "Unable to create file"},
   {LE_BADIMAGE,         "Not a valid BMP file"},
   {LE_BADTGATYPE,       "Invalid Targa file type (only type 2 supported)"},
   {LE_INVALIDFILETYPE,  "Invalid file type"},
   {LE_INVALIDGIFFILE,   "Not a valid GIF file"},
   {LE_NOCOMPBMP,        "Not a compatible BMP file type"},
   {LE_NOGIFINTERLACE,   "Interlace not allowed in GIF files"},
   {LE_NOMEMIMG,         "Not enough memory to load image file"},
   {LE_NOMEMPAL,         "Not enough memory to process colors"},
   {LE_NOOPEN,           "Unable to open file"},
   {LE_NOREAD,           "Unable to read from file"},
   {LE_NOTBMP,           "Not a valid BMP file"},
   {LE_NOTPCX,           "Not a valid PCX file"},
   {LE_NOTVALIDPCX,      "PCX files must be 256 or 24-bit true-color"},
   {LE_NOTPCXENCODE,     "PCX files must be RLE encoded"},
   {LE_NOWRITE,          "Unable to write to file"},
   {LE_BADCODE,          "Bad code in GIF file"},
   {LE_NOSERVICES,       "Spandex's services have not been opened or closed properly"},
   {LE_UNKNOWN,          "An unknown error occured"},
   {0,NULL}
};

/*----------------------------------------------------------------------------*/

static   INT   FatalErrors = FALSE;
static   INT   LoggedErrors = FALSE;
static   INT   LoggedStrings = FALSE;

/*----------------------------------------------------------------------------*/

CHR   *SPDXGetErrorString( WRD ErrorCode )
{
INT   i = 0;

   FUNC("SPDXGetErrorString");
   while( ErrorList[i].String )
   {
      if (ErrorList[i].ErrorCode == ErrorCode)
         return ErrorList[i].String;
      i++;
   }

   return "An unlisted error occured";
}

/*----------------------------------------------------------------------------*/

void  SPDXSetFatalErrors(INT Flag)
{
   FUNC("SPDXSetFatalErrors");
   if (Flag)
      FatalErrors = TRUE;
   else
      FatalErrors = FALSE;
}

/*----------------------------------------------------------------------------*/

INT   SPDXGetFatalErrors( )
{
   FUNC("SPDXGetFatalErrors");
   return FatalErrors;
}

/*----------------------------------------------------------------------------*/

void  SPDXSetLoggedErrors(INT Flag)
{
   FUNC("SPDXSetLoggedErrors");
   if (Flag)
      LoggedErrors = TRUE;
   else
      LoggedErrors = FALSE;
}

/*----------------------------------------------------------------------------*/

INT   SPDXGetLoggedErrors( )
{
   FUNC("SPDXGetLoggedErrors");
   return LoggedErrors;
}

/*----------------------------------------------------------------------------*/

void  SPDXSetLoggedStrings(INT Flag)
{
   FUNC("SPDXSetLoggedStrings");
   if (Flag)
      LoggedStrings = TRUE;
   else
      LoggedStrings = FALSE;
}

/*----------------------------------------------------------------------------*/

INT   SPDXGetLoggedStrings( )
{
   FUNC("SPDXGetLoggedStrings");
   return LoggedStrings;
}

/*----------------------------------------------------------------------------*/

void  _SPDXFatalError( CHR *File, INT Line, CHR *Function, INT ErrorCode, CHR *Notes )
{
   FUNC("_SPDXFatalError");
   printf( "SPANDEX FATAL ERROR[%03d]:\n   General:  %s\n", ErrorCode, SPDXGetErrorString((WRD)ErrorCode) );

   if (Notes)
      printf( "   Details:  %s\n", Notes );

   if (FatalErrors != FALSE)
      _SPDXLogError( File, Line, Function, ErrorCode, Notes );

#ifdef _OS_DOS
   printf( "Press any key...%c", 0x07 );
   if (!getch())getch();
#endif

   SPDXUninit( NULL );
   exit( ErrorCode );
}

/*----------------------------------------------------------------------------*/

void  SPDXResetErrorFile( void )
{
   FUNC("SPDXResetErrorFile");
   unlink( ERR_FILE );
}

/*----------------------------------------------------------------------------*/

void  _SPDXLogError( CHR *File, INT Line, CHR *Function, INT ErrorCode, CHR *Notes )
{
INT   Handle;
CHR   TempStr[255+DEF_FUNC_LEN];
time_t Time;
struct tm *Tm;

   FUNC("_SPDXLogError");
   if (LoggedErrors == FALSE)
      return;

   Handle = open( ERR_FILE, O_BINARY|O_CREAT|O_RDWR, S_IREAD|S_IWRITE);

   if (Handle > 0)
   {
      lseek( Handle, 0L, SEEK_END );

      Time = time( NULL );
      Tm = localtime( &Time );

      sprintf( TempStr, "[When] %02d/%02d/%02d %02d:%02d:%02d\n",
              Tm->tm_mon, Tm->tm_mday, Tm->tm_year,
              Tm->tm_hour, Tm->tm_min, Tm->tm_sec );
      write( Handle, TempStr, strlen(TempStr) );

      sprintf( TempStr, "[Code] (%03d) %s\n",
              ErrorCode, SPDXGetErrorString((WRD) ErrorCode) );
      write( Handle, TempStr, strlen(TempStr) );

      if (SPDXGetDebugState() & DEBUG_FUNCS)
      {
         sprintf( TempStr, "[Loct] %s [%s|%d]\n", Function, File, Line );
         write( Handle, TempStr, strlen(TempStr) );
      }

      if (Notes)
      {
         sprintf( TempStr, "[Note] %s\n", Notes );
         write( Handle, TempStr, strlen(TempStr) );
      }
      write( Handle, "-------------------------------------------------------------------------------\n", 80 );
   }
   else
   {
      SPDXFatalError( LE_NOERROPEN, "Unable to write errors out to error file" );
   }

   close( Handle );
}

/*----------------------------------------------------------------------------*/

void  _SPDXLogString( CHR *File, INT Line, CHR *Function, CHR *Notes )
{
INT   Handle;
CHR   TempStr[255+DEF_FUNC_LEN];
time_t Time;
struct tm *Tm;

   FUNC("_SPDXLogString");
   if (LoggedErrors == FALSE)
      return;

   Handle = open( ERR_FILE, O_BINARY|O_CREAT|O_RDWR, S_IREAD|S_IWRITE);

   if (Handle > 0)
   {
      lseek( Handle, 0L, SEEK_END );

      Time = time( NULL );
      Tm = localtime( &Time );

      sprintf( TempStr, "[When] %02d/%02d/%02d %02d:%02d:%02d\n",
              Tm->tm_mon, Tm->tm_mday, Tm->tm_year,
              Tm->tm_hour, Tm->tm_min, Tm->tm_sec );
      write( Handle, TempStr, strlen(TempStr) );

      if (SPDXGetDebugState() & DEBUG_FUNCS)
      {
         sprintf( TempStr, "[Loct] %s [%s|%d]\n", Function, File, Line );
         write( Handle, TempStr, strlen(TempStr) );
      }

      if (Notes)
      {
         sprintf( TempStr, "[Info] %s\n", Notes );
         write( Handle, TempStr, strlen(TempStr) );
      }
      write( Handle, "-------------------------------------------------------------------------------\n", 80 );
   }
   else
   {
      SPDXFatalError( LE_NOERROPEN, "Unable to write errors out to error file" );
   }

   close( Handle );
}

/*----------------------------------------------------------------------------
  -   [ERRORS.C    ] - End Of File                                           -
  ----------------------------------------------------------------------------*/
