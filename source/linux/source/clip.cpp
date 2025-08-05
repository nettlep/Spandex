// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [CLIP.C      ] - 3D and 2D clipping routines                           -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#include "spandex.h"

/*----------------------------------------------------------------------------*/

static   TRI   *AllocReservoirTri();
static   void  ResetTriReservoir();
         VER   *AllocReservoirVertex();
static   void  ResetVertexReservoir();

/*----------------------------------------------------------------------------*/

static   TRI   TriReservoir[TRI_RESERVOIR_SIZE];
static   INT   TriReservoirAllocated = 0;
static   VER   VertexReservoir[VERTEX_RESERVOIR_SIZE];
static   INT   VertexReservoirAllocated = 0;

/*----------------------------------------------------------------------------*/

#define  NewOne(NewV1, NewV2, V1, V2)                             \
{                                                                 \
         NewV1 = AllocReservoirVertex();  Assert(NewV1);          \
         NewV2 = AllocReservoirVertex();  Assert(NewV2);          \
         RList->Vers[RList->VertexCount++] = NewV1;               \
         RList->Vers[RList->VertexCount++] = NewV2;               \
         *NewV1 = *V1;                                            \
         *NewV2 = *V2;                                            \
}

#define  NewTwo(NewTri, NewV1, NewV2, V1, V2)                     \
{                                                                 \
         NewTri = AllocReservoirTri();    Assert(NewTri);         \
         NewV1  = AllocReservoirVertex(); Assert(NewV1);          \
         NewV2  = AllocReservoirVertex(); Assert(NewV2);          \
         RList->Tris[RList->TriCount++]    = NewTri;              \
         RList->Vers[RList->VertexCount++] = NewV1;               \
         RList->Vers[RList->VertexCount++] = NewV2;               \
         *NewTri = *Tri;                                          \
         *NewV1  = *V1;                                           \
         *NewV2  = *V2;                                           \
}

#define  ClipSetup()                                              \
{                                                                 \
         Material = Tri->Material->Surface;                       \
         Shade    = Tri->Object->ShadeModel;                      \
         NewTri1 = AllocReservoirTri();  Assert(NewTri1);         \
         *NewTri1 = *Tri;                                         \
         V1 = Tri->V1;   V2 = Tri->V2;   V3 = Tri->V3;            \
         M1 = &Tri->M1;  M2 = &Tri->M2;  M3 = &Tri->M3;           \
         E1 = &V1->Env;  E2 = &V2->Env;  E3 = &V3->Env;           \
         TriList[i] = NewTri1;                                    \
}

#define  Remove()                                                 \
{                                                                 \
         TriList[i] = TriList[--RList->TriCount];                 \
         continue;                                                \
}

/*----------------------------------------------------------------------------*/

#ifdef U_P
#define  ClipUVOne()                                              \
{                                                                 \
         if (Material & TEXTURED || Shade & BUMP)                 \
         {                                                        \
            NewTri1->M1    = *M1;                                 \
            NewTri1->M2.x  = (M2->x * V2->OneOverZ + Delta12 *    \
                             (M1->x * V1->OneOverZ -              \
                              M2->x * V2->OneOverZ )) /           \
                              NewV12->OneOverZ;                   \
            NewTri1->M2.y  = (M2->y * V2->OneOverZ + Delta12 *    \
                             (M1->y * V1->OneOverZ -              \
                              M2->y * V2->OneOverZ )) /           \
                              NewV12->OneOverZ;                   \
            NewTri1->M2.fx = FLT_TO_FXD(NewTri1->M2.x);           \
            NewTri1->M2.fy = FLT_TO_FXD(NewTri1->M2.y);           \
            NewTri1->M3.x  = (M3->x * V3->OneOverZ + Delta13 *    \
                             (M1->x * V1->OneOverZ -              \
                              M3->x * V3->OneOverZ )) /           \
                              NewV13->OneOverZ;                   \
            NewTri1->M3.y  = (M3->y * V3->OneOverZ + Delta13 *    \
                             (M1->y * V1->OneOverZ -              \
                              M3->y * V3->OneOverZ )) /           \
                              NewV13->OneOverZ;                   \
            NewTri1->M3.fx = FLT_TO_FXD(NewTri1->M3.x);           \
            NewTri1->M3.fy = FLT_TO_FXD(NewTri1->M3.y);           \
         }                                                        \
}

#define  ClipUVTwo()                                              \
{                                                                 \
         if (Material & TEXTURED || Shade & BUMP)                 \
         {                                                        \
            NewTri1->M1    = *M1;                                 \
            NewTri1->M2    = *M2;                                 \
            NewTri1->M3.x  = (M3->x * V3->OneOverZ + Delta23 *    \
                             (M2->x * V2->OneOverZ -              \
                              M3->x * V3->OneOverZ )) /           \
                              NewV23->OneOverZ;                   \
            NewTri1->M3.y  = (M3->y * V3->OneOverZ + Delta23 *    \
                             (M2->y * V2->OneOverZ -              \
                              M3->y * V3->OneOverZ )) /           \
                              NewV23->OneOverZ;                   \
            NewTri1->M3.fx = FLT_TO_FXD(NewTri1->M3.x);           \
            NewTri1->M3.fy = FLT_TO_FXD(NewTri1->M3.y);           \
            NewTri2->M1    = *M1;                                 \
            NewTri2->M2.x  = (M3->x * V3->OneOverZ + Delta13 *    \
                             (M1->x * V1->OneOverZ -              \
                              M3->x * V3->OneOverZ )) /           \
                              NewV13->OneOverZ;                   \
            NewTri2->M2.y  = (M3->y * V3->OneOverZ + Delta13 *    \
                             (M1->y * V1->OneOverZ -              \
                              M3->y * V3->OneOverZ )) /           \
                              NewV13->OneOverZ;                   \
            NewTri2->M2.fx = FLT_TO_FXD(NewTri2->M2.x);           \
            NewTri2->M2.fy = FLT_TO_FXD(NewTri2->M2.y);           \
            NewTri2->M3    = NewTri1->M3;                         \
         }                                                        \
}
#else
#define  ClipUVOne()                                              \
{                                                                 \
         if (Material & TEXTURED || Shade & BUMP)                 \
         {                                                        \
            NewTri1->M1    = *M1;                                 \
            NewTri1->M2.x  = M2->x + Delta12 * (M1->x - M2->x);   \
            NewTri1->M2.y  = M2->y + Delta12 * (M1->y - M2->y);   \
            NewTri1->M2.fx = FLT_TO_FXD(NewTri1->M2.x);           \
            NewTri1->M2.fy = FLT_TO_FXD(NewTri1->M2.y);           \
            NewTri1->M3.x  = M3->x + Delta13 * (M1->x - M3->x);   \
            NewTri1->M3.y  = M3->y + Delta13 * (M1->y - M3->y);   \
            NewTri1->M3.fx = FLT_TO_FXD(NewTri1->M3.x);           \
            NewTri1->M3.fy = FLT_TO_FXD(NewTri1->M3.y);           \
         }                                                        \
}

#define  ClipUVTwo()                                              \
{                                                                 \
         if (Material & TEXTURED || Shade & BUMP)                 \
         {                                                        \
            NewTri1->M1    = *M1;                                 \
            NewTri1->M2    = *M2;                                 \
            NewTri1->M3.x  = M3->x + Delta23 * (M2->x - M3->x);   \
            NewTri1->M3.y  = M3->y + Delta23 * (M2->y - M3->y);   \
            NewTri1->M3.fx = FLT_TO_FXD(NewTri1->M3.x);           \
            NewTri1->M3.fy = FLT_TO_FXD(NewTri1->M3.y);           \
            NewTri2->M1    = *M1;                                 \
            NewTri2->M2.x  = M3->x + Delta13 * (M1->x - M3->x);   \
            NewTri2->M2.y  = M3->y + Delta13 * (M1->y - M3->y);   \
            NewTri2->M2.fx = FLT_TO_FXD(NewTri2->M2.x);           \
            NewTri2->M2.fy = FLT_TO_FXD(NewTri2->M2.y);           \
            NewTri2->M3    = NewTri1->M3;                         \
         }                                                        \
}
#endif
/*----------------------------------------------------------------------------*/

#define  ClipEnvOne()                                             \
{                                                                 \
         if (Material & ENVMAP || Shade & (PHONG|BUMP))           \
         {                                                        \
            NewV12->Env.x += Delta12 * (V1->Env.x - V2->Env.x);   \
            NewV12->Env.y += Delta12 * (V1->Env.y - V2->Env.y);   \
            NewV13->Env.x += Delta13 * (V1->Env.x - V3->Env.x);   \
            NewV13->Env.y += Delta13 * (V1->Env.y - V3->Env.y);   \
            NewV12->Env.fx = FLT_TO_FXD(NewV12->Env.x);           \
            NewV12->Env.fy = FLT_TO_FXD(NewV12->Env.y);           \
            NewV13->Env.fx = FLT_TO_FXD(NewV13->Env.x);           \
            NewV13->Env.fy = FLT_TO_FXD(NewV13->Env.y);           \
         }                                                        \
}

#define  ClipEnvTwo()                                             \
{                                                                 \
         if (Material & ENVMAP || Shade & (PHONG|BUMP))           \
         {                                                        \
            NewV13->Env.x += Delta13 * (V1->Env.x - V3->Env.x);   \
            NewV13->Env.y += Delta13 * (V1->Env.y - V3->Env.y);   \
            NewV23->Env.x += Delta23 * (V2->Env.x - V3->Env.x);   \
            NewV23->Env.y += Delta23 * (V2->Env.y - V3->Env.y);   \
            NewV13->Env.fx = FLT_TO_FXD(NewV13->Env.x);           \
            NewV13->Env.fy = FLT_TO_FXD(NewV13->Env.y);           \
            NewV23->Env.fx = FLT_TO_FXD(NewV23->Env.x);           \
            NewV23->Env.fy = FLT_TO_FXD(NewV23->Env.y);           \
         }                                                        \
}

/*----------------------------------------------------------------------------*/

#define  ClipGOne(NewV12, NewV13)                                 \
{                                                                 \
         if (Shade & GOURAUD)                                     \
         {                                                        \
            NewV12->Int += Delta12 * (V1->Int - V2->Int);         \
            NewV13->Int += Delta13 * (V1->Int - V3->Int);         \
         }                                                        \
}

#define  ClipGTwo(NewV13, NewV23)                                 \
{                                                                 \
         if (Shade & GOURAUD)                                     \
         {                                                        \
            NewV13->Int += Delta13 * (V1->Int - V3->Int);         \
            NewV23->Int += Delta23 * (V2->Int - V3->Int);         \
         }                                                        \
}

/*----------------------------------------------------------------------------*/

#ifdef U_P
#define  ClipZOne()                                               \
{                                                                 \
         NewV12->OneOverZ += Delta12*(V1->OneOverZ-V2->OneOverZ); \
         NewV13->OneOverZ += Delta13*(V1->OneOverZ-V3->OneOverZ); \
}

#define  ClipZTwo()                                               \
{                                                                 \
         NewV13->OneOverZ += Delta13*(V1->OneOverZ-V3->OneOverZ); \
         NewV23->OneOverZ += Delta23*(V2->OneOverZ-V3->OneOverZ); \
}
#else
#define  ClipZOne() {}
#define  ClipZTwo() {}
#endif

/*----------------------------------------------------------------------------*/

void  SPDXResetClipper()
{
   ResetTriReservoir();
   ResetVertexReservoir();
}

/*----------------------------------------------------------------------------*/

void  SPDXSplitPolygonZ(TRI *Tri, TRI **Split1, TRI **Split2, VER *NewV1, VER *NewV2, VSCR *VScreen )
{
INT   Material, Shade;
FLT   Relz, NearZ;
FLT   Delta12, Delta13, Delta23;
TRI   *NewTri1, *NewTri2;
P2D   *M1, *M2, *M3, *tempMX;
P2D   *E1, *E2, *E3;
VER   *V1, *V2, *V3, *tVer;

   FUNC("SPDXSplitPolygonZ");
   Assert( Tri );
   Assert( Split1 );
   Assert( Split2 );
   Assert( NewV1 );
   Assert( NewV2 );
   Assert( VScreen );

   *Split2 = NULL;
   V1 =  Tri->V1;   V2 =  Tri->V2;   V3 =  Tri->V3;
   M1 = &Tri->M1;   M2 = &Tri->M2;   M3 = &Tri->M3;
   E1 = &V1->Env;   E2 = &V2->Env;   E3 = &V3->Env;

   // Re-order 'em (from farthest to closest)
   if (V1->CLoc.z < V2->CLoc.z){SWAP(V1,V2,tVer);SWAP(M1,M2,tempMX);SWAP(E1,E2,tempMX)};
   if (V2->CLoc.z < V3->CLoc.z){SWAP(V2,V3,tVer);SWAP(M2,M3,tempMX);SWAP(E2,E3,tempMX)};
   if (V1->CLoc.z < V2->CLoc.z){SWAP(V1,V2,tVer);SWAP(M1,M2,tempMX);SWAP(E1,E2,tempMX)};

   NearZ = SPDXGlobals.NearZ;

   // Every split will require at least one new tri
   NewTri1 = AllocReservoirTri();   Assert(NewTri1);
   *NewTri1 = *Tri;
   Material = Tri->Material->Surface;
   Shade    = Tri->Object->ShadeModel;

   // This will yield only one new tri
   if (V2->Visible == VIS_CLIPZ)
   {
      *NewV1 = *V2;
      *NewV2 = *V3;
      NewV1->Visible = VIS_YES;
      NewV2->Visible = VIS_YES;

      Delta12 = (NearZ - V2->CLoc.z) / (V1->CLoc.z - V2->CLoc.z);
      Delta13 = (NearZ - V3->CLoc.z) / (V1->CLoc.z - V3->CLoc.z);

      NewV1->CLoc.x += Delta12 * (V1->CLoc.x - V2->CLoc.x);
      NewV1->CLoc.y += Delta12 * (V1->CLoc.y - V2->CLoc.y);
      NewV1->CLoc.z  = NearZ;
      NewV1->FixedZ  = FLT_TO_FXD(NearZ);
      SPDXCalcProjection( Relz, NewV1, VScreen );

      NewV2->CLoc.x += Delta13 * (V1->CLoc.x - V3->CLoc.x);
      NewV2->CLoc.y += Delta13 * (V1->CLoc.y - V3->CLoc.y);
      NewV2->CLoc.z  = NearZ;
      NewV2->FixedZ  = FLT_TO_FXD(NearZ);
      SPDXCalcProjection( Relz, NewV2, VScreen );

      if (Material & (TEXTURED | BUMP))
      {
         NewTri1->M1    = *M1;
         NewTri1->M2.x  = M2->x + Delta12 * (M1->x - M2->x);
         NewTri1->M2.y  = M2->y + Delta12 * (M1->y - M2->y);
         NewTri1->M2.fx = FLT_TO_FXD(NewTri1->M2.x);
         NewTri1->M2.fy = FLT_TO_FXD(NewTri1->M2.y);
         NewTri1->M3.x  = M3->x + Delta13 * (M1->x - M3->x);
         NewTri1->M3.y  = M3->y + Delta13 * (M1->y - M3->y);
         NewTri1->M3.fx = FLT_TO_FXD(NewTri1->M3.x);
         NewTri1->M3.fy = FLT_TO_FXD(NewTri1->M3.y);
      }

      if (Material & (ENVMAP | PHONG))
      {
         NewV1->Env.x += Delta12 * (V1->Env.x - V2->Env.x);
         NewV1->Env.y += Delta12 * (V1->Env.y - V2->Env.y);
         NewV2->Env.x += Delta13 * (V1->Env.x - V3->Env.x);
         NewV2->Env.y += Delta13 * (V1->Env.y - V3->Env.y);
         NewV1->Env.fx = FLT_TO_FXD(NewV1->Env.x);
         NewV1->Env.fy = FLT_TO_FXD(NewV1->Env.y);
         NewV2->Env.fx = FLT_TO_FXD(NewV2->Env.x);
         NewV2->Env.fy = FLT_TO_FXD(NewV2->Env.y);
      }

      if (Shade & GOURAUD)
      {
         NewV1->CN.dx += Delta12 * (V1->CN.dx - V2->CN.dx);
         NewV1->CN.dy += Delta12 * (V1->CN.dy - V2->CN.dy);
         NewV1->CN.dz += Delta12 * (V1->CN.dz - V2->CN.dz);
         NewV2->CN.dx += Delta13 * (V1->CN.dx - V3->CN.dx);
         NewV2->CN.dy += Delta13 * (V1->CN.dy - V3->CN.dy);
         NewV2->CN.dz += Delta13 * (V1->CN.dz - V3->CN.dz);
      }

      NewTri1->V1 = V1;
      NewTri1->V2 = NewV1;
      NewTri1->V3 = NewV2;
      *Split1 = NewTri1;
   }
   // This will yield two new tris
   else
   {
      NewTri2 = AllocReservoirTri();  Assert(NewTri2);
      *NewTri2 = *Tri;
      *NewV1 = *V3;
      *NewV2 = *V3;
      NewV1->Visible = VIS_YES;
      NewV2->Visible = VIS_YES;

      Delta13 = (NearZ - V3->CLoc.z) / (V1->CLoc.z - V3->CLoc.z);
      Delta23 = (NearZ - V3->CLoc.z) / (V2->CLoc.z - V3->CLoc.z);

      NewV1->CLoc.x += Delta13 * (V1->CLoc.x - V3->CLoc.x);
      NewV1->CLoc.y += Delta13 * (V1->CLoc.y - V3->CLoc.y);
      NewV1->CLoc.z  = NearZ;
      NewV1->FixedZ  = FLT_TO_FXD(NearZ);
      SPDXCalcProjection( Relz, NewV1, VScreen );

      NewV2->CLoc.x += Delta23 * (V2->CLoc.x - V3->CLoc.x);
      NewV2->CLoc.y += Delta23 * (V2->CLoc.y - V3->CLoc.y);
      NewV2->CLoc.z  = NearZ;
      NewV2->FixedZ  = FLT_TO_FXD(NearZ);
      SPDXCalcProjection( Relz, NewV2, VScreen );

      if (Material & (TEXTURED | BUMP))
      {
         NewTri1->M1    = *M1;
         NewTri1->M2    = *M2;
         NewTri1->M3.x  = M3->x + Delta23 * (M2->x - M3->x);
         NewTri1->M3.y  = M3->y + Delta23 * (M2->y - M3->y);
         NewTri1->M3.fx = FLT_TO_FXD(NewTri1->M3.x);
         NewTri1->M3.fy = FLT_TO_FXD(NewTri1->M3.y);
         NewTri2->M1    = *M1;
         NewTri2->M2.x  = M3->x + Delta13 * (M1->x - M3->x);
         NewTri2->M2.y  = M3->y + Delta13 * (M1->y - M3->y);
         NewTri2->M2.fx = FLT_TO_FXD(NewTri2->M2.x);
         NewTri2->M2.fy = FLT_TO_FXD(NewTri2->M2.y);
         NewTri2->M3    = NewTri1->M3;
      }

      if (Material & (ENVMAP | PHONG))
      {
         NewV1->Env.x += Delta13 * (V1->Env.x - V3->Env.x);
         NewV1->Env.y += Delta13 * (V1->Env.y - V3->Env.y);
         NewV2->Env.x += Delta23 * (V2->Env.x - V3->Env.x);
         NewV2->Env.y += Delta23 * (V2->Env.y - V3->Env.y);
         NewV1->Env.fx = FLT_TO_FXD(NewV1->Env.x);
         NewV1->Env.fy = FLT_TO_FXD(NewV1->Env.y);
         NewV2->Env.fx = FLT_TO_FXD(NewV2->Env.x);
         NewV2->Env.fy = FLT_TO_FXD(NewV2->Env.y);
      }

      if (Shade & GOURAUD)
      {
         NewV1->CN.dx += Delta13 * (V1->CN.dx - V3->CN.dx);
         NewV1->CN.dy += Delta13 * (V1->CN.dy - V3->CN.dy);
         NewV1->CN.dz += Delta13 * (V1->CN.dz - V3->CN.dz);
         NewV2->CN.dx += Delta23 * (V2->CN.dx - V3->CN.dx);
         NewV2->CN.dy += Delta23 * (V2->CN.dy - V3->CN.dy);
         NewV2->CN.dz += Delta23 * (V2->CN.dz - V3->CN.dz);
      }

      NewTri1->V1 = V1;
      NewTri1->V2 = V2;
      NewTri1->V3 = NewV2;
      *Split1 = NewTri1;

      NewTri2->V1 = V1;
      NewTri2->V2 = NewV1;
      NewTri2->V3 = NewV2;
      *Split2 = NewTri2;
   }

   return;
}

/*----------------------------------------------------------------------------*/

void  SPDXClipRenderList(RLS *RList, VSCR *VScreen )
{
INT   i, Material, Shade;
FLT   MinX, MaxX, MinY, MaxY, Delta12, Delta13, Delta23;
FXD   fxMinX, fxMaxX, fxMinY, fxMaxY;
P2D   *S1, *S2, *S3, *M1, *M2, *M3, *E1, *E2, *E3, *tP;
VER   *V1, *V2, *V3, *NewV12, *NewV13, *NewV23, *tV;
TRI   **TriList, *Tri, *NewTri1, *NewTri2;

   FUNC("SPDXClipRenderList");
   Assert( RList );
   Assert( VScreen );

   MinX   = VScreen->ClipMinX;
   MaxX   = VScreen->ClipMaxX;
   MinY   = VScreen->ClipMinY;
   MaxY   = VScreen->ClipMaxY;
   fxMinX = FLT_TO_FXD(MinX);
   fxMaxX = FLT_TO_FXD(MaxX);
   fxMinY = FLT_TO_FXD(MinY);
   fxMaxY = FLT_TO_FXD(MaxY);

   TriList = RList->Tris;

   for( i = 0; i < RList->TriCount; )
   {
      Tri = TriList[i];
      S1 = &Tri->V1->Scr;
      S2 = &Tri->V2->Scr;
      S3 = &Tri->V3->Scr;

      // Clip left edge?
      if (S1->fx < fxMinX || S2->fx < fxMinX || S3->fx < fxMinX)
      {
         ClipSetup();

         // Re-order 'em (from right to left)
         if (S1->x < S2->x){SWAP(S1,S2,tP);SWAP(M1,M2,tP);SWAP(E1,E2,tP);SWAP(V1,V2,tV)};
         if (S2->x < S3->x){SWAP(S2,S3,tP);SWAP(M2,M3,tP);SWAP(E2,E3,tP);SWAP(V2,V3,tV)};
         if (S1->x < S2->x){SWAP(S1,S2,tP);SWAP(M1,M2,tP);SWAP(E1,E2,tP);SWAP(V1,V2,tV)};

         if (S1->x < MinX) Remove();

         // This will yield only one tri
         if (S2->x < MinX)
         {
            NewOne(NewV12, NewV13, V2, V3);

            Delta12 = (MinX - S2->x) / (S1->x - S2->x);
            Delta13 = (MinX - S3->x) / (S1->x - S3->x);

            NewV12->Scr.x  = MinX;
            NewV12->Scr.y += Delta12 * (S1->y - S2->y);
            NewV12->Scr.fx = FLT_TO_FXD(NewV12->Scr.x);
            NewV12->Scr.fy = FLT_TO_FXD(NewV12->Scr.y);

            NewV13->Scr.x  = MinX;
            NewV13->Scr.y += Delta13 * (S1->y - S3->y);
            NewV13->Scr.fx = FLT_TO_FXD(NewV13->Scr.x);
            NewV13->Scr.fy = FLT_TO_FXD(NewV13->Scr.y);

            ClipZOne();
            ClipUVOne();
            ClipEnvOne();
            ClipGOne(NewV12, NewV13);

            NewTri1->V1 = V1;
            NewTri1->V2 = NewV12;
            NewTri1->V3 = NewV13;
         }
         // This will yield two tris
         else
         {
            NewTwo( NewTri2, NewV13, NewV23, V3, V3 );

            Delta13 = (MinX - S3->x) / (S1->x - S3->x);
            Delta23 = (MinX - S3->x) / (S2->x - S3->x);

            NewV13->Scr.x  = MinX;
            NewV13->Scr.y += Delta13 * (S1->y - S3->y);
            NewV13->Scr.fx = FLT_TO_FXD(NewV13->Scr.x);
            NewV13->Scr.fy = FLT_TO_FXD(NewV13->Scr.y);

            NewV23->Scr.x  = MinX;
            NewV23->Scr.y += Delta23 * (S2->y - S3->y);
            NewV23->Scr.fx = FLT_TO_FXD(NewV23->Scr.x);
            NewV23->Scr.fy = FLT_TO_FXD(NewV23->Scr.y);

            ClipZTwo();
            ClipUVTwo();
            ClipEnvTwo();
            ClipGTwo(NewV13, NewV23);

            NewTri1->V1 = V1;
            NewTri1->V2 = V2;
            NewTri1->V3 = NewV23;

            NewTri2->V1 = V1;
            NewTri2->V2 = NewV13;
            NewTri2->V3 = NewV23;
         }
         continue;
      }
      // Clip top edge?
      else if (S1->fy < fxMinY || S2->fy < fxMinY || S3->fy < fxMinY)
      {
         ClipSetup();

         // Re-order 'em (from bottom to top)
         if (S1->y < S2->y){SWAP(S1,S2,tP);SWAP(M1,M2,tP);SWAP(E1,E2,tP);SWAP(V1,V2,tV)};
         if (S2->y < S3->y){SWAP(S2,S3,tP);SWAP(M2,M3,tP);SWAP(E2,E3,tP);SWAP(V2,V3,tV)};
         if (S1->y < S2->y){SWAP(S1,S2,tP);SWAP(M1,M2,tP);SWAP(E1,E2,tP);SWAP(V1,V2,tV)};

         if (S1->y < MinY) Remove();

         // This will yield only one tri
         if (S2->y < MinY)
         {
            NewOne(NewV12, NewV13, V2, V3);

            Delta12 = (MinY - S2->y) / (S1->y - S2->y);
            Delta13 = (MinY - S3->y) / (S1->y - S3->y);

            NewV12->Scr.x += Delta12 * (S1->x - S2->x);
            NewV12->Scr.y  = MinY;
            NewV12->Scr.fx = FLT_TO_FXD(NewV12->Scr.x);
            NewV12->Scr.fy = FLT_TO_FXD(NewV12->Scr.y);

            NewV13->Scr.x += Delta13 * (S1->x - S3->x);
            NewV13->Scr.y  = MinY;
            NewV13->Scr.fx = FLT_TO_FXD(NewV13->Scr.x);
            NewV13->Scr.fy = FLT_TO_FXD(NewV13->Scr.y);

            ClipZOne();
            ClipUVOne();
            ClipEnvOne();
            ClipGOne(NewV12, NewV13);

            NewTri1->V1 = V1;
            NewTri1->V2 = NewV12;
            NewTri1->V3 = NewV13;
         }
         // This will yield two tris
         else
         {
            NewTwo( NewTri2, NewV13, NewV23, V3, V3 );

            Delta13 = (MinY - S3->y) / (S1->y - S3->y);
            Delta23 = (MinY - S3->y) / (S2->y - S3->y);

            NewV13->Scr.x += Delta13 * (S1->x - S3->x);
            NewV13->Scr.y  = MinY;
            NewV13->Scr.fx = FLT_TO_FXD(NewV13->Scr.x);
            NewV13->Scr.fy = FLT_TO_FXD(NewV13->Scr.y);

            NewV23->Scr.x += Delta23 * (S2->x - S3->x);
            NewV23->Scr.y  = MinY;
            NewV23->Scr.fx = FLT_TO_FXD(NewV23->Scr.x);
            NewV23->Scr.fy = FLT_TO_FXD(NewV23->Scr.y);

            ClipZTwo();
            ClipUVTwo();
            ClipEnvTwo();
            ClipGTwo(NewV13, NewV23);

            NewTri1->V1 = V1;
            NewTri1->V2 = V2;
            NewTri1->V3 = NewV23;

            NewTri2->V1 = V1;
            NewTri2->V2 = NewV13;
            NewTri2->V3 = NewV23;
         }
         continue;
      }
      // Clip right edge?
      else if (S1->fx > fxMaxX || S2->fx > fxMaxX || S3->fx > fxMaxX)
      {
         ClipSetup();

         // Re-order 'em (from left to right)
         if (S1->x > S2->x){SWAP(S1,S2,tP);SWAP(M1,M2,tP);SWAP(E1,E2,tP);SWAP(V1,V2,tV)};
         if (S2->x > S3->x){SWAP(S2,S3,tP);SWAP(M2,M3,tP);SWAP(E2,E3,tP);SWAP(V2,V3,tV)};
         if (S1->x > S2->x){SWAP(S1,S2,tP);SWAP(M1,M2,tP);SWAP(E1,E2,tP);SWAP(V1,V2,tV)};

         if (S1->x > MaxX) Remove();

         // This will yield only one tri
         if (S2->x > MaxX)
         {
            NewOne(NewV12, NewV13, V2, V3);

            Delta12 = (MaxX - S2->x) / (S1->x - S2->x);
            Delta13 = (MaxX - S3->x) / (S1->x - S3->x);

            NewV12->Scr.x  = MaxX;
            NewV12->Scr.y += Delta12 * (S1->y - S2->y);
            NewV12->Scr.fx = FLT_TO_FXD(NewV12->Scr.x);
            NewV12->Scr.fy = FLT_TO_FXD(NewV12->Scr.y);

            NewV13->Scr.x  = MaxX;
            NewV13->Scr.y += Delta13 * (S1->y - S3->y);
            NewV13->Scr.fx = FLT_TO_FXD(NewV13->Scr.x);
            NewV13->Scr.fy = FLT_TO_FXD(NewV13->Scr.y);

            ClipZOne();
            ClipUVOne();
            ClipEnvOne();
            ClipGOne(NewV12, NewV13);

            NewTri1->V1 = V1;
            NewTri1->V2 = NewV12;
            NewTri1->V3 = NewV13;
         }
         // This will yield two tris
         else
         {
            NewTwo( NewTri2, NewV13, NewV23, V3, V3 );

            Delta13 = (MaxX - S3->x) / (S1->x - S3->x);
            Delta23 = (MaxX - S3->x) / (S2->x - S3->x);

            NewV13->Scr.x  = MaxX;
            NewV13->Scr.y += Delta13 * (S1->y - S3->y);
            NewV13->Scr.fx = FLT_TO_FXD(NewV13->Scr.x);
            NewV13->Scr.fy = FLT_TO_FXD(NewV13->Scr.y);

            NewV23->Scr.x  = MaxX;
            NewV23->Scr.y += Delta23 * (S2->y - S3->y);
            NewV23->Scr.fx = FLT_TO_FXD(NewV23->Scr.x);
            NewV23->Scr.fy = FLT_TO_FXD(NewV23->Scr.y);

            ClipZTwo();
            ClipUVTwo();
            ClipEnvTwo();
            ClipGTwo(NewV13, NewV23);

            NewTri1->V1 = V1;
            NewTri1->V2 = V2;
            NewTri1->V3 = NewV23;

            NewTri2->V1 = V1;
            NewTri2->V2 = NewV13;
            NewTri2->V3 = NewV23;
         }
         continue;
      }
      // Clip bottom edge?
      else if (S1->fy > fxMaxY || S2->fy > fxMaxY || S3->fy > fxMaxY)
      {
         ClipSetup();

         // Re-order 'em (from top to bottom)
         if (S1->y > S2->y){SWAP(S1,S2,tP);SWAP(M1,M2,tP);SWAP(E1,E2,tP);SWAP(V1,V2,tV)};
         if (S2->y > S3->y){SWAP(S2,S3,tP);SWAP(M2,M3,tP);SWAP(E2,E3,tP);SWAP(V2,V3,tV)};
         if (S1->y > S2->y){SWAP(S1,S2,tP);SWAP(M1,M2,tP);SWAP(E1,E2,tP);SWAP(V1,V2,tV)};

         if (S1->y > MaxY) Remove();

         // This will yield only one tri
         if (S2->y > MaxY)
         {
            NewOne(NewV12, NewV13, V2, V3);

            Delta12 = (MaxY - S2->y) / (S1->y - S2->y);
            Delta13 = (MaxY - S3->y) / (S1->y - S3->y);

            NewV12->Scr.x += Delta12 * (S1->x - S2->x);
            NewV12->Scr.y  = MaxY;
            NewV12->Scr.fx = FLT_TO_FXD(NewV12->Scr.x);
            NewV12->Scr.fy = FLT_TO_FXD(NewV12->Scr.y);

            NewV13->Scr.x += Delta13 * (S1->x - S3->x);
            NewV13->Scr.y  = MaxY;
            NewV13->Scr.fx = FLT_TO_FXD(NewV13->Scr.x);
            NewV13->Scr.fy = FLT_TO_FXD(NewV13->Scr.y);

            ClipZOne();
            ClipUVOne();
            ClipEnvOne();
            ClipGOne(NewV12, NewV13);

            NewTri1->V1 = V1;
            NewTri1->V2 = NewV12;
            NewTri1->V3 = NewV13;
         }
         // This will yield two tris
         else
         {
            NewTwo( NewTri2, NewV13, NewV23, V3, V3 );

            Delta13 = (MaxY - S3->y) / (S1->y - S3->y);
            Delta23 = (MaxY - S3->y) / (S2->y - S3->y);

            NewV13->Scr.x += Delta13 * (S1->x - S3->x);
            NewV13->Scr.y  = MaxY;
            NewV13->Scr.fx = FLT_TO_FXD(NewV13->Scr.x);
            NewV13->Scr.fy = FLT_TO_FXD(NewV13->Scr.y);

            NewV23->Scr.x += Delta23 * (S2->x - S3->x);
            NewV23->Scr.y  = MaxY;
            NewV23->Scr.fx = FLT_TO_FXD(NewV23->Scr.x);
            NewV23->Scr.fy = FLT_TO_FXD(NewV23->Scr.y);

            ClipZTwo();
            ClipUVTwo();
            ClipEnvTwo();
            ClipGTwo(NewV13, NewV23);

            NewTri1->V1 = V1;
            NewTri1->V2 = V2;
            NewTri1->V3 = NewV23;

            NewTri2->V1 = V1;
            NewTri2->V2 = NewV13;
            NewTri2->V3 = NewV23;
         }
         continue;
      }

      i++;
   }

   return;
}

/*----------------------------------------------------------------------------*/

void  ResetTriReservoir()
{
   FUNC("ResetTriReservoir");
   TriReservoirAllocated = 0;
}

/*----------------------------------------------------------------------------*/

TRI   *AllocReservoirTri()
{
   FUNC("AllocReservoirTri");

   // Is there room in the reservoir?
   Assert(TriReservoirAllocated < TRI_RESERVOIR_SIZE);

   return &TriReservoir[TriReservoirAllocated++];
}

/*----------------------------------------------------------------------------*/

void  ResetVertexReservoir()
{
   FUNC("ResetVertexReservoir");
   VertexReservoirAllocated = 0;
}

/*----------------------------------------------------------------------------*/

VER   *AllocReservoirVertex()
{
   FUNC("AllocReservoirVertex");

   // Is there room in the reservoir?
   Assert(VertexReservoirAllocated < VERTEX_RESERVOIR_SIZE);

   return &VertexReservoir[VertexReservoirAllocated++];
}

/*----------------------------------------------------------------------------
  -   [CLIP.C      ] - End Of File                                           -
  ----------------------------------------------------------------------------*/
