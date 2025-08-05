// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [MATH.C      ] - Misc math functions                                   -
  -                                                                          -
  ----------------------------------------------------------------------------*/

#include "spandex.h"

/*----------------------------------------------------------------------------*/

void  SPDXGetTriNormalFromOrientation( TRI *Triangle, VEC *Vector )
{
P3D   *P1, *P2, *P3;
VEC   A, B;
FLT   VLen;

   FUNC("SPDXGetTriNormalFromVertex");
   Assert( Triangle );
   Assert( Vector );

   P1 = &Triangle->V1->LLoc;
   P2 = &Triangle->V2->LLoc;
   P3 = &Triangle->V3->LLoc;

   A.dx = P3->x - P2->x;
   A.dy = P3->y - P2->y;
   A.dz = P3->z - P2->z;
   B.dx = P1->x - P2->x;
   B.dy = P1->y - P2->y;
   B.dz = P1->z - P2->z;

   SPDXVectorCrossProduct( Vector, &A, &B );

   SPDXVectorToUnitVector( *Vector, VLen );

   return;
}

/*----------------------------------------------------------------------------*/

FLT   SPDXGetTriDistance( TRI *Triangle, P3D *FromWhere )
{
FLT   fTemp;
P3D   P;

   FUNC("SPDXGetTriDistance");
   Assert( Triangle );
   Assert( FromWhere );

   fTemp = 1.0 / 3.0;

   P.x = (Triangle->V1->LLoc.x + Triangle->V2->LLoc.x + Triangle->V3->LLoc.x) * fTemp;
   P.y = (Triangle->V1->LLoc.y + Triangle->V2->LLoc.y + Triangle->V3->LLoc.y) * fTemp;
   P.z = (Triangle->V1->LLoc.z + Triangle->V2->LLoc.z + Triangle->V3->LLoc.z) * fTemp;

   return SPDXGetDistance3D(&P, FromWhere);
}

/*----------------------------------------------------------------------------*/

FLT   SPDXGetDistance3D( P3D *Point1, P3D *Point2 )
{
   FUNC("SPDXGetDistance3D");
   Assert( Point1 );
   Assert( Point2 );

   return FLT_SQRT((Point1->x - Point2->x) * (Point1->x - Point2->x) +
                   (Point1->y - Point2->y) * (Point1->y - Point2->y) +
                   (Point1->z - Point2->z) * (Point1->z - Point2->z));
}

/*----------------------------------------------------------------------------*/

// Careful not to overflow this!!

FLT   SPDXGetDistance2D( P2D *Point1, P2D *Point2 )
{
   FUNC("SPDXGetDistance2D");
   Assert( Point1 );
   Assert( Point2 );

   return FLT_SQRT((Point1->x - Point2->x) * (Point1->x - Point2->x) +
                   (Point1->y - Point2->y) * (Point1->y - Point2->y));
}

/*----------------------------------------------------------------------------*/

void  SPDXVectorCrossProduct( VEC *Result, VEC *V1, VEC *V2 )
{
   FUNC("SPDXVectorCrossProduct");
   Assert( Result );
   Assert( V1 );
   Assert( V2 );

   Result->dx = (V1->dy * V2->dz) - (V1->dz * V2->dy);
   Result->dy = (V1->dz * V2->dx) - (V1->dx * V2->dz);
   Result->dz = (V1->dx * V2->dy) - (V1->dy * V2->dx);

   return;
}

/*----------------------------------------------------------------------------*/

void  SPDXMakeMatrixFromVector( VEC *Vector, FLT Bank, MAT *Matrix )
{
FLT   fTemp;
VEC   GlobalYAxis, XAxis, YAxis, ZAxis;
MAT   TempMat, ZMat;

   SPDXVectorToUnitVector(*Vector, fTemp);

   if (!Vector->dx && !Vector->dz)
   {
      GlobalYAxis.dx = -Vector->dy;
      GlobalYAxis.dy = 0.0;
      GlobalYAxis.dz = 0.0;
   }
   else
   {
      GlobalYAxis.dx = 0.0;
      GlobalYAxis.dy = 1.0;
      GlobalYAxis.dz = 0.0;
   }

   SPDXVectorCrossProduct(&XAxis, &GlobalYAxis, Vector );
   SPDXVectorToUnitVector(XAxis, fTemp);

   SPDXVectorCrossProduct(&YAxis, &XAxis,       Vector );
   SPDXVectorToUnitVector(YAxis, fTemp);
   SPDXInvertVector(YAxis);

   ZAxis = *Vector;

   TempMat.m00 = XAxis.dx; TempMat.m01 = YAxis.dx; TempMat.m02 = ZAxis.dx;
   TempMat.m10 = XAxis.dy; TempMat.m11 = YAxis.dy; TempMat.m12 = ZAxis.dy;
   TempMat.m20 = XAxis.dz; TempMat.m21 = YAxis.dz; TempMat.m22 = ZAxis.dz;

   while( Bank >= (FLT) ROT_POINTS )
      Bank -= (FLT) ROT_POINTS;
   
   while( Bank < 0.0 )
      Bank += (FLT) ROT_POINTS;

   // Bank must be in degrees (as is in the .3ds file)
   SPDXMakeZMatrix(ZMat, FLT_TO_INT(Bank));

   SPDXMatrixMulMatrix(*Matrix, ZMat, TempMat);
}

/*----------------------------------------------------------------------------*/

FLT   Sine( int Value )
{
int   Index;
FLT   t1, t2;

   Index = Value >> SIN_TAB_BITS;
   t1 = (FLT) (Value & (~SIN_TAB_MASK)) / (FLT) (ROT_POINTS >> SIN_TAB_BITS);
   t2 = (SPDXGlobals.SineTable[Index+1] - SPDXGlobals.SineTable[Index]) * t1;
   return SPDXGlobals.SineTable[Index] + t2;
}

/*----------------------------------------------------------------------------*/

FLT   Cosine( int Value )
{
int   Index;
FLT   t1, t2;

   Index     = Value >> SIN_TAB_BITS;
   t1 = (FLT) (Value & (~SIN_TAB_MASK)) / (FLT) (ROT_POINTS >> SIN_TAB_BITS);
   t2 = (SPDXGlobals.CosTable[Index+1] - SPDXGlobals.CosTable[Index]) * t1;
   return t2 + SPDXGlobals.CosTable[Index];
}

/*----------------------------------------------------------------------------
  -   [MATH.C      ] - End Of File                                           -
  ----------------------------------------------------------------------------*/
