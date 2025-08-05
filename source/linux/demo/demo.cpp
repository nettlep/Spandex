// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [DEMO.C      ] - Demo's main file                                      -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "spandex.h"
#include "demo.h"
#include "xwin.h"

/*----------------------------------------------------------------------------*/
// Variables needed for Spandex

static   OBJ   *Universe = NULL;                // Top-most object in hierarchy

/*----------------------------------------------------------------------------*/
// Locals

static   VSCR  VScreen, A_VScreen;              // Virtual screens
static   PAL   MapPalette;                      // Palette from the MAP file
static   INT   windowLeft = 100, windowRight = 100, BorderSize = 3;
static   UINT  windowWidth = 256, windowHeight = 256;
static   INT   SleepTime = 0;
static   char  MyDirectory[256], MapFile[256], BumFile[256], UsdFile[256];

/*----------------------------------------------------------------------------*/
// Demo's Flags

static   INT   BumpMapping = FALSE;             // Pretty self-explanatory...
static   INT   AntiAlias = 0;                   // This is rather sloooow
static   INT   SmoothFilter = 0;                // This is rather not as slow

/*----------------------------------------------------------------------------*/

INT   main( INT argc, CHR *argv[] )
{
INT   RetCode;

   FUNC("main");

   // ---------------------- REMOVE THE STDIO BUFFERING ----------------------

   setbuf(stdout, 0);

   // ----------------------------- LEGAL STUFFS -----------------------------

   PrintCopyright();

   // ----------------------------- MISC. STUFFS -----------------------------

   memset(MyDirectory, 0, 256);
   strncpy(MyDirectory, argv[0], 255);

   char *temp = strrchr(MyDirectory, '/');

   if (temp)
   {
      temp[1] = '\0';
   }
   else
   {
      strcpy(MyDirectory, "./");
   }

   strcpy(MapFile, MyDirectory);   strcat(MapFile, MAP_FILE);
   strcpy(BumFile, MyDirectory);   strcat(BumFile, BUM_FILE);
   strcpy(UsdFile, MyDirectory);   strcat(UsdFile, USD_FILE);

   // ------------------------ PARSE THE COMMAND-LINE ------------------------

   if (!GetCommandLineParms( argc, argv ))
      return 1;

   // ---------------------- INIT THE SPANDEX SUBSYSTEM ----------------------

   RetCode = InitSystem();

   if (RetCode != E_NONE)
   {
      // [We can't call any of the error routines until Spandex has initialized]

      printf( "Init failure:\n   %s\n", SPDXGetErrorString(RetCode) );
      return 1;
   }

   // ---------------------------- INIT THE GUI ------------------------------

   cXWin	window;
   if (!window.create(windowLeft, windowRight, windowWidth, windowHeight, BorderSize, "SpandeX")) return 1;
   if (!window.preparePalette((char *) MapPalette.Colors)) return 1;

   // ---------------------------- RENDER A FRAME ----------------------------

   printf( "Rendering..." );

   int	x, y;
   unsigned int	w, h, ow = w, oh = h;
   window.getGeometry(x, y, ow, oh);

   while(1)
   {
      RenderFrame( Universe );

      window.getGeometry(x, y, w, h);

      if (w != ow || h != oh)
      {
         ow = w; oh = h;
         if (SPDXInitVirtualScreen( w, h, &VScreen ) != LE_NONE) return 0;
         SPDXSetCurrentVScreen( &VScreen );
      }
      else
      {
         if (!window.showImage((char *) VScreen.DrawScreen, (char *) MapPalette.Colors)) break;
         Rotate(Universe);
         usleep(SleepTime);
      }
   }

   printf( "Done\n" );

   // ----------- CLOSE DOWN THE SUBSYSTEM AND RETURN TO TEXT MODE -----------

   UninitSystem( Universe );

   return 0;
}

/*----------------------------------------------------------------------------*/

void  PrintCopyright( )
{
#ifdef COPYRIGHTS

   FUNC("PrintCopyright");

   printf("Copyright 1997 Paul D. Nettle, All rights reserved.\n");

#endif
}

/*----------------------------------------------------------------------------*/

void  PrintUsage( )
{
   FUNC("PrintUsage");

   printf( "\n" );
   printf( "Usage:  DEMO [options]\n\n" );
   printf( "       [-B nn]  Border size\n" );
   printf( "          [-F]  Enable smooth filtering\n" );
   printf( "  [-geom geom]  Like you'd expect it to work\n" );
   printf( "     [-? | -H]  Get this help\n" );
   printf( "          [-M]  Enable Multi-sample anti-aliasing\n" );
   printf( "       [-S nn]  Set the sleep time to nn microseconds\n" );
}

/*----------------------------------------------------------------------------*/

FLT      RenderFrame( OBJ *Universe )
{
OBJ      *Camera;

   FUNC("DoAnimation");

   // ----------------------- LOCATE THE CAMERA OBJECT -----------------------

   Camera = SPDXFindObjectByNameLen( "camera", Universe, 6, FALSE );

   if (!Camera)
      SPDXFatalError( LE_RANGE, "Unable to find the 'Camera' object" );

   // -------------------------------- RENDER --------------------------------

   SPDXRenderHierarchy( Universe, Camera, TRUE, FALSE, AntiAlias,
                        SmoothFilter, FALSE, &VScreen, &A_VScreen );

   return 0;
}

/*----------------------------------------------------------------------------*/

INT   GetCommandLineParms( INT argc, CHR *argv[] )
{
INT   i;

   FUNC("GetCommandLineParms");

   for( i = 1; i < argc; i++ )
   {
      if (argv[i][0] == '-' || argv[i][0] == '/' )
      {
         switch( toupper( argv[i][1] ) )
         {
            case 'B':
               BorderSize = atoi(argv[++i]);
               break;

            case 'F':
               SmoothFilter = TRUE;
               break;

            case 'G':
               XParseGeometry(argv[++i], &windowLeft, &windowRight, &windowWidth, &windowHeight);
               break;

            case 'M':
               AntiAlias = TRUE;
               break;

            case '?':
            case 'H':
               PrintUsage( );
               return 0;

            case 'S':
               SleepTime = atoi(argv[++i]);
               break;

            default:
               printf( "Unknown parameter [%s]\n", argv[i] );
               PrintUsage( );
               return 0;
         }
      }
   }

   return 1;
}   

/*----------------------------------------------------------------------------*/

WRD   InitSystem()
{
INT   RetCode, Handle;
RGB   Color;

   FUNC("InitSystem");

   // -------------------------- ENABLE THE LOGGING --------------------------

   SPDXSetFatalErrors( TRUE );
   SPDXSetLoggedErrors( TRUE );
   SPDXSetLoggedStrings( TRUE );

   // --------------------------- SPANDEX SERVICES ---------------------------

   SPDXOpenServices();

   // -------------------------- INITIALIZE SPANDEX --------------------------

   RetCode = SPDXInit( );

   if (RetCode != LE_NONE)
      return RetCode;

   // --------------- LOAD SOME TEXTURES FROM THE "STUFF" FILE ---------------

   printf( "Loading textures..." );

   RetCode = SPDXSelectRawMapFile( MapFile, &MapPalette );

   if (RetCode != E_NONE )
      SPDXFatalError( RetCode, "Unable to load MAP file" );
                            
   printf( "Done.\n" );

   // ------------------- LOAD A BUMPMAP FILE IF IT EXISTS -------------------

   Handle = SPDXOpen(BumFile, O_BINARY|O_RDONLY, 0 );

   if (Handle > 0)
   {
      printf( "Loading bump maps..." );

      RetCode = SPDXSelectRawBumFile( BumFile );

      if (RetCode != E_NONE )
         SPDXFatalError( RetCode, "Unable to load BUM file" );
                            
      SPDXClose(Handle);

      BumpMapping = TRUE;

      printf( "Done.\n" );
   }

   // -------------------------- LOAD SOME GEOMETRY --------------------------

   printf( "Loading geometry..." );

   RetCode = SPDXReadRawUSD( UsdFile, &Universe, &MapPalette, 0.75 );

   if (RetCode != E_NONE )
      SPDXFatalError( RetCode, "Unable to load USD file" );

   printf( "Done.\n" );

   // ----------------------- SET UP THE GRAPHICS MODE -----------------------

   SPDXSetGlobalResolution( windowWidth, windowHeight );
   SPDXGlobals.AspectX = 1.0;
   SPDXGlobals.AspectY = 1.0;
   SPDXGlobals.TotalColors = 256;  // 8-bit mode only

   // ---------------------- INIT THE VIRTUAL SCREEN(S) ----------------------

   if (AntiAlias)
   {
      RetCode = SPDXInitVirtualScreen( windowWidth<<1, windowHeight<<1, &VScreen );

      if (RetCode != LE_NONE)
         return RetCode;

      RetCode = SPDXInitVirtualScreen( windowWidth, windowHeight, &A_VScreen );

      if (RetCode != LE_NONE)
         return RetCode;
   }
   else
   {
      RetCode = SPDXInitVirtualScreen( windowWidth, windowHeight, &VScreen );

      if (RetCode != LE_NONE)
         return RetCode;
   }

   SPDXSetCurrentVScreen( &VScreen );

   // ------------------- SET BACKGROUND TO A MEDIUM COLOR -------------------

   Color.Red = Color.Blue = Color.Green = 55;
   SPDXSetBackgroundColor( &Color );

   return E_NONE;
}

/*----------------------------------------------------------------------------*/

void  UninitSystem( OBJ *Universe )
{
   FUNC("UninitSystem");

   // ------------------------------ CLEANUP... ------------------------------

   SPDXCleanupVirtualScreen( &VScreen );

   if (AntiAlias)
      SPDXCleanupVirtualScreen( &A_VScreen );

   SPDXUninit( Universe );
   SPDXCloseServices();
}

/*----------------------------------------------------------------------------*/

void  CaptureImage( )
{
INT   i, RetCode, ResX, ResY, ImageSize;
UCHR  *Ptr;
CHR   FileName[20];
IMAGE *Image;
static CurImageNumber = 0;

   FUNC("CaptureImage");

   Ptr = SPDXGetVirtualBuffer( &VScreen );
   SPDXGetVirtualiResolution( &ResX, &ResY, &VScreen );
   Image = SPDXCreateImage( ResX, ResY );
   ImageSize = ResX * ResY * 3;

   if (Image)
   {
      for( i = 0; i < ImageSize; i += 3)
      {
         Image->Buffer[i + 0] = MapPalette.Colors[*Ptr    ].Red   << 2;
         Image->Buffer[i + 1] = MapPalette.Colors[*Ptr    ].Green << 2;
         Image->Buffer[i + 2] = MapPalette.Colors[*(Ptr++)].Blue  << 2;
      }

      sprintf( FileName, "capt_%03d.gif", CurImageNumber++);

      RetCode = SPDXWriteImage( FileName, Image, 0 );

      if (RetCode != LE_NONE)
         SPDXLogError( RetCode, "Unable to save capture file" );

      SPDXDestroyImage( &Image );
   }
}

/*----------------------------------------------------------------------------*/

void  ShowPalette( )
{
INT   x, y;
UCHR  *Ptr;

   FUNC("ShowPalette");

   Ptr = SPDXGlobals.PalTab;

   for( y = 0; y < MAP_SHADES; y++ )
      for( x = 0; x < MAP_COLORS; x++ )
         SPDXSetVirtualPixel(x, y, Ptr[y*MAP_COLORS+x], &VScreen);

   for( y = MAP_SHADES; y < MAP_SHADES+10; y++ )
      for( x = 0; x < MAP_COLORS; x++ )
         SPDXSetVirtualPixel(x, y, x, &VScreen);
}

/*----------------------------------------------------------------------------*/

void  Rotate( OBJ *Universe )
{
static float CounterX, CounterY, CounterZ;
       OBJ   *Object;
static ROT   Orientation;

   FUNC("Rotate");

   CounterX += 0.4;
   CounterY += 0.8;
   CounterZ += 0.2;

   Object = SPDXGetChild( Universe, 1 );
   Orientation.x += (INT) (sin(CounterX) * 180);
   Orientation.y += (INT) (cos(CounterY) * 180);
   Orientation.z += (INT) (sin(CounterZ) * 180);
   SPDXRotateObject( Object, &Orientation );
}


/*----------------------------------------------------------------------------
  -   [DEMO.C      ] - End Of File                                           -
  ----------------------------------------------------------------------------*/
