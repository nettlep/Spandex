// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [DECODER.C   ] - File used to decode .GIF files...                     -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#include "spandex.h"

/*----------------------------------------------------------------------------*/

static   INT   InitDecoder(INT size);
static   INT   GetNextCode(INT Handle);

/*----------------------------------------------------------------------------*/

static   INT   BadCodeCount = 0;                // Errors for decoder

static   INT   CurrentCodeSize;                 // The current code size
static   INT   ClearCode;                       // Value for a Clear code
static   INT   EndingCode;                      // Value for a Ending code
static   INT   NewCodes;                        // First available code
static   INT   TopSlot;                         // Highest code for cur size
static   INT   Slot;                            // Last read code

static   INT   BytesLeftInBlock = 0;            // # bytes left in block
static   INT   BitsLeftInCurrentByte = 0;       // # bits left in curr byte
static   UCHR  CurrentByte;                     // Current byte
static   UCHR  CurrentBlock[257];               // Current block
static   UCHR  *PointerToNextByteInBlock;       // Ptr to next byte in block

static   WRD   CodeMask[13] =
{
        0, 0x0001, 0x0003, 0x0007, 0x000F, 0x001F,
   0x003F, 0x007F, 0x00FF, 0x01FF, 0x03FF, 0x07FF, 0x0FFF
};

/*----------------------------------------------------------------------------*/

/* The reason we have these seperated like this instead of using
   a structure like the original Wilhite code did, is because this
   stuff generally produces significantly faster code when compiled...
   This code is full of similar speedups...  (For a good book on writing
   C for speed or for space optomisation, see Efficient C by Tom Plum,
   published by Plum-Hall Associates...) */

static UCHR stack[MAX_CODES + 1];               /* Stack for storing pixels   */
static UCHR suffix[MAX_CODES + 1];              /* Suffix table               */
static INT  prefix[MAX_CODES + 1];              /* Prefix linked list         */

/*----------------------------------------------------------------------------*/

INT   InitDecoder(INT size)
{
   FUNC("InitDecoder");
   BadCodeCount = 0;
   CurrentCodeSize = size + 1;
   TopSlot = 1 << CurrentCodeSize;
   ClearCode = 1 << size;
   EndingCode = ClearCode + 1;
   Slot = NewCodes = EndingCode + 1;
   BytesLeftInBlock = BitsLeftInCurrentByte = 0;

   CurrentByte = 0;
   SPDXMemSetBYTE( CurrentBlock, 0, 257);
   SPDXMemSetBYTE( stack, 0, MAX_CODES + 1);
   SPDXMemSetBYTE( suffix, 0, MAX_CODES + 1);
   SPDXMemSetDWORD( prefix, 0, MAX_CODES + 1);
   PointerToNextByteInBlock = 0;

   return 0;
}

/*----------------------------------------------------------------------------*/

/* gets the next code from the GIF file.  Returns the code, or else a negative
   number in case of file errors... */

INT   GetNextCode( INT Handle )
{
INT   i, x;
INT   RetCode;

   FUNC("GetNextCode");

   if (BitsLeftInCurrentByte == 0)
   {
      if (BytesLeftInBlock <= 0)
      {
         /* Out of bytes in current block, so read next block */
         PointerToNextByteInBlock = CurrentBlock;

         if ((BytesLeftInBlock = (int) ((unsigned char) SPDXGetc(Handle))) < 0)
            return BytesLeftInBlock;
                            
         else if (BytesLeftInBlock)
         {
            for (i = 0; i < BytesLeftInBlock; ++i)
            {
               if ((x = (int) ((unsigned char) SPDXGetc(Handle))) < 0)
                  return x;
               CurrentBlock[i] = (CHR) x;
            }
         }
      }

      CurrentByte = *PointerToNextByteInBlock++;
      BitsLeftInCurrentByte = 8;
      --BytesLeftInBlock;
   }

   RetCode = CurrentByte >> (8 - BitsLeftInCurrentByte);

   while (CurrentCodeSize > BitsLeftInCurrentByte)
   {
      if (BytesLeftInBlock <= 0)
      {
         /* Out of bytes in current block, so read next block */
         PointerToNextByteInBlock = CurrentBlock;

         if ((BytesLeftInBlock = (int) ((unsigned char) SPDXGetc(Handle))) < 0)
            return BytesLeftInBlock;

         else if (BytesLeftInBlock)
         {
            for (i = 0; i < BytesLeftInBlock; ++i)
            {
               if ((x = (int) ((unsigned char) SPDXGetc(Handle))) < 0)
                  return x;
               CurrentBlock[i] = (CHR) x;
            }
         }
      }

      CurrentByte = *PointerToNextByteInBlock++;
      RetCode |= CurrentByte << BitsLeftInCurrentByte;
      BitsLeftInCurrentByte += 8;
      --BytesLeftInBlock;
   }

   BitsLeftInCurrentByte -= CurrentCodeSize;
   RetCode &= CodeMask[CurrentCodeSize];
   return RetCode;
}

/*----------------------------------------------------------------------------*/

/* INT   SPDXGifDecoder(INT Handle, INT LineWidth, CHR *Buffer, INT ImgWidth, ifunptr WriteLine)

   - This function decodes an LZW image, according to the method used
   in the GIF spec.  Every *linewidth* "characters" (ie. pixels) decoded
   will generate a call to StoreLine(), which is a user specific function
   to display a line of pixels.  The function gets it's codes from
   GetNextCode() which is responsible for reading blocks of data and
   seperating them into the proper size codes.  Finally, SPDXGetc(Handle)
   is the global routine to read the next byte from the GIF file.

   It is generally a good idea to have linewidth correspond to the actual
   width of a line (as specified in the Image header) to make your own
   code a bit simpler, but it isn't absolutely necessary.

   Returns: 0 if successful, else negative.  (See ERRS.H) */

/*----------------------------------------------------------------------------*/

INT   SPDXDecompress(INT Handle, INT LineWidth, UCHR *Buffer, INT ImgWidth, ifunptr WriteLine)
{
register UCHR  *sp, *bufptr;
register INT   code, fc, oc, bufcnt;
UCHR           *buf;
INT            c, size, RetCode;
INT            Offset = 0;

   FUNC("SPDXDecompress");
   Assert( Buffer );
   Assert( WriteLine );

   /* Initialize for decoding a new image... */

   if ((size = SPDXGetc(Handle)) < 0)
      return LE_NOREAD;

   if (size < 2 || size > 9)
      return LE_BADCODE;

   InitDecoder(size);

   /* Initialize in case they forgot to put in a ClearCode code.
      (This shouldn't happen, but we'll try and decode it anyway...) */

   oc = fc = 0;

   /* Allocate space for the decode buffer */

   if ((buf = (UCHR *) SPDXMalloc(ImgWidth + 1)) == NULL)
      return LE_NOMEMIMG;

   /* Set up the stack pointer and decode buffer pointer */

   sp = stack;
   bufptr = buf;
   bufcnt = ImgWidth;

   /* This is the main loop.  For each code we get we pass through the
      linked list of prefix codes, pushing the corresponding "character" for
      each code onto the stack.  When the list reaches a single "character"
      we push that on the stack too, and then start unstacking each
      character for output in the correct order.  Special handling is
      included for the ClearCode code, and the whole thing ends when we get
      an EndingCode code. */

   while ((c = GetNextCode(Handle)) != EndingCode)
   {
      /* If we had a file error, return without completing the decode */
      if (c < 0)
         return LE_NOREAD;

      /* If the code is a ClearCode code, reinitialize all necessary items. */

      if (c == ClearCode)
      {
         CurrentCodeSize = size + 1;
         Slot = NewCodes;
         TopSlot = 1 << CurrentCodeSize;

         /* Continue reading codes until we get a non-ClearCode code
          * (Another unlikely, but possible case...) */

         while ((c = GetNextCode(Handle)) == ClearCode);

         /* If we get an EndingCode code immediately after a ClearCode code
          * (Yet another unlikely case), then break out of the loop. */

         if (c == EndingCode)
            break;

         /* Finally, if the code is beyond the range of already set codes,
          * (This one had better NOT happen...  I have no idea what will
          * result from this, but I doubt it will look good...) then set it
          * to color zero. */

         if (c >= Slot)
            c = 0;

         oc = fc = c;

         /* And let us not forget to put the char into the buffer... And
          * if, on the off chance, we were exactly one pixel from the end
          * of the line, we have to send the buffer to the StoreLine()
          * routine... */

         *bufptr++ = (CHR) c;

         if (--bufcnt == 0)
         {
            Offset += WriteLine(buf, LineWidth, Buffer, ImgWidth, Offset/(ImgWidth*3) );

            bufptr = buf;
            bufcnt = ImgWidth;
         }
      }
      else
      {
         /* In this case, it's not a ClearCode code or an EndingCode code, so
          * it must be a code code...  So we can now decode the code into
          * a stack of character codes. (Clear as mud, right?) */

         code = c;

         /* Here we go again with one of those off chances...  If, on the
          * off chance, the code we got is beyond the range of those already
          * set up (Another thing which had better NOT happen...) we trick
          * the decoder into thinking it actually got the last code read.
          * (Hmmn... I'm not sure why this works...  But it does...) */

         if (code >= Slot)
         {
            if (code > Slot)
               ++BadCodeCount;
            code = oc;
            *sp++ = (CHR) fc;
         }

         /* Here we scan back along the linked list of prefixes, pushing
          * helpless characters (ie. suffixes) onto the stack as we do so. */

         while (code >= NewCodes)
         {
            *sp++ = suffix[code];
            code = prefix[code];
         }

         /* Push the last character on the stack, and set up the new
          * prefix and suffix, and if the required Slot number is greater
          * than that allowed by the current bit size, increase the bit
          * size.  (NOTE - If we are all full, we *don't* save the new
          * suffix and prefix...  I'm not certain if this is correct...
          * it might be more proper to overwrite the last code...  */

         *sp++ = (CHR) code;

         if (Slot < TopSlot)
         {
            fc = code;
            suffix[Slot] = (CHR) code;
            prefix[Slot++] = oc;
            oc = c;
         }

         if (Slot >= TopSlot)
            if (CurrentCodeSize < 12)
            {
               TopSlot <<= 1;
               ++CurrentCodeSize;
            }

         /* Now that we've pushed the decoded string (in reverse order)
          * onto the stack, lets pop it off and put it into our decode
          * buffer...  And when the decode buffer is full, write another
          * line...  */

         while (sp > stack)
         {
            *bufptr++ = *(--sp);

            if (--bufcnt == 0)
            {
               Offset += WriteLine(buf, LineWidth, Buffer, ImgWidth, Offset/(ImgWidth*3) );

               bufptr = buf;
               bufcnt = ImgWidth;
            }
         }
      }
   }

   RetCode = 0;

   //if (bufcnt != ImgWidth)
   //   WriteLine(buf, LineWidth, &Buffer[Offset], ImgWidth, Offset/(ImgWidth*3) );

   SPDXFree(buf);

   return LE_NONE;
}

/*----------------------------------------------------------------------------
  -   [DECODER.C   ] - End Of File                                           -
  ----------------------------------------------------------------------------*/     
