// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [3DS2USD.H   ] - Convertion software's header file                     ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#define  DEF_OUT_NAM "OUTPUT.USD"
#define  DEF_STR_LEN 512
#define  BELL        7
#define  DOT_TIME    5000

#ifndef TRUE
  #define TRUE 1
#endif

#ifndef FALSE
  #define FALSE 0
#endif

#define  SWAP(A,B,C) {(C)=(A);(A)=(B);(B)=(C);}
#define  ABS(val)    (((val) >= 0) ? (val):-(val))

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#define  CHUNK_HEADER_SIZE 6

#define  CHUNK_3DSFILE     0x4D4D      // 3DS file descriptor
#define  CHUNK_MLIFILE     0x3DAA      // MLI file descriptor
#define  CHUNK_PRJFILE     0xC23D      // PRJ file descriptor
#define  CHUNK_MESH         0x3D3D     // Mesh chunk
#define  CHUNK_AMBIENTCLR    0x2100    // Ambient Color Block
#define  CHUNK_NAMEDOBJECT   0x4000    // Named Object
#define  CHUNK_TRIMESH        0x4100   // Tri-Mesh
#define  CHUNK_VERTEXLIST      0x4110  // Vertex Coordinate List Chunk
#define  CHUNK_FACELIST        0x4120  // Face List Chunk
#define  CHUNK_MATERIALAPP     0x4130  // Material Application Chunk
#define  CHUNK_UVLIST          0x4140  // Mapping Coordinate List Chunk
#define  CHUNK_TRANSMATRIX     0x4160  // Translation Matrix
#define  CHUNK_LIGHT          0x4600   // Light
#define  CHUNK_RGB             0x0010  // RGB Color
#define  CHUNK_24BIT           0x0011  // 24-bit Color
#define  CHUNK_SPOTLIGHT       0x4610  // SpotLight
#define  CHUNK_NOLIGHT         0x4620  // Light is turned off
#define  CHUNK_CAMERA         0x4700   // Camera
#define  CHUNK_MATERIAL      0xAFFF    // Material chunk
#define  CHUNK_MATNAME        0xA000   // Material name
#define  CHUNK_MATAMBIENT     0xA010   // Material Ambient color
#define  CHUNK_MATDIFFUSE     0xA020   // Material Diffuse color
#define  CHUNK_MATSPECULAR    0xA030   // Material Specular color
#define  CHUNK_MATSHINE       0xA040   // Material Shininess
#define  CHUNK_MATTRANS       0xA050   // Material Transparency
#define  CHUNK_MATTRANSFAL    0xA052   // Material Transparency fall-off
#define  CHUNK_MATREFBLUR     0xA053   // Material Reflection blur
#define  CHUNK_MATTEXTURE     0xA200   // Material Texture map
#define  CHUNK_MATOPACITY     0xA210   // Material Opacity map
#define  CHUNK_MATTRANSFE     0xA240   // Material Transparency fall-off enable
#define  CHUNK_MATREFBE       0xA250   // Material Reflection blur enable

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

typedef  int                  FXD;
typedef  float                FLT;
typedef  double               DBL;
typedef  char                 BYT;
typedef  char                 CHR;
typedef  short int            WRD;
typedef  long int             INT;
typedef  long int             LNG;
typedef  unsigned char        UBYT;
typedef  unsigned short int   UWRD;
typedef  unsigned long int    UINT;
typedef  unsigned long int    ULNG;
typedef  UWRD                 ZBUF;

typedef struct
{
   FLT   x, y;
} P2D;

typedef struct
{
   FLT   x, y, z;
} P3D;

typedef struct
{
   FLT   r, g, b;
} RGB;

typedef struct
{
   UBYT  r, g, b;
} B24;

typedef struct
{
   P3D   loc;
   CHR   unknown[6];
   RGB   rgb;
   UBYT  NoLight, SpotLight;
   CHR   Name[80];

} LGT;

typedef struct
{
   P3D   loc;
   P3D   target;
   FLT   bank;
   FLT   lense;
   CHR   Name[80];
} CAM;

typedef struct
{
   UWRD  a, b, c, flags;
   CHR   MaterialName[80];
} FACE;

typedef struct
{
   CHR   Name[80];
   RGB   Ambient;
   RGB   Diffuse;
   CHR   Texture[80];
   FLT   Shininess;
   UBYT  Transparent;
} MAT;

typedef  struct chunk
{
   UWRD  ID;
   ULNG  Length;
} CHK;

typedef  struct
{
   UWRD  VertexCount;
   P3D   *VertexList;

   UWRD  FaceCount;
   FACE  *FaceList;

   UWRD  UVCount;
   P2D   *UVList;

   CHR   Name[80];

} MSH;

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#define  FOREVER     for(;;)
#define  MAX(A,B)    ((A) > (B) ? (A) : (B))
#define  SWAP(A,B,C) {(C)=(A);(A)=(B);(B)=(C);}
#define  SHOW_DOT(i) if (!(i % DOT_TIME)) printf( "." )

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

enum
{
   SWAP_XY = 1,
   SWAP_YZ,
   SWAP_ZX
};

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  OrientFace(FACE *Face, MSH *Msh);
void  GetNormal( FACE *Face, P3D *Vector, MSH *Msh);
void  GetSwappedNormal( FACE *Face, P3D *Vector, MSH *Msh);
void  Read3DS(FILE *InFile, FILE *OutFile);
INT   GetChunkID(FILE *File, CHK *Chunk);
void  Process3DS(FILE *InFile, FILE *OutFile, INT ChunkLength);
void  ReadAmbientColor(FILE *InFile, FILE *OutFile, INT ChunkLength);
void  ReadMeshChunk(FILE *InFile, FILE *OutFile, INT ChunkLength);
void  ReadNamedObject(FILE *InFile, FILE *OutFile, INT ChunkLength);
void  ReadTriMesh(FILE *InFile, FILE *OutFile, INT ChunkLength);
void  ReadVertexList(FILE *InFile, FILE *OutFile, INT ChunkLength);
void  ReadFaceList(FILE *InFile, FILE *OutFile, INT ChunkLength);
void  ReadMaterialApp(FILE *InFile, FILE *OutFile, INT ChunkLength);
void  ReadUVList(FILE *InFile, FILE *OutFile, INT ChunkLength);
void  ReadTranslationMatrix(FILE *InFile, FILE *OutFile, INT ChunkLength);
void  ReadLight(FILE *InFile, FILE *OutFile, INT ChunkLength);
void  ReadRGB(FILE *InFile, FILE *OutFile, INT ChunkLength);
void  Read24Bit(FILE *InFile, FILE *OutFile, INT ChunkLength);
void  ReadSpotLight(FILE *InFile, FILE *OutFile, INT ChunkLength);
void  ReadNoLight(FILE *InFile, FILE *OutFile, INT ChunkLength);
void  ReadCamera(FILE *InFile, FILE *OutFile, INT ChunkLength);
void  SkipChunk(FILE *File, ULNG Length);
void  Backup(FILE *File);
void  FatalError( CHR *Reason );
void  PrintCopyright();
void  PrintUsage();
void  PutIndent( FILE *Fp );
void  IncIndent( FILE *Fp );
void  DecIndent( FILE *Fp );
INT   MyRead( void *Buffer, INT Length, FILE *File);
void  GetString( FILE *InFile, CHR *String );


/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [3DS2USD.H   ] - End Of File                                           ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
