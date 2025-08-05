// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [READUSD.C   ] - Reads USD (Universe Scene Descriptor)                 ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <mem.h>

#include "spandex.h"

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static   FLT   *TranslationTable;
static   INT   TranslationTableOffset;
static   PAL   *WorkingPalette;
static   FLT   ColorIntensity = 1.0;

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static   INT   StoreUSDLine( CHR *Bytes, INT LineLength, CHR *Buffer, INT Width, INT Y );
static   INT   PushTranslationTable( FLT X, FLT Y, FLT Z );
static   void  PopTranslationTable( void );
static   void  TranslatePoint( P3D *Point );
static   INT   PreProcessFile( INT Handle, CHR **FileData, INT *MemoryAllocated );
static   INT   StripLTW( CHR *String );
static   void  StripComments( CHR *String );
static   CHR   *GetNextWord( CHR *FileData, CHR *Word );
static   INT   ProcessWordForObject( CHR **FileData, OBJ **Object );
static   INT   ProcessWordForMaterial( CHR **FileData, MTRL **Material );
static   INT   ProcessWordForLight( CHR **FileData, LGT **Light );
static   void  ProcessFile(CHR **FileData, OBJ **Universe);
static   INT   GetInteger( CHR **FileData, INT *Value );
static   INT   GetByte( CHR **FileData, BYT *Value );
static   INT   GetFloat( CHR **FileData, FLT *Value );
static   INT   GetP2D( CHR **FileData, P2D *Value );
static   INT   GetVER( CHR **FileData, VER *Value );
static   INT   GetRGB( CHR **FileData, RGB *Value );
static   INT   GetP3D( CHR **FileData, P3D *Value );
static   INT   GetVEC( CHR **FileData, VEC *Value );
static   INT   GetROT( CHR **FileData, ROT *Value );

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static   INT   ReadObjectTarget(CHR **FileData, OBJ **Object);
static   INT   ReadLense(CHR **FileData, OBJ **Object);
static   INT   ReadObjectDirection(CHR **FileData, OBJ **Object);
static   INT   ReadVertex(CHR **FileData, OBJ **Object);
static   INT   ReadTriangle(CHR **FileData, OBJ **Object);
static   INT   ReadQuad(CHR **FileData, OBJ **Object);
static   INT   ReadPoint(CHR **FileData, OBJ **Object);
static   INT   ReadChildObject(CHR **FileData, OBJ **Object);
static   INT   ReadLight(CHR **FileData, OBJ **Object);
static   INT   ReadObjectLocation(CHR **FileData, OBJ **Object);
static   INT   ReadOrientation(CHR **FileData, OBJ **Object);
static   INT   ReadShowBacks(CHR **FileData, OBJ **Object);
static   INT   ReadShadeModel(CHR **FileData, OBJ **Object);
static   INT   ReadDevUse(CHR **FileData, OBJ **Object);
static   INT   ReadDrawFlags(CHR **FileData, OBJ **Object);
static   INT   ReadType(CHR **FileData, OBJ **Object);
static   INT   BeginTranslationBlock(CHR **FileData, OBJ **Object);
static   INT   EndTranslationBlock(CHR **FileData, OBJ **Object);
static   INT   ReadUniverse(CHR **FileData, OBJ **Universe);
static   INT   ReadMaterial(CHR **FileData, OBJ **Object);

OBJPARSELIST ObjectParseList[] =
{
   {"TAR", ReadObjectTarget},
   {"LEN", ReadLense},
   {"DIR", ReadObjectDirection},
   {"VER", ReadVertex},
   {"TRI", ReadTriangle},
   {"QUA", ReadQuad},
   {"POI", ReadPoint},
   {"OBJ", ReadChildObject},
   {"CAM", ReadChildObject},
   {"LIG", ReadLight},
   {"LOC", ReadObjectLocation},
   {"CEN", ReadObjectLocation},
   {"ORI", ReadOrientation},
   {"SHO", ReadShowBacks},
   {"BAC", ReadShowBacks},
   {"SHA", ReadShadeModel},
   {"DEV", ReadDevUse},
   {"DRA", ReadDrawFlags},
   {"TYP", ReadType},
   {"TRB", BeginTranslationBlock},
   {"TRE", EndTranslationBlock},
   {"UNI", ReadUniverse},
   {"MAT", ReadMaterial},
   {NULL,  NULL}
};

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static   INT   ReadDiffuseCo(CHR **FileData, MTRL **Material);
static   INT   ReadAmbientCo(CHR **FileData, MTRL **Material);
static   INT   ReadSpecularCo(CHR **FileData, MTRL **Material);
static   INT   ReadShineValue(CHR **FileData, MTRL **Material);
static   INT   ReadTexture(CHR **FileData, MTRL **Material);
static   INT   ReadBumpMap(CHR **FileData, MTRL **Material);
static   INT   ReadMaterialColor(CHR **FileData, MTRL **Material);
static   INT   ReadSurface(CHR **FileData, MTRL **Material);
static   INT   ReadAmbient(CHR **FileData, MTRL **Material);
static   INT   ReadTransparent(CHR **FileData, MTRL **Material);

MTRLPARSELIST MaterialParseList[] =
{
   {"KD",  ReadDiffuseCo},
   {"KA",  ReadAmbientCo},
   {"KS",  ReadSpecularCo},
   {"SHI", ReadShineValue},
   {"BUM", ReadBumpMap},
   {"TEX", ReadTexture},
   {"COL", ReadMaterialColor},
   {"SUR", ReadSurface},
   {"AMB", ReadAmbient},
   {"TRA", ReadTransparent},
   {NULL,  NULL}
};

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static   INT   ReadLightLocation(CHR **FileData, LGT **Light);
static   INT   ReadLightIntensity(CHR **FileData, LGT **Light);

LGTPARSELIST LightParseList[] =
{
   {"LOC", ReadLightLocation},
   {"CEN", ReadLightLocation},
   {"INT", ReadLightIntensity},
   {"COL", ReadLightIntensity},
   {NULL,  NULL}
};

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

BYTEPARSELIST ByteParseList[] =
{
   {"ON",   TRUE},
   {"OFF",  FALSE},
   {"YES",  TRUE},
   {"NO",   FALSE},
   {"TRU",  TRUE},
   {"FAL",  FALSE},
   {"TRI",  DRAW_TRIS},
   {"QUA",  DRAW_QUADS},
   {"POI",  DRAW_POINTS},
   {"ALL",  DRAW_ALL},
   {"NON",  DRAW_NONE},
   {"BUM",  BUMP},
   {"PHO",  PHONG},
   {"GOU",  GOURAUD},
   {"FLA",  LAMBERT},
   {"LAM",  LAMBERT},
   {"AMB",  AMBIENT},
   {"PUR",  SOLID},
   {"SOL",  SOLID},
   {"LOO",  TEXTURED},
   {"TEX",  TEXTURED},
   {"ENV",  ENVMAP},
   {"REF",  ENVMAP},
   {NULL,   NULL}
};

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXReadRawUSD( CHR *FileName, OBJ **Universe, PAL *MapPalette, FLT ShineValue )
{
INT   RetCode;
INT   MemoryAllocated;
CHR   *FileData, *SavedFileData;
INT   Handle;

   FUNC("SPDXReadRawUSD");
   Assert( FileName );
   Assert( Universe );
   Assert( MapPalette );

   ColorIntensity = ShineValue;
   WorkingPalette = MapPalette;

   Handle = SPDXOpen( FileName, O_RDONLY|O_BINARY, 0 );

   if (Handle < 0)
      return LE_NOUSDOPEN;

   MemoryAllocated = USD_FILE_CHUNK;

   FileData = (CHR *) SPDXMalloc( MemoryAllocated );

   if (!FileData)
   {
      SPDXLogError( LE_NOMEM, "Unable to allocate memory for USD file" );
      SPDXClose( Handle );
      return LE_NOMEM;
   }

   TranslationTable = (FLT *) SPDXMalloc( MAX_TRANS_DEPTH * 3 * sizeof(FLT) );

   if (!TranslationTable)
   {
      SPDXLogError( LE_NOMEM, "Unable to allocate memory for translation table" );
      SPDXClose( Handle );
      SPDXFree( FileData );
      return LE_NOMEM;
   }

   TranslationTableOffset = 0;

   FileData[0] = '\0';

   RetCode = PreProcessFile( Handle, &FileData, &MemoryAllocated );

   SPDXClose( Handle );

   SavedFileData = FileData;

   if (RetCode != LE_NONE)
   {
      SPDXLogError( LE_NOMEM, "Unable to pre-process USD file" );
      SPDXFree( TranslationTable );
      SPDXFree( FileData );
      return RetCode;
   }

   ProcessFile( &FileData, Universe );

   SPDXFree( TranslationTable );
   SPDXFree( SavedFileData );

   if (TranslationTableOffset)
      SPDXLogError( LE_RANGE, "Uneven translation recursion" );

   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXReadCompressedUSD( OBJ **Universe, INT Handle, PAL *MapPalette, FLT ShineValue )
{
INT   RetCode;
CHR   *FileData, *SavedFileData;
INT   TotalMem, ArrayLength, ArrayCount;

   FUNC("SPDXReadCompressedUSD");
   Assert( Universe );
   Assert( Handle );
   Assert( MapPalette );

   ColorIntensity = ShineValue;
   WorkingPalette = MapPalette;

   RetCode = SPDXRead( Handle, (CHR *) &TotalMem, 4 );

   if (RetCode != 4)
      return LE_NOREAD;

   RetCode = SPDXRead( Handle, (CHR *) &ArrayLength, 4 );

   if (RetCode != 4)
      return LE_NOREAD;

   RetCode = SPDXRead( Handle, (CHR *) &ArrayCount, 4 );

   if (RetCode != 4)
      return LE_NOREAD;

   FileData = (CHR *) SPDXMalloc( ArrayLength * ArrayCount );

   if (!FileData)
   {
      SPDXLogError( LE_NOMEM, "Unable to allocate memory for USD file" );
      SPDXClose( Handle );
      return LE_NOMEM;
   }
   SPDXMemSetDWORD( FileData, 0, (ArrayLength * ArrayCount) >> 2 );
   TranslationTable = (FLT *) SPDXMalloc( MAX_TRANS_DEPTH * 3 * sizeof(FLT) );

   if (!TranslationTable)
   {
      SPDXLogError( LE_NOMEM, "Unable to allocate memory for translation table" );
      SPDXClose( Handle );
      SPDXFree( FileData );
      return LE_NOMEM;
   }

   TranslationTableOffset = 0;

   RetCode = SPDXDecompress(Handle, ArrayLength, FileData, ArrayLength, StoreUSDLine );

   if (RetCode != LE_NONE)
      return RetCode;

   SavedFileData = FileData;

   if (RetCode != LE_NONE)
   {
      SPDXLogError( LE_NOMEM, "Unable to pre-process USD file" );
      SPDXFree( TranslationTable );
      SPDXFree( FileData );
      return RetCode;
   }

   ProcessFile( &FileData, Universe );

   SPDXFree( TranslationTable );
   SPDXFree( SavedFileData );

   if (TranslationTableOffset)
      SPDXLogError( LE_RANGE, "Uneven translation recursion" );

   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   StoreUSDLine( CHR *Bytes, INT LineLength, CHR *Buffer, INT Width, INT Y )
{
   FUNC("StoreUSDLine");
   Assert( Bytes );
   Assert( Buffer );

   SPDXMemCopyBYTE( &Buffer[Y * Width], Bytes, Width );
   return LineLength * 3;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   PushTranslationTable( FLT X, FLT Y, FLT Z )
{
   FUNC("PushTranslationTable");
   if (TranslationTableOffset >= MAX_TRANS_DEPTH)
   {
      SPDXLogError( LE_RANGE, "TranslationTable overflow" );
      return LE_RANGE;
   }

   TranslationTable[TranslationTableOffset*3+0] = X;
   TranslationTable[TranslationTableOffset*3+1] = Y;
   TranslationTable[TranslationTableOffset*3+2] = Z;

   TranslationTableOffset++;

   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  PopTranslationTable( void )
{
   FUNC("PopTranslationTable");
   if (TranslationTableOffset > 0)
      TranslationTableOffset--;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  TranslatePoint( P3D *Point )
{
   FUNC("TranslatePoint");
   Assert( Point );

   if (TranslationTableOffset <= 0)
      return;

   Point->x += TranslationTable[(TranslationTableOffset-1)*3+0];
   Point->y += TranslationTable[(TranslationTableOffset-1)*3+1];
   Point->z += TranslationTable[(TranslationTableOffset-1)*3+2];
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   BeginTranslationBlock(CHR **FileData, OBJ **Object)
{
INT   RetCode;
FLT   Tx, Ty, Tz;

   FUNC("BeginTranslationBlock");
   Assert( FileData );
   Object = Object;                             // Supress Compiler Warning

   RetCode = GetFloat( FileData, &Tx);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read translation block" );
      return RetCode;
   }

   RetCode = GetFloat( FileData, &Ty);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read translation block" );
      return RetCode;
   }
   
   RetCode = GetFloat( FileData, &Tz);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read translation block" );
      return RetCode;
   }

   if (PushTranslationTable( Tx, Ty, Tz ) != LE_NONE)
   {
      SPDXLogError( LE_RANGE, "Unable to push translation block" );
      return PARSE_STOP;
   }

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   EndTranslationBlock(CHR **FileData, OBJ **Object)
{
   FUNC("EndTranslationBlock");
   FileData = FileData;                         // Supress Compiler Warning
   Object = Object;                             // Supress Compiler Warning

   PopTranslationTable( );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   PreProcessFile( INT Handle, CHR **FileData, INT *MemoryAllocated )
{
INT   RetCode, Len, FDLength;
CHR   CurrentLine[USD_LINE_LENGTH], *pTmp;
CHR   Label[USD_LINE_LENGTH], FileName[USD_LINE_LENGTH];
INT   NewHandle;

   FUNC("PreProcessFile");
   Assert( FileData );
   Assert( MemoryAllocated );

   FDLength = strlen(*FileData);

   FOREVER
   {
      if (!SPDXReadString( Handle, CurrentLine, USD_LINE_LENGTH ))
         break;

      StripComments( CurrentLine );
      Len = StripLTW( CurrentLine );

      if (!Len)
         continue;

      CurrentLine[Len] = ' ';
      Len++;
      CurrentLine[Len] = '\0';

      *Label = '\0';
      *FileName = '\0';

      sscanf( CurrentLine, "%s %s", Label, FileName );

      if (!stricmp( Label, "#INCLUDE" ))
      {
         NewHandle = SPDXOpen( FileName, O_RDONLY|O_BINARY, 0 );

         if (NewHandle < 0)
            return LE_NOUSDOPENINC;

         RetCode = PreProcessFile( NewHandle, FileData, MemoryAllocated );

         SPDXClose( NewHandle );

         if (RetCode != LE_NONE )
         {
            SPDXLogError( LE_RANGE, "Unable to #include object file" );
            return RetCode;
         }

         FDLength = strlen(*FileData);
      }
      else
      {
         if (Len + FDLength >= *MemoryAllocated)
         {
            *MemoryAllocated += USD_FILE_CHUNK;
            pTmp = (CHR *) SPDXRealloc( *FileData, *MemoryAllocated );

            if (!pTmp)
            {
               SPDXLogError( LE_NOMEM, "Unable to allocate memory for object file" );
               return LE_NOMEM;
            }

            *FileData = pTmp;
         }

         strcpy( &((*FileData)[FDLength]), CurrentLine );
         FDLength += Len;
      }
   }

   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   StripLTW( CHR *String )
{
INT   front, back, off;

   FUNC("StripLTW");
   Assert( String );

   // Find the first non-whitespace
   front = strspn( String, " \f\n\r\t\v" );

   // If the whole string is whitespace, then return...
   if (!String[front])
   {
      *String = '\0';
      return 0;
   }

   // Reverse the string
   strrev(String);

   // Starting at the back, find the last non-whitespace char
   back = strspn( String, " \f\n\r\t\v" );

   // Reverse the string (back to normal)
   strrev(String);

   // Get the offset into the forward string where back lies...
   back = strlen(String) - back;

   // End the string on the last whitespace
   String[back] = '\0';

   // Copy the portion of String into itself starting at 'front'
   memmove( String, &String[front], back-front+1 );

   // Turn these into spaces...
   FOREVER
   {
      off = strcspn( String, "\"[](),|*:<>" );
      if (String[off] == '\0')
         break;

      String[off] = ' ';
   }

   return back-front;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  StripComments( CHR *String )
{
   FUNC("StripComments");
   Assert( String );

   String[strcspn( String, ";/" )] = '\0';
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

CHR   *GetNextWord( CHR *FileData, CHR *Word )
{
CHR   *Tmp1, *Tmp2;

   FUNC("GetNextWord");
   Assert( FileData );
   Assert( Word );

   Tmp1 = &FileData[strspn( FileData, " \f\n\r\t\v" )];

   if (!*Tmp1)
      return NULL;

   Tmp2 = &Tmp1[strcspn( Tmp1, " \f\n\r\t\v" )];

   strncpy( Word, Tmp1, Tmp2 - Tmp1 );

   Word[Tmp2 - Tmp1] = '\0';

   return Tmp2;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   GetInteger( CHR **FileData, INT *Value )
{
CHR   Word[80];

   FUNC("GetInteger");
   Assert( FileData );
   Assert( Value );

   *FileData = GetNextWord( *FileData, Word );

   if (!*FileData)
   {
      SPDXLogError( LE_RANGE, "Unable to read INT value" );
      return PARSE_STOP;
   }

   *Value = atoi( Word );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   GetByte( CHR **FileData, BYT *Value )
{
INT   i;
CHR   Word[80];

   FUNC("GetByte");
   Assert( FileData );
   Assert( Value );

   *FileData = GetNextWord( *FileData, Word );

   if (!*FileData)
   {
      SPDXLogError( LE_RANGE, "Unable to read BYT value" );
      return PARSE_STOP;
   }

   for (i = 0; *ByteParseList[i].Label; i++)
   {
      if (!strnicmp( Word, ByteParseList[i].Label, strlen(ByteParseList[i].Label)))
      {
         *Value = ByteParseList[i].Value;
         return PARSE_CONTINUE;
      }
   }

   *Value = (BYT) atoi( Word );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   GetFloat( CHR **FileData, FLT *Value )
{
CHR   Word[80];

   FUNC("GetFloat");
   Assert( FileData );
   Assert( Value );

   *FileData = GetNextWord( *FileData, Word );

   if (!*FileData)
   {
      SPDXLogError( LE_RANGE, "Unable to read FLT value" );
      return PARSE_STOP;
   }

   *Value = atof( Word );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   GetP2D( CHR **FileData, P2D *Value )
{
INT   RetCode;

   FUNC("GetP2D");
   Assert( FileData );
   Assert( Value );

   RetCode = GetFloat( FileData, &Value->x);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read P2D->x" );
      return PARSE_STOP;
   }

   RetCode = GetFloat( FileData, &Value->y);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read P2D->y" );
      return PARSE_STOP;
   }

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   GetVER( CHR **FileData, VER *Value )
{
INT   RetCode;

   FUNC("GetVER");
   Assert( FileData );
   Assert( Value );

   RetCode = GetP3D( FileData, &Value->LLoc );

   if (RetCode != PARSE_CONTINUE)
      SPDXLogError( LE_RANGE, "Unable to read VER->LLoc" );

   return RetCode;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   GetRGB( CHR **FileData, RGB *Value )
{
INT   RetCode;
FLT   Val;

   FUNC("GetRGB");
   Assert( FileData );
   Assert( Value );

   RetCode = GetFloat( FileData, &Val);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read RGB->r" );
      return RetCode;
   }

   Value->Red = (CHR) ((UBYT) FLT_TO_INT(Val * (FLT) (MAP_COLORS-1)));

   RetCode = GetFloat( FileData, &Val);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read RGB->g" );
      return RetCode;
   }

   Value->Green = (CHR) ((UBYT) FLT_TO_INT(Val * (FLT) (MAP_COLORS-1)));

   RetCode = GetFloat( FileData, &Val);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read RGB->b" );
      return RetCode;
   }

   Value->Blue = (CHR) ((UBYT) FLT_TO_INT(Val * (FLT) (MAP_COLORS-1)));

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   GetP3D( CHR **FileData, P3D *Value )
{
INT   RetCode;

   FUNC("GetP3D");
   Assert( FileData );
   Assert( Value );

   RetCode = GetFloat( FileData, &Value->x);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read P3D->x" );
      return RetCode;
   }

   RetCode = GetFloat( FileData, &Value->y);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read P3D->y" );
      return RetCode;
   }

   RetCode = GetFloat( FileData, &Value->z);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read P3D->z" );
      return RetCode;
   }

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   GetVEC( CHR **FileData, VEC *Value )
{
INT   RetCode;

   FUNC("GetVEC");
   Assert( FileData );
   Assert( Value );

   RetCode = GetFloat( FileData, &Value->dx);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read VEC->x" );
      return RetCode;
   }

   RetCode = GetFloat( FileData, &Value->dy);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read VEC->y" );
      return RetCode;
   }

   RetCode = GetFloat( FileData, &Value->dz);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read VEC->z" );
      return RetCode;
   }

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   GetROT( CHR **FileData, ROT *Value )
{
INT   RetCode;

   FUNC("GetROT");
   Assert( FileData );
   Assert( Value );

   RetCode = GetFloat( FileData, &Value->x);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read ROT->x" );
      return RetCode;
   }

   RetCode = GetFloat( FileData, &Value->y);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read ROT->y" );
      return RetCode;
   }

   RetCode = GetFloat( FileData, &Value->z);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read ROT->z" );
      return RetCode;
   }

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  ProcessFile(CHR **FileData, OBJ **Universe)
{
   FUNC("ProcessFile");
   Assert( FileData );
   Assert( Universe );

   while(ProcessWordForObject( FileData, Universe ) == PARSE_CONTINUE);

   // After adding all the lights, gotta re-build the global list
   SPDXRebuildGlobalLightList(*Universe);
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ProcessWordForObject( CHR **FileData, OBJ **Object )
{
INT   i;
CHR   TempStr[80];
CHR   Word[80];

   FUNC("ProcessWordForObject");
   Assert( FileData );
   Assert( Object );

   *FileData = GetNextWord( *FileData, Word );

   if (!*FileData)
      return PARSE_STOP;

   if (Word[0] == '{')
      return PARSE_BRACE_BEG;

   if (Word[0] == '}')
   {
      // After Loading up an object, gotta unify its normals
      SPDXUnifyObjectNormals( *Object );

      return PARSE_BRACE_END;
   }

   for( i = 0; ObjectParseList[i].Handler; i++ )
      if ( !strnicmp( Word, ObjectParseList[i].Label, strlen(ObjectParseList[i].Label) ) )
         return (ObjectParseList[i].Handler)(FileData, Object);

   sprintf( TempStr, "Unknown label \"%s\" in USD file", Word );
   SPDXLogError( LE_USDSYNTAX, TempStr );

   return PARSE_STOP;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ProcessWordForMaterial( CHR **FileData, MTRL **Material )
{
INT   i;
CHR   TempStr[80];
CHR   Word[80];

   FUNC("ProcessWordForMaterial");
   Assert( FileData );
   Assert( Material );

   *FileData = GetNextWord( *FileData, Word );

   if (!*FileData)
      return PARSE_STOP;

   if (Word[0] == '{')
      return PARSE_BRACE_BEG;

   if (Word[0] == '}')
      return PARSE_BRACE_END;

   for( i = 0; MaterialParseList[i].Handler; i++ )
      if ( !strnicmp( Word, MaterialParseList[i].Label, strlen(MaterialParseList[i].Label) ) )
         return (MaterialParseList[i].Handler)(FileData, Material);

   sprintf( TempStr, "Unknown label \"%s\" in USD file", Word );
   SPDXLogError( LE_USDSYNTAX, TempStr );

   return PARSE_STOP;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ProcessWordForLight( CHR **FileData, LGT **Light )
{
INT   i;
CHR   TempStr[80];
CHR   Word[80];

   FUNC("ProcessWordForLight");
   Assert( FileData );
   Assert( Light );

   *FileData = GetNextWord( *FileData, Word );

   if (!*FileData)

   if (Word[0] == '{')
      return PARSE_BRACE_BEG;

   if (Word[0] == '}')
      return PARSE_BRACE_END;

   for( i = 0; LightParseList[i].Handler; i++ )
      if ( !strnicmp( Word, LightParseList[i].Label, strlen(LightParseList[i].Label) ) )
         return (LightParseList[i].Handler)(FileData, Light);

   sprintf( TempStr, "Unknown label \"%s\" in USD file", Word );
   SPDXLogError( LE_USDSYNTAX, TempStr );

   return PARSE_STOP;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadUniverse(CHR **FileData, OBJ **Universe)
{
INT   RetCode;
CHR   Word[80];

   FUNC("ReadUniverse");
   Assert( FileData );
   Assert( Universe );

   RetCode = SPDXCreateObject( Universe );

   if (RetCode != LE_NONE)
      SPDXFatalError( RetCode, "Unable to create universe object" );

   *FileData = GetNextWord( *FileData, Word );
   
   if (!*FileData)
   {
      SPDXLogError( LE_RANGE, "Incomplete universe" );
      return PARSE_STOP;
   }

   SPDXSetObjectName( *Universe, Word );

   *FileData = GetNextWord( *FileData, Word );
   
   if (!*FileData)
   {
      SPDXLogError( LE_RANGE, "Incomplete universe" );
      return PARSE_STOP;
   }

   if (Word[0] != '{')
   {
      SPDXLogError( LE_USDSYNTAX, "Missing '{' from USD file" );
      return PARSE_STOP;
   }

   while(ProcessWordForObject( FileData, Universe ) == PARSE_CONTINUE);

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadMaterial(CHR **FileData, OBJ **Object)
{
INT   RetCode;
CHR   Word[80];
MTRL  *Material;

   FUNC("ReadMaterial");
   Assert( FileData );
   Assert( Object );

   *FileData = GetNextWord( *FileData, Word );
   
   if (!*FileData)
   {
      SPDXLogError( LE_RANGE, "Incomplete Material (name is missing)" );
      return PARSE_STOP;
   }

   RetCode = SPDXAddMaterial( Word );

   if (RetCode != LE_NONE)
   {
      SPDXLogError( RetCode, "Unable to add material" );
      return PARSE_STOP;
   }

   Material = SPDXFindMaterialByName( Word );

   if (!Material)
   {
      SPDXLogError( RetCode, "Unable to add material" );
      return PARSE_STOP;
   }

   *FileData = GetNextWord( *FileData, Word );
   
   if (!*FileData)
   {
      SPDXLogError( LE_RANGE, "Incomplete Material" );
      return PARSE_STOP;
   }

   if (Word[0] != '{')
   {
      SPDXLogError( LE_USDSYNTAX, "Missing '{' from USD file" );
      return PARSE_STOP;
   }

   while(ProcessWordForMaterial( FileData, &Material ) == PARSE_CONTINUE);

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadChildObject(CHR **FileData, OBJ **Object)
{
CHR   Word[80];
OBJ   *Child;

   FUNC("ReadChildObject");
   Assert( FileData );
   Assert( Object );

   if (SPDXAddChild( *Object ) != LE_NONE)
      return PARSE_STOP;

   Child = SPDXGetChild( *Object, SPDXGetChildCount(*Object) - 1 );

   if (!Child)
   {
      SPDXLogError( LE_RANGE, "Unable to load child object" );
      return PARSE_STOP;
   }

   *FileData = GetNextWord( *FileData, Word );
   
   if (!*FileData)
   {
      SPDXLogError( LE_RANGE, "Incomplete child object" );
      return PARSE_STOP;
   }

   SPDXSetObjectName( Child, Word );

   *FileData = GetNextWord( *FileData, Word );
   
   if (!*FileData)
   {
      SPDXLogError( LE_RANGE, "Incomplete child object" );
      return PARSE_STOP;
   }

   if (Word[0] != '{')
   {
      SPDXLogError( LE_USDSYNTAX, "Missing '{' from USD file" );
      return PARSE_STOP;
   }

   while(ProcessWordForObject( FileData, &Child ) == PARSE_CONTINUE);

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadLight(CHR **FileData, OBJ **Object)
{
CHR   Word[80];
LGT   *Light;

   FUNC("ReadLight");
   Assert( FileData );
   Assert( Object );

   Light = SPDXAddLight( *Object );

   if (!Light)
   {
      SPDXLogError( LE_RANGE, "Unable to add light object" );
      return PARSE_STOP;
   }

   *FileData = GetNextWord( *FileData, Word );
   
   if (!*FileData)
   {
      SPDXLogError( LE_RANGE, "Incomplete light object" );
      return PARSE_STOP;
   }

   SPDXSetLightName( Light, Word );

   *FileData = GetNextWord( *FileData, Word );
   
   if (!*FileData)
   {
      SPDXLogError( LE_RANGE, "Incomplete light object" );
      return PARSE_STOP;
   }

   if (Word[0] != '{')
   {
      SPDXLogError( LE_USDSYNTAX, "Missing '{' from USD file" );
      return PARSE_STOP;
   }

   while(ProcessWordForLight( FileData, &Light ) == PARSE_CONTINUE);

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadTexture(CHR **FileData, MTRL **Material)
{
INT   RetCode;
CHR   Name[DEF_STR_LEN], Msg[80];

   FUNC("ReadTexture");
   Assert( FileData );
   Assert( Material );

   *FileData = GetNextWord( *FileData, Name );
   
   if (!*FileData)
   {
      SPDXLogError( LE_RANGE, "Unable to read material texture" );
      return PARSE_STOP;
   }

   RetCode = SPDXSetMaterialTexture( *Material, Name );

   if (RetCode != LE_NONE)
   {
      sprintf( Msg, "Unable to assign texture map \"%s\"...continuing anyway", Name );
      SPDXLogError( RetCode, Msg );
   }

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadBumpMap(CHR **FileData, MTRL **Material)
{
INT   RetCode;
CHR   Name[DEF_STR_LEN], Msg[80];

   FUNC("ReadBumpMap");
   Assert( FileData );
   Assert( Material );

   *FileData = GetNextWord( *FileData, Name );
   
   if (!*FileData)
   {
      SPDXLogError( LE_RANGE, "Unable to read material bump map" );
      return PARSE_STOP;
   }

   RetCode = SPDXSetMaterialBumpMap( *Material, Name );

   if (RetCode != LE_NONE)
   {
      sprintf( Msg, "Unable to assign bump map \"%s\"...continuing anyway", Name );
      SPDXLogError( RetCode, Msg );
   }

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadObjectTarget(CHR **FileData, OBJ **Object)
{
INT   RetCode;
P3D   Target;
VEC   Vector;
FLT   Bank;

   FUNC("ReadObjectTarget");
   Assert( FileData );
   Assert( Object );

   RetCode = GetP3D( FileData, &Target );

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read object target" );
      return RetCode;
   }

   RetCode = GetFloat( FileData, &Bank );

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read object target (bank)" );
      return RetCode;
   }

   Bank *= (FLT) (ROT_POINTS/360);

   Vector.dx = Target.x - (*Object)->LLoc.x;
   Vector.dy = Target.y - (*Object)->LLoc.y;
   Vector.dz = Target.z - (*Object)->LLoc.z;

   SPDXMakeMatrixFromVector( &Vector, Bank, &(*Object)->RotMat );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadObjectDirection(CHR **FileData, OBJ **Object)
{
INT   RetCode;
VEC   Direction;

   FUNC("ReadObjectDirection");
   Assert( FileData );
   Assert( Object );

   RetCode = GetVEC( FileData, &Direction );

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read object Direction" );
      return RetCode;
   }

   SPDXSetObjectDirection( *Object, &Direction );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadObjectLocation(CHR **FileData, OBJ **Object)
{
INT   RetCode;
P3D   Location;

   FUNC("ReadObjectLocation");
   Assert( FileData );
   Assert( Object );

   RetCode = GetP3D( FileData, &Location );

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read object location" );
      return RetCode;
   }

   TranslatePoint( &Location );

   SPDXSetObjectLocal( *Object, &Location );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadLightLocation(CHR **FileData, LGT **Light)
{
INT   RetCode;
P3D   Location;

   FUNC("ReadLightLocation");
   Assert( FileData );
   Assert( Light );

   RetCode = GetP3D( FileData, &Location );

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read light object location" );
      return RetCode;
   }

   TranslatePoint( &Location );

   SPDXSetLightLocal( *Light, &Location );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadLightIntensity(CHR **FileData, LGT **Light)
{
INT   RetCode;
RGB   Color;

   FUNC("ReadLightIntensity");
   Assert( FileData );
   Assert( Light );

   RetCode = GetRGB( FileData, &Color );

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read light intensity" );
      return RetCode;
   }

   SPDXSetLightIntensity( *Light, &Color );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadOrientation(CHR **FileData, OBJ **Object)
{
INT   RetCode;
ROT   Orientation;

   FUNC("ReadOrientation");
   Assert( FileData );
   Assert( Object );

   RetCode = GetROT( FileData, &Orientation );

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read object orientation" );
      return RetCode;
   }

   SPDXSetObjectSteadyOrientation( *Object, &Orientation );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadShowBacks(CHR **FileData, OBJ **Object)
{
INT   RetCode;
BYT   ShowBacks;

   FUNC("ReadShowBacks");
   Assert( FileData );
   Assert( Object );

   RetCode = GetByte( FileData, &ShowBacks );

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read showbacks attribute" );
      return RetCode;
   }

   SPDXSetObjectShowBacks( *Object, ShowBacks );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadShadeModel(CHR **FileData, OBJ **Object)
{
INT   RetCode;
BYT   ShadeModel;

   FUNC("ReadShadeModel");
   Assert( FileData );
   Assert( Object );

   RetCode = GetByte( FileData, &ShadeModel );

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read shademodel attribute" );
      return RetCode;
   }

   SPDXSetObjectShadeModel( *Object, ShadeModel );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadSurface(CHR **FileData, MTRL **Material)
{
INT   RetCode;
BYT   Type;

   FUNC("ReadSurface");
   Assert( FileData );
   Assert( Material );

   RetCode = GetByte( FileData, &Type );

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read surface attribute" );
      return RetCode;
   }

   SPDXSetMaterialSurface( *Material, Type );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadMaterialColor(CHR **FileData, MTRL **Material)
{
INT   RetCode;
RGB   Color;

   FUNC("ReadMaterialColor");
   Assert( FileData );
   Assert( Material );

   RetCode = GetRGB( FileData, &Color );

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read color RGB" );
      return RetCode;
   }

   SPDXSetMaterialColor( *Material, &Color, ColorIntensity, WorkingPalette );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadAmbient(CHR **FileData, MTRL **Material)
{
INT   RetCode;
RGB   Ambient;

   FUNC("ReadAmbient");
   Assert( FileData );
   Assert( Material );

   RetCode = GetRGB( FileData, &Ambient );

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read ambient light attribute" );
      return RetCode;
   }

   SPDXSetMaterialAmbientLight( *Material, &Ambient );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadTransparent(CHR **FileData, MTRL **Material)
{
INT   RetCode;
BYT   Transparent;

   FUNC("ReadTransparent");
   Assert( FileData );
   Assert( Material );

   RetCode = GetByte( FileData, &Transparent );

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read opacity flag" );
      return RetCode;
   }

   SPDXSetMaterialTransparency( *Material, Transparent );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadAmbientCo(CHR **FileData, MTRL **Material)
{
INT   RetCode;
FLT   Val;

   FUNC("ReadAmbientCo");
   Assert( FileData );
   Assert( Material );

   RetCode = GetFloat( FileData, &Val );

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read ambient-reflection coefficient" );
      return RetCode;
   }

   SPDXSetMaterialAmbientReflectCoefficient( *Material, Val );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadLense(CHR **FileData, OBJ **Object)
{
INT   RetCode;
FLT   Val;

   FUNC("ReadLense");
   Assert( FileData );
   Assert( Object );

   RetCode = GetFloat( FileData, &Val );

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read diffuse-reflection coefficient" );
      return RetCode;
   }

   SPDXSetObjectLense( *Object, Val );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadDiffuseCo(CHR **FileData, MTRL **Material)
{
INT   RetCode;
FLT   Val;

   FUNC("ReadDiffuseCo");
   Assert( FileData );
   Assert( Material );

   RetCode = GetFloat( FileData, &Val );

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read diffuse-reflection coefficient" );
      return RetCode;
   }

   SPDXSetMaterialDiffuseReflectCoefficient( *Material, Val );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadSpecularCo(CHR **FileData, MTRL **Material)
{
INT   RetCode;
FLT   Val;

   FUNC("ReadSpecularCo");
   Assert( FileData );
   Assert( Material );

   RetCode = GetFloat( FileData, &Val );

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read specular-reflection coefficient" );
      return RetCode;
   }

   SPDXSetMaterialSpecularReflectCoefficient( *Material, Val );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadShineValue(CHR **FileData, MTRL **Material)
{
INT   RetCode;
FLT   Val;

   FUNC("ReadShineValue");
   Assert( FileData );
   Assert( Material );

   RetCode = GetFloat( FileData, &Val );

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read shine coefficient" );
      return RetCode;
   }

   SPDXSetMaterialShineCoefficient( *Material, Val );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadDevUse(CHR **FileData, OBJ **Object)
{
INT   i, RetCode, Val;

   FUNC("ReadDevUse");
   Assert( FileData );
   Assert( Object );

   for( i = 0; i < DEV_USE_COUNT; i++)
   {
      RetCode = GetInteger( FileData, &Val );

      if (RetCode != PARSE_CONTINUE)
      {
         SPDXLogError( LE_RANGE, "Unable to read devuse attributes" );
         return RetCode;
      }

      SPDXSetObjectDevUse( *Object, i, Val );
   }

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadDrawFlags(CHR **FileData, OBJ **Object)
{
INT   RetCode;
BYT   DrawFlags;

   FUNC("ReadDrawFlags");
   Assert( FileData );
   Assert( Object );

   RetCode = GetByte( FileData, &DrawFlags );

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read drawflags attribute" );
      return RetCode;
   }

   SPDXSetObjectDrawFlags( *Object, DrawFlags );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadType(CHR **FileData, OBJ **Object)
{
INT   RetCode;
INT   Type;

   FUNC("ReadType");
   Assert( FileData );
   Assert( Object );

   RetCode = GetInteger( FileData, &Type );

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read type attribute" );
      return RetCode;
   }

   SPDXSetObjectType( *Object, Type );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadVertex(CHR **FileData, OBJ **Object)
{
INT   RetCode, Index;
VER   Vertex;

   FUNC("ReadVertex");
   Assert( FileData );
   Assert( Object );

   Index = SPDXAddObjectVertex( *Object );

   if (Index < 0)
   {
      SPDXLogError( LE_RANGE, "Unable to add object vertex" );
      return PARSE_STOP;
   }

   RetCode = GetVER( FileData, &Vertex );

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read VER" );
      return RetCode;
   }

   SPDXSetVertex( *Object, Index, &Vertex );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadTriangle(CHR **FileData, OBJ **Object)
{
INT   RetCode;
INT   Index;
INT   V1, V2, V3;
CHR   MatName[20], Msg[80];
P2D   Map1, Map2, Map3;

   FUNC("ReadTriangle");
   Assert( FileData );
   Assert( Object );

   Index = SPDXAddObjectTri( *Object );

   if (Index < 0)
   {
      SPDXLogError( LE_RANGE, "Unable to add TRI to OBJ" );
      return PARSE_STOP;
   }

   RetCode = GetInteger( FileData, &V1);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read INT" );
      return RetCode;
   }

   RetCode = GetInteger( FileData, &V2);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read INT" );
      return RetCode;
   }

   RetCode = GetInteger( FileData, &V3);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read INT" );
      return RetCode;
   }

   SPDXSetTriVertices( *Object, Index,
                       &(*Object)->VertexList.Vers[V1],
                       &(*Object)->VertexList.Vers[V2],
                       &(*Object)->VertexList.Vers[V3]);

   SPDXSetTriNormal( *Object, Index );

   *FileData = GetNextWord( *FileData, MatName );

   if (MatName[0] == '*')
      return PARSE_CONTINUE;

   RetCode = GetP2D( FileData, &Map1);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read P2D" );
      return RetCode;
   }

   RetCode = GetP2D( FileData, &Map2);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read P2D" );
      return RetCode;
   }

   RetCode = GetP2D( FileData, &Map3);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read P2D" );
      return RetCode;
   }

   RetCode = SPDXAssignTriMaterial( MatName, *Object, Index, &Map1, &Map2, &Map3 );

   if (RetCode != LE_NONE)
   {
      sprintf( Msg, "Unable to assign material \"%s\" to polygon (tri)", MatName );
      SPDXFatalError( RetCode, Msg );
      return PARSE_STOP;
   }

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadQuad(CHR **FileData, OBJ **Object)
{
INT   RetCode;
INT   Index1, Index2;
INT   V1, V2, V3, V4;
CHR   MatName[20], Msg[80];
P2D   Map1, Map2, Map3, Map4;

   FUNC("ReadQuad");
   Assert( FileData );
   Assert( Object );

   Index1 = SPDXAddObjectTri( *Object );

   if (Index1 < 0)
   {
      SPDXLogError( LE_RANGE, "Unable to add QUAD to OBJ" );
      return PARSE_STOP;
   }

   Index2 = SPDXAddObjectTri( *Object );

   if (Index2 < 0)
   {
      SPDXLogError( LE_RANGE, "Unable to add QUAD to OBJ" );
      return PARSE_STOP;
   }

   RetCode = GetInteger( FileData, &V1);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read INT" );
      return RetCode;
   }

   RetCode = GetInteger( FileData, &V2);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read INT" );
      return RetCode;
   }

   RetCode = GetInteger( FileData, &V3);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read INT" );
      return RetCode;
   }

   RetCode = GetInteger( FileData, &V4);

   if (RetCode != PARSE_CONTINUE)
      return RetCode;

   SPDXSetTriVertices( *Object, Index1,
                       &(*Object)->VertexList.Vers[V1],
                       &(*Object)->VertexList.Vers[V2],
                       &(*Object)->VertexList.Vers[V3]);

   SPDXSetTriVertices( *Object, Index2,
                       &(*Object)->VertexList.Vers[V2],
                       &(*Object)->VertexList.Vers[V3],
                       &(*Object)->VertexList.Vers[V4] );

   *FileData = GetNextWord( *FileData, MatName );

   RetCode = GetP2D( FileData, &Map1);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read P2D" );
      return RetCode;
   }

   RetCode = GetP2D( FileData, &Map2);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read P2D" );
      return RetCode;
   }

   RetCode = GetP2D( FileData, &Map3);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read P2D" );
      return RetCode;
   }

   RetCode = GetP2D( FileData, &Map4);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read P2D" );
      return RetCode;
   }

   RetCode = SPDXAssignTriMaterial( MatName, *Object, Index1, &Map1, &Map2, &Map3 );

   if (RetCode != LE_NONE)
   {
      sprintf( Msg, "Unable to assign material \"%s\" to polygon (quad)", MatName );
      SPDXFatalError( RetCode, Msg );
      return PARSE_STOP;
   }

   RetCode = SPDXAssignTriMaterial( MatName, *Object, Index2, &Map2, &Map3, &Map4 );

   if (RetCode != LE_NONE)
   {
      sprintf( Msg, "Unable to assign material \"%s\" to polygon (quad)", MatName );
      SPDXFatalError( RetCode, Msg );
      return PARSE_STOP;
   }

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadPoint(CHR **FileData, OBJ **Object)
{
INT   RetCode;
INT   Index;
INT   V;

   FUNC("ReadPoint");
   Assert( FileData );
   Assert( Object );

   Index = SPDXAddObjectPoint( *Object );

   if (Index < 0)
   {
      SPDXLogError( LE_RANGE, "Unable to add POI to OBJ" );
      return PARSE_STOP;
   }

   RetCode = GetInteger( FileData, &V);

   if (RetCode != PARSE_CONTINUE)
   {
      SPDXLogError( LE_RANGE, "Unable to read INT" );
      return RetCode;
   }

   SPDXSetPointVertex( *Object, Index, &(*Object)->VertexList.Vers[V] );

   return PARSE_CONTINUE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [READUSD.C   ] - End Of File                                           ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
