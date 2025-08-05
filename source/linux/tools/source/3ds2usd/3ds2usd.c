// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

//   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.
//
//   [3DS2USD.C   ] - Conversion utility used to convert from 3DS to USD
//
//   Updated for Linux on 9/2/97 by Matt Wilhelm

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <dos.h>
#include <math.h>
#include <malloc.h>
//#include <io.h>
//#include <process.h>
#include <ctype.h>

#include "3ds2usd.h"

#define  __DEBUG__

///////////////////////////////////////////////////////////////////////////////

INT      Swap = 0, NoPolyOutput = FALSE, Quiet = FALSE, UniverseFlag = FALSE;
INT      UniqueVertices = FALSE, DebugFlag = FALSE;
INT      IndentLevel = 1, InObject = FALSE, FixUV = FALSE;
CHR      MapName[80] = {"MAPFILE"}, CurrentNamedObject[80];
DBL      XMultiplier = 1.0, YMultiplier = 1.0, ZMultiplier = 1.0;

///////////////////////////////////////////////////////////////////////////////

UWRD  MatCount = 0;
MAT   *MatList = 0;

UWRD  MeshCount = 0;
MSH   *MeshList = 0;

UWRD  CamCount  = 0;
CAM   *CamList  = 0;

UWRD  LgtCount  = 0;
LGT   *LgtList  = 0;

// function prototypes
void  ReadMaterial(FILE *InFile, FILE *OutFile, INT ChunkLength);
void  ReadMaterialName(FILE *InFile, FILE *OutFile, INT ChunkLength);
void  ReadMaterialTexture(FILE *InFile, FILE *OutFile, INT ChunkLength);
void  ReadMaterialAmbient(FILE *InFile, FILE *OutFile, INT ChunkLength);
void  ReadMaterialColor(FILE *InFile, FILE *OutFile, INT ChunkLength);
void  ReadMaterialShine(FILE *InFile, FILE *OutFile, INT ChunkLength);
void  ReadMaterialTrans(FILE *InFile, FILE *OutFile, INT ChunkLength);
void  ReadLightRGB(FILE *InFile, FILE *OutFile, INT ChunkLength);
void  ReadLight24Bit(FILE *InFile, FILE *OutFile, INT ChunkLength);

///////////////////////////////////////////////////////////////////////////////

int   main(INT argc, CHR *argv[])
{
INT   i, j;
CHR   InName[80], OutName[80];
FILE  *InFile, *OutFile;

   setbuf(stdout, 0);

   if (argc < 2)
   {
      PrintUsage();
      return 1;
   }

   *InName = 0;
   strcpy( OutName, DEF_OUT_NAM );

   for( i = 1; i < argc; i++ )
   {
      if (argv[i][0] == '-' || argv[i][0] == '/')
      {
         switch( toupper(argv[i][1]) )
         {
            case '?':
            case 'H':
               PrintUsage();
               return 1;

            case 'C':
               UniverseFlag = TRUE;
               break;

            case 'D':
               DebugFlag = TRUE;
               break;

            case 'F':
               FixUV = TRUE;
               break;

            case 'M':
               strcpy(MapName, &argv[i][2]);
               break;

            case 'P':
               NoPolyOutput = TRUE;
               break;

            case 'S':
               if (!strcmp( "XY", &argv[i][2] ) || !strcmp( "YX", &argv[i][2] ))
                  Swap = SWAP_XY;
               else if (!strcmp( "YZ", &argv[i][2] ) || !strcmp( "ZY", &argv[i][2] ))
                  Swap = SWAP_YZ;
               else
                  Swap = SWAP_ZX;
               break;

            case 'U':
               UniqueVertices = TRUE;
               break;

            case 'Q':
               Quiet = TRUE;
               break;

            case 'X':
               XMultiplier = atof(&argv[i][2]);
               break;

            case 'Y':
               YMultiplier = atof(&argv[i][2]);
               break;

            case 'Z':
               ZMultiplier = atof(&argv[i][2]);
               break;
         }
      }
      else
      {
         if (!*InName)
            strcpy( InName, argv[i] );
         else
            strcpy( OutName, argv[i] );
      }
   }

   if (!Quiet) PrintCopyright();

   InFile = fopen(InName, "rb");

   if (!InFile)
      FatalError("Unable to open input file");

   OutFile = fopen(OutName, "w");

   if (!OutFile)
      FatalError("Unable to open output file");

   fprintf( OutFile, ";//////////////////////////////////////////////////////////////////////////////\n");
   fprintf( OutFile, ";//                                                                          //\n" );
   fprintf( OutFile, ";//                     This file was created by 3DS2USD                     //\n" );
   fprintf( OutFile, ";//                                                                          //\n" );
   fprintf( OutFile, ";//////////////////////////////////////////////////////////////////////////////\n\n");

   Read3DS(InFile, OutFile);

   for( i = 0; i < MatCount; i++ )
   {
      MAT *Mat;

      Mat = &MatList[i];

      fprintf(OutFile, "Material( %s )\n", Mat->Name);
      fprintf(OutFile, "{\n");

      if (Mat->Texture[0])
      {
         fprintf(OutFile, "   Surface      Textured\n" );
         fprintf(OutFile, "   Texture      %s\n", Mat->Texture );
      }
      else
      {
         fprintf(OutFile, "   Surface      Solid\n" );
      }

      fprintf(OutFile, "   Ambient      %12.5f %12.5f %12.5f\n", Mat->Ambient.r, Mat->Ambient.g, Mat->Ambient.b );
      fprintf(OutFile, "   Color        %12.5f %12.5f %12.5f\n", Mat->Diffuse.r, Mat->Diffuse.g, Mat->Diffuse.b );
      fprintf(OutFile, "   Shininess    %12.5f\n", Mat->Shininess );
      fprintf(OutFile, "   Transparency %s\n", Mat->Transparent ? "On":"Off" );
      fprintf(OutFile, "}\n\n");
   }

   if (UniverseFlag == TRUE)
   {
      fprintf( OutFile, "Universe( Universe )\n{\n" );
      PutIndent(OutFile);
      fprintf( OutFile, "ShadeModel Gouraud\n\n" );
   }

   for( i = 0; i < CamCount; i++ )
   {
      CAM *Cam;

      Cam = &CamList[i];

      PutIndent(OutFile);
      fprintf(OutFile, "Camera( %s )\n", Cam->Name);
      IncIndent(OutFile);
      PutIndent(OutFile);
      fprintf(OutFile, "Location     %12.5f %12.5f %12.5f\n", Cam->loc.x, Cam->loc.z, Cam->loc.y );
      PutIndent(OutFile);
      fprintf(OutFile, "Target       %12.5f %12.5f %12.5f ", Cam->target.x, Cam->target.z, Cam->target.y );
      fprintf(OutFile, "%12.5f\n", Cam->bank );
      PutIndent(OutFile);
      fprintf(OutFile, "Lense        %12.5f\n", Cam->lense );
      DecIndent(OutFile);
   }

   for( i = 0; i < LgtCount; i++ )
   {
      LGT *Lgt;

      Lgt = &LgtList[i];

      PutIndent(OutFile);
      fprintf(OutFile, "Light( %s )\n", Lgt->Name);
      IncIndent(OutFile);
      PutIndent(OutFile);
      fprintf(OutFile, "Location     %12.5f %12.5f %12.5f\n", Lgt->loc.x, Lgt->loc.z, Lgt->loc.y );
      PutIndent(OutFile);
      fprintf(OutFile, "Color        %12.5f %12.5f %12.5f\n", Lgt->rgb.r, Lgt->rgb.g, Lgt->rgb.b );
      if (Lgt->SpotLight)
      {
         PutIndent(OutFile);
         fprintf(OutFile, "; NOTE:  This light was a spotlight in the 3DS file\n" );
      }
      if (Lgt->NoLight)
      {
         PutIndent(OutFile);
         fprintf(OutFile, "; NOTE:  This light was disabled in the 3DS file\n" );
      }
      DecIndent(OutFile);
   }

   for( i = 0; i < MeshCount; i++ )
   {
      MSH *Msh;

      Msh = &MeshList[i];

      PutIndent(OutFile);
      fprintf(OutFile, "Object( %s )\n", Msh->Name);
      IncIndent(OutFile);

      for( j = 0; j < Msh->VertexCount; j++ )
      {
         P3D *Ver;
         Ver = &Msh->VertexList[j];

         PutIndent(OutFile);
         fprintf(OutFile, "Vertex   %12.5f %12.5f %12.5f\n", Ver->x, Ver->z, Ver->y );
      }

      fprintf(OutFile, "\n" );

      for( j = 0; j < Msh->FaceCount; j++ )
      {
         FACE *Face;
         Face = &Msh->FaceList[j];

         OrientFace(Face, Msh);

         PutIndent(OutFile);
         fprintf(OutFile, "Triangle %5d %5d %5d ", Face->a, Face->b, Face->c );
         if (Face->MaterialName[0])
            fprintf(OutFile, "%20s ", Face->MaterialName );
         else
            fprintf(OutFile, "%20s ", "DEFAULT" );

         if (Msh->UVList)
         {
            fprintf(OutFile, "%9.5f %9.5f ", Msh->UVList[Face->a].x, Msh->UVList[Face->a].y );
            fprintf(OutFile, "%9.5f %9.5f ", Msh->UVList[Face->b].x, Msh->UVList[Face->b].y );
            fprintf(OutFile, "%9.5f %9.5f", Msh->UVList[Face->c].x, Msh->UVList[Face->c].y );
         }
         else
         {
            fprintf(OutFile, "[0 0 0 0 0 0] ; (no UV)" );
         }
         fprintf(OutFile, "\n" );
      }

      DecIndent(OutFile);
   }

   if (UniverseFlag)
      while (IndentLevel > 0)
         DecIndent(OutFile);
   else
      while (IndentLevel > 1)
         DecIndent(OutFile);

   fclose(InFile);
   fclose(OutFile);

   return 0;
}

///////////////////////////////////////////////////////////////////////////////

void  OrientFace(FACE *Face, MSH *Mesh)
{
P3D   Original, New;
UWRD  a, b, c;

   a = Face->a;
   b = Face->b;
   c = Face->c;

   // Get the original normal
   GetNormal( Face, &Original, Mesh );

   // ---------------------------------------------------------------------

   Face->a = a;
   Face->b = c;
   Face->c = b;
   GetSwappedNormal( Face, &New, Mesh );
   if (Original.x == New.x && Original.y == New.z && Original.z == New.y)
      return;

   // ---------------------------------------------------------------------

   Face->a = b;
   Face->b = a;
   Face->c = c;
   GetSwappedNormal( Face, &New, Mesh );
   if (Original.x == New.x && Original.y == New.z && Original.z == New.y)
      return;

   // ---------------------------------------------------------------------

   Face->a = b;
   Face->b = c;
   Face->c = a;
   GetSwappedNormal( Face, &New, Mesh );
   if (Original.x == New.x && Original.y == New.z && Original.z == New.y)
      return;

   // ---------------------------------------------------------------------

   Face->a = c;
   Face->b = a;
   Face->c = b;
   GetSwappedNormal( Face, &New, Mesh );
   if (Original.x == New.x && Original.y == New.z && Original.z == New.y)
      return;

   // ---------------------------------------------------------------------

   Face->a = c;
   Face->b = b;
   Face->c = a;
   GetSwappedNormal( Face, &New, Mesh );
   if (Original.x == New.x && Original.y == New.z && Original.z == New.y)
      return;

   // Gotta leave it alone. :<
   printf( "Not swapped\n" );
   return;
}

///////////////////////////////////////////////////////////////////////////////

void  GetNormal( FACE *Face, P3D *Vector, MSH *Msh )
{
P3D   *P1, *P2, *P3, A, B;
FLT   VLen;

   P1 = &Msh->VertexList[Face->a];
   P2 = &Msh->VertexList[Face->b];
   P3 = &Msh->VertexList[Face->c];

   A.x = P3->x - P2->x;
   A.y = P3->y - P2->y;
   A.z = P3->z - P2->z;
   B.x = P1->x - P2->x;
   B.y = P1->y - P2->y;
   B.z = P1->z - P2->z;

   // Cross product...
   Vector->x = (A.y * B.z) - (A.z * B.y);
   Vector->y = (A.z * B.x) - (A.x * B.z);
   Vector->z = (A.x * B.y) - (A.y * B.x);

   // Normalize it...
   VLen =1.0/sqrt(Vector->x*Vector->x+Vector->y*Vector->y+Vector->z*Vector->z);
   Vector->x *= VLen;
   Vector->y *= VLen;
   Vector->z *= VLen;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void  GetSwappedNormal( FACE *Face, P3D *Vector, MSH *Msh )
{
P3D   *P1, *P2, *P3, A, B;
FLT   VLen;

   P1 = &Msh->VertexList[Face->a];
   P2 = &Msh->VertexList[Face->b];
   P3 = &Msh->VertexList[Face->c];

   A.x = P3->x - P2->x;
   A.y = P3->z - P2->z; // swapped
   A.z = P3->y - P2->y; // swapped
   B.x = P1->x - P2->x;
   B.y = P1->z - P2->z; // swapped
   B.z = P1->y - P2->y; // swapped

   // Cross product...
   Vector->x = (A.y * B.z) - (A.z * B.y);
   Vector->y = (A.z * B.x) - (A.x * B.z);
   Vector->z = (A.x * B.y) - (A.y * B.x);

   // Normalize it...
   VLen =1.0/sqrt(Vector->x*Vector->x+Vector->y*Vector->y+Vector->z*Vector->z);
   Vector->x *= VLen;
   Vector->y *= VLen;
   Vector->z *= VLen;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void  Read3DS(FILE *InFile, FILE *OutFile)
{
CHK   Chunk;

   // Get the first chunk... verify the integrity of the file...
   if (!GetChunkID(InFile, &Chunk))
      FatalError( "Unable to read 3DS file" );

   if (Chunk.ID != CHUNK_3DSFILE && Chunk.ID != CHUNK_MLIFILE && Chunk.ID != CHUNK_PRJFILE)
      FatalError( "Not a valid 3DS/MLI/PRJ file" );

   Process3DS(InFile, OutFile, Chunk.Length);
}

///////////////////////////////////////////////////////////////////////////////

INT   GetChunkID(FILE *File, CHK *Chunk)
{
   if (MyRead( Chunk, sizeof(CHK), File ))
      return 1;
   else
      return 0;
}

///////////////////////////////////////////////////////////////////////////////

void  Process3DS(FILE *InFile, FILE *OutFile, INT ChunkLength)
{
INT   RetCode, EndPos;
CHK   Chunk;

   EndPos = ChunkLength - CHUNK_HEADER_SIZE + ftell(InFile);

   FOREVER
   {
      // Done?
      if (EndPos <= ftell(InFile))
         break;

      RetCode = GetChunkID(InFile, &Chunk);

      // Are we done?
      if (!RetCode)
         break;

      switch(Chunk.ID)
      {
         case CHUNK_MESH:
            ReadMeshChunk(InFile, OutFile, Chunk.Length);
            break;

         case CHUNK_MATERIAL:
            ReadMaterial(InFile, OutFile, Chunk.Length);
            break;

         default:
            if (DebugFlag)
               printf( "Skipping chunk 0x%04X\n", Chunk.ID );

            SkipChunk(InFile, Chunk.Length);
            break;
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void  ReadMeshChunk(FILE *InFile, FILE *OutFile, INT ChunkLength)
{
INT   RetCode, EndPos;
CHK   Chunk;

   if (DebugFlag)
      printf( "Mesh Chunk\n" );

   EndPos = ChunkLength - CHUNK_HEADER_SIZE + ftell(InFile);

   FOREVER
   {
      // Done?
      if (EndPos <= ftell(InFile))
         break;

      RetCode = GetChunkID(InFile, &Chunk);

      // Make sure it's a sub-chunk...
      if (Chunk.ID >> 12 >= 0xB || Chunk.ID == CHUNK_MESH)
      {
         Backup(InFile);
         return;
      }

      // Are we done?
      if (!RetCode)
         break;

      switch(Chunk.ID)
      {
         case CHUNK_NAMEDOBJECT:
            ReadNamedObject(InFile, OutFile, Chunk.Length);
            break;

         case CHUNK_MATERIAL:
            ReadMaterial(InFile, OutFile, Chunk.Length);
            break;

         default:
            if (DebugFlag)
               printf( "Skipping chunk 0x%04X\n", Chunk.ID );

            SkipChunk(InFile, Chunk.Length);
            break;
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void  ReadMaterial(FILE *InFile, FILE *OutFile, INT ChunkLength)
{
INT   RetCode, EndPos;
CHK   Chunk;

   if (DebugFlag)
      printf( "Material\n" );

   MatList = (MAT *) realloc(MatList, (MatCount+1) * sizeof(MAT));

   if (!MatList)
      FatalError( "Out of memory for materials list" );

   memset( &MatList[MatCount], 0, sizeof(MAT) );

   EndPos = ChunkLength - CHUNK_HEADER_SIZE + ftell(InFile);

   FOREVER
   {
      // Done?
      if (EndPos <= ftell(InFile))
         break;

      RetCode = GetChunkID(InFile, &Chunk);

      // Make sure it's a sub-chunk...
      if ((Chunk.ID >> 12 != 0xA && Chunk.ID >> 12) || Chunk.ID == CHUNK_MATERIAL)
      {
         Backup(InFile);
         break;
      }

      // Are we done?
      if (!RetCode)
         break;

      switch(Chunk.ID)
      {
         case CHUNK_MATNAME:
            ReadMaterialName(InFile, OutFile, Chunk.Length);
            break;

         case CHUNK_MATTEXTURE:
            ReadMaterialTexture(InFile, OutFile, Chunk.Length);
            break;

         case CHUNK_MATAMBIENT:
            ReadMaterialAmbient(InFile, OutFile, Chunk.Length);
            break;

         case CHUNK_MATDIFFUSE:
            ReadMaterialColor(InFile, OutFile, Chunk.Length);
            break;

         case CHUNK_MATSHINE:
            ReadMaterialShine(InFile, OutFile, Chunk.Length);
            break;

         case CHUNK_MATTRANS:
            ReadMaterialTrans(InFile, OutFile, Chunk.Length);
            break;

         default:
            if (DebugFlag)
               printf( "Skipping chunk 0x%04X\n", Chunk.ID );

            SkipChunk(InFile, Chunk.Length);
            break;
      }
   }
   
   MatCount++;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void  ReadMaterialName(FILE *InFile, FILE *, INT)
{
   if (DebugFlag)
      printf( "Material Name\n" );

   GetString( InFile, MatList[MatCount].Name );
   return;
}

///////////////////////////////////////////////////////////////////////////////

void  ReadMaterialAmbient(FILE *InFile, FILE *, INT)
{
CHR   TempStr[80];
B24   Amb;

   if (DebugFlag)
      printf( "Material Ambient\n" );

   MyRead(TempStr, 6, InFile );
   MyRead(&Amb, sizeof(Amb), InFile );
   MatList[MatCount].Ambient.r = (FLT) Amb.r / 255.0;
   MatList[MatCount].Ambient.g = (FLT) Amb.g / 255.0;
   MatList[MatCount].Ambient.b = (FLT) Amb.b / 255.0;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void  ReadMaterialColor(FILE *InFile, FILE *, INT)
{
CHR   TempStr[80];
B24   Clr;

   if (DebugFlag)
      printf( "Material Color\n" );

   MyRead(TempStr, 6, InFile );

   MyRead(&Clr, sizeof(Clr), InFile );
   MatList[MatCount].Diffuse.r = (FLT) Clr.r / 255.0;
   MatList[MatCount].Diffuse.g = (FLT) Clr.g / 255.0;
   MatList[MatCount].Diffuse.b = (FLT) Clr.b / 255.0;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void  ReadMaterialShine(FILE *InFile, FILE *, INT)
{
CHR   TempStr[80];
INT   Shine = 0;

   if (DebugFlag)
      printf( "Material Shine\n" );

   MyRead(TempStr, 6, InFile );
   MyRead(&Shine, 1, InFile );
   MyRead(TempStr, 1, InFile );   // PDNDEBUG -- Ks?

   MatList[MatCount].Shininess = (FLT) Shine / 100.0;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void  ReadMaterialTrans(FILE *InFile, FILE *, INT)
{
CHR   TempStr[80];
INT   Trans = 0;

   if (DebugFlag)
      printf( "Material Transparency\n" );

   MyRead(TempStr, 6, InFile );
   MyRead(&Trans, 1, InFile );
   MyRead(TempStr, 1, InFile );

   MatList[MatCount].Transparent = Trans ? 1:0;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void  ReadMaterialTexture(FILE *InFile, FILE *, INT)
{
CHR   TempStr[80];

   if (DebugFlag)
      printf( "Material Texture\n" );

   MyRead(TempStr, 14, InFile );

   GetString( InFile, TempStr );

   TempStr[strcspn( TempStr, "." )] = '\0';

   strcpy(MatList[MatCount].Texture, TempStr);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void  ReadNamedObject(FILE *InFile, FILE *OutFile, INT ChunkLength)
{
INT   RetCode, EndPos;
CHK   Chunk;

   EndPos = ChunkLength - CHUNK_HEADER_SIZE + ftell(InFile);

   GetString( InFile, CurrentNamedObject );

   if (DebugFlag)
      printf( "Named Object:  %s\n", CurrentNamedObject );

   FOREVER
   {
      // Done?
      if (EndPos <= ftell(InFile))
         break;

      RetCode = GetChunkID(InFile, &Chunk);

      // Make sure it's a sub-chunk...
      if (Chunk.ID >> 12 != 4 || Chunk.ID == CHUNK_NAMEDOBJECT )
      {
         Backup(InFile);
         break;
      }

      // Are we done?
      if (!RetCode)
         break;

      switch(Chunk.ID)
      {
         case CHUNK_TRIMESH:
            if (!Quiet) printf( "Named Object [%s]\n", CurrentNamedObject );
            ReadTriMesh(InFile, OutFile, Chunk.Length);
            break;

         case CHUNK_LIGHT:
            if (!Quiet) printf( "Light [%s]\n", CurrentNamedObject );
            ReadLight(InFile, OutFile, Chunk.Length);
            break;

         case CHUNK_NOLIGHT:
            ReadNoLight(InFile, OutFile, Chunk.Length);
            break;

         case CHUNK_CAMERA:
            if (!Quiet) printf( "Camera [%s]\n", CurrentNamedObject );
            ReadCamera(InFile, OutFile, Chunk.Length);
            break;

         default:
            if (DebugFlag)
               printf( "Skipping chunk 0x%04X\n", Chunk.ID );

            SkipChunk(InFile, Chunk.Length);
            break;
      }
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void  ReadTriMesh(FILE *InFile, FILE *OutFile, INT ChunkLength)
{
INT   RetCode, EndPos;
CHK   Chunk;

   if (DebugFlag)
      printf( "TriMesh\n" );

   MeshList = (MSH *) realloc( MeshList, (MeshCount+1) * sizeof(MSH) );

   if (!MeshList)
      FatalError( "Out of memory for mesh list" );

   memset( &MeshList[MeshCount], 0, sizeof(MSH) );

   strcpy(MeshList[MeshCount].Name, CurrentNamedObject );

   EndPos = ChunkLength - CHUNK_HEADER_SIZE + ftell(InFile);

   FOREVER
   {
      // Done?
      if (EndPos <= ftell(InFile))
         break;

      RetCode = GetChunkID(InFile, &Chunk);

      // Make sure it's a sub-chunk...
      if (Chunk.ID >> 8 != 0x41 || Chunk.ID == CHUNK_TRIMESH )
      {
         Backup(InFile);
         break;
      }

      // Are we done?
      if (!RetCode)
         break;

      switch(Chunk.ID)
      {
         case CHUNK_VERTEXLIST:
            ReadVertexList(InFile, OutFile, Chunk.Length);
            break;

         case CHUNK_FACELIST:
            ReadFaceList(InFile, OutFile, Chunk.Length);
            break;

         case CHUNK_MATERIALAPP:
            ReadMaterialApp(InFile, OutFile, Chunk.Length);
            break;

         case CHUNK_UVLIST:
            ReadUVList(InFile, OutFile, Chunk.Length);
            break;

         default:
            if (DebugFlag)
               printf( "Skipping chunk 0x%04X\n", Chunk.ID );

            SkipChunk(InFile, Chunk.Length);
            break;
      }
   }

   MeshCount++;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void  ReadVertexList(FILE *InFile, FILE *, INT)
{
   if (DebugFlag)
      printf( "VertexList\n" );

   MyRead(&MeshList[MeshCount].VertexCount, sizeof(MeshList[MeshCount].VertexCount), InFile );

   MeshList[MeshCount].VertexList = (P3D *) malloc( sizeof(P3D) * MeshList[MeshCount].VertexCount );

   if (!MeshList[MeshCount].VertexList)
      FatalError( "Unable to allocate RAM for the vertex list" );

   MyRead( MeshList[MeshCount].VertexList, sizeof(P3D) * MeshList[MeshCount].VertexCount, InFile );

   return;
}

///////////////////////////////////////////////////////////////////////////////

void  ReadFaceList(FILE *InFile, FILE *, INT)
{
INT   i;

   if (DebugFlag)
      printf( "FaceList\n" );

   MyRead(&MeshList[MeshCount].FaceCount, sizeof(MeshList[MeshCount].FaceCount), InFile );

   MeshList[MeshCount].FaceList = (FACE *) malloc( sizeof(FACE) * MeshList[MeshCount].FaceCount );

   if (!MeshList[MeshCount].FaceList)
      FatalError( "Unable to allocate RAM for the face list" );

   memset( MeshList[MeshCount].FaceList, 0, sizeof(FACE) * MeshList[MeshCount].FaceCount );

   for( i = 0; i < MeshList[MeshCount].FaceCount; i++)
   {
      MyRead( &MeshList[MeshCount].FaceList[i].a, sizeof(UWRD), InFile );
      MyRead( &MeshList[MeshCount].FaceList[i].b, sizeof(UWRD), InFile );
      MyRead( &MeshList[MeshCount].FaceList[i].c, sizeof(UWRD), InFile );
      MyRead( &MeshList[MeshCount].FaceList[i].flags, sizeof(UWRD), InFile );
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void  ReadMaterialApp(FILE *InFile, FILE *, INT ChunkLength)
{
INT   Count;
CHR   Name[DEF_STR_LEN+1];
UWRD  FaceNumber;

   if (DebugFlag)
      printf( "MaterialApp\n" );

   GetString( InFile, Name );

   // Unknown word...
   MyRead( &FaceNumber, sizeof(FaceNumber), InFile );

   Count = (ChunkLength - CHUNK_HEADER_SIZE - strlen(Name) - 3) / 2;
   
   while( Count-- )
   {
      MyRead( &FaceNumber, sizeof(FaceNumber), InFile );

      if (FaceNumber > MeshList[MeshCount].FaceCount)
         FatalError( "Material name index overrun" );

      strncpy( MeshList[MeshCount].FaceList[FaceNumber].MaterialName, Name, 80 );
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void  ReadUVList(FILE *InFile, FILE *, INT)
{
INT   i;
FLT   *fTemp;

   if (DebugFlag)
      printf( "UVList\n" );

   MyRead(&MeshList[MeshCount].UVCount, sizeof(MeshList[MeshCount].UVCount), InFile );

   MeshList[MeshCount].UVList = (P2D *) malloc( sizeof(P2D) * MeshList[MeshCount].UVCount );

   if (!MeshList[MeshCount].UVList)
      FatalError( "Unable to allocate RAM for the UV list" );

   memset( MeshList[MeshCount].UVList, 0, sizeof(P2D) * MeshList[MeshCount].UVCount );

   MyRead( MeshList[MeshCount].UVList, sizeof(P2D) * MeshList[MeshCount].UVCount, InFile );

   for( i = 0; i < MeshList[MeshCount].UVCount; i++ )
   {
      fTemp = &MeshList[MeshCount].UVList[i].x;
      if (*fTemp > 99.9 || *fTemp < -99.0) *fTemp = 0.0;
      fTemp = &MeshList[MeshCount].UVList[i].y;
      if (*fTemp > 99.9 || *fTemp < -99.0) *fTemp = 0.0;
   }

   return;
}

///////////////////////////////////////////////////////////////////////////////

void  ReadLight(FILE *InFile, FILE *OutFile, INT ChunkLength)
{
INT   RetCode, EndPos;
CHK   Chunk;

   LgtList = (LGT *) realloc( LgtList, (LgtCount+1) * sizeof(LGT) );

   if (!LgtList)
      FatalError( "Out of memory for light list" );

   memset( &LgtList[LgtCount], 0, sizeof(LGT) );

   strcpy(LgtList[LgtCount].Name, CurrentNamedObject );

   EndPos = ChunkLength - CHUNK_HEADER_SIZE + ftell(InFile);

   if (DebugFlag)
      printf( "Light\n" );

   MyRead(&LgtList[LgtCount].loc, sizeof(P3D), InFile );

   FOREVER
   {
      // Done?
      if (EndPos <= ftell(InFile))
         break;

      RetCode = GetChunkID(InFile, &Chunk);

      // Make sure it's a sub-chunk...
      if ((Chunk.ID >> 8 != 0x46 && Chunk.ID >> 8 != 0x00) || Chunk.ID == CHUNK_LIGHT )
      {
         Backup(InFile);
         break;
      }

      // Are we done?
      if (!RetCode)
         break;

      switch(Chunk.ID)
      {
         case CHUNK_RGB:
            ReadLightRGB(InFile, OutFile, Chunk.Length);
            break;

         case CHUNK_24BIT:
            ReadLight24Bit(InFile, OutFile, Chunk.Length);
            break;

         case CHUNK_SPOTLIGHT:
            ReadSpotLight(InFile, OutFile, Chunk.Length);
            break;

         case CHUNK_NOLIGHT:
            ReadNoLight(InFile, OutFile, Chunk.Length);
            break;

         default:
            if (DebugFlag)
               printf( "Skipping chunk 0x%04X\n", Chunk.ID );

            SkipChunk(InFile, Chunk.Length);
            break;
      }
   }
   
   LgtCount++;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void  ReadLightRGB(FILE *InFile, FILE *, INT)
{
   if (DebugFlag)
      printf( "RGB\n" );

   MyRead(&LgtList[LgtCount].rgb, sizeof(RGB), InFile );
   return;
}

///////////////////////////////////////////////////////////////////////////////

void  ReadLight24Bit(FILE *InFile, FILE *, INT)
{
  B24 Color;

   if (DebugFlag)
      printf( "24-Bit\n" );

   MyRead(&Color, sizeof(Color), InFile );

   LgtList[LgtCount].rgb.r = (FLT) Color.r / 255.0;
   LgtList[LgtCount].rgb.g = (FLT) Color.g / 255.0;
   LgtList[LgtCount].rgb.b = (FLT) Color.b / 255.0;

   return;
}

///////////////////////////////////////////////////////////////////////////////

void  ReadSpotLight(FILE *InFile, FILE *OutFile, INT ChunkLength)
{
   if (DebugFlag)
      printf( "SpotLight\n" );

   PutIndent( OutFile );

   LgtList[LgtCount].SpotLight = 1;

   // This one's unused...
   SkipChunk(InFile, ChunkLength);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void  ReadNoLight(FILE *InFile, FILE *, INT ChunkLength)
{
   if (DebugFlag)
      printf( "NoLight\n" );

   LgtList[LgtCount].NoLight = 1;

   // This one's unused...
   SkipChunk(InFile, ChunkLength);

   return;
}

///////////////////////////////////////////////////////////////////////////////

void  ReadCamera(FILE *InFile, FILE *, INT)
{
   if (DebugFlag)
      printf( "Camera\n" );

   CamList = (CAM *) realloc( CamList, (CamCount+1) * sizeof(CAM) );

   if (!CamList)
      FatalError( "Out of memory for light list" );

   MyRead(&CamList[CamCount].loc, sizeof(P3D), InFile );
   MyRead(&CamList[CamCount].target, sizeof(P3D), InFile );
   MyRead(&CamList[CamCount].bank, sizeof(FLT), InFile );
   MyRead(&CamList[CamCount].lense, sizeof(FLT), InFile );
   strcpy(CamList[CamCount].Name, CurrentNamedObject );

   CamCount++;
   return;
}

///////////////////////////////////////////////////////////////////////////////

void  SkipChunk(FILE *File, ULNG Length)
{
   fseek( File, Length + ftell(File) - CHUNK_HEADER_SIZE, SEEK_SET );
}

///////////////////////////////////////////////////////////////////////////////

void  Backup(FILE *File)
{
   fseek( File, ftell(File) - CHUNK_HEADER_SIZE, SEEK_SET );
}

///////////////////////////////////////////////////////////////////////////////

void  FatalError( CHR *Reason )
{
   printf("\n" );
   printf("///////////////////////////////////////////////////////////////////////////////\n");
   printf("//                                                                  //\n" );
   printf("// Fatal error:  %-50s //\n", Reason );
   printf("//                                                                   //\n" );
   printf("///////////////////////////////////////////////////////////////////////////////\n");
   printf("\n%c", BELL );

   exit(1);
}

///////////////////////////////////////////////////////////////////////////////

void  PrintCopyright()
{
   printf("3DS2USD v1.70 -- Converts 3DS files to USD files\n");
   printf("(c) Copyright 1997 Paul D. Nettle, All rights reserved.\n\n");
}

///////////////////////////////////////////////////////////////////////////////

void  PrintUsage()
{
   printf("USAGE:\n");
   printf("  3DS2USD [options] <infile> [outfile]\n\n");
   printf("  Options:  [?|H] - This help information\n" );
   printf("           [Mnnn] - nnn = Mapname (default = MAPNAME)\n" );
   printf("            [Snn] - nn = Swap (xy | yz | zx )\n\n" );
   printf("              [C] - Complete USD file (adds Universe declaration)\n" );
   printf("              [D] - Debugging information on\n" );
   printf("              [F] - Fixup the UV coordinates (fix overflow)\n" );
   printf("              [P] - Exclude polygons from USD output (vertices only)\n" );
   printf("              [U] - Output with unique vertices for all faces\n" );
   printf("              [X] - X Multiplier (default = 1.0)\n" );
   printf("              [Y] - Y Multiplier (default = 1.0)\n" );
   printf("              [Z] - Z Multiplier (default = 1.0)\n" );
}

///////////////////////////////////////////////////////////////////////////////

void  PutIndent( FILE *OutFile )
{
CHR  IndentString[81];

   strcpy( IndentString, "                                                                                " );
   IndentString[IndentLevel*3] = 0;
   fprintf( OutFile, IndentString );
}

///////////////////////////////////////////////////////////////////////////////

void  IncIndent( FILE *OutFile )
{
   PutIndent(OutFile);
   fprintf( OutFile, "{\n" );
   IndentLevel++;
}

///////////////////////////////////////////////////////////////////////////////

void  DecIndent( FILE *OutFile )
{
   IndentLevel--;
   IndentLevel = MAX(0, IndentLevel);
   PutIndent(OutFile);
   fprintf( OutFile, "}\n\n" );
}

///////////////////////////////////////////////////////////////////////////////

INT   MyRead( void *Buffer, INT Length, FILE *File)
{
INT   Count;

   Count = fread(Buffer, 1, Length, File);

   if (Count == 0 && feof(File))
      return 0;

   if (Count != Length)
      FatalError( "Unable to read from input file" );

   return 1;
}

///////////////////////////////////////////////////////////////////////////////

void  GetString( FILE *InFile, CHR *String )
{
INT   i, Char;

   for( i = 0; i < DEF_STR_LEN; i++)
   {
      Char = fgetc(InFile);

      if (Char == ' ')
         Char = '_';

      String[i] = Char;

      if (!Char)
         return;
   }
}

///////////////////////////////////////////////////////////////////////////////

// [3DS2USD.C   ] - End Of File
