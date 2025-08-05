// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [LOADIMG.C   ] - Misc. image format loading routines                   ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <string.h>
#include <dos.h>
#include <graph.h>

#include "spandex.h"

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static   INT   GifHeight;
static   CHR   Palette[256][3];

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static   INT   ReadIPI( CHR *FileName, IMAGE **Image );
static   INT   ReadIMG( CHR *FileName, IMAGE **Image );
static   INT   ReadTGA( CHR *FileName, IMAGE **Image );
static   INT   ReadBMP( CHR *FileName, IMAGE **Image );
static   INT   ReadGIF( CHR *FileName, IMAGE **Image, INT G24Flag );
static   INT   ReadPCX( CHR *FileName, IMAGE **Image );
static   INT   StoreLineGif( CHR *Pixels, INT LineLength, CHR *Buffer, INT ImgWidth, INT Y );
static   INT   StoreLineG24( CHR *Pixels, INT LineLength, CHR *Buffer, INT ImgWidth, INT Y );

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXReadImage( CHR *FileName, IMAGE **Image )
{
INT   RetCode = LE_UNKNOWN;

   FUNC("SPDXReadImage");
   Assert( FileName );
   Assert( Image );

   switch( SPDXGetFileType( FileName ) )
   {
      case IMAGETYPE_IPI:
         RetCode = ReadIPI( FileName, Image );
         break;

      case IMAGETYPE_IMG:
         RetCode = ReadIMG( FileName, Image );
         break;

      case IMAGETYPE_GIF:
         RetCode = ReadGIF( FileName, Image, 0 );
         break;

      case IMAGETYPE_G24:
         RetCode = ReadGIF( FileName, Image, 1 );
         break;

      case IMAGETYPE_TGA:
         RetCode = ReadTGA( FileName, Image );
         break;

      case IMAGETYPE_BMP:
         RetCode = ReadBMP( FileName, Image );
         break;

      case IMAGETYPE_PCX:
         RetCode = ReadPCX( FileName, Image );
         break;

      default:
         RetCode = LE_INVALIDFILETYPE;
   }

   return RetCode;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadIPI( CHR *FileName, IMAGE **Image )
{
INT   Handle;
IMAGE *TImage;

   FUNC("ReadIPI");
   Assert( FileName );
   Assert( Image );

   TImage = (IMAGE *) SPDXMalloc( sizeof(IMAGE) );

   if (!TImage)
      return LE_NOMEMIMG;

   (TImage)->Saved = 0;

   Handle = SPDXOpen( FileName, O_RDONLY|O_BINARY, 0 );

   if (Handle <= 0)
   {
      SPDXFree( TImage );
      return LE_NOOPEN;
   }

   TImage->ResX  = SPDXGetc( Handle ) * 256;
   TImage->ResX += SPDXGetc( Handle );
   TImage->ResY  = SPDXGetc( Handle ) * 256;
   TImage->ResY += SPDXGetc( Handle );

   TImage->Buffer = (CHR *) SPDXMalloc( TImage->ResX * TImage->ResY * 3 );

   if (!TImage->Buffer)
   {
      SPDXClose( Handle );
      SPDXFree( TImage );
      return LE_NOMEMIMG;
   }

   if (SPDXRead(Handle, TImage->Buffer, TImage->ResX * TImage->ResY * 3) != TImage->ResX * TImage->ResY * 3)
   {
      SPDXClose( Handle );
      SPDXFree(TImage->Buffer);
      SPDXFree(TImage);
      return LE_NOREAD;
   }

   SPDXClose( Handle );

   SPDXFree( (*Image)->Buffer );
   (*Image)->Buffer = TImage->Buffer;
   (*Image)->ResX = TImage->ResX;
   (*Image)->ResY = TImage->ResY;
   SPDXFree( TImage );

   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadGIF( CHR *FileName, IMAGE **Image, INT G24Flag )
{
INT   i, RetCode, ColorCount, Left, Top, Width, Height;
CHR   Signature[7];                             /* GIF signature              */
INT   Handle;
IMAGE *TImage;

   FUNC("ReadGIF");
   Assert( FileName );
   Assert( Image );

   TImage = (IMAGE *) SPDXMalloc( sizeof(IMAGE) );

   if (!TImage)
      return LE_NOMEMIMG;

   (TImage)->Saved = 0;

   Handle = SPDXOpen(FileName, O_RDONLY|O_BINARY, 0);

   if(Handle <= 0)
   {
      SPDXFree( TImage );
      return LE_NOOPEN;
   }

/* CHECK GIF SIGNATURE */

   RetCode = SPDXRead(Handle, Signature, 6);
   Signature[6] = 0;

   if(RetCode!=6 || strncmp(Signature, "GIF", 3))
   {
      SPDXClose( Handle );
      SPDXFree(TImage);
      return LE_INVALIDGIFFILE;
   }

/* GET SCREEN DESCRIPTOR INFO */

   TImage->ResX = SPDXGetc(Handle);
   TImage->ResX += 256 * SPDXGetc(Handle);
   TImage->ResY = SPDXGetc(Handle);
   TImage->ResY += 256 * SPDXGetc(Handle);

   if (G24Flag)
      TImage->ResY /= 3;

/* CHECK COLOR MAP INFO */

   RetCode = SPDXGetc(Handle);

   if ((RetCode & LOCAL_MAP) == 0)
   {
      SPDXClose( Handle );
      SPDXFree(TImage);
      return LE_INVALIDGIFFILE;
   }

   ColorCount = 1 << ( (RetCode & 0x07) + 1 );

   SPDXGetc(Handle);
   SPDXGetc(Handle);

/* GET GLOBAL COLOR MAP */

   for(i = 0; i < ColorCount; i++ )
   {
      Palette[i][0] = (CHR) SPDXGetc(Handle);
      Palette[i][1] = (CHR) SPDXGetc(Handle);
      Palette[i][2] = (CHR) SPDXGetc(Handle);
   }

/* GET IMAGE SEPARATOR */

   RetCode = SPDXGetc(Handle);

   if (RetCode != ',')
   {
      close( Handle );
      SPDXFree(TImage);
      return LE_INVALIDGIFFILE;
   }

/* GET LEFT TOP WIDTH AND HEIGHT */

   Left    =       SPDXGetc(Handle);
   Left   += 256 * SPDXGetc(Handle);
   Top     =       SPDXGetc(Handle);
   Top    += 256 * SPDXGetc(Handle);
   Width   =       SPDXGetc(Handle);
   Width  += 256 * SPDXGetc(Handle);
   Height  =       SPDXGetc(Handle);
   Height += 256 * SPDXGetc(Handle);

   if ( Width != (*Image)->ResX || Height != (*Image)->ResY )
      TImage->ResX = Width, TImage->ResY = Height;

   if (G24Flag)
      TImage->ResY /= 3;

/* ALLOCATE BUFFER */

   TImage->Buffer = (CHR *) SPDXMalloc( TImage->ResX * TImage->ResY * 3 );

   if (!TImage->Buffer)
   {
      close( Handle );
      SPDXFree( TImage );
      return LE_NOMEMIMG;
   }

/* CHECK FOR INTERLACE OR LOCAL COLOR MAP */

   RetCode = SPDXGetc(Handle);

   if(RetCode & LOCAL_MAP)
   {
      ColorCount = 1 << ((RetCode & 0x7)+1);

      for( i = 0; i < ColorCount; i++ )
      {
         Palette[i][0] = (CHR) SPDXGetc(Handle);
         Palette[i][1] = (CHR) SPDXGetc(Handle);
         Palette[i][2] = (CHR) SPDXGetc(Handle);
      }
   }

   if(RetCode & INTERLACE)
   {
      close( Handle );
      SPDXFree( TImage );
      SPDXFree( TImage->Buffer );
      return LE_NOGIFINTERLACE;
   }

/* DECODE THIS IMAGE */

   GifHeight = TImage->ResY;

   if (G24Flag)
      RetCode = SPDXDecompress(Handle, TImage->ResX, TImage->Buffer, Width, StoreLineG24 );
   else
      RetCode = SPDXDecompress(Handle, TImage->ResX, TImage->Buffer, Width, StoreLineGif );

   close(Handle);

   if (RetCode != LE_NONE)
   {
      SPDXFree( TImage );
      SPDXFree( TImage->Buffer );
      return LE_NOGIFINTERLACE;
   }

   SPDXFree( (*Image)->Buffer );
   (*Image)->Buffer = TImage->Buffer;
   (*Image)->ResX = TImage->ResX;
   (*Image)->ResY = TImage->ResY;
   SPDXFree( TImage );

   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   StoreLineGif( CHR *Pixels, INT LineLength, CHR *Buffer, INT ImgWidth, INT Y )
{
INT   i;
CHR   *cTemp;

   FUNC("StoreLineGif");
   Assert( Pixels );
   Assert( Buffer );

   Y *= ImgWidth * 3;

   for (i = 0; i < ImgWidth*3; i+=3 )
   {
      cTemp = Palette[*(Pixels++)];
      Buffer[Y+i+0] = *(cTemp)++;
      Buffer[Y+i+1] = *(cTemp)++;
      Buffer[Y+i+2] = *cTemp;
   }

   return LineLength * 3;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   StoreLineG24( CHR *Pixels, INT LineLength, CHR *Buffer, INT ImgWidth, INT Y )
{
INT   i;

   FUNC("StoreLineG24");
   Assert( Pixels );
   Assert( Buffer );

   if (Y >= GifHeight * 2)
      for (i = 0; i < ImgWidth; i++ )
      {
         Buffer[(Y-GifHeight*2) * ImgWidth *3+ i*3+2] = Pixels[i];
      }

   else if (Y >= GifHeight)
      for (i = 0; i < ImgWidth; i++ )
      {
         Buffer[(Y-GifHeight) * ImgWidth *3+ i*3+1] = Pixels[i];
      }

   else
      for (i = 0; i < ImgWidth; i++ )
      {
         Buffer[Y * ImgWidth *3+ i*3+0] = Pixels[i];
      }

   return LineLength * 3;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadIMG( CHR *FileName, IMAGE **Image )
{
INT   FirstScanLine, LastScanLine, x, y, i, Handle, YIndex;
INT   Count, Red, Blue, Green;
IMAGE *TImage;

   FUNC("ReadIMG");
   Assert( FileName );
   Assert( Image );

   TImage = (IMAGE *) SPDXMalloc( sizeof(IMAGE) );

   if (!TImage)
      return LE_NOMEMIMG;

   (TImage)->Saved = 0;

   Handle = SPDXOpen(FileName, O_RDONLY|O_BINARY, 0);

   if(Handle <= 0)
   {
      SPDXFree( TImage );
      return LE_NOOPEN;
   }

   TImage->ResX = 256 * SPDXGetc(Handle);
   TImage->ResX += SPDXGetc(Handle);
   TImage->ResY = 256 * SPDXGetc(Handle);
   TImage->ResY += SPDXGetc(Handle);
   FirstScanLine = 256 * SPDXGetc(Handle);
   FirstScanLine += SPDXGetc(Handle);
   LastScanLine = 256 * SPDXGetc(Handle);
   LastScanLine += SPDXGetc(Handle);
   SPDXGetc(Handle);
   SPDXGetc(Handle);

/* ALLOCATE BUFFER */

   TImage->Buffer = (CHR *) SPDXMalloc( TImage->ResX * TImage->ResY * 3 );

   if (!TImage->Buffer)
   {
      close( Handle );
      SPDXFree( TImage );
      return LE_NOMEMIMG;
   }

/* DECODE THIS IMAGE */

   for( y = FirstScanLine; y < LastScanLine; y++ )
   {
      YIndex = y * TImage->ResX * 3;

      for( x = 0; x < TImage->ResX; )
      {
         Count = SPDXGetc(Handle);
         Blue  = SPDXGetc(Handle);
         Green = SPDXGetc(Handle);
         Red   = SPDXGetc(Handle);

         for (i = 0; i < Count; i++)
         {
            TImage->Buffer[YIndex + (x+i) * 3 + 0] = (CHR) Red;
            TImage->Buffer[YIndex + (x+i) * 3 + 1] = (CHR) Green;
            TImage->Buffer[YIndex + (x+i) * 3 + 2] = (CHR) Blue;
         }

         x += Count;
      }
   }

   close(Handle);

   SPDXFree( (*Image)->Buffer );
   (*Image)->Buffer = TImage->Buffer;
   (*Image)->ResX = TImage->ResX;
   (*Image)->ResY = TImage->ResY;
   SPDXFree( TImage );

   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadTGA( CHR *FileName, IMAGE **Image )
{
INT   i, j, Type, YOrigin, Handle;
IMAGE *TImage;

   FUNC("ReadTGA");
   Assert( FileName );
   Assert( Image );

   TImage = (IMAGE *) SPDXMalloc( sizeof(IMAGE) );

   if (!TImage)
      return LE_NOMEMIMG;

   (TImage)->Saved = 0;

   Handle = SPDXOpen(FileName, O_RDONLY|O_BINARY, 0);

   if(Handle <= 0)
   {
      SPDXFree( TImage );
      return LE_NOOPEN;
   }

   SPDXGetc(Handle);
   SPDXGetc(Handle);

   Type = SPDXGetc(Handle);

   if(Type != 2)
   {
      close( Handle );
      SPDXFree( TImage );
      return LE_BADTGATYPE;
   }

   for(i = 3; i < 12; i++)
      SPDXGetc(Handle);

   TImage->ResX = SPDXGetc(Handle);
   TImage->ResX += 256 * SPDXGetc(Handle);
   TImage->ResY = SPDXGetc(Handle);
   TImage->ResY += 256 * SPDXGetc(Handle);

   SPDXGetc(Handle);

   YOrigin = SPDXGetc(Handle) & 32;

/* ALLOCATE BUFFER */

   TImage->Buffer = (CHR *) SPDXMalloc( TImage->ResX * TImage->ResY * 3 );

   if (!TImage->Buffer)
   {
      close( Handle );
      SPDXFree( TImage );
      return LE_NOMEMIMG;
   }

/* DECODE THIS IMAGE */

   if (YOrigin)
   {
      for( j = 0; j < TImage->ResY * TImage->ResX * 3; j += TImage->ResX * 3 )
         for( i = 0; i < TImage->ResX * 3; i+=3 )
         {
            TImage->Buffer[j+i + 2] = (CHR) SPDXGetc(Handle);
            TImage->Buffer[j+i + 1] = (CHR) SPDXGetc(Handle);
            TImage->Buffer[j+i + 0] = (CHR) SPDXGetc(Handle);
         }
   }
   else
   {
      for( j = (TImage->ResY-1) * TImage->ResX * 3; j >= 0; j -= TImage->ResX * 3 )
         for( i = 0; i < TImage->ResX * 3; i+=3 )
         {
            TImage->Buffer[j+i + 2] = (CHR) SPDXGetc(Handle);
            TImage->Buffer[j+i + 1] = (CHR) SPDXGetc(Handle);
            TImage->Buffer[j+i + 0] = (CHR) SPDXGetc(Handle);
         }
   }

   close(Handle);

   SPDXFree( (*Image)->Buffer );
   (*Image)->Buffer = TImage->Buffer;
   (*Image)->ResX = TImage->ResX;
   (*Image)->ResY = TImage->ResY;
   SPDXFree( TImage );

   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadBMP( CHR *FileName, IMAGE **Image )
{
INT   i, Size, Bits, ColorsUsed = 0, Byte, x, y, os2 = 0, Handle;
CHR   *Palette;
IMAGE *TImage;

   FUNC("ReadBMP");
   Assert( FileName );
   Assert( Image );

   TImage = (IMAGE *) SPDXMalloc( sizeof(IMAGE) );

   if (!TImage)
      return LE_NOMEMIMG;

   (TImage)->Saved = 0;

   Handle = SPDXOpen(FileName, O_RDONLY|O_BINARY, 0);

   if(Handle <= 0)
   {
      SPDXFree( TImage );
      return LE_NOOPEN;
   }

   if (SPDXGetc(Handle) != 'B')
   {
      SPDXFree( TImage );
      close( Handle );
      return LE_NOTBMP;
   }

   if (SPDXGetc(Handle) != 'M')
   {
      SPDXFree( TImage );
      close( Handle );
      return LE_NOTBMP;
   }

   SPDXGetc(Handle);  SPDXGetc(Handle);  SPDXGetc(Handle);
   SPDXGetc(Handle);  SPDXGetc(Handle);  SPDXGetc(Handle);
   SPDXGetc(Handle);  SPDXGetc(Handle);  SPDXGetc(Handle);
   SPDXGetc(Handle);  SPDXGetc(Handle);  SPDXGetc(Handle);

   Size = SPDXGetc(Handle);
   Size += SPDXGetc(Handle)<<8;
   Size += SPDXGetc(Handle)<<16;
   Size += SPDXGetc(Handle)<<24;

   if(Size == 12)
   {
      TImage->ResX = SPDXGetc(Handle);
      TImage->ResX += SPDXGetc(Handle)<<8;
      TImage->ResY = SPDXGetc(Handle);
      TImage->ResY += SPDXGetc(Handle)<<8;

      SPDXGetc(Handle);
      SPDXGetc(Handle);

      Bits = SPDXGetc(Handle);
      Bits += SPDXGetc(Handle) << 8;

      ColorsUsed = 1 << Bits;

      os2 = 1;
   }
   else
   {
      TImage->ResX = SPDXGetc(Handle);
      TImage->ResX += SPDXGetc(Handle)<<8;
      TImage->ResX += SPDXGetc(Handle)<<16;
      TImage->ResX += SPDXGetc(Handle)<<24;
      TImage->ResY = SPDXGetc(Handle);
      TImage->ResY += SPDXGetc(Handle)<<8;
      TImage->ResY += SPDXGetc(Handle)<<16;
      TImage->ResY += SPDXGetc(Handle)<<24;

      SPDXGetc(Handle);  SPDXGetc(Handle);

      Bits = SPDXGetc(Handle);
      Bits += SPDXGetc(Handle)<<8;

      if (Size >= 20)
      {
         if (SPDXGetc(Handle) ||
            SPDXGetc(Handle) ||
            SPDXGetc(Handle) ||
            SPDXGetc(Handle) )
         {
            SPDXFree( TImage );
            close( Handle );
            return LE_NOCOMPBMP;
         }
      }

      if (Size >= 24)
      {
         SPDXGetc(Handle);
         SPDXGetc(Handle);
         SPDXGetc(Handle);
         SPDXGetc(Handle);
      }

      if (Size >= 28)
      {
         SPDXGetc(Handle);
         SPDXGetc(Handle);
         SPDXGetc(Handle);
         SPDXGetc(Handle);
      }

      if (Size >= 32)
      {
         SPDXGetc(Handle);
         SPDXGetc(Handle);
         SPDXGetc(Handle);
         SPDXGetc(Handle);
      }

      if (Size >= 36)
      {
         ColorsUsed = SPDXGetc(Handle);
         ColorsUsed += SPDXGetc(Handle)<<8;
         ColorsUsed += SPDXGetc(Handle)<<16;
         ColorsUsed += SPDXGetc(Handle)<<24;
      }

      if (Size >= 40)
      {
         SPDXGetc(Handle);
         SPDXGetc(Handle);
         SPDXGetc(Handle);
         SPDXGetc(Handle);
      }
   }

/* ALLOCATE BUFFER */

   TImage->Buffer = (CHR *) SPDXMalloc(TImage->ResX * TImage->ResY * 3 );

   if (!TImage->Buffer)
   {
      close( Handle );
      SPDXFree( TImage );
      return LE_NOMEMIMG;
   }

/* DECODE THIS IMAGE */

   if (ColorsUsed)
   {
      Palette = (CHR *) SPDXMalloc( ColorsUsed * 3 );
      if (!Palette)
      {
         close( Handle );
         SPDXFree( TImage );
         SPDXFree( TImage->Buffer );
         return LE_NOMEMPAL;
      }

      for( i = 0; i < ColorsUsed; i++ )
      {
         Palette[i*3+2] = (CHR) SPDXGetc( Handle );
         Palette[i*3+1] = (CHR) SPDXGetc( Handle );
         Palette[i*3+0] = (CHR) SPDXGetc( Handle );
         if (!os2)
            SPDXGetc( Handle );
      }

      for( y = TImage->ResY-1; y >= 0; y-- )
      {
         for( x = 0; x < TImage->ResX; x++ )
         {
            Byte = SPDXGetc(Handle);

            TImage->Buffer[y*TImage->ResX*3+x*3+0] = Palette[Byte*3+0];
            TImage->Buffer[y*TImage->ResX*3+x*3+1] = Palette[Byte*3+1];
            TImage->Buffer[y*TImage->ResX*3+x*3+2] = Palette[Byte*3+2];
         }
      }

      SPDXFree( Palette );
   }
   else if (Bits == 1)
   {
      Palette = (CHR *) SPDXMalloc( 2 * 3 );

      if (!Palette)
      {
         close( Handle );
         SPDXFree( TImage );
         SPDXFree( TImage->Buffer );
         return LE_NOMEMPAL;
      }

      for( i = 0; i < 2; i++ )
      {
         Palette[i*3+2] = (CHR) SPDXGetc( Handle );
         Palette[i*3+1] = (CHR) SPDXGetc( Handle );
         Palette[i*3+0] = (CHR) SPDXGetc( Handle );
         if (!os2)
            SPDXGetc( Handle );
      }

      for( y = TImage->ResY-1; y >= 0; y-- )
      {
         for( x = 0; x < TImage->ResX; x+=8 )
         {
            Byte = SPDXGetc( Handle );

            TImage->Buffer[y*TImage->ResX*3+x*3+0] = Palette[((Byte & 128) == 1)*3+0];
            TImage->Buffer[y*TImage->ResX*3+x*3+1] = Palette[((Byte & 128) == 1)*3+1];
            TImage->Buffer[y*TImage->ResX*3+x*3+2] = Palette[((Byte & 128) == 1)*3+2];

            TImage->Buffer[y*TImage->ResX*3+x*3+3] = Palette[((Byte & 64) == 1)*3+0];
            TImage->Buffer[y*TImage->ResX*3+x*3+4] = Palette[((Byte & 64) == 1)*3+1];
            TImage->Buffer[y*TImage->ResX*3+x*3+5] = Palette[((Byte & 64) == 1)*3+2];

            TImage->Buffer[y*TImage->ResX*3+x*3+6] = Palette[((Byte & 32) == 1)*3+0];
            TImage->Buffer[y*TImage->ResX*3+x*3+7] = Palette[((Byte & 32) == 1)*3+1];
            TImage->Buffer[y*TImage->ResX*3+x*3+8] = Palette[((Byte & 32) == 1)*3+2];

            TImage->Buffer[y*TImage->ResX*3+x*3+ 9] = Palette[((Byte & 16) == 1)*3+0];
            TImage->Buffer[y*TImage->ResX*3+x*3+10] = Palette[((Byte & 16) == 1)*3+1];
            TImage->Buffer[y*TImage->ResX*3+x*3+11] = Palette[((Byte & 16) == 1)*3+2];

            TImage->Buffer[y*TImage->ResX*3+x*3+12] = Palette[((Byte & 8) == 1)*3+0];
            TImage->Buffer[y*TImage->ResX*3+x*3+13] = Palette[((Byte & 8) == 1)*3+1];
            TImage->Buffer[y*TImage->ResX*3+x*3+14] = Palette[((Byte & 8) == 1)*3+2];

            TImage->Buffer[y*TImage->ResX*3+x*3+15] = Palette[((Byte & 4) == 1)*3+0];
            TImage->Buffer[y*TImage->ResX*3+x*3+16] = Palette[((Byte & 4) == 1)*3+1];
            TImage->Buffer[y*TImage->ResX*3+x*3+17] = Palette[((Byte & 4) == 1)*3+2];

            TImage->Buffer[y*TImage->ResX*3+x*3+18] = Palette[((Byte & 2) == 1)*3+0];
            TImage->Buffer[y*TImage->ResX*3+x*3+19] = Palette[((Byte & 2) == 1)*3+1];
            TImage->Buffer[y*TImage->ResX*3+x*3+20] = Palette[((Byte & 2) == 1)*3+2];

            TImage->Buffer[y*TImage->ResX*3+x*3+21] = Palette[((Byte & 1) == 1)*3+0];
            TImage->Buffer[y*TImage->ResX*3+x*3+22] = Palette[((Byte & 1) == 1)*3+1];
            TImage->Buffer[y*TImage->ResX*3+x*3+23] = Palette[((Byte & 1) == 1)*3+2];
         }

         if (TImage->ResX & 1)
            Byte = SPDXGetc(Handle);

      }
      SPDXFree( Palette );
   }
   else if (Bits == 4)
   {
      Palette = (CHR *) SPDXMalloc( 16 * 3 );

      if (!Palette)
      {
         close( Handle );
         SPDXFree( TImage );
         SPDXFree( TImage->Buffer );
         return LE_NOMEMPAL;
      }

      for( i = 0; i < 16; i++ )
      {
         Palette[i*3+2] = (CHR) SPDXGetc( Handle );
         Palette[i*3+1] = (CHR) SPDXGetc( Handle );
         Palette[i*3+0] = (CHR) SPDXGetc( Handle );
         if (!os2)
            SPDXGetc( Handle );
      }

      for( y = TImage->ResY-1; y >= 0; y-- )
      {
         for( x = 0; x < TImage->ResX; x+=2 )
         {
            Byte = SPDXGetc( Handle );

            TImage->Buffer[y*TImage->ResX*3+x*3+0] = Palette[(Byte>>4)*3+0];
            TImage->Buffer[y*TImage->ResX*3+x*3+1] = Palette[(Byte>>4)*3+1];
            TImage->Buffer[y*TImage->ResX*3+x*3+2] = Palette[(Byte>>4)*3+2];

            TImage->Buffer[y*TImage->ResX*3+x*3+3] = Palette[(Byte&0xf)*3+0];
            TImage->Buffer[y*TImage->ResX*3+x*3+4] = Palette[(Byte&0xf)*3+1];
            TImage->Buffer[y*TImage->ResX*3+x*3+5] = Palette[(Byte&0xf)*3+2];
         }

         if (TImage->ResX & 1)
            Byte = SPDXGetc(Handle);
      }
      SPDXFree( Palette );
   }
   else if (Bits == 8)
   {
      Palette = (CHR *) SPDXMalloc( 256 * 3 );

      if (!Palette)
      {
         close( Handle );
         SPDXFree( TImage );
         SPDXFree( TImage->Buffer );
         return LE_NOMEMPAL;
      }

      for( i = 0; i < 256; i++ )
      {
         Palette[i*3+2] = (CHR) SPDXGetc( Handle );
         Palette[i*3+1] = (CHR) SPDXGetc( Handle );
         Palette[i*3+0] = (CHR) SPDXGetc( Handle );
         if (!os2)
            SPDXGetc( Handle );
      }

      for( y = TImage->ResY-1; y >= 0; y-- )
      {
         for( x = 0; x < TImage->ResX; x++ )
         {
            Byte = SPDXGetc(Handle);

            TImage->Buffer[y*TImage->ResX*3+x*3+0] = Palette[Byte*3+0];
            TImage->Buffer[y*TImage->ResX*3+x*3+1] = Palette[Byte*3+1];
            TImage->Buffer[y*TImage->ResX*3+x*3+2] = Palette[Byte*3+2];
         }

         if (TImage->ResX & 1)
            Byte = SPDXGetc(Handle);
      }

      SPDXFree( Palette );
   }
   else
   {
      for( y = TImage->ResY-1; y >= 0; y-- )
      {
         for( x = 0; x < TImage->ResX; x++ )
         {
            TImage->Buffer[y*TImage->ResX*3+x*3+2] = (CHR) SPDXGetc(Handle);
            TImage->Buffer[y*TImage->ResX*3+x*3+1] = (CHR) SPDXGetc(Handle);
            TImage->Buffer[y*TImage->ResX*3+x*3+0] = (CHR) SPDXGetc(Handle);
         }
      }
   }

   close(Handle);

   SPDXFree( (*Image)->Buffer );
   (*Image)->Buffer = TImage->Buffer;
   (*Image)->ResX = TImage->ResX;
   (*Image)->ResY = TImage->ResY;
   SPDXFree( TImage );

   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ReadPCX( CHR *FileName, IMAGE **Image )
{
INT   x, y, i, Colors;
CHR   *tempbuf, tbyte, count, ExtendedPalette[256*3+1];
INT   Handle;
IMAGE *TImage;
PCXFL PCXFile;

   FUNC("ReadPCX");
   Assert( FileName );
   Assert( Image );

   TImage = (IMAGE *) SPDXMalloc( sizeof(IMAGE) );

   if (!TImage)
      return LE_NOMEMIMG;

   (TImage)->Saved = 0;

   Handle = SPDXOpen(FileName, O_RDONLY|O_BINARY, 0);

   if(Handle <= 0)
   {
      SPDXFree( TImage );
      return LE_NOOPEN;
   }

   if (!SPDXRead(Handle, (void *) &PCXFile, sizeof(PCXFL) ))
   {
      SPDXFree( TImage );
      SPDXClose( Handle );
      return LE_NOTPCX;
   }

   if (PCXFile.Header.Identifier != 10)
   {
      SPDXFree( TImage );
      SPDXClose( Handle );
      return LE_NOTPCX;
   }

   if (PCXFile.Header.Version == 4 || PCXFile.Header.Version > 5)
   {
      SPDXFree( TImage );
      SPDXClose( Handle );
      return LE_NOTPCX;
   }

   if (!PCXFile.Header.Encode)
   {
      SPDXFree( TImage );
      SPDXClose( Handle );
      return LE_NOTPCXENCODE;
   }

   TImage->ResX = PCXFile.Header.Right - PCXFile.Header.Left + 1;
   TImage->ResY = PCXFile.Header.Bottom - PCXFile.Header.Top + 1;

   Colors = ((1<<PCXFile.Header.Bits)*PCXFile.Info.Planes);

   if (Colors != 256 && (PCXFile.Header.Bits != 8 && PCXFile.Info.Planes != 3))
   {
      SPDXFree( TImage );
      SPDXClose( Handle );
      return LE_NOTVALIDPCX;
   }

/* ALLOCATE BUFFERS */

   TImage->Buffer = (CHR *) SPDXMalloc(TImage->ResX * TImage->ResY * 3 );

   if (!TImage->Buffer)
   {
      SPDXClose( Handle );
      SPDXFree( TImage );
      return LE_NOMEMIMG;
   }

   SPDXMemSetBYTE( TImage->Buffer, 0, TImage->ResX * TImage->ResY * 3 );

/* DECODE THE IMAGE */

   if (Colors == 256)
   {
      tempbuf = (CHR *) SPDXMalloc(TImage->ResX * TImage->ResY);

      if (!tempbuf)
      {
         SPDXClose( Handle );
         SPDXFree( TImage );
         SPDXFree( TImage->Buffer );
         return LE_NOMEMIMG;
      }

      /* READ IN THE IMAGE DATE */
      for (y =  0; y < TImage->ResY; y++)
      {
         for (x =  0; x < TImage->ResX; x++)
         {
            tbyte = (CHR) SPDXGetc( Handle );

            /* Skip count? */
            if ((tbyte & 0xC0) == 0xC0)
            {
               count = (CHR) (tbyte & 0x3F);
               tbyte = (CHR) SPDXGetc( Handle );
               SPDXMemSetBYTE( &tempbuf[y*TImage->ResX+x], tbyte, count );
               x += count - 1;
            }
            else
            {
               tempbuf[y*TImage->ResX+x] = tbyte;
            }
         }
      }

      /* READ IN THE EXTENDED PALETTE */
      if (SPDXGetc( Handle ) != 12)
      {
         SPDXFree( TImage );
         SPDXClose( Handle );
         return LE_NOTVALIDPCX;
      }

      if (!SPDXRead(Handle, ExtendedPalette, 256*3 ))
      {
         SPDXFree( TImage );
         SPDXClose( Handle );
         return LE_NOTVALIDPCX;
      }

      SPDXClose(Handle);

      /* CONVERT THE TEMP BUFFER INTO THE REAL BUFFER */
      for (y =  0; y < TImage->ResY; y++)
         for (x =  0; x < TImage->ResX; x++)
         {
            TImage->Buffer[y*TImage->ResX*3+x*3+0] =
               ExtendedPalette[tempbuf[y*TImage->ResX+x]*3+0];
            TImage->Buffer[y*TImage->ResX*3+x*3+1] =
               ExtendedPalette[tempbuf[y*TImage->ResX+x]*3+1];
            TImage->Buffer[y*TImage->ResX*3+x*3+2] =
               ExtendedPalette[tempbuf[y*TImage->ResX+x]*3+2];
         }

      SPDXFree( tempbuf );
   }
   else
   {
      for (y =  0; y < TImage->ResY; y++)
      {
         /* READ IN THE RED IMAGE DATA */
         for (x =  0; x < TImage->ResX; x++)
         {
            tbyte = (CHR) SPDXGetc( Handle );

            /* Skip count? */
            if ((tbyte & 0xC0) == 0xC0)
            {
               count = (CHR) (tbyte & 0x3F);
               tbyte = (CHR) SPDXGetc( Handle );
               for (i = 0; i < count; i++ )
               {
                  TImage->Buffer[y*TImage->ResX*3+(x+i)*3+0] = tbyte;
               }
               x += count - 1;
            }
            else
            {
               TImage->Buffer[y*TImage->ResX*3+x*3+0] = tbyte;
            }
         }

         /* READ IN THE GREEN IMAGE DATA */
         for (x =  0; x < TImage->ResX; x++)
         {
            tbyte = (CHR) SPDXGetc( Handle );

            /* Skip count? */
            if ((tbyte & 0xC0) == 0xC0)
            {
               count = (CHR) (tbyte & 0x3F);
               tbyte = (CHR) SPDXGetc( Handle );
               for (i = 0; i < count; i++ )
               {
                  TImage->Buffer[y*TImage->ResX*3+(x+i)*3+1] = tbyte;
               }
               x += count - 1;
            }
            else
            {
               TImage->Buffer[y*TImage->ResX*3+x*3+1] = tbyte;
            }
         }

         /* READ IN THE BLUE IMAGE DATA */
         for (x =  0; x < TImage->ResX; x++)
         {
            tbyte = (CHR) SPDXGetc( Handle );

            /* Skip count? */
            if ((tbyte & 0xC0) == 0xC0)
            {
               count = (CHR) (tbyte & 0x3F);
               tbyte = (CHR) SPDXGetc( Handle );
               for (i = 0; i < count; i++ )
               {
                  TImage->Buffer[y*TImage->ResX*3+(x+i)*3+2] = tbyte;
               }
               x += count - 1;
            }
            else
            {
               TImage->Buffer[y*TImage->ResX*3+x*3+2] = tbyte;
            }
         }
      }
   }

   SPDXFree( (*Image)->Buffer );
   (*Image)->Buffer = TImage->Buffer;
   (*Image)->ResX = TImage->ResX;
   (*Image)->ResY = TImage->ResY;
   SPDXFree( TImage );

   return LE_NONE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [LOADIMG.C      ] - End Of File                                        ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

