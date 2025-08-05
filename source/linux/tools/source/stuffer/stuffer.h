// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [STUFFER.H   ] - MakeMap's header file                                 -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#define  MAX_INFILES 3000

/*----------------------------------------------------------------------------*/

enum
{
   E_NONE = 0,
   E_BADPARMS,
   E_NOCREATE,
   E_NOREAD,
   E_NOWRITE,
   E_NOOPEN,
   E_NONAMES
};

/*----------------------------------------------------------------------------*/

void  GetNames( INT NameCount );
void  PrintCopyright( void );
void  ExitSystem( INT ErrorCode );
void  ErrorExit( INT ErrorCode );
void  PrintErrorString( INT ErrorCode );
void  PrintUsage( INT ErrorCode );
void  Convert24To8( IMAGE *Image );
INT   MyWrite( INT Handle, void *Buffer, INT Length );

/*----------------------------------------------------------------------------
  -   [STUFFER.H   ] - End Of File                                           -
  ----------------------------------------------------------------------------*/
