// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [SPANDEX.H     ] - Spandex's header file                               -
  -                                                                          -
  ----------------------------------------------------------------------------*/

//----------------------------------------------------------------------------
// Configuration

#include "config.h"

//----------------------------------------------------------------------------
// System-dependant includes

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef _OS_DOS
#include <conio.h>
#include <io.h>
#endif

#ifdef _OS_LINUX
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#endif

//----------------------------------------------------------------------------
// Spandex-specific includes

#include "sdxdefs.h"
#include "sdxtypes.h"
#include "sdxdecl.h"
#include "sdxint86.h"
#include "sdxmacro.h"
#include "sdxextrn.h"
#include "sdxfixed.h"

#include "sdxmisc.h"

/*----------------------------------------------------------------------------
  -   [SPANDEX.H     ] - End Of File                                         -
  ----------------------------------------------------------------------------*/
