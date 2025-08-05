// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [DEMO.H      ] - Header file for the demo                              ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

// The application should usually set up it's own error codes.. starting at 0

enum
{
   E_NONE = 0,
   E_UNKNOWN
};

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#define  USD_FILE       "DEMO.USD"
#define  MAP_FILE       "DEMO.MAP"
#define  BUMP_FILE      "DEMO.BUM"

#define  COPYRIGHTS

#ifdef __DEBUG__
#define  STF_FILE       "DEMO.STF"
#else
#define  STF_FILE       "DEMO.EXE"
#endif

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  PrintCopyright( void );
void  PrintUsage( void );
FLT   DoAnimation( OBJ *Universe, INT AutoMove, INT TimeIt, time_t StartTime );
INT   GetCommandLineParms( INT argc, CHR *argv[] );
WRD   InitSystem( STFP *StuffPack );
void  UninitSystem( OBJ *Universe, STFP *StuffPack );
void  DrawTopLevelObjects( OBJ *Universe, OBJ *Camera );
INT   HandleKeys( OBJ *Object, OBJ *Camera );
INT   HandleMouse( OBJ *Universe );
void  CaptureImage( void );
void  ShowPalette( void );

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [DEMO.H      ] - End Of File                                           ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
