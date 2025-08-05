// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [MEM.C       ] - Memory allocation functions                           ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#include <mem.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <malloc.h>

#include "spandex.h"

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static   MEMLIST  *MemArray = 0;
static   INT   InitFlag = FALSE;
static   INT   MaxMemAtOneTime = 0;
static   INT   TotalMemInUse = 0;
static   INT   TotalElements = 0;

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static   INT   ValidateChunk(MEMLIST *Chunk);

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXInitMem()
{
   FUNC("SPDXInitMem");
   if (InitFlag == TRUE)
      SPDXUninitMem();

   InitFlag = TRUE;

   MemArray = (MEMLIST *) malloc( sizeof( MEMLIST ) );

   if (!MemArray)
      SPDXFatalError( LE_NOMEM, "Unable to allocate initial memory link" );

   MemArray->Address = 0;
   MemArray->Next = 0;
   TotalElements = 0;

   return;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXUninitMem()
{
CHR      Str[250];
MEMLIST  *Cur = MemArray, *Next;

   FUNC("SPDXUninitMem");
   Assert(InitFlag == TRUE);

   if (TotalMemInUse || TotalElements)
   {
      if (SPDXGetDebugState() & DEBUG_BASIC)
      {
         sprintf( Str, "%d memory leak(s) detected using %d bytes", TotalElements, TotalMemInUse );
         SPDXLogError( LE_MEMLEAK, Str );
      }

      while( Cur )
      {
         if (SPDXGetDebugState() & DEBUG_MEM)
         {
            if (SPDXGetDebugState() & DEBUG_FUNCS)
               sprintf( Str, "Memory leak At:0x%08X Size:%d Owner:%s", Cur->Address, Cur->Size, Cur->Owner );
            else
               sprintf( Str, "Memory leak At:0x%08X Size:%d", Cur->Address, Cur->Size );

            SPDXLogError( LE_MEMLEAK, Str );
         }
         
         Next = Cur->Next;
         free( Cur->Address );
         free( Cur );
         Cur = Next;
      }
   }

   MemArray = 0;
   InitFlag = FALSE;
   TotalMemInUse = 0;
   TotalElements = 0;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  *_SPDXMalloc( INT Size, CHR *OwnerFile, INT OwnerLine, CHR *OwnerFunc )
{
CHR   Owner[DEF_NAME_LEN+DEF_FUNC_LEN], TempStr[DEF_NAME_LEN+DEF_FUNC_LEN+80];
void  *Ptr;

   FUNC("_SPDXMalloc");
   Assert(InitFlag == TRUE);
   Assert( OwnerFile );
   Assert( OwnerFunc );

   Size += WRAP_SIZE;

   Ptr = malloc( Size );

   if (SPDXGetDebugState() & DEBUG_MEMTRACE )
   {
      if (SPDXGetDebugState() & DEBUG_FUNCS)
         sprintf( TempStr, "%s [%s|%d] called SPDXMalloc for %d bytes (at 0x%08X)", OwnerFunc, OwnerFile, OwnerLine, Size-WRAP_SIZE, Ptr );
      else
         sprintf( TempStr, "SPDXMalloc for %d bytes (at 0x%08X)", Size-WRAP_SIZE, Ptr );

      SPDXLogString( TempStr );
   }

   if (!Ptr)
   {
      if (SPDXGetDebugState() & DEBUG_FUNCS)
         sprintf( TempStr, "%s [%s|%d] failed to allocate RAM for %d bytes", OwnerFunc, OwnerFile, OwnerLine, Size-WRAP_SIZE );
      else
         sprintf( TempStr, "Failed to allocate RAM for %d bytes", Size-WRAP_SIZE );

      SPDXLogError( LE_NOMEM, TempStr );
      return 0;
   }

   SPDXMemSetBYTE( Ptr, 0, Size);

   sprintf( Owner, "%s [%s|%d]", OwnerFunc, OwnerFile, OwnerLine );

   SPDXMemAdd( (CHR *) Ptr + HDR_SIZE, Size-WRAP_SIZE, Owner );

   TotalMemInUse += Size;

   if (TotalMemInUse > MaxMemAtOneTime)
      MaxMemAtOneTime = TotalMemInUse;

   SPDXMemSetBYTE( Ptr, HDR_BYTES, HDR_SIZE );
   SPDXMemSetBYTE( &((CHR *) Ptr)[Size-HDR_SIZE], HDR_BYTES, HDR_SIZE );

   return (CHR *) Ptr + HDR_SIZE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  *_SPDXRealloc( void *Address, INT Size, CHR *OwnerFile, INT OwnerLine, CHR *OwnerFunc )
{
CHR      TempStr[DEF_NAME_LEN+DEF_FUNC_LEN+180];
void     *Ptr;
MEMLIST  *Cur = MemArray;

   FUNC("_SPDXRealloc");
   Assert( OwnerFile );
   Assert( OwnerFunc );

   if (!Address)
      return _SPDXMalloc( Size, OwnerFile, OwnerLine, OwnerFunc );

   Size += WRAP_SIZE;

   while( Cur->Address != Address )
   {
      if (!Cur->Next)
      {
         sprintf( TempStr, "Unable to find realloc address in mem list [%s(%d) -- %s]", OwnerFile, OwnerLine, OwnerFunc );
         SPDXFatalError( LE_RANGE, TempStr );
      }

      Cur = Cur->Next;
   }

   Ptr = realloc( (CHR *) Address - HDR_SIZE, Size);

   if (SPDXGetDebugState() & DEBUG_MEMTRACE )
   {
      if (SPDXGetDebugState() & DEBUG_FUNCS)
         sprintf( TempStr, "%s [%s|%d] called SPDXRealloc for %d bytes (at 0x%08X)", OwnerFunc, OwnerFile, OwnerLine, Size-WRAP_SIZE, Address );
      else
         sprintf( TempStr, "SPDXRealloc for %d bytes (at 0x%08X)", Size-WRAP_SIZE, Address );

      SPDXLogString( TempStr );
   }

   if (!Ptr)
   {
      if (Size-WRAP_SIZE)
      {
         if (SPDXGetDebugState() & DEBUG_FUNCS)
            sprintf( TempStr, "%s [%s|%d] failed to reallocate RAM for %d bytes", OwnerFunc, OwnerFile, OwnerLine, Size-WRAP_SIZE );
         else
            sprintf( TempStr, "Failed to reallocate RAM for %d bytes", Size-WRAP_SIZE );

         SPDXLogError( LE_NOMEM, TempStr );
      }
      return 0;
   }

   TotalMemInUse = TotalMemInUse - (Cur->Size+WRAP_SIZE) + Size;

   if (TotalMemInUse > MaxMemAtOneTime)
      MaxMemAtOneTime = TotalMemInUse;

   sprintf( Cur->Owner, "%s [%s|%d]", OwnerFunc, OwnerFile, OwnerLine );
   Cur->Address = (CHR *) Ptr + HDR_SIZE;
   Cur->Size = Size-WRAP_SIZE;

   SPDXMemSetBYTE( Ptr, HDR_BYTES, HDR_SIZE );
   SPDXMemSetBYTE( &((CHR *)Ptr)[Size-HDR_SIZE], HDR_BYTES, HDR_SIZE );

   return (CHR *) Ptr + HDR_SIZE;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  _SPDXFree( void *Address, CHR *OwnerFile, INT OwnerLine, CHR *OwnerFunc )
{
CHR      TempStr[DEF_NAME_LEN+DEF_FUNC_LEN+80];
MEMLIST  *Cur = MemArray, *Prev = 0;

   FUNC("_SPDXFree");
   Assert(InitFlag == TRUE);
   Assert( Address );
   Assert( OwnerFile );
   Assert( OwnerFunc );

   if (SPDXGetDebugState() & DEBUG_MEMTRACE )
   {
      if (SPDXGetDebugState() & DEBUG_FUNCS)
         sprintf( TempStr, "%s [%s|%d] called SPDXFree of memory at 0x%08X", OwnerFunc, OwnerFile, OwnerLine, Address );
      else
         sprintf( TempStr, "SPDXFree of memory at 0x%08X", Address );

      SPDXLogString( TempStr );
   }

   while( Cur->Address != Address )
   {
      if (Cur->Next)
      {
         Prev = Cur;
         Cur = Cur->Next;
      }
      else
      {   
         if (SPDXGetDebugState() & DEBUG_FUNCS)
            sprintf( TempStr, "%s [%s|%d] Unable to find free address in mem list at 0x%08X", OwnerFunc, OwnerFile, OwnerLine, Address );
         else
            sprintf( TempStr, "Unable to find free address in mem list at 0x%08X", Address );

         SPDXLogError( LE_RANGE, TempStr );
         return;
      }
   }

   ValidateChunk( Cur );

   free( (CHR *) Address - HDR_SIZE);

   TotalMemInUse -= Cur->Size+WRAP_SIZE;
   TotalElements--;

   // First element in list?
   if (!Prev)
      MemArray = Cur->Next;
   else
      Prev->Next = Cur->Next;

   free(Cur);

   return;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   ValidateChunk(MEMLIST *Chunk)
{
INT      i;
CHR      TempStr[DEF_NAME_LEN+DEF_FUNC_LEN+80], *Ptr;
MEMLIST  *Cur;

   FUNC("ValidateChunk");
   Assert( Chunk );

   Cur = Chunk;

   Ptr = (CHR *) Cur->Address - HDR_SIZE;

   for( i = 0; i < HDR_SIZE; i++ )
   {
      if (Ptr[i] != HDR_BYTES)
      {
         if (SPDXGetDebugState() & DEBUG_FUNCS)
            sprintf( TempStr, "Memory Overrun (< 0) at address 0x%08X, Owner is %s", Ptr, Cur->Owner );
         else
            sprintf( TempStr, "Memory Overrun (< 0) at address 0x%08X", Ptr );

         SPDXLogError( LE_NOMEM, TempStr );
         return 1;
      }

      if (Ptr[i+Cur->Size+HDR_SIZE] != HDR_BYTES)
      {
         if (SPDXGetDebugState() & DEBUG_FUNCS)
            sprintf( TempStr, "Memory Overrun (> Size) at address 0x%08X, Owner is %s", Ptr, Cur->Owner );
         else
            sprintf( TempStr, "Memory Overrun (> Size) at address 0x%08X", Ptr );

         SPDXLogError( LE_NOMEM, TempStr );
         return 1;
      }
   }

   return 0;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

INT   SPDXValidateMemory( )
{
INT      TotalErrors = 0;
MEMLIST  *Cur = MemArray;

   FUNC("SPDXValidateMemory");
   Assert(InitFlag == TRUE);

   FOREVER
   {
      if (Cur->Address)
      {
         TotalErrors += ValidateChunk( Cur );

         if (Cur->Next)
            Cur = Cur->Next;
         else
            break;
      }
      else
      {
         break;
      }
   }

   return TotalErrors;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXMemAdd( void *Address, INT Size, CHR *Owner )
{
MEMLIST  *Cur = MemArray;

   FUNC("SPDXMemAdd");
   Assert(InitFlag == TRUE);
   Assert( Address );
   Assert( Owner );

   // Not top of list?
   if (Cur->Address)
   {
      while( Cur->Next )
         Cur = Cur->Next;

      Cur->Next = (MEMLIST *) malloc( sizeof( MEMLIST ) );

      if (!Cur->Next)
         SPDXFatalError( LE_NOMEM, "Unable to add member to mem list" );

      Cur = Cur->Next;
   }

   Cur->Address = Address;
   Cur->Next = 0;
   Cur->Size = Size;
   strcpy( Cur->Owner, Owner );
   TotalElements++;

   return;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXDumpMemLog()
{
INT      Count = 0, TotalMemUsed = 0;
FILE     *FP;
MEMLIST  *Cur = MemArray;

   FUNC("SPDXDumpMemLog");
   FP = fopen( MEM_FILE, "w" );

   if (!FP)
      SPDXFatalError( LE_NOCREATE, "Unable to create memory log file" );

   fprintf( FP, "***********************************************************\n" );
   fprintf( FP, "*                                                         *\n" );
   fprintf( FP, "* This file was created by the SPANDEX graphics engine.   *\n" );
   fprintf( FP, "*                                                         *\n" );
   fprintf( FP, "* Pertinent information regarding the state of the memory *\n" );
   fprintf( FP, "* allocation units will follow...                         *\n" );
   fprintf( FP, "*                                                         *\n" );
   fprintf( FP, "***********************************************************\n\n" );

   if (Cur->Address)
   {
      while( Cur->Address )
      {
         fprintf( FP, "----------------------------------\n" );
         fprintf( FP, "   Address:  0x%08X\n", Cur->Address );
         fprintf( FP, "    Length:  0x%X (%d)\n", Cur->Size, Cur->Size );

         if (SPDXGetDebugState() & DEBUG_FUNCS)
            fprintf( FP, "     Owner:  %s\n", Cur->Owner );

         Count++;
         TotalMemUsed += Cur->Size + WRAP_SIZE;

         if (Cur->Next)
            Cur = Cur->Next;
         else
            break;
      }
   }

   fprintf( FP, "--------------------------------------------------------\n" );
   fprintf( FP, "Total memory in use right now:  0x%X (%d)\n", TotalMemUsed, TotalMemUsed );
   fprintf( FP, "        Total memory elements:  0x%X (%d)\n", Count, Count );

   if (Count != TotalElements)
      fprintf( FP, "*** COUNTS ARE OUT OF SYNC! ***\n (%d & %d)", Count, TotalElements );

   if (TotalMemUsed != TotalMemInUse)
      fprintf( FP, "*** USES ARE OUT OF SYNC! (%d & %d)***\n", TotalMemUsed, TotalMemInUse );

   fprintf( FP, "Most memory used at any point:  0x%X (%d)\n", MaxMemAtOneTime, MaxMemAtOneTime );

   fclose( FP );
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXGetMemReport( MEMREP *MemReport )
{
   FUNC("SPDXGetMemReport");
   Assert( MemReport );

   MemReport->InitFlag        = InitFlag;
   MemReport->MaxMemAtOneTime = MaxMemAtOneTime;
   MemReport->TotalMemInUse   = TotalMemInUse;
   MemReport->TotalElements   = TotalElements;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

CHR   *SPDXFindMemOwner( void *Address )
{
MEMLIST  *Cur = MemArray;

   FUNC("_SPDXFindMemOwner");
   Assert(InitFlag == TRUE);
   Assert( Address );

   while( Cur )
   {
      if (Cur->Address == Address )
         return Cur->Owner;

      Cur = Cur->Next;
   }

   return NULL;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [MEM.C       ] - End Of File                                           ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
