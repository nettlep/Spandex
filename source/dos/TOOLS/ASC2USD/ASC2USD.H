// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [ASC2USD.H   ] - Convertion software's header file                     ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

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

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#define  MAX(A,B)    ((A) > (B) ? (A) : (B))
#define  SWAP(A,B,C) {(C)=(A);(A)=(B);(B)=(C);}
#define  SHOW_DOT(i) if (!(i % DOT_TIME)) printf( "." )

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

typedef struct parser
{
   char  *String;
   int   (*Handler)(FILE *InFile, FILE *OutFile, char *Line);
} PARSE;

typedef struct
{
   double   x,  y,  z;
   double   u,  v;
   double   nx, ny, nz;
} VER;

typedef struct
{
   int      a, b, c;
   char     Mat[DEF_STR_LEN];
} TRI;

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

enum
{
   SWAP_XY = 1,
   SWAP_YZ,
   SWAP_ZX
};

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  ReadASC(FILE *InFile, FILE *OutFile);
int   ReadAmbient(FILE *InFile, FILE *OutFile, char *Line);
int   ReadNamedObject( FILE *InFile, FILE *OutFile, char *Line );
int   ReadDirectLight( FILE *InFile, FILE *OutFile, char *Line );
int   ReadCamera( FILE *InFile, FILE *OutFile, char *Line );
int   ReadPosition( FILE *InFile, FILE *OutFile, char *Line );
int   ReadLightColor( FILE *InFile, FILE *OutFile, char *Line );
int   ReadTriMesh( FILE *InFile, FILE *OutFile, char *Line );
void  ReadVertices(FILE *InFile, int VertexCount, VER *List);
void  ReadFaces(FILE *InFile, int FaceCount, TRI *List);
void  WriteUSDFile(FILE *Fp, int VertexCount, int FaceCount, VER *VList, TRI *FList);
void  FatalError( char *Reason );
void  PrintCopyright();
void  PrintUsage();
void  PutIndent( FILE *Fp );
void  IncIndent( FILE *Fp );
void  DecIndent( FILE *Fp );

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [ASC2USD.H   ] - End Of File                                           ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
