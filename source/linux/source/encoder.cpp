// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [ENCODER.C   ] - GIF Encoding routines                                 -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#include "spandex.h"

/*----------------------------------------------------------------------------*/

static   INT   GIFNextPixel( ifunptrgp getpixel );
static   void  Putword( INT Handle, INT w );
static   INT   Output( INT code );
static   INT   ClBlock( void );
static   void  ClHash(register INT hsize);
static   void  CharOut( INT c );
static   void  FlushChar( void );

/*----------------------------------------------------------------------------*/

#define CODE_BITS       12
#define HSIZE           5003
#define MAXCODE(n_bits) (((INT) 1 << (n_bits)) - 1)
#define BITS_PER_PIXEL  8
#define BACKGROUND      0
#define COLOR_MAP_SIZE  (1 << BITS_PER_PIXEL)
#define HashTabOf(i)    htab[i]
#define CodeTabOf(i)    codetab[i]

/*----------------------------------------------------------------------------*/

static INT  Width, Height;
static INT  curx, cury;
static INT  Pass = 0;
static INT  n_bits;
static INT  maxbits = CODE_BITS;
static INT  maxcode;
static INT  maxmaxcode = (INT)1 << CODE_BITS;
static INT  hsize = HSIZE;
static INT  free_ent = 0;
static INT  clear_flg = 0;
static INT  offset;
static INT  g_init_bits;
static INT  ClearCode;
static INT  EOFCode;
static INT  cur_bits = 0;
static INT  a_count;
static INT  CountDown;
static INT  htab [HSIZE];
static INT  in_count = 1;
static INT  out_count = 0;
static CHR  accum[256];
static INT  g_outfile;
static INT  codetab [HSIZE];
static INT  cur_accum = 0;
static INT  masks[] = {   0x0000,
            0x0001, 0x0003, 0x0007, 0x000F,
            0x001F, 0x003F, 0x007F, 0x00FF,
            0x01FF, 0x03FF, 0x07FF, 0x0FFF,
            0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF };

/*----------------------------------------------------------------------------*/

INT   SPDXGifEncoder( CHR *FName, INT GWidth, INT GHeight, UCHR *Palette, ifunptrgp GetPixel )
{
INT   B, RetCode, Handle;
INT   RWidth, RHeight;
INT   LeftOfs, TopOfs;
INT   Resolution;
INT   InitCodeSize;
INT   i;

   FUNC("SPDXGifEncoder");
   Assert( FName );
   Assert( Palette );
   Assert( GetPixel );

   RWidth = Width = GWidth;
   RHeight = Height = GHeight;
   LeftOfs = TopOfs = 0;
   
   Resolution = BITS_PER_PIXEL;

   /* Calculate number of bits we are expecting */
   CountDown = (LNG)Width * (LNG)Height;

   /* Indicate which pass we are on (if interlace) */
   Pass = 0;

   /* The initial code size */
   InitCodeSize = BITS_PER_PIXEL;

   if (InitCodeSize <= 1)
      InitCodeSize = 2;

   /* Open the GIF file for binary write */
   Handle = SPDXOpen( FName, O_RDWR|O_TRUNC|O_CREAT|O_BINARY, S_IREAD|S_IWRITE );

   if (Handle <= 0)
      return LE_NOOPEN;

   /* Write the Magic header */
   SPDXWrite( Handle, "GIF87a", 6 );

   /* Write out the screen width and height */
   Putword( Handle, RWidth );
   Putword( Handle, RHeight );

   /* Indicate that there is a global colour map */
   B = 0x80;   /* Yes, there is a color map */

   /* OR in the resolution */
   B |= (Resolution - 1) << 5;

   /* OR in the Bits per Pixel */
   B |= (BITS_PER_PIXEL - 1);

   /* Write it out */
   SPDXPutc( Handle, B );

   /* Write out the Background colour */
   SPDXPutc( Handle, BACKGROUND );

   /* Byte of 0's (future expansion) */
   SPDXPutc( Handle, 0 );

   /* Write out the Global Colour Map */
   for( i=0; i<COLOR_MAP_SIZE; i++ )
   {
      SPDXPutc( Handle, Palette[i*3+0] );
      SPDXPutc( Handle, Palette[i*3+1] );
      SPDXPutc( Handle, Palette[i*3+2] );
   }

   /* Write an Image separator */
   SPDXPutc( Handle, ',' );

   /* Write the Image header */

   Putword( Handle, LeftOfs );
   Putword( Handle, TopOfs );
   Putword( Handle, Width );
   Putword( Handle, Height );

   /* Write out whether or not the image is interlaced */
   SPDXPutc( Handle, 0 );

   /* Write out the initial code size */
   SPDXPutc( Handle, InitCodeSize );

   /* Go and actually compress the data */
   RetCode = SPDXCompress( InitCodeSize+1, Handle, GetPixel );

   if (RetCode != LE_NONE )
   {
      SPDXClose(Handle);
      return RetCode;
   }

   /* Write out a Zero-length packet (to end the series) */
   SPDXPutc( Handle, 0 );

   /* Write the GIF file terminator */
   SPDXPutc( Handle, ';' );

   /* And close the file */
   SPDXClose(Handle);

   return LE_NONE;
}

/*----------------------------------------------------------------------------*/

INT   SPDXCompress( INT init_bits, INT Handle, ifunptrgp ReadValue )
{
INT   RetCode;
INT   fcode;
INT   i = 0;
INT   c;
INT   ent;
INT   disp;
INT   hsize_reg;
INT   hshift;

   FUNC("SPDXCompress");
   Assert( ReadValue );

   cur_bits = 0;

   /* Set up the current x and y position */
   curx = cury = 0;

   /* Set up the globals:  g_init_bits - initial number of bits
                           g_outfile   - pointer to output file */
   g_init_bits = init_bits;
   g_outfile = Handle;

   /* Set up the necessary values */
   offset = 0;
   out_count = 0;
   clear_flg = 0;
   in_count = 1;
   maxcode = MAXCODE(n_bits = g_init_bits);

   ClearCode = (1 << (init_bits - 1));
   EOFCode = ClearCode + 1;
   free_ent = ClearCode + 2;

   a_count = 0;

   ent = GIFNextPixel( ReadValue );

   hshift = 0;
   for ( fcode = (LNG) hsize;  fcode < 65536L; fcode *= 2L )
   hshift++;
   hshift = 8 - hshift;                /* set hash code range bound */

   hsize_reg = hsize;
   ClHash( (LNG) hsize_reg);            /* clear hash table */

   RetCode = Output( (INT)ClearCode );

   if (RetCode != LE_NONE )
      return RetCode;

   while ( (c = GIFNextPixel( ReadValue )) != EOF )
   {
      in_count++;

      fcode = (LNG) (((LNG) c << maxbits) + ent);
      i = (((INT)c << hshift) ^ ent);    /* xor hashing */

      if ( HashTabOf (i) == fcode )
      {
         ent = CodeTabOf (i);
         continue;
      }
      else if ( (LNG)HashTabOf (i) < 0 )      /* empty slot */
         goto nomatch;

      disp = hsize_reg - i;           /* secondary hash (after G. Knott) */

      if ( i == 0 )
         disp = 1;

probe:
      if ( (i -= disp) < 0 )
         i += hsize_reg;

      if ( HashTabOf (i) == fcode )
      {
         ent = CodeTabOf (i);
         continue;
      }

      if ( (LNG)HashTabOf (i) > 0 )
         goto probe;

nomatch:
      RetCode = Output ( (INT) ent );

      if (RetCode != LE_NONE )
         return RetCode;

      out_count++;
      ent = c;
      if ( free_ent < maxmaxcode )
      {
         CodeTabOf (i) = free_ent++; /* code -> hashtable */
         HashTabOf (i) = fcode;
      }
      else
      {
         RetCode = ClBlock();
         if (RetCode != LE_NONE)
            return RetCode;
      }
   }

   /* Put out the final code. */
   RetCode = Output( (INT) ent );

   if (RetCode != LE_NONE )
      return RetCode;

   out_count++;
   RetCode = Output( (INT) EOFCode );

   if (RetCode != LE_NONE )
      return RetCode;

   return LE_NONE;
}

/*----------------------------------------------------------------------------*/

INT   GIFNextPixel( ifunptrgp getpixel )
{
INT   r;

   FUNC("GIFNextPixel");
   Assert( getpixel );

   if ( CountDown == 0 )
      return EOF;

   CountDown--;

   r = ( * getpixel )( curx, cury );

   curx++;
   if( curx == Width )
   {
      curx = 0;
      cury++;
   }

   return r;
}

/*----------------------------------------------------------------------------*/

void  SPDXSetCompressDataCount( INT Count )
{
   FUNC("SPDXSetCompressDataCount");
   CountDown = Count;
}

/*----------------------------------------------------------------------------*/

void  Putword( INT Handle, INT w )
{
   FUNC("Putword");
   SPDXPutc( Handle, w & 0xff );
   SPDXPutc( Handle, (w / 256) & 0xff );
}

/*----------------------------------------------------------------------------*/

INT   Output( INT code )
{
   FUNC("Output");
   cur_accum &= masks[cur_bits];

   if( cur_bits > 0 )
      cur_accum |= ((LNG)code << cur_bits);
   else
      cur_accum = code;

   cur_bits += n_bits;

   while( cur_bits >= 8 )
   {
      CharOut( (UINT)(cur_accum & 0xff) );
      cur_accum >>= 8;
      cur_bits -= 8;
   }

   /* If the next entry is going to be too big for the code size,
      then increase it, if possible. */

   if ( free_ent > maxcode || clear_flg )
   {
      if( clear_flg )
      {
         maxcode = MAXCODE (n_bits = g_init_bits);
         clear_flg = 0;

      }
      else
      {
         n_bits++;
         if ( n_bits == maxbits )
            maxcode = maxmaxcode;
         else
            maxcode = MAXCODE(n_bits);
      }
   }

   if( code == EOFCode )
   {
      /* At EOF, write the rest of the buffer. */

      while( cur_bits > 0 )
      {
         CharOut( (UINT)(cur_accum & 0xff) );
         cur_accum >>= 8;
         cur_bits -= 8;
      }

      FlushChar();
   }

   return LE_NONE;
}

/*----------------------------------------------------------------------------*/

INT   ClBlock( )
{
   FUNC("ClBlock");
   ClHash ( (LNG) hsize );
   free_ent = ClearCode + 2;
   clear_flg = 1;

   return Output( (INT)ClearCode );
}

/*----------------------------------------------------------------------------*/

void  ClHash(register INT hsize)
{
INT   *htab_p = htab+hsize;
INT   i;
INT   m1 = -1;

   FUNC("ClHash");
   i = hsize - 16;

   do
   {
      *(htab_p-16) = m1;
      *(htab_p-15) = m1;
      *(htab_p-14) = m1;
      *(htab_p-13) = m1;
      *(htab_p-12) = m1;
      *(htab_p-11) = m1;
      *(htab_p-10) = m1;
      *(htab_p-9) = m1;
      *(htab_p-8) = m1;
      *(htab_p-7) = m1;
      *(htab_p-6) = m1;
      *(htab_p-5) = m1;
      *(htab_p-4) = m1;
      *(htab_p-3) = m1;
      *(htab_p-2) = m1;
      *(htab_p-1) = m1;
      htab_p -= 16;
   } while ((i -= 16) >= 0);

   for ( i += 16; i > 0; i-- )
      *--htab_p = m1;
}

/*----------------------------------------------------------------------------*/

void  CharOut( INT c )
{
   FUNC("CharOut");
   accum[ a_count++ ] = (CHR) c;

   if ( a_count >= 254 )
      FlushChar();
}

/*----------------------------------------------------------------------------*/

void  FlushChar( )
{
   FUNC("FlushChar");
   if ( a_count > 0 )
   {
      SPDXPutc( g_outfile, a_count );
      SPDXWrite( g_outfile, accum, a_count );
      a_count = 0;
   }
}

/*----------------------------------------------------------------------------
  -   [ENCODER.C   ] - End Of File                                           -
  ----------------------------------------------------------------------------*/
