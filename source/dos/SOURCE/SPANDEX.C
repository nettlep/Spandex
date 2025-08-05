// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [SPANDEX.C   ] - SPANDEX library interface                             ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#include <stdlib.h>
#include <stdio.h>
#include <mem.h>
#include <i86.h>
#include <time.h>
#include <conio.h>

#include "spandex.h"

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static   LNG   ZeroMemory;

static   CHR   CopyrightMessage[160];
int      SPDXServicesOpened = 0;

#ifdef   U_E
INT      e_ar = FALSE;
INT      e_et;
time_t   e_st;
#endif

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXInit( )
{
INT   RetCode;

   FUNC("SPDXInit");
   SPDXMemCopyBYTE( &ZeroMemory, 0, sizeof( LNG ) );

#ifdef   U_E
   if (e_ar != FALSE)
   {
      SPDXGlobals.InitFlag = 0;
      SPDXSetTextMode();
      printf( "You must restart your application to reset the evaluation timer\n" );
      exit(-1);
   }
   else
   {
      e_st = time(NULL);
   }
#endif

   if (!SPDXServicesOpened)
      return LE_NOSERVICES;

   SPDXResetErrorFile();

   RetCode = SPDXInitGlobals( );

   if (RetCode != LE_NONE)
      return RetCode;

   SPDXInitMem( );

   RetCode = SPDXInitTables( );

   if (RetCode != LE_NONE)
      return RetCode;

   RetCode = SPDXInitScreen( );

   if (RetCode != LE_NONE)
      return RetCode;

   SPDXGlobals.InitFlag = INIT_VAL;

   if (SPDXGlobals.UseIRQ)
   {
      SPDXLogError( LE_NONE, "Using interrupts for page swapping" );
      SPDXInitializeInterrupts( );
   }

   RetCode = SPDXInitRenderer();

   if (RetCode != LE_NONE)
      return RetCode;

   // This must be last to tell caller what the VESA status is...
   RetCode = SPDXGetVESAInfo(&SPDXGlobals.VESAInfo);

   return RetCode;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXUninit( OBJ *Universe )
{
   FUNC("SPDXUninit");
   MLTIUninit();

   // Make sure this puppy isn't being re-entered...
   if (SPDXGlobals.InitFlag != INIT_VAL)
      return;
      
   SPDXGlobals.InitFlag = !INIT_VAL;

   if (memcmp( &ZeroMemory, 0, sizeof( LNG ) ))
      SPDXLogError( LE_NOMEM, "Overwrite of zero memory (NULL POINTER)" );

   SPDXCleanupGlobals();

   SPDXUninitRenderer();

   if (Universe)
   {
      SPDXCleanupObject( Universe );
      SPDXFree(Universe);
      Universe = NULL;
   }

   SPDXUninitMouse();
   SPDXUninitMem();
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

CHR   *SPDXGetCopyrightMessage( )
{
   FUNC("SPDXGetCopyrightMessage");
   sprintf( CopyrightMessage, "Spandex v%3.2f Copyright 1997 Paul D. Nettle.  All rights reserved.", SPDXGetVersion() );
   return CopyrightMessage;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

FLT   SPDXGetVersion( )
{
   FUNC("SPDXGetVersion");
   return SPANDEX_VERSION;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXOpenServices( )
{
   FUNC("SPDXOpenServices");
   printf( "%s\n", SPDXGetCopyrightMessage() );
   SPDXServicesOpened = 1;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXCloseServices( )
{
   FUNC("SPDXCloseServices");
   SPDXServicesOpened = 0;
   return;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [SPANDEX.C     ] - End Of File                                         ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
