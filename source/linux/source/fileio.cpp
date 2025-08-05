// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [FILEIO.C    ] - Fast File IO stuffs                                   -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#include "spandex.h"

/*----------------------------------------------------------------------------*/

static   INT   BytesInBuffer = 0;
static   INT   LastHandle = -1;
static   INT   BufferOffset = 0;
static   CHR   IOBuffer[IO_BUFFER_SIZE];

/*----------------------------------------------------------------------------*/

static   void  RefreshBuffer( INT Handle );

/*----------------------------------------------------------------------------*/

void  RefreshBuffer( INT Handle )
{
   FUNC("RefreshBuffer");
   memmove( IOBuffer, &IOBuffer[BufferOffset], BytesInBuffer );
   BytesInBuffer += _SPDXRead(Handle, &IOBuffer[BytesInBuffer], IO_BUFFER_SIZE - BytesInBuffer );
   BufferOffset = 0;
}

/*----------------------------------------------------------------------------*/

INT   SPDXTell( INT Handle )
{
   FUNC("SPDXTell");
   return SPDXSeek( Handle, 0, SEEK_CUR );
}

/*----------------------------------------------------------------------------*/

INT   SPDXGetc( INT Handle )
{
unsigned char byte;

   FUNC("SPDXGetc");
   SPDXRead( Handle, &byte, 1 );
   return byte;
}

/*----------------------------------------------------------------------------*/

void  SPDXPutc( INT Handle, INT Byte )
{
   FUNC("SPDXPutc");
   SPDXWrite( Handle, (CHR *) &Byte, 1 );
}

/*----------------------------------------------------------------------------*/

INT   SPDXRead( INT Handle, void *Buffer, INT Length )
{
CHR   *pTemp;

   FUNC("SPDXRead");
   Assert( Buffer );

   if (Handle != LastHandle)
   {
      SPDXFlushBuffer(LastHandle);
      LastHandle = Handle;
   }

   // Pull data directly out of the buffer?
   if (BytesInBuffer >= Length)
   {
      SPDXMemCopyBYTE( Buffer, &IOBuffer[BufferOffset], Length );
      BufferOffset += Length;
      BytesInBuffer -= Length;
      return Length;
   }

   // Is the buffer large enough to serve?
   if (Length < IO_BUFFER_SIZE)
   {
      RefreshBuffer( Handle );

      // Did we reach the end if the file?
      if (BytesInBuffer < Length)
      {
         SPDXMemCopyBYTE( Buffer, &IOBuffer[BufferOffset], BytesInBuffer );
         BytesInBuffer = 0;
         BufferOffset = 0;
         return BytesInBuffer;
      }

      SPDXMemCopyBYTE( Buffer, &IOBuffer[BufferOffset], Length );
      BufferOffset += Length;
      BytesInBuffer -= Length;
      return Length;
   }

   // The buffer is not large enough to serve
   SPDXMemCopyBYTE( Buffer, &IOBuffer[BufferOffset], BytesInBuffer );
   pTemp = (CHR *) Buffer;
   Length = BytesInBuffer + _SPDXRead( Handle, (void *) &pTemp[BytesInBuffer], Length - BytesInBuffer);
   BufferOffset = 0;
   BytesInBuffer = 0;
   return Length;
}

/*----------------------------------------------------------------------------*/

void  SPDXFlushBuffer( INT Handle )
{
LNG   Offset;

   FUNC("SPDXFlushBuffer");
   if (Handle == LastHandle)
   {
      Offset = _SPDXSeek( Handle, 0, SEEK_CUR ) - BytesInBuffer;
      _SPDXSeek( Handle, Offset, SEEK_SET );
   }

   BytesInBuffer = BufferOffset = 0;
   LastHandle = -1;
}

/*----------------------------------------------------------------------------*/

INT   SPDXWrite( INT Handle, void *Buffer, INT Length )
{
   FUNC("SPDXWrite");
   Assert( Buffer );

   SPDXFlushBuffer(Handle);
   return _SPDXWrite( Handle, Buffer, Length );
}

/*----------------------------------------------------------------------------*/

LNG   SPDXSeek( INT Handle, LNG Offset, INT Origin )
{
   FUNC("SPDXSeek");
   SPDXFlushBuffer(Handle);
   return _SPDXSeek( Handle, Offset, Origin );
}

/*----------------------------------------------------------------------------*/

INT   SPDXOpen( CHR *FileName, INT Flags1, INT Flags2 )
{
   FUNC("SPDXOpen");
   Assert( FileName );

   SPDXFlushBuffer(LastHandle);
   return _SPDXOpen( FileName, Flags1, Flags2 );
}

/*----------------------------------------------------------------------------*/

INT   SPDXClose( INT Handle )
{
   FUNC("SPDXClose");
   SPDXFlushBuffer(Handle);
   return _SPDXClose( Handle );
}

/*----------------------------------------------------------------------------*/

CHR   *SPDXReadString( INT Handle, CHR *Buffer, INT MaxLength )
{
INT   i = 0, Count = 1;
CHR   ch = 0;

   FUNC("SPDXReadString");
   Assert( Buffer );

   if (Handle != LastHandle)
   {
      SPDXFlushBuffer(LastHandle);
      LastHandle = Handle;
   }

   MaxLength--;

   while(i < MaxLength && Count && ch != '\n')
   {
      Count = SPDXRead( Handle, &ch, 1 );
      Buffer[i++] = ch;
   }

   Buffer[i] = '\0';

   return Count ? Buffer:0;
}

/*----------------------------------------------------------------------------*/

INT   _SPDXOpen( CHR *FileName, INT Flags1, INT Flags2 )
{
   FUNC("_SPDXOpen");
   Assert( FileName );

   return open( FileName, Flags1, Flags2 );
}

/*----------------------------------------------------------------------------*/

INT   _SPDXClose( INT Handle )
{
   FUNC("_SPDXClose");
   return close( Handle );
}

/*----------------------------------------------------------------------------*/

INT   _SPDXRead( INT Handle, void *Buffer, INT Length )
{
  return read(Handle, Buffer, Length);
/*
UINT  MemRead = 0, AttemptLength, TotalRead = 0;
CHR   *pTemp;

union    REGS  inregs, outregs;
struct   SREGS sregs;

   FUNC("_SPDXRead");
   Assert( Buffer );

   pTemp = (CHR *) Buffer;

   while( Length )
   {
      AttemptLength = (Length > GENERIC_DOS_RAM_LEN) ? GENERIC_DOS_RAM_LEN:Length;
      // TODO ASM
      inregs.x.eax = 0x3F00;
      inregs.x.ebx = Handle;
      inregs.x.ecx = AttemptLength;
      inregs.x.edx = PROT_OFF(SPDXGlobals.DOSMemGeneric);
      sregs.ds = (UWRD) PROT_SEG(SPDXGlobals.DOSMemGeneric);

      SPDXint86x( DOS_INT, &inregs, &outregs, &sregs );

      MemRead = outregs.x.eax;

      // Copy it into the buffer
      SPDXMemCopyBYTE( pTemp, SPDXGlobals.DOSMemGeneric, MemRead );

      // Check carry flag (on == ERROR)
      if (outregs.x.cflag & 1)
         return -1;
      pTemp += AttemptLength;
      Length -= AttemptLength;
      TotalRead += MemRead;
   }
      */
}

/*----------------------------------------------------------------------------*/

INT   _SPDXWrite( INT Handle, void *Buffer, INT Length )
{
  return write(Handle, Buffer, Length);

/*
CHR   *pTemp;
UINT  MemWrote = 0, AttemptLength, TotalWrote = 0;

union    REGS  inregs, outregs;
struct   SREGS sregs;

   FUNC("_SPDXWrite");
   Assert( Buffer );

   pTemp = (CHR *) Buffer;

   while( Length )
   {
      AttemptLength = (Length > GENERIC_DOS_RAM_LEN) ? GENERIC_DOS_RAM_LEN:Length;
      // TODO ASM

      inregs.x.eax = 0x4000;
      inregs.x.ebx = Handle;
      inregs.x.ecx = AttemptLength;
      inregs.x.edx = PROT_OFF(SPDXGlobals.DOSMemGeneric);
      sregs.ds = (UWRD) PROT_SEG(SPDXGlobals.DOSMemGeneric);

      // Copy it to the DOS buffer
      SPDXMemCopyBYTE( SPDXGlobals.DOSMemGeneric, pTemp, AttemptLength );

      SPDXint86x( DOS_INT, &inregs, &outregs, &sregs );

      MemWrote = outregs.x.eax;

      // Check carry flag (on == ERROR)
      if (outregs.x.cflag & 1)
         return -1;

      pTemp += AttemptLength;
      Length -= AttemptLength;
      TotalWrote += MemWrote;
   }

   return TotalWrote;
*/
}

/*----------------------------------------------------------------------------*/

LNG   _SPDXSeek( INT Handle, LNG Offset, INT Origin)
{
  return lseek(Handle, Offset, Origin);
  /*
//union REGS  inregs, outregs;

// TODO asm

   FUNC("_SPDXSeek");
   inregs.x.eax = 0x4200 + Origin;
   inregs.x.ebx = Handle;
   inregs.x.ecx = Offset >> 16;
   inregs.x.edx = Offset & 0xffff;
   SPDXint86( DOS_INT, &inregs, &outregs );

   // Check carry flag (on == ERROR)
   if (outregs.x.cflag & 1)
      return -1;

   return (outregs.x.edx<<16) + outregs.x.eax;

   return(-1);
   */
}

/*----------------------------------------------------------------------------*/

INT   _SPDXTell( INT Handle )
{
   FUNC("_SPDXTell");
   return _SPDXSeek( Handle, 0, SEEK_CUR );
}

/*----------------------------------------------------------------------------*/

INT   _SPDXGetc( INT Handle )
{
unsigned char byte;

   FUNC("_SPDXGetc");
   _SPDXRead( Handle, &byte, 1 );
   return byte;
}

/*----------------------------------------------------------------------------*/

void  _SPDXPutc( INT Handle, INT Byte )
{
   FUNC("_SPDXPutc");
   _SPDXWrite( Handle, (CHR *) &Byte, 1 );
}

/*----------------------------------------------------------------------------
  -   [FILEIO.C    ] - End Of File                                           -
  ----------------------------------------------------------------------------*/
