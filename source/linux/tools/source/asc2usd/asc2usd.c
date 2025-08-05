// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

//   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.
//
//   [ASC2USD.C   ] - Conversion utility used to convert from ASC to USD
//
//   Updated for Linux on 9/2/97 by Matt Wilhelm

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <ctype.h>

#include "asc2usd.h"

///////////////////////////////////////////////////////////////////////////////

PARSE ParseList[] = {
   {"Ambient light color",  ReadAmbient},
   {"Named object",         ReadNamedObject},
   {"Direct light",         ReadDirectLight},
   {"Position",             ReadPosition},
   {"Light color",          ReadLightColor},
   {"Camera",               ReadCamera},
   {"Tri-mesh",             ReadTriMesh},
   {NULL,NULL}
};

///////////////////////////////////////////////////////////////////////////////

int      IndentLevel = 1, InObject = FALSE, FixUV = FALSE;
char     CurrentObjectName[80];

double   XMultiplier = 1.0, YMultiplier = 1.0, ZMultiplier = 1.0;
char     MapName[80] = {"MAPFILE"};
int      Swap = 0, NoPolyOutput = FALSE, Quiet = FALSE, UniverseFlag = FALSE;
int      UniqueVertices = FALSE;

///////////////////////////////////////////////////////////////////////////////

int   main(int argc, char *argv[])
{
int   i;
char  InName[80], OutName[80];
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

   InFile = fopen(InName, "r");

   if (!InFile)
      FatalError("Unable to open input file");

   OutFile = fopen(OutName, "w");

   if (!OutFile)
      FatalError("Unable to open output file");

   fprintf( OutFile, ";//////////////////////////////////////////////////////////////////////////////\n");
   fprintf( OutFile, ";//                                                                          //\n" );
   fprintf( OutFile, ";//                     This file was created by ASC2USD                     //\n" );
   fprintf( OutFile, ";//                                                                          //\n" );
   fprintf( OutFile, ";///////////////////////////////////////////////////////////////////////////////\n");

   if (UniverseFlag == TRUE)
   {
      fprintf( OutFile, "Universe\n{\n" );
      PutIndent(OutFile);
      fprintf( OutFile, "Name Universe\n\n" );
      PutIndent(OutFile);
      fprintf( OutFile, "Surface Textured        ;Stock values... modify at will\n" );
      PutIndent(OutFile);
      fprintf( OutFile, "Shading Gouraud         ;\n" );
      PutIndent(OutFile);
      fprintf( OutFile, "Color   1.0 1.0 1.0 1.0 ;\n" );
      PutIndent(OutFile);
      fprintf( OutFile, "kd      1.0             ;\n" );
      PutIndent(OutFile);
      fprintf( OutFile, "ka      1.0             ;\n" );
      PutIndent(OutFile);
      fprintf( OutFile, "ks      1.0             ;\n" );
      PutIndent(OutFile);
      fprintf( OutFile, "shi     1.0             ;\n\n" );
   }

   ReadASC(InFile, OutFile);

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

void  ReadASC(FILE *InFile, FILE *OutFile)
{
int   i;
char  Line[DEF_STR_LEN+1];
   
   while(1)
   {
      if (!fgets(Line, DEF_STR_LEN, InFile))
         break;

      for (i = 0; ParseList[i].String; i++ )
      {
         if (!strncmp(Line, ParseList[i].String, strlen(ParseList[i].String)))
            if (!(ParseList[i].Handler)(InFile, OutFile, Line))
               FatalError( "Error during ASC parsing" );
      }
   }
}

///////////////////////////////////////////////////////////////////////////////

int   ReadAmbient(FILE *, FILE *OutFile, char *Line)
{
double   Red = 0.0, Green = 0.0, Blue = 0.0;

   sscanf(Line, "Ambient light color: Red=%lf Green=%lf Blue=%lf",
                                     &Red,   &Green,   &Blue);
   PutIndent(OutFile);
   fprintf( OutFile, "Ambient %f %f %f\n\n", Red, Green, Blue);
   return 1;
}

///////////////////////////////////////////////////////////////////////////////

int   ReadNamedObject( FILE *, FILE *, char *Line )
{
   strcpy( CurrentObjectName, &Line[15] );
   CurrentObjectName[strlen(CurrentObjectName)-2] = 0;

   return 1;
}

///////////////////////////////////////////////////////////////////////////////

int   ReadDirectLight( FILE *, FILE *OutFile, char * )
{
   if (InObject == TRUE)
      DecIndent(OutFile);

   PutIndent(OutFile);
   fprintf( OutFile, "Light\n" );

   IncIndent(OutFile);
   PutIndent(OutFile);
   fprintf( OutFile, "Name %s\n", CurrentObjectName );

   InObject = TRUE;

   return 1;
}

///////////////////////////////////////////////////////////////////////////////

int   ReadPosition( FILE *, FILE *OutFile, char *Line )
{
double   X = 0, Y = 0, Z = 0;

   sscanf(Line, "Position: X:%lf Y:%lf Z:%lf", &X, &Y, &Z);

   PutIndent(OutFile);
   fprintf( OutFile, "Location %f %f %f\n", X, Y, Z );

   PutIndent(OutFile);
   fprintf( OutFile, "Center   %f %f %f\n", X, Y, Z );
   
   return 1;
}

///////////////////////////////////////////////////////////////////////////////

int   ReadLightColor( FILE *, FILE *OutFile, char *Line )
{
double   Red = 0.0, Green = 0.0, Blue = 0.0;

   sscanf(Line, "Light color: Red=%lf Green=%lf Blue=%lf", &Red, &Green, &Blue);

   PutIndent(OutFile);
   fprintf( OutFile, "Intensity %f %f %f\n", Red, Green, Blue);

   return 1;
}

///////////////////////////////////////////////////////////////////////////////

int   ReadCamera( FILE *, FILE *OutFile, char * )
{
   if (InObject == TRUE)
      DecIndent(OutFile);

   PutIndent(OutFile);
   fprintf( OutFile, "Camera\n" );

   IncIndent(OutFile);
   PutIndent(OutFile);
   fprintf( OutFile, "Name %s\n", CurrentObjectName );

   InObject = TRUE;

   return 1;
}

///////////////////////////////////////////////////////////////////////////////

int   ReadTriMesh( FILE *InFile, FILE *OutFile, char *Line )
{
int   VertexCount = 0, FaceCount = 0;
VER   *VertexList = NULL;
TRI   *FaceList = NULL;

   if (InObject == TRUE)
      DecIndent(OutFile);

   PutIndent(OutFile);
   fprintf( OutFile, "Object\n" );
   IncIndent(OutFile);

   PutIndent(OutFile);
   fprintf( OutFile, "Name %s\n", CurrentObjectName );

   sscanf(Line, "Tri-mesh, Vertices: %d Faces: %d", &VertexCount, &FaceCount );

   if (!Quiet) printf( "Object %s contains %d vertices and %d faces.\n", CurrentObjectName, VertexCount, FaceCount );

   if (VertexList)
      free(VertexList);

   if (FaceList)
      free(FaceList);

   VertexList = (VER *) malloc(sizeof(VER) * VertexCount);

   if (!VertexList)
      FatalError( "Unable to allocate RAM for vertex list" );

   FaceList = (TRI *) malloc(sizeof(TRI) * FaceCount);

   if (!FaceList)
      FatalError( "Unable to allocate RAM for face list" );

   ReadVertices(InFile, VertexCount, VertexList);
   ReadFaces(InFile, FaceCount, FaceList);

   WriteUSDFile(OutFile, VertexCount, FaceCount, VertexList, FaceList);

   DecIndent(OutFile);
   InObject = FALSE;

   return 1;
}

///////////////////////////////////////////////////////////////////////////////

void  ReadVertices(FILE *InFile, int VertexCount, VER *List)
{
int      i, Index;
char     Line[DEF_STR_LEN+1];
double   dTemp, X = 0.0, Y = 0.0, Z = 0.0, U = 0.0, V = 0.0;

   if (!Quiet) printf( "Reading Vertices" );

   for( i = 0; i < VertexCount; i++ )
   {
      if (!fgets(Line, DEF_STR_LEN, InFile))
         FatalError( "Unable to read vertex" );

      if (!strncmp(Line, "Vertex list:", 12))
      {
         i--;
         continue;
      }

      if (strncmp(Line, "Vertex", 6))
      {
         i--;
         continue;
      }

      Index = -1;

      sscanf(Line, "Vertex %d: X:%lf Y:%lf Z:%lf U:%lf V:%lf",
            &Index, &X, &Y, &Z, &U, &V);

      if (Index == -1)
         FatalError( "Unable to read vertex" );

      if (Swap == SWAP_XY) SWAP( X, Y, dTemp );
      if (Swap == SWAP_YZ) SWAP( Y, Z, dTemp );
      if (Swap == SWAP_ZX) SWAP( Z, X, dTemp );

      X *= XMultiplier;
      Y *= YMultiplier;
      Z *= ZMultiplier;

      if (FixUV)
      {
         while(U > 1.0) U -= 1.0;
         while(U < 0.0) U += 1.0;
         while(V > 1.0) V -= 1.0;
         while(V < 0.0) V += 1.0;
      }

      List[Index].x = X;
      List[Index].y = Y;
      List[Index].z = Z;
      List[Index].u = U;
      List[Index].v = V;
   }

   if (!Quiet) printf( "\r                \r" );
}

///////////////////////////////////////////////////////////////////////////////

void  ReadFaces(FILE *InFile, int, TRI *List)
{
int   Index = 0, Offset;
int   a = 0, b = 0, c = 0;
char  Line[DEF_STR_LEN] = "", Material[80] = "";

   if (!Quiet) printf( "Reading Faces" );

   while( 1 )
   {
      Offset = ftell( InFile );

      if (!fgets(Line, DEF_STR_LEN, InFile))
         break;

      if (!strncmp(Line, "Face list:", 10))
         continue;
      if (!strncmp(Line, "Smoothing", 9))
         continue;
      if (!strncmp(Line, "Page", 4))
         continue;
      if (Line[0] == ' ')
         continue;
      if (Line[0] == '\n')
         continue;
      if (!Line[0])
         continue;

      if (!strncmp(Line, "Face", 4))
      {
         Index = -1;

         sscanf (Line, "Face %d: A:%d B:%d C:%d", &Index, &a, &b, &c);

         if (Index == -1)
            FatalError( "Unable to read face" );

         List[Index].a = a;
         List[Index].b = b;
         List[Index].c = c;
         strcpy(List[Index].Mat, MapName);
      }
      else if (!strncmp(Line, "Material:", 9))
      {
         strcpy( Material, &Line[10] );
         Material[strlen(Material)-2] = 0;
         strcpy(List[Index].Mat, Material);
      }
      else
         break;
   }

   fseek( InFile, Offset, SEEK_SET );

   if (!Quiet) printf( "\r             \r" );
}

///////////////////////////////////////////////////////////////////////////////

void  WriteUSDFile(FILE *Fp, int VertexCount, int FaceCount, VER *VList, TRI *FList)
{
int   i;

   if (!Quiet) printf( "Writing USD Vertices" );

   if (UniqueVertices == FALSE)
   {
      for (i = 0; i < VertexCount; i++)
      {
         PutIndent(Fp);
         fprintf(Fp, "Vertex %12.5f %12.5f %12.5f\n",
               VList[i].x,  VList[i].y,  VList[i].z);
      }
   }
   else
   {
      for (i = 0; i < FaceCount; i++)
      {
         PutIndent(Fp);
         fprintf(Fp, "Vertex %12.5f %12.5f %12.5f\n",
               VList[FList[i].a].x, VList[FList[i].a].y, VList[FList[i].a].z);
         PutIndent(Fp);
         fprintf(Fp, "Vertex %12.5f %12.5f %12.5f\n",
               VList[FList[i].b].x, VList[FList[i].b].y, VList[FList[i].b].z);
         PutIndent(Fp);
         fprintf(Fp, "Vertex %12.5f %12.5f %12.5f\n",
               VList[FList[i].c].x, VList[FList[i].c].y, VList[FList[i].c].z);
      }
   }

   if (!Quiet) printf( "\r                    \r" );

   if (NoPolyOutput == TRUE)
      return;

   fprintf(Fp, "\n" );

   if (!Quiet) printf( "Writing USD Triangles" );

   if (UniqueVertices == FALSE)
   {
      for (i = 0; i < FaceCount; i++)
      {
         PutIndent(Fp);
         fprintf(Fp, "Triangle %4d %4d %4d %8s %9.5f %9.5f %9.5f %9.5f %9.5f %9.5f\n",
               FList[i].a, FList[i].b, FList[i].c, FList[i].Mat,
               1.0-VList[FList[i].a].u, 1.0-VList[FList[i].a].v,
               1.0-VList[FList[i].b].u, 1.0-VList[FList[i].b].v,
               1.0-VList[FList[i].c].u, 1.0-VList[FList[i].c].v );
      }
   }
   else
   {
      for (i = 0; i < FaceCount; i++)
      {
         PutIndent(Fp);
         fprintf(Fp, "Triangle %4d %4d %4d %8s %9.5f %9.5f %9.5f %9.5f %9.5f %9.5f\n",
               i*3+0, i*3+1, i*3+2, FList[i].Mat,
               1.0-VList[FList[i].a].u, 1.0-VList[FList[i].a].v,
               1.0-VList[FList[i].b].u, 1.0-VList[FList[i].b].v,
               1.0-VList[FList[i].c].u, 1.0-VList[FList[i].c].v );
      }
   }
   if (!Quiet) printf( "\r                     \r" );
}

///////////////////////////////////////////////////////////////////////////////

void  FatalError( char *Reason )
{
   printf("\n" );
   printf("///////////////////////////////////////////////////////////////////////////////\n");
   printf("//                                                                  //\n" );
   printf("// Fatal error:  %-50s //\n", Reason );
   printf("//                                                                  //\n" );
   printf("///////////////////////////////////////////////////////////////////////////////\n");
   printf("\n%c", BELL );

   exit(1);
}

///////////////////////////////////////////////////////////////////////////////

void  PrintCopyright()
{
   printf("ASC2USD v1.70 -- Converts ASC files to USD files\n");
   printf("(c) Copyright 1997 Paul D. Nettle, All rights reserved.\n\n");
}

///////////////////////////////////////////////////////////////////////////////

void  PrintUsage()
{
   printf("USAGE:\n");
   printf("  ASC2USD [options] <infile> [outfile]\n\n");
   printf("  Options:  [?|H] - This help information\n" );
   printf("           [Mnnn] - nnn = Mapname (default = MAPNAME)\n" );
   printf("            [Snn] - nn = Swap (xy | yz | zx )\n\n" );
   printf("              [C] - Complete USD file (adds Universe declaration)\n" );
   printf("              [F] - Fixup the UV coordinates (fix overflow)\n" );
   printf("              [P] - Exclude polygons from USD output (vertices only)\n" );
   printf("              [U] - Output with unique vertices for all faces\n" );
   printf("              [X] - X Multiplier (default = 1.0)\n" );
   printf("              [Y] - Y Multiplier (default = 1.0)\n" );
   printf("              [Z] - Z Multiplier (default = 1.0)\n" );
}

///////////////////////////////////////////////////////////////////////////////

void  PutIndent( FILE *Fp )
{
char  IndentString[81];

   strcpy( IndentString, "                                                                                " );
   IndentString[IndentLevel*3] = 0;
   fprintf( Fp, IndentString );
}

///////////////////////////////////////////////////////////////////////////////

void  IncIndent( FILE *Fp )
{
   PutIndent(Fp);
   fprintf( Fp, "{\n" );
   IndentLevel++;
}

///////////////////////////////////////////////////////////////////////////////

void  DecIndent( FILE *Fp )
{
   IndentLevel--;
   IndentLevel = MAX(0, IndentLevel);
   PutIndent(Fp);
   fprintf( Fp, "}\n\n" );
}

///////////////////////////////////////////////////////////////////////////////

//  [ASC2USD.C   ] - End Of File
