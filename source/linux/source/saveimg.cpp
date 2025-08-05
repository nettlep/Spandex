// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [SAVEIMG.C   ] - Misc. image format saving routines                    -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#include "spandex.h"

/*----------------------------------------------------------------------------*/

static   IMAGE *GlobBuffer = 0;
static   UBYT  Palette[256*3];

/*----------------------------------------------------------------------------*/

static   INT   WriteIPI( CHR *FileName, IMAGE *Image );
static   INT   WriteTGA( CHR *FileName, IMAGE *Image, INT UpsideDownFlag );
static   INT   WriteBMP( CHR *FileName, IMAGE *Image );
static   INT   WriteIMG( CHR *FileName, IMAGE *Image );
static   INT   WriteGIF( CHR *FileName, IMAGE *Image );
static   INT   WriteG24( CHR *FileName, IMAGE *Image );
static   INT   WritePCX( CHR *FileName, IMAGE *Image );
static   INT   GetGifPixel( INT x, INT y );
static   INT   GetG24Pixel( INT x, INT y );

/*----------------------------------------------------------------------------*/

INT   SPDXWriteImage( CHR *FileName, IMAGE *Image, INT UpsideDownFlag )
{
INT   RetCode;

   FUNC("SPDXWriteImage");
   Assert( FileName );
   Assert( Image );

   if (!Image || !Image->Buffer)
      return LE_BADIMAGE;

   switch( SPDXGetFileType( FileName ) )
   {
      case IMAGETYPE_IPI:
         RetCode = WriteIPI( FileName, Image );
         break;

      case IMAGETYPE_IMG:
         RetCode = WriteIMG( FileName, Image );
         break;

      case IMAGETYPE_GIF:
         RetCode = WriteGIF( FileName, Image );
         break;

      case IMAGETYPE_G24:
         RetCode = WriteG24( FileName, Image );
         break;

      case IMAGETYPE_TGA:
         RetCode = WriteTGA( FileName, Image, UpsideDownFlag  );
         break;

      case IMAGETYPE_BMP:
         RetCode = WriteBMP( FileName, Image );
         break;

      case IMAGETYPE_PCX:
         RetCode = WritePCX( FileName, Image );
         break;

      default:
         RetCode = LE_INVALIDFILETYPE;
   }

   return RetCode;
}

/*----------------------------------------------------------------------------*/

INT   WriteGIF( CHR *FileName, IMAGE *Image )
{
   FUNC("WriteGIF");
   Assert( FileName );
   Assert( Image );

   SPDXNeuquantImage( Image, 8, Palette );

   GlobBuffer = Image;

   return SPDXGifEncoder(FileName, Image->ResX, Image->ResY, (UCHR *) Palette, GetGifPixel);
}

/*----------------------------------------------------------------------------*/

INT   WriteG24( CHR *FileName, IMAGE *Image )
{
INT   i, RetCode;

   FUNC("WriteG24");
   Assert( FileName );
   Assert( Image );

   for (i = 0; i < 256; i++ )
   {
      Palette[i*3+0] = (CHR) i;
      Palette[i*3+1] = (CHR) i;
      Palette[i*3+2] = (CHR) i;
   }

   GlobBuffer = Image;

   RetCode = SPDXGifEncoder(FileName, Image->ResX, Image->ResY * 3, (UCHR *) Palette, GetG24Pixel);

   GlobBuffer = 0;

   return RetCode;
}

/*----------------------------------------------------------------------------*/

INT   GetGifPixel( INT x, INT y )
{
   FUNC("GetGifPixel");
   return GlobBuffer->Buffer[y*GlobBuffer->ResX+x];
}

/*----------------------------------------------------------------------------*/

INT   GetG24Pixel( INT x, INT y )
{
   FUNC("GetG24Pixel");
   if (y >= GlobBuffer->ResY * 2)
      return GlobBuffer->Buffer[(y-GlobBuffer->ResY*2)*GlobBuffer->ResX*3+x*3+2];

   if (y >= GlobBuffer->ResY)
      return GlobBuffer->Buffer[(y-GlobBuffer->ResY)*GlobBuffer->ResX*3+x*3+1];

   return GlobBuffer->Buffer[y*GlobBuffer->ResX*3+x*3+0];
}

/*----------------------------------------------------------------------------*/

INT   WriteIPI( CHR *FileName, IMAGE *Image )
{
INT   Handle;

   FUNC("WriteIPI");
   Assert( FileName );
   Assert( Image );

   Handle = SPDXOpen( FileName, O_RDWR|O_BINARY|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE );

   if (Handle <= 0)
      return LE_NOOPEN;

   SPDXPutc(Handle, Image->ResX >> 8 );
   SPDXPutc(Handle, Image->ResX & 0xff );
   SPDXPutc(Handle, Image->ResY>> 8 );
   SPDXPutc(Handle, Image->ResY & 0xff );

   if (SPDXWrite(Handle, Image->Buffer, Image->ResX * Image->ResY * 3 ) != Image->ResX * Image->ResY * 3)
   {
      SPDXClose(Handle);
      return LE_NOWRITE;
   }

   SPDXClose(Handle);

   return LE_NONE;
}

/*----------------------------------------------------------------------------*/

INT   WriteIMG( CHR *FileName, IMAGE *Image )
{
INT   x, y, i, Count, br, bg, bb;
INT   Handle;

   FUNC("WriteIMG");
   Assert( FileName );
   Assert( Image );

   Handle = SPDXOpen(FileName, O_RDWR|O_BINARY|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE);

   if(Handle <= 0)
      return LE_NOOPEN;

   SPDXPutc( Handle, Image->ResX >> 8 );
   SPDXPutc( Handle, Image->ResX & 0xff );
   SPDXPutc( Handle, Image->ResY >> 8 );
   SPDXPutc( Handle, Image->ResY & 0xff );

   SPDXPutc( Handle, 0 );
   SPDXPutc( Handle, 0 );
   SPDXPutc( Handle, (Image->ResY-1) >> 8 );
   SPDXPutc( Handle, (Image->ResY-1) & 0xff );

   SPDXPutc( Handle, 0);
   SPDXPutc( Handle, 24);

/* ENCODE THIS IMAGE */

   for( y = 0; y < Image->ResY; y++ )
   {
      for( x = 0; x < Image->ResX; )
      {
         br = Image->Buffer[y * Image->ResX * 3 + x * 3 + 0];
         bg = Image->Buffer[y * Image->ResX * 3 + x * 3 + 1];
         bb = Image->Buffer[y * Image->ResX * 3 + x * 3 + 2];

         Count = 1;

         for( i = x+1; i < Image->ResX; i++ )
         {
            if (Image->Buffer[y * Image->ResX * 3 + i * 3 + 0] == br &&
                Image->Buffer[y * Image->ResX * 3 + i * 3 + 1] == bg &&
                Image->Buffer[y * Image->ResX * 3 + i * 3 + 2] == bb)
            {
               if (++Count == 255)
                  break;
            }
            else
            {
               break;
            }
         }

         SPDXPutc(Handle, Count);
         SPDXPutc(Handle,    bb);
         SPDXPutc(Handle,    bg);
         SPDXPutc(Handle,    br);

         x += Count;
      }
   }

   SPDXClose(Handle);
   return LE_NONE;
}

/*----------------------------------------------------------------------------*/

INT   WriteTGA( CHR *FileName, IMAGE *Image, INT UpsideDownFlag )
{
INT   i, j;
INT   Handle;

   FUNC("WriteTGA");
   Assert( FileName );
   Assert( Image );

   Handle = SPDXOpen(FileName, O_RDWR|O_BINARY|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE);

   if(Handle <= 0)
      return LE_NOOPEN;

   SPDXPutc(Handle, 0);
   SPDXPutc(Handle, 0);

   SPDXPutc(Handle, 2);

   for(i = 3; i < 12; i++)
      SPDXPutc(Handle, 0);

   SPDXPutc(Handle, Image->ResX & 0xff);
   SPDXPutc(Handle, Image->ResX >> 8);
   SPDXPutc(Handle, Image->ResY & 0xff);
   SPDXPutc(Handle, Image->ResY >> 8);

   SPDXPutc(Handle, 24);

   if (UpsideDownFlag == TOP_DOWN)
      SPDXPutc(Handle, 32);
   else
      SPDXPutc(Handle, 0);

/* ENCODE THIS IMAGE */

   if (UpsideDownFlag == TOP_DOWN)
      for( j = 0; j < Image->ResY * Image->ResX * 3; j += Image->ResX * 3 )
      {
         for( i = 0; i < Image->ResX * 3; i+=3 )
         {
            SPDXPutc(Handle, Image->Buffer[j+i+2]);
            SPDXPutc(Handle, Image->Buffer[j+i+1]);
            SPDXPutc(Handle, Image->Buffer[j+i+0]);
         }
      }
   else
      for( j = (Image->ResY-1) * Image->ResX * 3; j >= 0; j -= Image->ResX * 3 )
      {
         for( i = 0; i < Image->ResX * 3; i+=3 )
         {
            SPDXPutc(Handle, Image->Buffer[j+i+2]);
            SPDXPutc(Handle, Image->Buffer[j+i+1]);
            SPDXPutc(Handle, Image->Buffer[j+i+0]);
         }
      }
        
   SPDXClose(Handle);
   return LE_NONE;
}

/*----------------------------------------------------------------------------*/

INT   WriteBMP( CHR *FileName, IMAGE *Image )
{
INT   x, y, size;
INT   Handle;

   FUNC("WriteBMP");
   Assert( FileName );
   Assert( Image );

   Handle = SPDXOpen(FileName, O_RDWR|O_BINARY|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE);

   if(Handle <= 0)
      return LE_NOOPEN;

   SPDXPutc(Handle, 'B');                     // FILE ID
   SPDXPutc(Handle, 'M');

   size = 54 + Image->ResX * Image->ResY * 3;

   SPDXPutc(Handle, size & 0xff);             // SIZE
   SPDXPutc(Handle, (size >>  8) & 0xff);
   SPDXPutc(Handle, (size >> 16) & 0xff);
   SPDXPutc(Handle, (size >> 24) & 0xff);

   SPDXPutc(Handle, 0);                       // RESERVED
   SPDXPutc(Handle, 0);

   SPDXPutc(Handle, 0);                       // RESERVED
   SPDXPutc(Handle, 0);

   SPDXPutc(Handle, 54);                      // OFFSET TO BITMAP BITS
   SPDXPutc(Handle,  0);
   SPDXPutc(Handle,  0);
   SPDXPutc(Handle,  0);

   SPDXPutc(Handle, 40);                      // SIZE OF FOLLOWING STRUCT
   SPDXPutc(Handle,  0);
   SPDXPutc(Handle,  0);
   SPDXPutc(Handle,  0);

   SPDXPutc(Handle, (Image->ResX)     & 0xff); // XRES
   SPDXPutc(Handle, (Image->ResX>>8 ) & 0xff);
   SPDXPutc(Handle, (Image->ResX>>16) & 0xff);
   SPDXPutc(Handle, (Image->ResX>>24) & 0xff);

   SPDXPutc(Handle, (Image->ResY)     & 0xff); // YRES
   SPDXPutc(Handle, (Image->ResY>> 8) & 0xff);
   SPDXPutc(Handle, (Image->ResY>>16) & 0xff);
   SPDXPutc(Handle, (Image->ResY>>24) & 0xff);

   SPDXPutc(Handle,  1);                      // PLANES
   SPDXPutc(Handle,  0);

   SPDXPutc(Handle, 24);                      // 24 BITS PER PIXEL
   SPDXPutc(Handle,  0);

   SPDXPutc(Handle,  0);                      // COMPRESSION (NONE)
   SPDXPutc(Handle,  0);
   SPDXPutc(Handle,  0);
   SPDXPutc(Handle,  0);

   SPDXPutc(Handle,  0);                      // SIZE OF BITMAP BITS IN BYTES
   SPDXPutc(Handle,  0);
   SPDXPutc(Handle,  0);
   SPDXPutc(Handle,  0);

   SPDXPutc(Handle,  0);                      // Horiz pixels per meter
   SPDXPutc(Handle,  0);
   SPDXPutc(Handle,  0);
   SPDXPutc(Handle,  0);

   SPDXPutc(Handle,  0);                      // Vert pixels per meter
   SPDXPutc(Handle,  0);
   SPDXPutc(Handle,  0);
   SPDXPutc(Handle,  0);

   SPDXPutc(Handle,  0);                      // Colors Used
   SPDXPutc(Handle,  0);
   SPDXPutc(Handle,  0);
   SPDXPutc(Handle,  0);

   SPDXPutc(Handle,  0);                      // Important colors
   SPDXPutc(Handle,  0);
   SPDXPutc(Handle,  0);
   SPDXPutc(Handle,  0);

/* ENCODE THIS IMAGE */

   for( y = Image->ResY-1; y >= 0; y-- )
   {
      for( x = 0; x < Image->ResX; x++ )
      {
         SPDXPutc(Handle, Image->Buffer[y*Image->ResX*3+x*3+2]);
         SPDXPutc(Handle, Image->Buffer[y*Image->ResX*3+x*3+1]);
         SPDXPutc(Handle, Image->Buffer[y*Image->ResX*3+x*3+0]);
      }
   }

   SPDXClose(Handle);
   return LE_NONE;
}

/*----------------------------------------------------------------------------*/

INT   WritePCX( CHR *FileName, IMAGE *Image )
{
INT   x, y, i;
INT   Handle;
PCXFL PCXFile;

   FUNC("WritePCX");
   Assert( FileName );
   Assert( Image );

   Handle = SPDXOpen(FileName, O_RDWR|O_BINARY|O_CREAT|O_TRUNC, S_IREAD|S_IWRITE);

   if(Handle <= 0)
      return LE_NOOPEN;

/* WRITE THE HEADER */

   PCXFile.Header.Identifier = 10;
   PCXFile.Header.Version = 5;
   PCXFile.Header.Encode = 1;
   PCXFile.Header.Bits = 8;
   PCXFile.Header.Left = 0;
   PCXFile.Header.Right = (UWRD) (Image->ResX-1);
   PCXFile.Header.Top = 0;
   PCXFile.Header.Bottom = (UWRD) (Image->ResY-1);
   PCXFile.Header.AdapterWidth = 0;
   PCXFile.Header.AdapterHeight = 0;
   SPDXMemSetDWORD( PCXFile.Pal16, 0, (16*3)>>2 );
   PCXFile.Info.VMode = 0;
   PCXFile.Info.Planes = 3;
   PCXFile.Info.BytesPerLine = (WRD) Image->ResX;
   SPDXMemSetWORD( PCXFile.Info.RFU, 0, 60>>1 );

   SPDXWrite( Handle, (void *) &PCXFile, sizeof(PCXFL) );

/* ENCODE THIS IMAGE */

   for (y =  0; y < Image->ResY; y++)
   {
      /* WRITE THE RED IMAGE DATA */
      for (x =  0; x < Image->ResX; x++)
      {
         for( i = 1; i < Image->ResX-x && i < 0x3F; i++ )
            if (Image->Buffer[y*Image->ResX*3+(i+x+0)*3+0] !=
                Image->Buffer[y*Image->ResX*3+(i+x-1)*3+0])
               break;

         if (i == 1)
         {
            if (Image->Buffer[y*Image->ResX*3+x*3+0] & 0xC0)
            {
               SPDXPutc(Handle,  1 | 0xC0 );
               SPDXPutc(Handle,  Image->Buffer[y*Image->ResX*3+x*3+0] );
            }
            else
               SPDXPutc(Handle,  Image->Buffer[y*Image->ResX*3+x*3+0] );
         }
         else
         {
            SPDXPutc(Handle,  i | 0xC0 );
            SPDXPutc(Handle,  Image->Buffer[y*Image->ResX*3+x*3+0] );
            x += i-1;
         }
      }

      /* WRITE THE GREEN IMAGE DATA */
      for (x =  0; x < Image->ResX; x++)
      {
         for( i = 1; i < Image->ResX-x && i < 0x3F; i++ )
            if (Image->Buffer[y*Image->ResX*3+(i+x+0)*3+1] !=
                Image->Buffer[y*Image->ResX*3+(i+x-1)*3+1])
               break;

         if (i == 1)
         {
            if (Image->Buffer[y*Image->ResX*3+x*3+1] & 0xC0)
            {
               SPDXPutc(Handle,  1 | 0xC0 );
               SPDXPutc(Handle,  Image->Buffer[y*Image->ResX*3+x*3+1] );
            }
            else
               SPDXPutc(Handle,  Image->Buffer[y*Image->ResX*3+x*3+1] );
         }
         else
         {
            SPDXPutc(Handle,  i | 0xC0 );
            SPDXPutc(Handle,  Image->Buffer[y*Image->ResX*3+x*3+1] );
            x += i-1;
         }
      }

      /* WRITE THE BLUE IMAGE DATA */
      for (x =  0; x < Image->ResX; x++)
      {
         for( i = 1; i < Image->ResX-x && i < 0x3F; i++ )
            if (Image->Buffer[y*Image->ResX*3+(i+x+0)*3+2] !=
                Image->Buffer[y*Image->ResX*3+(i+x-1)*3+2])
               break;

         if (i == 1)
         {
            if (Image->Buffer[y*Image->ResX*3+x*3+2] & 0xC0)
            {
               SPDXPutc(Handle,  1 | 0xC0 );
               SPDXPutc(Handle,  Image->Buffer[y*Image->ResX*3+x*3+2] );
            }
            else
               SPDXPutc(Handle,  Image->Buffer[y*Image->ResX*3+x*3+2] );
         }
         else
         {
            SPDXPutc(Handle,  i | 0xC0 );
            SPDXPutc(Handle,  Image->Buffer[y*Image->ResX*3+x*3+2] );
            x += i-1;
         }
      }
   }

   SPDXClose(Handle);
   return LE_NONE;
}

/*----------------------------------------------------------------------------
  -   [SAVEIMG.C   ] - End Of File                                           -
  ----------------------------------------------------------------------------*/

