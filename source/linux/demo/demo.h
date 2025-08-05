// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [DEMO.H      ] - Header file for the demo                              -
  -                                                                          -
  ----------------------------------------------------------------------------*/

// The application should usually set up it's own error codes.. starting at 0

enum
{
   E_NONE = 0,
   E_UNKNOWN
};

/*----------------------------------------------------------------------------*/

#define  USD_FILE       "demo.usd"
#define  MAP_FILE       "demo.map"
#define  BUM_FILE       "demo.bump"

#define  COPYRIGHTS

/*----------------------------------------------------------------------------*/

void  PrintCopyright( void );
void  PrintUsage( void );
FLT   RenderFrame( OBJ *Universe );
INT   GetCommandLineParms( INT argc, CHR *argv[] );
WRD   InitSystem();
void  UninitSystem( OBJ *Universe );
void  CaptureImage( void );
void  ShowPalette();
void  Rotate( OBJ *Universe );

/*----------------------------------------------------------------------------
  -   [DEMO.H      ] - End Of File                                           -
  ----------------------------------------------------------------------------*/
