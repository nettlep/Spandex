// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [SDXTYPES.H  ] - Types for SPANDEX library                             -
  -                                                                          -
  ----------------------------------------------------------------------------*/

typedef  int                  FXD;
typedef  float                FLT;
typedef  double               DBL;
typedef  char                 BYT;
typedef  unsigned char        UCHR;
typedef  char                 CHR;
typedef  short int            WRD;

#if _OS_DOS
typedef  long int             INT;
#else
typedef  int                  INT;
#endif

typedef  long int             LNG;
typedef  unsigned char        UBYT;
typedef  unsigned short int   UWRD;
typedef  unsigned int         UINT;
typedef  unsigned long int    ULNG;
typedef  UWRD                 ZBUF;

/*----------------------------------------------------------------------------*/

typedef  INT      (* ifunptr)(UCHR *Pixels, INT LineLength, UCHR *Buffer, INT ImgWidth, INT Y);
typedef  INT      (* ifunptrgp)(INT X, INT Y);
typedef  void     (* rendfunptr)(struct tri *Tri);

/*----------------------------------------------------------------------------*/

typedef struct matrix
{

   FLT   m00, m01, m02;
   FLT   m10, m11, m12;
   FLT   m20, m21, m22;

} MAT;

/*----------------------------------------------------------------------------*/

typedef struct vesainfo
{

   CHR   Signature[4];
   WRD   Version;
   CHR   *OEMString;
   UBYT  Capabilities[4];
   WRD   *VideoModeList;
   WRD   Available64KBlocks;
   CHR   reserved[236];

} VESA;

/*----------------------------------------------------------------------------*/

typedef struct svgainfo
{

   WRD   ModeAttributes;
   UBYT  WinAAttrubutes;
   UBYT  WinBAttrubutes;
   WRD   WinGranularity;
   WRD   WinSize;
   UWRD  WinASegment;
   UWRD  WinBSegment;
   void  *FunctionPointer;
   WRD   BytesPerScanline;

// Optional fields (now mandatory)

   WRD   XResolution;
   WRD   YResolution;
   UBYT  XCharSize;
   UBYT  YCharSize;
   UBYT  NumberOfPlanes;
   UBYT  BitsPerPixel;
   UBYT  NumberOfBanks;
   UBYT  MemoryModel;
   UBYT  BankSize;
   UBYT  NumberOfImagePages;
   UBYT  reserved01;

// New direct color fields

   UBYT  RedMaskSize;
   UBYT  RedFieldPosition;
   UBYT  GreenMaskSize;
   UBYT  GreenFieldPosition;
   UBYT  BlueMaskSize;
   UBYT  BlueFieldPosition;
   UBYT  ReservedMaskSize;
   UBYT  ReservedFieldPosition;
   UBYT  DirectColorModeInfo;

   UBYT  reserved02[216];

// SPANDEX internal stuff...This is calculated in PRIMS.C

   INT   WinPerScreen;
   INT   WinPerScreenLeftover;
   INT   WinIncrement;

} SVGA;

/*----------------------------------------------------------------------------*/

typedef struct rgb_values
{

   UCHR   Red;
   UCHR   Green;
   UCHR   Blue;

} RGB;

/*----------------------------------------------------------------------------*/

typedef struct palette
{

   RGB   Colors[TOTAL_COLORS];

} PAL;

/*----------------------------------------------------------------------------*/

typedef struct vfont
{

   LNG   Offsets[TOTAL_FONT_CHARS];
   CHR   *Font;

} VFONT;

/*----------------------------------------------------------------------------*/

typedef  struct bmap
{

   INT   *STab;                                 // IF THIS CHANGES, SEE ASM
   WRD   UMask;                                 //
   WRD   VMask;                                 //

   UBYT  *Data;

   CHR   Name[DEF_NAME_LEN];
   WRD   XRes;
   WRD   YRes;

} BMAP;

/*----------------------------------------------------------------------------*/

typedef struct vector
{

   FLT   dx;
   FLT   dy;
   FLT   dz;

} VEC;

/*----------------------------------------------------------------------------*/

typedef struct p2d
{

   FLT   x;                                     // IF THIS CHANGES, SEE ASM
   FLT   y;                                     //
   FXD   fx;                                    //
   FXD   fy;                                    //

} P2D;

/*----------------------------------------------------------------------------*/

typedef struct uv
{

   FLT   u;                                     // IF THIS CHANGES, SEE ASM
   FLT   v;                                     //
   FXD   fu;                                    //
   FXD   fv;                                    //

} UV;

/*----------------------------------------------------------------------------*/

typedef struct p3d
{

   FLT   x;
   FLT   y;
   FLT   z;

} P3D;

/*----------------------------------------------------------------------------*/

typedef struct virtual_screen
{
   FLT   fResX, fResY;
   INT   iResX, iResY;

   INT   ClipMinX, ClipMaxX;
   INT   ClipMinY, ClipMaxY;

   P2D   Center;

   UCHR   *DrawScreen;
   INT   STab[MAX_Y_RES];
#ifdef U_Z
   ZBUF  *ZBuffer;
#endif

   FLT   ScaleX;
   FLT   ScaleY;

   INT   MaxPolysToRender;
} VSCR;

/*----------------------------------------------------------------------------*/

typedef struct material
{
   BMAP  *Map;                                  // IF THIS CHANGES, SEE ASM

   CHR   Name[DEF_NAME_LEN];                    // Material name

   FLT   _AM;                                   // Internal use
   FLT   _IR;                                   // Internal use
   
   FLT   Ambient;                               // Ambient light
   BMAP  *BumpMap;                              // Bump map
   UBYT  Color;                                 // Color index
   UBYT  Surface;                               // Surface material info.
   UBYT  Transparency;                          // Transparency flag

   FLT   Ka;                                    // Ambient reflection coeff.
   FLT   Kd;                                    // Diffuse reflection coeff.
   FLT   Ks;                                    // Specular reflection coeff.
   FLT   Shininess;                             // Value passed into MAKEMAP

} MTRL;

/*----------------------------------------------------------------------------*/

typedef struct vertex
{

   P2D   Scr;                                   // IF THIS CHANGES, SEE ASM
   P2D   Env;                                   //
   FLT   Int;                                   //

   P3D   LLoc;
   P3D   CLoc;

   FXD   FixedZ;

#ifdef U_P
   FLT   OneOverZ;
#endif
   VEC   LN;
   VEC   CN;

   UBYT  ShadeModel;
   MTRL  *Material;

   BYT   Visible;

} VER;

/*----------------------------------------------------------------------------*/

typedef struct tri
{

   VER   *V1;                                   // IF THIS CHANGES, SEE ASM
   VER   *V2;                                   //
   VER   *V3;                                   //
   MTRL  *Material;                             //
   P2D   T1, T2, T3;                            //

   P2D   M1, M2, M3;

   P3D   Center;

   VEC   LN;
   VEC   CN;

   FLT   Int;

#ifndef U_Z
   FXD   MinZ;
   struct tri *Next;
#endif

   BYT   Visible;
   struct object *Object;

} TRI;

/*----------------------------------------------------------------------------*/

typedef struct rotation
{

   FLT   x;
   FLT   y;
   FLT   z;

} ROT;

/*----------------------------------------------------------------------------*/

typedef struct light
{

   CHR   Name[DEF_NAME_LEN];
   P3D   LLoc;
   P3D   CLoc;
   FLT   Int;

} LGT;

/*----------------------------------------------------------------------------*/

typedef struct lightlist
{

   INT   Count;
   LGT   *Lgts;

} LLS;

/*----------------------------------------------------------------------------*/

typedef struct verlist
{

   INT   Count;
   VER   *Vers;

} VLS;

/*----------------------------------------------------------------------------*/

typedef struct pointlist
{

   INT   Count;
   VER   **Points;

} PLS;

/*----------------------------------------------------------------------------*/

typedef struct trilist
{

   INT   Count;
   TRI   *Tris;

} TLS;

/*----------------------------------------------------------------------------*/

typedef struct renderlist
{
   INT   TriReservoir;
   INT   TriCount;
   TRI   **Tris;

   INT   PointReservoir;
   INT   PointCount;
   VER   **Points;

   INT   VertexReservoir;
   INT   VertexCount;
   VER   **Vers;

} RLS;

/*----------------------------------------------------------------------------*/

typedef struct objectlist
{

   INT   Count;
   struct object *Objs;

} OBL;

/*----------------------------------------------------------------------------*/

typedef struct object
{
   CHR   Name[DEF_NAME_LEN];                    // Contains text name of object

   UBYT  ShadeModel;                            // Shading model for object

   LLS   LightList;                             // List of light sources

   BYT   BackFace;                              // Flag to show backs of prims
   BYT   DrawFlags;                             // If 0, not drawn

   MAT   RotMat;                                // Accumulative Rotation matrix
   MAT   PhaseOne;                              // (used in the pipeline)
   MAT   PhaseTwo;                              // (used in the pipeline)

   ROT   SteadyRot;                             // Continuous rotational values

   VEC   LDir;                                  // Local -- Looking at
   VEC   WDir;                                  // Relative to Parent

   P3D   LLoc;                                  // Local -- Location
   P3D   WLoc;                                  // Relative to Parent
   P3D   CLoc;                                  // Relative to Camera

   TLS   TriList;                               // Tris
   PLS   PointList;                             // Points
   VLS   VertexList;                            // Vertex List

   INT   Type;                                  // For developer's use
   INT   DevUse[DEV_USE_COUNT];                 // For use by the developer

   FLT   Lense;                                 // Camera lense

   OBL   Children;                              // Children
} OBJ;

/*----------------------------------------------------------------------------*/

typedef struct globals
{
   // Map information
   INT   MapCount;
   BMAP  *MapList;
   INT   MapFlags;

   // Map information
   INT   MatCount;
   MTRL  *MatList;
   INT   MatFlags;

   // VESA information
   VESA  VESAInfo;
   VESA  *TempVESAInfo;
   SVGA  SVGAInfo;
   SVGA  *TempSVGAInfo;

   // Graphics stuff
   INT   IRQ_OkToCopy;
   INT   UseIRQ;
   WRD   Port3x4;

   // Palette manipulation
   PAL   *DOSMemPalette;
   PAL   *PaletteStack;
   INT   PaletteStackDepth;
   INT   TotalColors;
   INT   BackgroundColor;

   // Global list of lights
   INT   LightCount;
   LGT   **LightList;

   // Misc. tables
   FLT   *SineTable;
   FLT   *CosTable;
   UCHR  *PalTab;
   UCHR  *TransTab;
   INT   *PhongTab;

   // Other misc. stuff
   INT   InitFlag;
   INT   DebugState;
   CHR   *DOSMemGeneric;

   INT   ScrResX, ScrResY;

   FLT   NearZ;

   FLT   AspectX, AspectY;

   VSCR  CurrentVScreen;

} GLOBS;

/*----------------------------------------------------------------------------*/

typedef struct objparselist
{

   CHR   Label[10];
   INT   (*Handler)(CHR **FileData, OBJ **Object);

} OBJPARSELIST;

/*----------------------------------------------------------------------------*/

typedef struct mtrlparselist
{

   CHR   Label[10];
   INT   (*Handler)(CHR **FileData, MTRL **Material);

} MTRLPARSELIST;

/*----------------------------------------------------------------------------*/

typedef struct lgtparselist
{

   CHR   Label[10];
   INT   (*Handler)(CHR **FileData, LGT **Light);

} LGTPARSELIST;

/*----------------------------------------------------------------------------*/

typedef struct byteparselist
{
   CHR   Label[10];
   BYT   Value;

} BYTEPARSELIST;

/*----------------------------------------------------------------------------*/

typedef struct memreport
{

   INT   InitFlag;
   INT   MaxMemAtOneTime;
   INT   TotalMemInUse;
   INT   TotalElements;

} MEMREP;

/*----------------------------------------------------------------------------*/

typedef struct memlist
{
   void  *Address;
   INT   Size;
   char  Owner[DEF_NAME_LEN+DEF_FUNC_LEN+1];
   struct memlist *Next;

} MEMLIST;

/*----------------------------------------------------------------------------*/

typedef struct Image
{

   CHR   Used;
   INT   ResX, ResY;
   INT   WindowBorderLeft, WindowBorderTop, WindowBorderRight, WindowBorderBottom;
   INT   WindowLeft, WindowTop, WindowRight, WindowBottom;
   UCHR   *Buffer;
   UCHR   *Saved;

} IMAGE;

/*----------------------------------------------------------------------------*/

typedef  struct   pcx_header
{

   UBYT  Identifier;
   UBYT  Version;
   UBYT  Encode;
   UBYT  Bits;
   UWRD  Left, Top, Right, Bottom;
   UWRD  AdapterWidth, AdapterHeight;

} PCXHD;

/*----------------------------------------------------------------------------*/

typedef  struct   pcx_info
{

   UBYT  VMode;
   UBYT  Planes;
   UWRD  BytesPerLine;
   UBYT  RFU[60];

} PCXIN;

/*----------------------------------------------------------------------------*/

typedef  struct   pcx_file
{

   PCXHD Header;
   UBYT  Pal16[16*3];
   PCXIN Info;

} PCXFL;

/*----------------------------------------------------------------------------*/

typedef struct stuffer_file_header
{
   CHR Name[DEF_NAME_LEN];
   INT Offset;
   INT Length;
} STFH;

/*----------------------------------------------------------------------------*/

typedef struct stuffer_file_pack
{
   INT Handle;
   INT FileStart;
} STFP;

/*----------------------------------------------------------------------------*/

typedef struct error_strings
{
   INT ErrorCode;
   CHR *String;
} ERRLIST;

/*----------------------------------------------------------------------------*/

typedef union _hilo
{
   struct bytes
   {
      UBYT l, lx, h, hx;
   } b;

   struct words
   {
      UWRD l, h;
   } w;

   INT     d;
} HILO;

/*----------------------------------------------------------------------------
  -   [SDXTYPES.H  ] - End Of File                                           -
  ----------------------------------------------------------------------------*/
