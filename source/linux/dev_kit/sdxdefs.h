// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [SDXDEFS.H   ] - Definitions for SPANDEX library                       -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#define  SPANDEX_VERSION        1.0

/*----------------------------------------------------------------------------*/
// Setup for a 16 pixel perspective correct affine approximation

#define F_PERSP_SPANS (16.0)
#define I_PERSP_SPANS (16)
#define PERSP_SHIFT   (4)

/*----------------------------------------------------------------------------*/

#define  DIVIDE_SAFE          (FXD_ONE)

#define  ONE_OVER_VAL         (256.0)

#define  IO_BUFFER_SIZE       (1024)

#define  GENERIC_DOS_RAM_LEN  (64000)

#define  FOREVER              for(;;)

#define  DEF_NEAR_Z           (20.0)

#define  SPANDEX_DEBUG_FILE   "spandex.dbg"
#define  ERR_FILE             "spandex.log"
#define  MEM_FILE             "spandex.mem"

#define  DOS_INT              0x21

#define  MOUSE_INT            0x33

#define  MOUSE_INIT           0x0000
#define  MOUSE_RESET          0x0021
#define  MOUSE_READ_INF       0x0003
#define  MOUSE_SET_POS        0x0004
#define  MOUSE_SET_COL        0x0008
#define  MOUSE_SET_ROW        0x0007

#define  INT_VB               0x02
#define  VGA_STATUS_REGISTER  0x3C2
#define  CRT_STATUS_REGISTER  0x3DA
#define  VGA_ATTR_REGISTER    0x3C0
#define  OVERSCAN             0x11
#define  PALRESET             0x20

#define  INT_A00              0x20
#define  EOI                  0x20

#define  ROT_POINTS           (0xFFFF)
#define  SIN_TAB_BITS         (8)
#define  SIN_TAB_LEN          (1<<SIN_TAB_BITS)
#define  SIN_TAB_MASK         (0xff00)

#define  DEF_DELTA            1000

#define  DRAW_TRIS            0x01
#define  DRAW_QUADS           0x01
#define  DRAW_POINTS          0x02
#define  DRAW_ALL             0x03
#define  DRAW_NONE            0x00

#define  VESA_INT             0x10
#define  VIDEO_INT            0x10
#define  GET_VESA_INFO        0x00
#define  GET_SVGA_INFO        0x01
#define  SET_SVGA_MODE        0x02
#define  GET_SVGA_MODE        0x03
#define  SET_VID_WINDOW       0x05
#define  GET_VID_WINDOW       0x05

#define  TOTAL_FONT_CHARS     96
#define  FONT_CHAR_OFFSET     ' '

#define  TOTAL_COLORS         256

#define  DEF_NAME_LEN         20

#ifdef U_R
#define  DEF_FUNC_LEN         0
#else
#define  DEF_FUNC_LEN         255
#endif

#define  MAP_COLORS           TOTAL_COLORS
#define  MAP_SHADES           64

#define  PHONG_TAB_LEN        256

#define  USD_LINE_LENGTH      512
#define  USD_FILE_CHUNK       1024

#define  DEV_USE_COUNT        10

#define  MAX_LIGHTS           100

#define  BACKFACE_ANGLE       0

#define  DEF_STR_LEN          40

#define  INIT_VAL             0x1234

#define  MAX_TRANS_DEPTH      1000

#define  MAX_X_RES            1280
#define  MAX_Y_RES            1024

#define  HDR_BYTES            0xAA
#define  HDR_SIZE             4
#define  WRAP_SIZE            (HDR_SIZE*2)

#define  TRI_RESERVOIR_SIZE   4096
#define  VERTEX_RESERVOIR_SIZE (TRI_RESERVOIR_SIZE*2)

#define  AMBIENT              0x01
#define  LAMBERT              0x02
#define  GOURAUD              0x04
#define  PHONG                0x08
#define  BUMP                 0x10
#define  SOLID                0x20
#define  TEXTURED             0x40
#define  ENVMAP               0x80

/*----------------------------------------------------------------------------*/
// MAP FILE FLAGS

#define  MAP_FLAGS_TRANS      0x00000001
#define  MAP_FLAGS_COMP       0x00000002

/*----------------------------------------------------------------------------*/

#define  DEBUG_NONE           0
#define  DEBUG_BASIC          0x00000001
#define  DEBUG_MEM            0x00000002
#define  DEBUG_MEMTRACE       0x00000004
#define  DEBUG_FUNCS          0x00000008
#define  DEBUG_ALL            0xffffffff

/*----------------------------------------------------------------------------*/

enum { PARSE_STOP, PARSE_CONTINUE, PARSE_BRACE_BEG, PARSE_BRACE_END };
enum { TOP_DOWN, BOTTOM_UP };
enum { VIS_NO, VIS_YES, VIS_CLIPZ };

/*----------------------------------------------------------------------------*/

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/*----------------------------------------------------------------------------*/

#define  LE_NONE        0

enum
{
   LE_NOMEM = 400,
   LE_MEMLEAK,
   LE_NOMEMOFF,
   LE_NOTAVAIL,
   LE_NOCREATE,
   LE_VINOTAVAIL,
   LE_SINOTAVAIL,
   LE_MODENOTAVAIL,
   LE_GMODENOTAVAIL,
   LE_SWNOTAVAIL,
   LE_GWNOTAVAIL,
   LE_NOMAPFILE,
   LE_NOMAPREAD,
   LE_NOMAPSEEK,
   LE_WRONGMAP,
   LE_MAPNOTFOUND,
   LE_MATNOTFOUND,
   LE_NOSTFOPEN,
   LE_NOSTFREAD,
   LE_NOUSDOPEN,
   LE_NOUSDOPENINC,
   LE_USDSYNTAX,
   LE_RANGE,
   LE_NOERROPEN,
   LE_NOERRRESET,
   LE_VFAIL,
   LE_AFAIL,
   LE_NOMOUSE,
   LE_BADIMAGE,
   LE_BADTGATYPE,
   LE_INVALIDFILETYPE,
   LE_INVALIDGIFFILE,
   LE_NOCOMPBMP,
   LE_NOGIFINTERLACE,
   LE_NOMEMIMG,
   LE_NOMEMPAL,
   LE_NOOPEN,
   LE_NOREAD,
   LE_NOTBMP,
   LE_NOTPCX,
   LE_NOTVALIDPCX,
   LE_NOTPCXENCODE,
   LE_NOWRITE,
   LE_BADCODE,
   LE_NOSERVICES,
   LE_UNKNOWN
};

/*----------------------------------------------------------------------------*/

// The following modes are defined with XXXX_YYY where XXXX is the vertical
// resolution, and YYY is the number of colors.

enum
{
   VESA_MODE_200_256 = 0xFF,
   VESA_MODE_400_256,
   VESA_MODE_480_256,
   VESA_MODE_600_16,
   VESA_MODE_600_256,
   VESA_MODE_768_16,
   VESA_MODE_768_256,
   VESA_MODE_1024_16,
   VESA_MODE_1024_256,
   VESA_MODE_TEXT01,
   VESA_MODE_TEXT02,
   VESA_MODE_TEXT03,
   VESA_MODE_TEXT04,
   VESA_MODE_TEXT05,
   VESA_MODE_200_32K,
   VESA_MODE_200_64K,
   VESA_MODE_200_16M,
   VESA_MODE_480_32K,
   VESA_MODE_480_64K,
   VESA_MODE_480_16M,
   VESA_MODE_600_32K,
   VESA_MODE_600_64K,
   VESA_MODE_600_16M,
   VESA_MODE_768_32K,
   VESA_MODE_768_64K,
   VESA_MODE_768_16M,
   VESA_MODE_1024_32K,
   VESA_MODE_1024_64K,
   VESA_MODE_1024_16M
};

/*----------------------------------------------------------------------------*/

enum
{
   IMAGETYPE_IPI,
   IMAGETYPE_TGA,
   IMAGETYPE_BMP,
   IMAGETYPE_IMG,
   IMAGETYPE_GIF,
   IMAGETYPE_G24,
   IMAGETYPE_PCX,
   IMAGETYPE_UNKNOWN
};

/*----------------------------------------------------------------------------*/
// Misc. GIF defines

#define  LOCAL_MAP            (0x80)
#define  INTERLACE            (0x40)
#define  MAX_CODES            4095

/*----------------------------------------------------------------------------*/
// Misc. octree quantization defines

#define  TESTBIT(a,i) (((a)&(1<<(i)))>>(i))
#define  MAXCOLOR     256                        /* max number of colors */
#define  MAXDEPTH     7                          /* max depth of octree - 1 */
#define  MAXCOLS      2048                       /* max width of image */

/*----------------------------------------------------------------------------*/

#define  MAXCOLREGVAL          63
#define  MAX256PALETTECOLORS   256

/*----------------------------------------------------------------------------
  -   [SDXDEFS.H   ] - End Of File                                           -
  ----------------------------------------------------------------------------*/
