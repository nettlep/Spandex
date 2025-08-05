// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [DEMO.C      ] - Demo's main file                                      ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#include <ctype.h>
#include <dos.h>
#include <io.h>
#include <mem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include <time.h>

#include <spandex.h>
#include "DEMO.H"

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
// Variables needed for Spandex

static   OBJ   *Universe = NULL;                // Top-most object in hierarchy
static   VSCR  VScreen, A_VScreen;              // Virtual screens
static   PAL   MapPalette;                      // Palette from the MAP file

static   INT   MouseMax = 10000;
static   INT   MouseCenter = 10000 >> 1;
static   INT   FileOnly = FALSE, FileResX = 1024, FileResY = 1024;

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
// Demo's Flags

static   INT   BumpMapping = FALSE;             // If a bump-map file was found
static   INT   ConstRot = 1;                    // Constant rotation interface
static   INT   TotalFrames = 0;                 // For timing
static   INT   AntiAlias = 0;                   // This is rather sloooow
static   INT   SmoothFilter = 0;                // This is rather not as slow
static   INT   VideoMode = VESA_MODE_200_256;   // Default Video Mode
static   INT   SquarePixel = FALSE;             // Square pixel mode flag
static   INT   AutoMove = FALSE;                // Just a rotating object demo
static   INT   Timeit = FALSE;                  // Timing flag

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   main( INT argc, CHR *argv[] )
{
INT   RetCode;
CHR   TempStr[80];
FLT   FramesPerSecond, ElapsedTime;
STFP  StuffPack;
time_t StartTime;

   FUNC("main");

   // ---------------------- REMOVE THE STDIO BUFFERING ----------------------

   setbuf(stdout, 0);

   // ----------------------------- LEGAL STUFFS -----------------------------

   PrintCopyright();

   // ------------------------ PARSE THE COMMAND-LINE ------------------------

   if (!GetCommandLineParms( argc, argv ))
      return 1;

   // ---------------------- INIT THE SPANDEX SUBSYSTEM ----------------------

   RetCode = InitSystem(&StuffPack);

   if (RetCode != E_NONE)
   {
      // [We can't call any of the error routines until Spandex has initialized]

      printf( "Init failure:\n   %s\n", SPDXGetErrorString(RetCode) );
      return 1;
   }

   // ----------------- WAIT FOR AN EXACT ONE-SECOND BOUNDRY -----------------

   if (Timeit)
   {
      StartTime = time(NULL);

      ElapsedTime = difftime( time(NULL), StartTime );

      while (difftime( time(NULL), StartTime ) == ElapsedTime);

      ElapsedTime = difftime( time(NULL), StartTime );

      while (difftime( time(NULL), StartTime ) == ElapsedTime);

      StartTime = time(NULL);
   }

   // --------------------------- ANIMATE THE DEMO ---------------------------

   FramesPerSecond = DoAnimation( Universe, AutoMove, Timeit, StartTime );

   // ----------- CLOSE DOWN THE SUBSYSTEM AND RETURN TO TEXT MODE -----------

   UninitSystem( Universe, &StuffPack );

   // -------------------- DISPLAY THE TIMING INFORMATION --------------------

   if (Timeit)
   {
      sprintf( TempStr, "Frames Per Sec.:  %f", FramesPerSecond );
      printf("%s\n", TempStr );
      SPDXLogString( TempStr );
   }

   return 0;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  PrintCopyright( )
{
#ifdef COPYRIGHTS

   FUNC("PrintCopyright");

   printf("Copyright 1997 Paul D. Nettle, All rights reserved.\n");

#endif
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  PrintUsage( )
{
   FUNC("PrintUsage");

   printf( "\n" );
   printf( "Usage:  DEMO [USDFILE] [MAPFILE] [options]\n\n" );
   printf( "          [-A]  Auto Rotations\n" );
   printf( "          [-F]  Enable smooth filtering\n" );
   printf( "         [-G#]  Set graphics mode - valid values for `#' are:\n" );
   printf( "                   0 = 320x200x256   (MCGA) -> default\n" );
   printf( "                   1 = 640x400x256   (VESA)\n" );
   printf( "                   2 = 640x480x256   (VESA)\n" );
   printf( "                   4 = 800x600x256   (VESA)\n" );
   printf( "                   6 = 1024x768x256  (VESA)\n" );
   printf( "                   8 = 1280x1024x256 (VESA)\n" );
   printf( "     [-? | -H]  Get this help\n" );
   printf( "          [-M]  Enable Multi-sample anti-aliasing\n" );
   printf( "          [-S]  Square-pixels for 320x200 mode\n" );
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

FLT      DoAnimation( OBJ *Universe, INT AutoMove, INT Timeit, time_t StartTime )
{
INT      i = 0;
OBJ      *Object;
OBJ      *Camera;
ROT      Orientation;
FLT      ElapsedTime;

   FUNC("DoAnimation");

   // --------- LOCATE THE FIRST NON-CAMERA  OBJECT FOR MANIPULATION ---------
   do
   {
      Object = SPDXGetChild( Universe, i++ );

      if (!Object)
         SPDXFatalError( LE_RANGE, "There are no objects" );
   }
   while (!strnicmp(Object->Name, "camera", 6));

   // ----------------------- LOCATE THE CAMERA OBJECT -----------------------

   Camera = SPDXFindObjectByNameLen( "camera", Universe, 6, FALSE );

   if (!Camera)
      SPDXFatalError( LE_RANGE, "Unable to find the 'Camera' object" );

   // ---------------------- SET UP THE STEADY ROTATION ----------------------

   if (AutoMove)
   {
      Orientation.x = 500.0;
      Orientation.y = 500.0;
      Orientation.z = 1000.0;

      Orientation.x = 0.0;
      Orientation.y = 0.0;
      Orientation.z = 10.0;
      SPDXSetObjectSteadyOrientation( Object, &Orientation );
   }

   // ------------------------------ MAIN  LOOP ------------------------------

   FOREVER
   {
      DrawTopLevelObjects(Universe, Camera );

      if (FileOnly) break;

      TotalFrames++;

      if (Timeit)
      {
         if (difftime( time(NULL), StartTime ) > 10.0)
            break;
      }
      else
      {
         if (!HandleKeys( Object, Camera ))
            break;

         if (!HandleMouse( Universe ))
            break;
      }
   }

   // ---------------------- CAPTURE THE CURRENT TIMING ----------------------

   if (Timeit)
   {
      ElapsedTime = difftime( time(NULL), StartTime );
      return TotalFrames/ElapsedTime;
   }

   return 0;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

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
            case 'G':
               switch (argv[i][2])
               {
                  case '0':
                     VideoMode = VESA_MODE_200_256;
                     break;

                  case '1':
                     VideoMode = VESA_MODE_400_256;
                     break;

                  case '2':
                     VideoMode = VESA_MODE_480_256;
                     break;

                  case '4':
                     VideoMode = VESA_MODE_600_256;
                     break;

                  case '6':
                     VideoMode = VESA_MODE_768_256;
                     break;

                  case '8':
                     VideoMode = VESA_MODE_1024_256;
                     break;

                  default:
                     printf( "Using default MCGA mode\n" );
                     VideoMode = VESA_MODE_200_256;
                     break;
               }
               break;

            case 'A':
               AutoMove = TRUE;
               break;

            case 'F':
               SmoothFilter = TRUE;
               break;

            case 'O':
               FileOnly = TRUE;
               break;

            case 'M':
               AntiAlias = TRUE;
               break;

            case 'S':
               SquarePixel = TRUE;
               break;

            case 'T':
               Timeit = TRUE;
               break;

            case '?':
            case 'H':
               PrintUsage( );
               return 0;

            default:
               printf( "Unknown parameter [%s]\n", argv[i] );
               PrintUsage( );
               return 0;
         }
      }
   }

   return 1;
}   

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

WRD   InitSystem( STFP *StuffPack )
{
INT   RetCode, Handle, ResX, ResY;
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

   // ----------------------- PREPARE THE "STUFF" FILE -----------------------

   RetCode = SPDXSelectSTF( STF_FILE, StuffPack );

   if (RetCode != LE_NONE)
      return RetCode;

   // --------------- LOAD SOME TEXTURES FROM THE "STUFF" FILE ---------------

   printf( "Loading textures..." );

   Handle = SPDXOpenSTF(MAP_FILE, StuffPack );

   if (Handle <= 0)
      SPDXFatalError( LE_NOMAPFILE, "Unable to open MAP file" );

   RetCode = SPDXSelectCompressedMapFile( &MapPalette, Handle );

   if (RetCode != E_NONE )
      SPDXFatalError( RetCode, "Unable to load MAP file" );
                            
   SPDXCloseSTF(Handle);

   printf( "Done.\n" );

   // ------------------- LOAD A BUMPMAP FILE IF IT EXISTS -------------------

   Handle = SPDXOpen(BUMP_FILE, O_BINARY|O_RDONLY, 0 );

   if (Handle > 0)
   {
      printf( "Loading bump maps..." );

      RetCode = SPDXSelectRawBumFile( BUMP_FILE );

      if (RetCode != E_NONE )
         SPDXFatalError( RetCode, "Unable to load BUM file" );
                            
      SPDXClose(Handle);

      BumpMapping = TRUE;

      printf( "Done.\n" );
   }

   // --------------- LOAD SOME GEOMETRY FROM THE "STUFF" FILE ---------------

   printf( "Loading geometry..." );

   Handle = SPDXOpenSTF(USD_FILE, StuffPack );

   if (Handle <= 0)
      SPDXFatalError( LE_NOMAPFILE, "Unable to open MAP file" );

   RetCode = SPDXReadCompressedUSD( &Universe, Handle, &MapPalette, 0.75 );

   if (RetCode != E_NONE )
      SPDXFatalError( RetCode, "Unable to load USD file" );

   SPDXCloseSTF(Handle);

   printf( "Done.\n" );

   // ----------------------- SET UP THE GRAPHICS MODE -----------------------

   if (!FileOnly)
   {
      SPDXSetMode( VideoMode, TRUE, SquarePixel );
   }
   else
   {
      SPDXSetGlobalResolution( FileResX, FileResY );
      SPDXGlobals.AspectX = 1.0;
      SPDXGlobals.AspectY = 1.0;
      SPDXGlobals.TotalColors = 256;  // 8-bit mode only
   }

   SPDXGetGlobalResolution( &ResX, &ResY );

   // ---------------------- INIT THE VIRTUAL SCREEN(S) ----------------------

   if (AntiAlias)
   {
      RetCode = SPDXInitVirtualScreen( ResX<<1, ResY<<1, &VScreen );

      if (RetCode != LE_NONE)
         return RetCode;

      RetCode = SPDXInitVirtualScreen( ResX, ResY, &A_VScreen );

      if (RetCode != LE_NONE)
         return RetCode;
   }
   else
   {
      RetCode = SPDXInitVirtualScreen( ResX, ResY, &VScreen );

      if (RetCode != LE_NONE)
         return RetCode;
   }

   SPDXSetCurrentVScreen( &VScreen );

   // --------------------------- SET THE  PALETTE ---------------------------

   if (!FileOnly)
   {
      SPDXSetPalette( 0, TOTAL_COLORS, &MapPalette );

      // ------ MUST WAIT UNTIL AFTER THE  MODE HAS BEEN SET TO INIT MOUSE ------

      RetCode = SPDXInitMouse( );
   
      if (RetCode != E_NONE)
         return RetCode;

      SPDXSetMouseRange( 0, MouseMax, 0, MouseMax );
      SPDXSetMousePosition( MouseCenter, MouseCenter);
   }

   // ------------------- SET BACKGROUND TO A MEDIUM COLOR -------------------

   Color.Red = Color.Blue = Color.Green = 55;
   SPDXSetBackgroundColor( &Color );

   return E_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  UninitSystem( OBJ *Universe, STFP *StuffPack )
{
   FUNC("UninitSystem");

   // ------------------------------ CLEANUP... ------------------------------

   SPDXCleanupVirtualScreen( &VScreen );

   if (AntiAlias)
      SPDXCleanupVirtualScreen( &A_VScreen );

   SPDXReleaseSTF( StuffPack );

   if (!FileOnly)
      SPDXSetTextMode();

   SPDXUninit( Universe );
   SPDXCloseServices();
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  DrawTopLevelObjects( OBJ *Universe, OBJ *Camera )
{
   FUNC("DrawTopLevelObjects");

   if (FileOnly)
   {
      SPDXRenderHierarchy( Universe, Camera, TRUE, FALSE, AntiAlias,
                           SmoothFilter, FALSE, &VScreen, &A_VScreen );
      CaptureImage();
   }
   else
   {
      SPDXRenderHierarchy( Universe, Camera, TRUE, TRUE, AntiAlias,
                           SmoothFilter, FALSE, &VScreen, &A_VScreen );

      //SPDXRenderHierarchy( Universe, Camera, TRUE, FALSE, AntiAlias,
      //                     SmoothFilter, FALSE, &VScreen, &A_VScreen );
      //
      //SPDXCopyToScreenClipped( VScreen.DrawScreen, 0, 240 - 100, 640, 240 + 100 );
   }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT      HandleKeys( OBJ *Object, OBJ *Camera )
{
static   INT   Key, MoveLights = 0, i;
static   FLT   ZRotVal = 0, XRotVal = 0, YRotVal = 0, Lense = 35.0;
static   ROT   Orientation;

   FUNC("HandleKeys");

   if (kbhit())
   {
      Key = getch();

      switch( toupper(Key) )
      {
         case 27:
            return 0;

         case 'Q':
            ConstRot = !ConstRot;
            break;

         case 'C':
            CaptureImage();
            break;

         case 'P':
            ShowPalette();
            break;

         case 'L':
            MoveLights = !MoveLights;
            break;

         case 'M':
            SPDXDumpMemLog();
            break;

         case 'S':
            if (BumpMapping == FALSE)
               switch(SPDXGetObjectShadeModel( Object ))
               {
                  case AMBIENT:
                     SPDXSetObjectShadeModel( Object, LAMBERT );
                     break;
                  case LAMBERT:
                     SPDXSetObjectShadeModel( Object, GOURAUD );
                     break;
                  case GOURAUD:
                     SPDXSetObjectShadeModel( Object, PHONG );
                     break;
                  default:
                     SPDXSetObjectShadeModel( Object, AMBIENT );
                     break;
               }
            else
               switch(SPDXGetObjectShadeModel( Object ))
               {
                  case AMBIENT:
                     SPDXSetObjectShadeModel( Object, LAMBERT );
                     break;
                  case LAMBERT:
                     SPDXSetObjectShadeModel( Object, GOURAUD );
                     break;
                  case GOURAUD:
                     SPDXSetObjectShadeModel( Object, PHONG );
                     break;
                  case PHONG:
                     SPDXSetObjectShadeModel( Object, BUMP );
                     break;
                  default:
                     SPDXSetObjectShadeModel( Object, AMBIENT );
                     break;
               }
            break;

         case '+':
            Lense = SPDXGetObjectLense(Camera) + 2.0;
            SPDXSetObjectLense( Camera, Lense );
            break;

         case '-':
            Lense = SPDXGetObjectLense(Camera) - 2.0;
            SPDXSetObjectLense( Camera, Lense );
            break;

         case '1':
            VScreen.ClipMinX--;
            if (VScreen.ClipMinX < 0) VScreen.ClipMinX = 0;
            break;

         case '2':
            VScreen.ClipMinX++;
            break;

         case '3':
            VScreen.ClipMaxX--;
            if (VScreen.ClipMaxX < 0) VScreen.ClipMaxX = 0;
            break;

         case '4':
            VScreen.ClipMaxX++;
            break;

         case '5':
            VScreen.ClipMinY--;
            if (VScreen.ClipMinY < 0) VScreen.ClipMinY = 0;
            break;

         case '6':
            VScreen.ClipMinY++;
            break;

         case '7':
            VScreen.ClipMaxY--;
            if (VScreen.ClipMaxY < 0) VScreen.ClipMaxY = 0;
            break;

         case '8':
            VScreen.ClipMaxY++;
            break;

         case '>':
            SPDXSetNearZ( SPDXGetNearZ() + 1 );
            break;

         case '<':
            SPDXSetNearZ( SPDXGetNearZ() - 1 );
            break;

         case 'R':
            Orientation.x = 0;
            Orientation.y = 0;
            Orientation.z = -1;
            SPDXRotateObject( Object, &Orientation );
            break;

         case 'T':
            Orientation.x = 0;
            Orientation.y = 0;
            Orientation.z = 1;
            SPDXRotateObject( Object, &Orientation );
            break;

         case 'X':
            for( i = 0; i < SPDXGlobals.MatCount; i++ )
            {
               switch (SPDXGlobals.MatList[i].Surface)
               {
                  case SOLID:
                     SPDXGlobals.MatList[i].Surface = TEXTURED;
                     break;

                  case TEXTURED:
                     SPDXGlobals.MatList[i].Surface = ENVMAP;
                     break;

                  default:
                     SPDXGlobals.MatList[i].Surface = SOLID;
                     break;
               }
            }
            break;
      }
   }

   if (MoveLights)
   {
      ZRotVal += 320;

      if (ZRotVal >= ROT_POINTS) ZRotVal -= ROT_POINTS;

      XRotVal += 240;

      if (XRotVal >= ROT_POINTS) XRotVal -= ROT_POINTS;

      YRotVal += 160;

      if (YRotVal >= ROT_POINTS) YRotVal -= ROT_POINTS;

      // NOTE:
      //
      // Really shouldn't use these directly -- the proper way is to
      // find the light by name, and manipulate it that way, but for
      // the purpose of this demo, I didn't want to have to make sure
      // that all my lights were named the same for any object loaded
      // into the demo.

      SPDXGlobals.LightList[0]->LLoc.x = Sine(ZRotVal) * 3400;
      SPDXGlobals.LightList[0]->LLoc.y = Cosine(YRotVal) * 3400;
      SPDXGlobals.LightList[0]->LLoc.z = Sine(XRotVal) * 3400;
   }
   return 1;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   HandleMouse( OBJ *Universe )
{
INT   MouseX, MouseY, LeftButton, RightButton;
OBJ   *Object, *Camera;
VEC   Trans;
static ROT   Orientation;

   FUNC("HandleMouse");

   Object = SPDXGetChild( Universe, 1 );
   Camera = SPDXFindObjectByName( "Camera", Universe, FALSE );

   SPDXReadMouse( &MouseX, &MouseY, &LeftButton, &RightButton );

   if (!LeftButton && !RightButton)
   {
      Orientation.x = (MouseCenter-MouseY);
      Orientation.y = (MouseX-MouseCenter);
      Orientation.z = 0;
      SPDXRotateObject( Object, &Orientation );

      Orientation.x = (MouseCenter-MouseY);
      Orientation.y = (MouseX-MouseCenter);
      Orientation.z = 0;
      SPDXRotateObject( Object, &Orientation );

      if (!ConstRot)
         SPDXSetMousePosition( MouseCenter, MouseCenter );
   }
   else if (LeftButton && !RightButton)
   {
      Orientation.x = 0;
      Orientation.y = 0;
      Orientation.z = (MouseX-MouseCenter);
      SPDXRotateObject( Object, &Orientation );

      Trans.dx = 0;
      Trans.dy = 0;
      Trans.dz = MouseCenter-MouseY;

      SPDXTranslateObject( Object, &Trans );
      SPDXSetMousePosition( MouseCenter, MouseCenter );
   }
   else if (RightButton && !LeftButton)
   {
      Trans.dx = MouseX-MouseCenter;
      Trans.dy = MouseCenter-MouseY;
      Trans.dz = 0;

      SPDXTranslateObject( Object, &Trans );
      SPDXSetMousePosition( MouseCenter, MouseCenter );
   }
   else if (RightButton && LeftButton)
      return 0;

   return 1;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  CaptureImage( )
{
INT   i, RetCode, ResX, ResY, ImageSize;
CHR   *Ptr, FileName[20];
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

      sprintf( FileName, "CAPT_%03d.TGA", CurImageNumber++);

      RetCode = SPDXWriteImage( FileName, Image, 0 );

      if (RetCode != LE_NONE)
         SPDXLogError( RetCode, "Unable to save capture file" );

      SPDXDestroyImage( &Image );
   }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  ShowPalette( )
{
INT   x, y;
CHR   *Ptr;

   FUNC("ShowPalette");

   Ptr = SPDXGlobals.PalTab;

   for( y = 0; y < MAP_SHADES; y++ )
      for( x = 0; x < MAP_COLORS; x++ )
         SPDXSetScreenPixel(x, y, Ptr[y*MAP_COLORS+x]);

   for( y = MAP_SHADES; y < MAP_SHADES+10; y++ )
      for( x = 0; x < MAP_COLORS; x++ )
         SPDXSetScreenPixel(x, y, x);

   if (!getch()) getch();
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [DEMO.C      ] - End Of File                                           ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
