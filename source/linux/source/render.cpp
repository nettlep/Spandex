// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [RENDER.C    ] - The rendering pipeline                                -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#include "spandex.h"

#ifdef   U_E
extern INT e_et;
extern INT e_st;
extern INT e_ar;
#define EVAL_TIME (30*60)
#endif

// #define  BORDER_TIMER

/*----------------------------------------------------------------------------*/

#define AddTri(Tri, Object, Ambient, IntRange, V1, V2, V3 )             \
   Tri->Object    = Object;                                             \
   Tri->MinZ      = MIN(V1->FixedZ,MIN(V2->FixedZ,V3->FixedZ))

/*----------------------------------------------------------------------------*/

static   RLS   RList;
static   rendfunptr SolidRenderers[256];
static   rendfunptr TransRenderers[256];

/*----------------------------------------------------------------------------*/

static   void  BuildRenderList(OBJ *Object, OBJ *Cam, VSCR *VScreen);
static   void  AddObjectToRenderList(OBJ *Object, VSCR *VScreen);

/*----------------------------------------------------------------------------*/

INT   SPDXInitRenderer( void )
{
   FUNC("SPDXInitRenderer");
#ifndef U_Z
   SPDXInitByteSortStack();
#endif

   // Fill the list with a default
   SPDXMemSetDWORD( (void *) SolidRenderers, (INT) SPDXSolFTri, 256 );

   SolidRenderers[AMBIENT|SOLID]    = SPDXSolFTri;
   SolidRenderers[AMBIENT|ENVMAP]   = SPDXTexFTri;
   SolidRenderers[AMBIENT|TEXTURED] = SPDXTexFTri;
   SolidRenderers[LAMBERT|SOLID]    = SPDXSolFTri;
   SolidRenderers[LAMBERT|ENVMAP]   = SPDXTexFTri;
   SolidRenderers[LAMBERT|TEXTURED] = SPDXTexFTri;
   SolidRenderers[GOURAUD|SOLID]    = SPDXSolGTri;
   SolidRenderers[GOURAUD|ENVMAP]   = SPDXTexGTri;
   SolidRenderers[GOURAUD|TEXTURED] = SPDXTexGTri;
   SolidRenderers[PHONG  |SOLID]    = SPDXSolPTri;
   SolidRenderers[PHONG  |ENVMAP]   = SPDXTexPTri;
   SolidRenderers[PHONG  |TEXTURED] = SPDXTexPTri;
   SolidRenderers[BUMP   |SOLID]    = SPDXSolBTri;
   SolidRenderers[BUMP   |ENVMAP]   = SPDXEnvBTri;
   SolidRenderers[BUMP   |TEXTURED] = SPDXTexBTri;

   // Fill the list with a default
   SPDXMemSetDWORD( (void *) TransRenderers, (INT) SPDXTransparentSolFTri, 256 );

   TransRenderers[AMBIENT|SOLID]    = SPDXTransparentSolFTri;
   TransRenderers[AMBIENT|ENVMAP]   = SPDXTransparentTexFTri;
   TransRenderers[AMBIENT|TEXTURED] = SPDXTransparentTexFTri;
   TransRenderers[LAMBERT|SOLID]    = SPDXTransparentSolFTri;
   TransRenderers[LAMBERT|ENVMAP]   = SPDXTransparentTexFTri;
   TransRenderers[LAMBERT|TEXTURED] = SPDXTransparentTexFTri;
   TransRenderers[GOURAUD|SOLID]    = SPDXTransparentSolGTri;
   TransRenderers[GOURAUD|ENVMAP]   = SPDXTransparentTexGTri;
   TransRenderers[GOURAUD|TEXTURED] = SPDXTransparentTexGTri;
   TransRenderers[PHONG  |SOLID]    = SPDXTransparentSolPTri;
   TransRenderers[PHONG  |ENVMAP]   = SPDXTransparentTexPTri;
   TransRenderers[PHONG  |TEXTURED] = SPDXTransparentTexPTri;
   TransRenderers[BUMP   |SOLID]    = SPDXTransparentSolBTri;
   TransRenderers[BUMP   |ENVMAP]   = SPDXTransparentEnvBTri;
   TransRenderers[BUMP   |TEXTURED] = SPDXTransparentTexBTri;

   return LE_NONE;
}

/*----------------------------------------------------------------------------*/

void  SPDXUninitRenderer( void )
{
   FUNC("SPDXUninitRenderer");
   if (RList.Vers)
   {
      SPDXFree( RList.Vers );
      RList.Vers = NULL;
      RList.VertexCount = 0;
      RList.VertexReservoir = 0;
   }

   if (RList.Tris)
   {
      SPDXFree( RList.Tris );
      RList.Tris = NULL;
      RList.TriCount = 0;
      RList.TriReservoir = 0;
   }

   if (RList.Points)
   {
      SPDXFree( RList.Points );
      RList.Points = NULL;
      RList.PointCount = 0;
      RList.PointReservoir = 0;
   }

#ifndef U_Z
   SPDXUninitByteSortStack();
#endif

   return;
}

/*----------------------------------------------------------------------------*/

INT   SPDXRenderHierarchy( OBJ *Object, OBJ *Cam, BYT ClearScreenFlag, BYT CopyFlag, BYT AntiAliasFlag, BYT SmoothFlag, BYT WaitVBFlag, VSCR *VScreen, VSCR *AAVScreen )
{
INT   Ticks_hi, Ticks_lo, Ticks;

   FUNC("SPDXRenderHierarchy");
   Assert( Object );
   Assert( Cam );
   Assert( VScreen );

   // Update the material lighting pre-calcs
   SPDXUpdateMaterialsLighting();

   // Set the lense (ScaleX and ScaleY) for this view...
   SPDXSetVirtualLense( SPDXGetObjectLense(Cam), VScreen );
   SPDXCorrectVirtualAspect( SPDXGlobals.AspectX, SPDXGlobals.AspectY, VScreen );

   // Clear the screen
   if (ClearScreenFlag == TRUE)
      SPDXClearVirtualScreen( (CHR) SPDXGlobals.BackgroundColor, VScreen );

#ifdef U_Z
   SPDXResetVirtualZBuffer( VScreen );
#endif

   // This is the rendering pipeline
   SPDXOrientHierarchy( Object, Cam );
   SPDXCullHierarchy( Object );
   SPDXTransformHierarchy( Object );
   SPDXProjectHierarchy( Object, Cam, VScreen );

   SPDXBuildRenderList( Object, Cam, VScreen );
   SPDXShadeRenderList( &RList );
   SPDXClipRenderList( &RList, VScreen );

#ifdef BORDER_TIMER
   SPDXTimeBorder(200);
#endif

#ifndef U_Z
   SPDXByteSortRenderList( &RList );
#endif

#ifdef BORDER_TIMER
   SPDXSetBordColor(0);
#endif

   RDTSC_Start(&Ticks_hi, &Ticks_lo);
   SPDXDrawRenderList( &RList, SolidRenderers, TransRenderers, VScreen );
   Ticks = RDTSC_End(Ticks_hi, Ticks_lo);

   if (AntiAliasFlag == TRUE)
   {
      SPDXVirtualAntiAlias( AAVScreen, VScreen );
      VScreen = AAVScreen;
   }

   if (SmoothFlag == TRUE)
      SPDXVirtualSmoothFrame( VScreen );

   if (CopyFlag == TRUE)
   {
      // Copy it off to the screen
      if (!SPDXGlobals.UseIRQ)
      {
         if (WaitVBFlag == TRUE)
         {
#ifdef _OS_DOS
            WAIT_FOR_NO_VB;
#endif
         }

#ifdef _OS_DOS
         SPDXCopyToScreen(VScreen->DrawScreen);
#endif

      }
      else
      {
         SPDXGlobals.IRQ_OkToCopy = 1;
      }
   }

   return Ticks;
}

/*----------------------------------------------------------------------------*/

void  SPDXBuildRenderList(OBJ *Object, OBJ *Cam, VSCR *VScreen)
{
   FUNC("SPDXBuildRenderList");
   Assert( Object );
   Assert( Cam );
   Assert( VScreen );

   SPDXResetClipper();

   RList.TriCount = 0;
   RList.PointCount = 0;
   RList.VertexCount = 0;

   BuildRenderList( Object, Cam, VScreen );
}

/*----------------------------------------------------------------------------*/

void  BuildRenderList(OBJ *Object, OBJ *Cam, VSCR *VScreen)
{
INT   i;

   FUNC("BuildRenderList");
   Assert( Object );
   Assert( Cam );
   Assert( VScreen );

#ifdef   U_E
   e_et = difftime( time(NULL), e_st );
   if (e_et > EVAL_TIME)
   {
      e_ar = TRUE;
      return;
   }
#endif

   // Add my children to the list
   for( i = 0; i < Object->Children.Count; i++ )
      BuildRenderList(&Object->Children.Objs[i], Cam, VScreen);

   // Add me to the list
   AddObjectToRenderList(Object, VScreen);
}

/*----------------------------------------------------------------------------*/

void  AddObjectToRenderList(OBJ *Object, VSCR *VScreen)
{
INT   i, Total, Count, MinX, MinY, MaxX, MaxY;
UBYT  ShadeModel;
FXD   fxMinX, fxMinY, fxMaxX, fxMaxY;
TRI   **ppTri, *Tri, *SplitTri1, *SplitTri2;
VER   **ppVer, *Ver, *V1, *V2, *V3, *NewV1, *NewV2;
extern   VER   *AllocReservoirVertex();

   FUNC("AddObjectToRenderList");
   Assert( Object );
   Assert( VScreen );

   ShadeModel = Object->ShadeModel;

   // Need more room for vers?
   Total = Object->VertexList.Count + RList.VertexCount + VERTEX_RESERVOIR_SIZE;

   if (Total > RList.VertexReservoir)
   {
      ppVer = (VER **) SPDXRealloc( RList.Vers, sizeof(VER **) * Total );

      if (!ppVer)
      {
         SPDXLogError( LE_NOMEM, "No memory for vertices in render list" );
         return;
      }

      RList.Vers = ppVer;
      RList.VertexReservoir = Total;
   }

   Count = Object->VertexList.Count;
   Ver   = Object->VertexList.Vers;

   for( i = 0; i < Count; i++, Ver++ )
   {
      if (Ver->Visible == VIS_YES)
      {
         Ver->ShadeModel = ShadeModel;
#ifdef U_P
         Ver->OneOverZ = ONE_OVER_VAL / Ver->CLoc.z;
#endif
         RList.Vers[RList.VertexCount++] = Ver;
      }
   }

   // Add the tris
   if (Object->DrawFlags & DRAW_TRIS)
   {
      // Need more room for tris?
      Total = Object->TriList.Count + RList.TriCount + TRI_RESERVOIR_SIZE;
      if (Total > RList.TriReservoir)
      {
         ppTri = (TRI **) SPDXRealloc( RList.Tris, sizeof(TRI **) * Total );

         if (!ppTri)
         {
            SPDXLogError( LE_NOMEM, "No memory for tris in render list" );
            return;
         }

         RList.Tris = ppTri;
         RList.TriReservoir = Total;
      }

      MinX = VScreen->ClipMinX;
      MinY = VScreen->ClipMinY;
      MaxX = VScreen->ClipMaxX;
      MaxY = VScreen->ClipMaxY;

      fxMinX = INT_TO_FXD(MinX);
      fxMinY = INT_TO_FXD(MinY);
      fxMaxX = INT_TO_FXD(MaxX);
      fxMaxY = INT_TO_FXD(MaxY);

      Count = Object->TriList.Count;
      Tri   = Object->TriList.Tris;

      for( i = 0; i < Count; i++, Tri++ )
      {
         if (Tri->Visible == VIS_NO) continue;

         V1 = Tri->V1; V2 = Tri->V2; V3 = Tri->V3;

         // Eliminate culled polygons
         if (V1->Visible==VIS_NO || V2->Visible==VIS_NO || V3->Visible==VIS_NO)
            continue;

         // If the whole thing is behind the clipping plane, skip it
         if (V1->Visible==VIS_CLIPZ && V2->Visible==VIS_CLIPZ && V3->Visible==VIS_CLIPZ)
            continue;
               
         // Does the poly require Z-Plane Clipping?
         if (V1->Visible==VIS_CLIPZ || V2->Visible==VIS_CLIPZ || V3->Visible==VIS_CLIPZ)
         {
            // Every split will require two new vertices
            NewV1 = AllocReservoirVertex();  if (!NewV1) return;
            NewV2 = AllocReservoirVertex();  if (!NewV2) return;

            // Add those vertices into the renderlist
            NewV1->ShadeModel = NewV2->ShadeModel = ShadeModel;
            RList.Vers[RList.VertexCount++] = NewV1;
            RList.Vers[RList.VertexCount++] = NewV2;

            Tri->Object = Object;
            SPDXSplitPolygonZ(Tri, &SplitTri1, &SplitTri2, NewV1, NewV2, VScreen );

#ifdef U_P
            NewV1->OneOverZ  = ONE_OVER_VAL / NewV1->CLoc.z;
            NewV2->OneOverZ  = ONE_OVER_VAL / NewV2->CLoc.z;
#endif

            V1 = SplitTri1->V1; V2 = SplitTri1->V2; V3 = SplitTri1->V3;

            // If it's still onscreen, then add it
            if ((V1->Scr.fx <  fxMinX && V2->Scr.fx <  fxMinX && V3->Scr.fx <  fxMinX) ||
                (V1->Scr.fy <  fxMinY && V2->Scr.fy <  fxMinY && V3->Scr.fy <  fxMinY) ||
                (V1->Scr.fx >= fxMaxX && V2->Scr.fx >= fxMaxX && V3->Scr.fx >= fxMaxX) ||
                (V1->Scr.fy >= fxMaxY && V2->Scr.fy >= fxMaxY && V3->Scr.fy >= fxMaxY) ||
                (V1->Visible!=VIS_YES || V2->Visible!=VIS_YES || V3->Visible!=VIS_YES))
            {
               // Do nothing... this is easier to read (for me)
            }
            else
            {
               AddTri(SplitTri1, Object, Ambient, IntRange, V1, V2, V3 );
               RList.Tris[RList.TriCount++] = SplitTri1;
            }

            if (SplitTri2)
            {
               V1 = SplitTri2->V1; V2 = SplitTri2->V2; V3 = SplitTri2->V3;

               // If it's still onscreen, then add it
               if ((V1->Scr.fx <  fxMinX && V2->Scr.fx <  fxMinX && V3->Scr.fx <  fxMinX) ||
                   (V1->Scr.fy <  fxMinY && V2->Scr.fy <  fxMinY && V3->Scr.fy <  fxMinY) ||
                   (V1->Scr.fx >= fxMaxX && V2->Scr.fx >= fxMaxX && V3->Scr.fx >= fxMaxX) ||
                   (V1->Scr.fy >= fxMaxY && V2->Scr.fy >= fxMaxY && V3->Scr.fy >= fxMaxY) ||
                   (V1->Visible!=VIS_YES || V2->Visible!=VIS_YES || V3->Visible!=VIS_YES))
               {
                  // Do nothing... this is easier to read (for me)
               }
               else
               {
                  AddTri(SplitTri2, Object, Ambient, IntRange, V1, V2, V3 );
                  RList.Tris[RList.TriCount++] = SplitTri2;
               }
            }
            continue;
         }

         AddTri(Tri, Object, Ambient, IntRange, V1, V2, V3 );
         RList.Tris[RList.TriCount++] = Tri;
      }
   }

   // Add the points
   if (Object->DrawFlags & DRAW_POINTS)
   {
      // Need more room for points?
      Total = Object->PointList.Count + RList.PointCount;

      if (Total > RList.PointReservoir)
      {
         ppVer = (VER **) SPDXRealloc( RList.Points, sizeof(VER **) * Total );

         if (!ppVer)
         {
            SPDXLogError( LE_NOMEM, "No memory for points in render list" );
            return;
         }

         RList.Points = ppVer;
         RList.PointReservoir = Total;
      }

      Count = Object->PointList.Count;
      ppVer = Object->PointList.Points;

      for( i = 0; i < Count; i++ )
         if (ppVer[i]->Visible == VIS_YES)
            RList.Points[RList.PointCount++] = ppVer[i];
   }
}

/*----------------------------------------------------------------------------*/

RLS   *SPDXGetRenderList()
{
   return &RList;
}

/*----------------------------------------------------------------------------*/

FLT   SPDXCountPixelsInRenderList( RLS *RList)
{
INT   i, Count;
TRI   *Tri;
VER   *Ver1, *Ver2, *Ver3;
FLT   MinX, MinY, MaxX, MaxY, PixelCount = 0.0;

   Count = RList->TriCount;

   for(i = 0; i < Count; i++ )
   {
      Tri  = RList->Tris[i];
      Ver1 = Tri->V1;
      Ver2 = Tri->V2;
      Ver3 = Tri->V3;

      MinX = MIN(Ver1->Scr.x, MIN(Ver2->Scr.x, Ver3->Scr.x));
      MinY = MIN(Ver1->Scr.y, MIN(Ver2->Scr.y, Ver3->Scr.y));
      MaxX = MAX(Ver1->Scr.x, MAX(Ver2->Scr.x, Ver3->Scr.x));
      MaxY = MAX(Ver1->Scr.y, MAX(Ver2->Scr.y, Ver3->Scr.y));
      PixelCount += (MaxX - MinX) * (MaxY - MinY);
   }

   return PixelCount * 0.5;
}

/*----------------------------------------------------------------------------*/

INT   SPDXCountPolysInRenderList( RLS *RList)
{
   return RList->TriCount;
}

/*----------------------------------------------------------------------------
  -   [RENDER.C    ] - End Of File                                           -
  ----------------------------------------------------------------------------*/
