// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*----------------------------------------------------------------------------
  -                                                                          -
  -   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               -
  -                                                                          -
  -   [SDXDECL.H   ] - Fuction declarations for SPANDEX library              -
  -                                                                          -
  ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
// CLIP.C

void  SPDXResetClipper( void );
void  SPDXSplitPolygonZ(TRI *Tri, TRI **Split1, TRI **Split2, VER *NewV1, VER *NewV2, VSCR *VScreen );
void  SPDXClipRenderList(RLS *RList, VSCR *VScreen );

/*----------------------------------------------------------------------------*/
// CULL.C

void  SPDXCullHierarchy( OBJ *Object );

/*----------------------------------------------------------------------------*/
// DECODER.C

INT   SPDXDecompress(INT Handle, INT LineWidth, UCHR *Buffer, INT ImgWidth, ifunptr WriteLine);

/*----------------------------------------------------------------------------*/
// DRAW.C

void  SPDXDrawRenderList( RLS *RList, rendfunptr *SolidRenderers, rendfunptr *TransRenderers, VSCR *VScreen );

/*----------------------------------------------------------------------------*/
// ENCODER.C

INT   SPDXGifEncoder( CHR *FName, INT GWidth, INT GHeight, UCHR *Palette, ifunptrgp GetPixel );
INT   SPDXCompress( INT init_bits, INT Handle, ifunptrgp ReadValue );
void  SPDXSetCompressDataCount( INT Count );

/*----------------------------------------------------------------------------*/
// ERRORS.C

CHR   *SPDXGetErrorString( WRD ErrorCode );
void  SPDXSetFatalErrors(INT Flag);
INT   SPDXGetFatalErrors( void );
void  SPDXSetLoggedErrors(INT Flag);
INT   SPDXGetLoggedErrors( void );
void  SPDXSetLoggedStrings(INT Flag);
INT   SPDXGetLoggedStrings( void );
void  _SPDXFatalError( CHR *File, INT Line, CHR *Function, INT ErrorCode, CHR *Notes );
void  SPDXResetErrorFile( void );
void  _SPDXLogError( CHR *File, INT Line, CHR *Function, INT ErrorCode, CHR *Notes );
void  _SPDXLogString( CHR *File, INT Line, CHR *Function, CHR *Notes );

/*----------------------------------------------------------------------------*/
// FILEIO.C

INT   SPDXTell( INT Handle );
INT   SPDXGetc( INT Handle );
void  SPDXPutc( INT Handle, INT Byte );
INT   SPDXRead( INT Handle, void *Buffer, INT Length );
void  SPDXFlushBuffer( INT Handle );
INT   SPDXWrite( INT Handle, void *Buffer, INT Length );
LNG   SPDXSeek( INT Handle, LNG Offset, INT Origin );
INT   SPDXOpen( CHR *FileName, INT Flags1, INT Flags2 );
INT   SPDXClose( INT Handle );
CHR   *SPDXReadString( INT Handle, CHR *Buffer, INT MaxLength );
INT   _SPDXOpen( CHR *FileName, INT Flags1, INT Flags2 );
INT   _SPDXClose( INT Handle );
INT   _SPDXRead( INT Handle, void *Buffer, INT Length );
INT   _SPDXWrite( INT Handle, void *Buffer, INT Length );
LNG   _SPDXSeek( INT Handle, LNG Offset, INT Origin );
INT   _SPDXTell( INT Handle );
INT   _SPDXGetc( INT Handle );
void  _SPDXPutc( INT Handle, INT Byte );

/*----------------------------------------------------------------------------*/
// GLOBALS.C

INT   SPDXInitGlobals( void );
INT   SPDXInitTables( void );
void  SPDXReplacePhongMap( BMAP *Map );
INT   SPDXSetDebugStateFromFile( void );
INT   SPDXGetDebugState( void );
void  SPDXSetDebugState( INT State );
void  SPDXSetGlobalResolution(INT ResX, INT ResY );
void  SPDXGetGlobalResolution(INT *ResX, INT *ResY );
void  SPDXCleanupGlobals( void );
void  SPDXRebuildGlobalLightList( OBJ *Universe );
void  SPDXSetCurrentVScreen( VSCR *VScreen );
FLT   SPDXGetNearZ( void );
void  SPDXSetNearZ( FLT NearZ );
INT   SPDXGetBackgroundColor( void );
void  SPDXSetBackgroundColor( RGB *Color );

/*----------------------------------------------------------------------------*/
// IMAGE.C

INT   SPDXGetFileType( CHR *Name );
IMAGE *SPDXCreateImage( INT XRes, INT YRes );
void  SPDXDestroyImage( IMAGE **Image );

/*----------------------------------------------------------------------------*/
// INT.C

#ifdef OS_DOS_
void  SPDXInitializeInterrupts( void );
void  SPDXResetInterrupts( void );
void  interrupt far SPDXInterruptRoutine( );
#endif

/*----------------------------------------------------------------------------*/
// INT86.C

#ifdef OS_DOS_
void  SPDXint86( INT intno, union REGS *InputRegs, union REGS *OutputRegs );
void  SPDXint86x( INT intno, union REGS *InputRegs, union REGS *OutputRegs, struct SREGS *sregs );
void  *SPDXDMalloc( INT Size );
#endif

/*----------------------------------------------------------------------------*/
// SPANDEX.C

INT   SPDXInit( void );
void  SPDXUninit( OBJ *Universe );
CHR   *SPDXGetCopyrightMessage( void );
FLT   SPDXGetVersion( );
void  SPDXOpenServices( void );
void  SPDXCloseServices( void );

/*----------------------------------------------------------------------------*/
// LIGHTS.C

LGT   *SPDXAddLight( OBJ *Object );
INT   SPDXGetLightCount( LLS *Light );
void  SPDXSetLightName( LGT *Light, CHR *Name );
CHR   *SPDXGetLightName( LGT *Light );
void  SPDXSetLightLocal( LGT *Light, P3D *Location );
P3D   *SPDXGetLightLocal( LGT *Light);
void  SPDXSetLightIntensity( LGT *Light, RGB *Color );
FLT   SPDXGetLightIntensity( LGT *Light);
void  SPDXCleanupLights( void );
LGT   *SPDXFindLightByName( CHR *Name, BYT CaseSense );
LGT   *SPDXFindLightByNameLen( CHR *Name, INT Len, BYT CaseSense );
void  SPDXTranslateLight( LGT *Light, VEC *Vector );

/*----------------------------------------------------------------------------*/
// LOADIMG.C

INT   SPDXReadImage( CHR *FileName, IMAGE **Image );

/*----------------------------------------------------------------------------*/
// MAPS.C

INT   SPDXInitMaps(void);
INT   SPDXSelectRawMapFile( CHR *MapFileName, PAL *MapPalette );
INT   SPDXSelectCompressedMapFile( PAL *MapPalette, INT Handle);
INT   SPDXSelectRawBumFile( CHR *BumFileName );
INT   SPDXSelectCompressedBumFile( INT Handle );
INT   SPDXAddMap(CHR *Name, UBYT *Data, INT XRes, INT YRes );
void  SPDXCleanupMaps( void );
BMAP  *SPDXFindMapByName( CHR *Name );

/*----------------------------------------------------------------------------*/
// MATERIAL.C

INT   SPDXInitMaterials( void );
INT   SPDXAddMaterial( CHR *Name );
INT   SPDXAssignTriMaterial( CHR *MatName, OBJ *Object, INT Index, P2D *Map1, P2D *Map2, P2D *Map3 );
void  SPDXSetMaterialName( MTRL *Material, CHR *Name );
CHR   *SPDXGetMaterialName( MTRL *Material );
INT   SPDXSetMaterialTexture( MTRL *Material, CHR *Name );
BMAP  *SPDXGetMaterialTexture( MTRL *Material );
void  SPDXSetMaterialSurface( MTRL *Material, BYT Surface );
BYT   SPDXGetMaterialSurface( MTRL *Material );
INT   SPDXSetMaterialBumpMap( MTRL *Material, CHR *Name );
BMAP  *SPDXGetMaterialBumpMap( MTRL *Material );
void  SPDXSetMaterialTransparency( MTRL *Material, BYT Transparency );
BYT   SPDXGetMaterialTransparency( MTRL *Material );
void  SPDXSetMaterialAmbientLight( MTRL *Material, RGB *Ambient );
FLT   SPDXGetMaterialAmbientLight( MTRL *Material );
void  SPDXSetMaterialAmbientReflectCoefficient( MTRL *Material, FLT Ka );
FLT   SPDXGetMaterialAmbientReflectCoefficient( MTRL *Material );
void  SPDXSetMaterialDiffuseReflectCoefficient( MTRL *Material, FLT Kd );
FLT   SPDXGetMaterialDiffuseReflectCoefficient( MTRL *Material );
void  SPDXSetMaterialSpecularReflectCoefficient( MTRL *Material, FLT Ks );
FLT   SPDXGetMaterialSpecularReflectCoefficient( MTRL *Material );
void  SPDXSetMaterialShineCoefficient( MTRL *Material, FLT Shininess );
FLT   SPDXGetMaterialShineCoefficient( MTRL *Material );
void  SPDXSetMaterialColor( MTRL *Material, RGB *Color, FLT Intensity, PAL *Palette );
UBYT  SPDXGetMaterialColor( MTRL *Material );
MTRL  *SPDXFindMaterialByName( CHR *Name );
void  SPDXUpdateMaterialsLighting( void );
void  SPDXCleanupMaterials( void );


/*----------------------------------------------------------------------------*/
// MATH.C

void  SPDXGetTriNormalFromOrientation( TRI *Triangle, VEC *Vector );
FLT   SPDXGetTriDistance( TRI *Triangle, P3D *FromWhere );
FLT   SPDXGetDistance3D( P3D *Point1, P3D *Point2 );
FLT   SPDXGetDistance2D( P2D *Point1, P2D *Point2 );
void  SPDXVectorCrossProduct( VEC *Result, VEC *P1, VEC *P2 );
void  SPDXMakeMatrixFromVector( VEC *Vector, FLT Bank, MAT *Matrix );
FLT   Sine( int Value );
FLT   Cosine( int Value );

/*----------------------------------------------------------------------------*/
// MEM.C

void  SPDXInitMem( void );
void  SPDXUninitMem( void );
void  *_SPDXMalloc( INT Size, CHR *OwnerFile, INT OwnerLine, CHR *OwnerFunc );
void  *_SPDXRealloc( void *Address, INT Size, CHR *OwnerFile, INT OwnerLine, CHR *OwnerFunc );
void  _SPDXFree( void *Address, CHR *OwnerFile, INT OwnerLine, CHR *OwnerFunc );
INT   SPDXValidateMemory( void );
void  SPDXMemAdd( void *Address, INT Size, CHR *Owner );
void  SPDXDumpMemLog( void );
void  SPDXGetMemReport( MEMREP *MemReport );
CHR   *SPDXFindMemOwner( void *Address );

/*----------------------------------------------------------------------------*/
// MOUSE.C

#ifdef _OS_DOS
INT   SPDXInitMouse(void);
void  SPDXSetMousePosition( INT X, INT Y );
void  SPDXSetMouseRange( INT X1, INT X2, INT Y1, INT Y2 );
void  SPDXReadMouse( INT *Col, INT *Row, INT *BLeft, INT *BRight );
void  SPDXUninitMouse(void);
#endif

/*----------------------------------------------------------------------------*/
// MULTI.C

#ifdef _OS_DOS
void  MLTIInit( INT ReservedStack, WRD Speed );
void  MLTIUninit(void);
void  MLTISetSpeed( WRD Speed );
INT   MLTIGetProcID(void);
INT   MLTIFork( INT StackSize, INT Priority );
void  MLTISetPriority( INT ProcessID, INT Priority );
void  MLTISetProcessActive( INT ProcessID, INT ActiveFlag );
INT   MLTISemaphoreSet( INT Value, INT *PreviousOwner );
INT   MLTISemaphoreGet( INT *PreviousOwner );
void  _MLTIInit( INT ReservedStack );
void  _MLTIUninit(void);
void  __interrupt __far MLTIInterrupt(void);
#endif

/*----------------------------------------------------------------------------*/
// NEUQUANT.C

void  SPDXNeuquantImage(IMAGE *image, INT SampleFactor, UBYT *Palette);
void  SPDXNeuquantImages(IMAGE **images, UINT AddImages, UINT TotalImages, INT SampleFactor, UBYT *Palette);

/*----------------------------------------------------------------------------*/
// OBJECTS.C

INT   SPDXCreateObject( OBJ **Object );
INT   SPDXAddChild( OBJ *Parent );
INT   SPDXRemoveChild( OBJ *Parent, INT WhichChild );
OBJ   *SPDXGetChild( OBJ *Parent, INT Index );
void  SPDXInheritObject( OBJ *Parent, OBJ *Child );
INT   SPDXGetChildCount( OBJ *Parent );
void  SPDXMakeObjectDefault( OBJ *Child );
INT   SPDXGetObjectCount( OBL *Object );
void  SPDXSetObjectName( OBJ *Object, CHR *Name );
CHR   *SPDXGetObjectName( OBJ *Object );
void  SPDXSetObjectLense( OBJ *Object, FLT Lense );
FLT   SPDXGetObjectLense( OBJ *Object );
void  SPDXSetObjectShowBacks( OBJ *Object, BYT ShowBacks );
BYT   SPDXGetObjectShowBacks( OBJ *Object );
void  SPDXSetObjectShadeModel( OBJ *Object, BYT Model );
BYT   SPDXGetObjectShadeModel( OBJ *Object );
void  SPDXSetObjectDrawFlags( OBJ *Object, BYT DrawFlags );
BYT   SPDXGetObjectDrawFlags( OBJ *Object );
void  SPDXSetObjectSteadyOrientation( OBJ *Object, ROT *Rotation );
ROT   *SPDXGetObjectSteadyOrientation( OBJ *Object);
void  SPDXSetObjectDirection( OBJ *Object, VEC *Direction );
VEC   *SPDXGetObjectDirection( OBJ *Object );
void  SPDXSetObjectLocal( OBJ *Object, P3D *Location );
P3D   *SPDXGetObjectLocal( OBJ *Object);
void  SPDXSetObjectWorld( OBJ *Object, P3D *Location );
P3D   *SPDXGetObjectWorld( OBJ *Object);
void  SPDXSetObjectType( OBJ *Object, INT Type );
INT   SPDXGetObjectType( OBJ *Object );
void  SPDXSetObjectDevUse( OBJ *Object, INT Index, INT Value );
INT   SPDXGetObjectDevUse( OBJ *Object, INT Index );
void  SPDXTranslateObject( OBJ *Object, VEC *Vector );
void  SPDXTranslateObjectFacing( OBJ *Object, FLT Amplitude );
void  SPDXRotateObject( OBJ *Object, ROT *Rotation );
OBJ   *SPDXFindObjectByName( CHR *Name, OBJ *Universe, BYT CaseSense );
OBJ   *SPDXFindObjectByNameLen( CHR *Name, OBJ *Universe, INT Len, BYT CaseSense );
void  SPDXUnifyObjectNormals( OBJ *Object );
void  SPDXCleanupObject( OBJ *Object );

/*----------------------------------------------------------------------------*/
// OCTREE.C

void  SPDXOctreeQuantizeImage(IMAGE *image, UINT number_colors, UINT tree_depth, UBYT *Palette);
void  SPDXOctreeQuantizeImages(IMAGE **images, UINT AddImages, UINT TotalImages, UINT number_colors, UINT tree_depth, UBYT *Palette);

/*----------------------------------------------------------------------------*/
// ORIENT.C

void  SPDXOrientHierarchy( OBJ *Object, OBJ *Camera );
void  SPDXBuildPhaseOne( OBJ *Object, P3D *ParentLoc, MAT *ParentMat );
void  SPDXBuildPhaseTwo( OBJ *Object, MAT *CamMat, P3D *CamLoc );

/*----------------------------------------------------------------------------*/
// PALETTE.C

void  SPDXSetPalette( INT Start, INT Count, PAL *Palette );
void  SPDXGetPalette( INT Start, INT Count, PAL *Palette );
INT   SPDXPushPalette( PAL *Palette );
INT   SPDXPopPalette( PAL *Palette, INT Times );
void  SPDXFadePalettes( PAL *From, PAL *To, INT Step );
INT   SPDXGetClosestColor( UINT Red, UINT Green, UINT Blue, PAL *Palette );

/*----------------------------------------------------------------------------*/
// POINTS.C

INT   SPDXAddObjectPoint(OBJ *Object);
INT   SPDXGetPointCount( PLS *PList);
void  SPDXSetPointVertex( OBJ *Object, INT Index, VER *Ver );
VER   *SPDXGetPointVertex( OBJ *Object, INT Index);

/*----------------------------------------------------------------------------*/
// POLYS.C

void  SPDXSolFTri( TRI *Tri );
void  SPDXTexFTri( TRI *Tri );
void  SPDXSolGTri( TRI *Tri );
void  SPDXTexGTri( TRI *Tri );
void  SPDXSolPTri( TRI *Tri );
void  SPDXTexPTri( TRI *Tri );
void  SPDXSolBTri( TRI *Tri );
void  SPDXTexBTri( TRI *Tri );
void  SPDXEnvBTri( TRI *Tri );
void  SPDXTransparentSolFTri( TRI *Tri );
void  SPDXTransparentTexFTri( TRI *Tri );
void  SPDXTransparentSolGTri( TRI *Tri );
void  SPDXTransparentTexGTri( TRI *Tri );
void  SPDXTransparentSolPTri( TRI *Tri );
void  SPDXTransparentTexPTri( TRI *Tri );
void  SPDXTransparentSolBTri( TRI *Tri );
void  SPDXTransparentTexBTri( TRI *Tri );
void  SPDXTransparentEnvBTri( TRI *Tri );

/*----------------------------------------------------------------------------*/
// PRIMS.C

INT   SPDXInitScreen( void );
INT   SPDXGetVESAInfo( VESA *VESAInfoStruct );
INT   SPDXGetSVGAInfo( SVGA *SVGAInfoStruct, INT ModeNumber );
INT   SPDXSetMode( INT ModeNumber, INT ClearFlag, INT SquareFlag );
INT   SPDXGetMode( INT *ModeNumber );
void  SPDXCopyToScreen( CHR *CopyFrom );
void  SPDXCopyToScreenClipped( CHR *CopyFrom, int Left, int Top, int Right, int Bottom );
void  SPDXCopyFromScreen( CHR *CopyTo );
INT   SPDXSetVideoWindow( INT WindowFlag, INT WindowPos );
INT   SPDXGetVideoWindow( INT WindowFlag, INT *WindowPos );
INT   SPDXSetTextMode( void );
void  SPDXSetScreenPixel( UINT X, UINT Y, CHR Color );
UINT  SPDXGetScreenPixel( UINT X, UINT Y );
void  SPDXDrawScreenLine( INT X1, INT Y1, INT X2, INT Y2, CHR Color );
VFONT *SPDXRegisterFont( UBYT *Font );
void  SPDXDrawScreenText( VFONT *Font, CHR *String, INT X, INT Y );
void  SPDXConvertMap( BMAP *Map, INT Intensity );

/*----------------------------------------------------------------------------*/
// PROJECT.C

void  SPDXProjectHierarchy( OBJ *Object, OBJ *Cam, VSCR *VScreen );
void  SPDXCalcObjectProjection( OBJ *Object, VSCR *VScreen );
FLT   SPDXCalcFOV( FLT FocalLength );

/*----------------------------------------------------------------------------*/
// READUSD.C

INT   SPDXReadRawUSD( CHR *FileName, OBJ **Universe, PAL *MapPalette, FLT ShineValue );
INT   SPDXReadCompressedUSD( OBJ **Universe, INT Handle, PAL *MapPalette, FLT ShineValue );

/*----------------------------------------------------------------------------*/
// RENDER.C

INT   SPDXInitRenderer( void );
void  SPDXUninitRenderer( void );
INT   SPDXRenderHierarchy( OBJ *Object, OBJ *Cam, BYT ClearScreenFlag, BYT CopyFlag, BYT AntiAliasFlag, BYT SmoothFlag, BYT WaitVBFlag, VSCR *VScreen, VSCR *AAVScreen );
void  SPDXBuildRenderList( OBJ *Object, OBJ *Cam, VSCR *VScreen );
RLS   *SPDXGetRenderList();
FLT   SPDXCountPixelsInRenderList( RLS *RList );
INT   SPDXCountPolysInRenderList( RLS *RList);

/*----------------------------------------------------------------------------*/
// SAVEIMG.C

INT   SPDXWriteImage( CHR *FileName, IMAGE *Image, INT UpsideDownFlag );

/*----------------------------------------------------------------------------*/
// SHADE.C

void  SPDXShadeRenderList( RLS *RList );

/*----------------------------------------------------------------------------*/
// SORT.C

#ifndef U_Z
void  SPDXInitByteSortStack( void );
void  SPDXUninitByteSortStack( void );
void  SPDXByteSortRenderList( RLS *RList );
#endif

/*----------------------------------------------------------------------------*/
// STUFF.C

INT   SPDXSelectSTF( CHR *FileName, STFP *StuffPack );
void  SPDXReleaseSTF( STFP *StuffPack );
INT   SPDXOpenSTF( CHR *FileName, STFP *StuffPack );
void  SPDXCloseSTF( INT Handle );

/*----------------------------------------------------------------------------*/
// TEXFILL.ASM

void  SPDXTexRTriASMP(TRI *Tri, CHR *VScreen, INT LineSize);

/*----------------------------------------------------------------------------*/
// TRANS.C

void  SPDXTransformHierarchy( OBJ *Object );

/*----------------------------------------------------------------------------*/
// TRIS.C

INT   SPDXAddObjectTri( OBJ *Object );
INT   SPDXGetObjectTriCount( OBJ *Object );
void  SPDXSetTriVertices( OBJ *Object, INT Index, VER *V1, VER *V2, VER *V3);
void  SPDXGetTriVertices( OBJ *Object, INT Index, VER *V1, VER *V2, VER *V3);
void  SPDXSetTriNormal( OBJ *Object, INT Index );
void  SPDXGetTriNormal( OBJ *Object, INT Index, VEC *Normal );
void  SPDXSetTri( OBJ *Object, INT Index, TRI *Triangle );
TRI   *SPDXGetTri( OBJ *Object, INT Index );

/*----------------------------------------------------------------------------*/
// VERTICES.C

INT   SPDXAddObjectVertex( OBJ *Obj );
INT   SPDXGetObjectVertexCount( OBJ *Obj );
void  SPDXSetVertex( OBJ *Object, INT Index, VER *Vertex );
VER   *SPDXGetVertex( OBJ *Object, INT Index );

/*----------------------------------------------------------------------------*/
// VSCREEN.C

INT   SPDXInitVirtualScreen( INT ResX, INT ResY, VSCR *VScreen );
void  SPDXCleanupVirtualScreen(VSCR *VScreen);
void  SPDXSetVirtualClipping(INT Left, INT Right, INT Top, INT Bottom, VSCR *VScreen );
void  SPDXGetVirtualClipping(INT *Left, INT *Right, INT *Top, INT *Bottom, VSCR *VScreen );
void  SPDXGetVirtualCenter( INT *CenterX, INT *CenterY, VSCR *VScreen );
void  SPDXSetVirtualCenter(INT CenterX, INT CenterY, VSCR *VScreen );
void  SPDXSetVirtualResolution(INT ResX, INT ResY, VSCR *VScreen );
void  SPDXGetVirtualiResolution(INT *ResX, INT *ResY, VSCR *VScreen );
void  SPDXGetVirtualfResolution(FLT *ResX, FLT *ResY, VSCR *VScreen );
UCHR  *SPDXGetVirtualBuffer( VSCR *VScreen );
void  SPDXSetVirtualBuffer( VSCR *VScreen, UCHR *Buffer );
ZBUF  *SPDXGetVirtualZBuffer( VSCR *VScreen );
void  SPDXSetVirtualZBuffer( VSCR *VScreen, ZBUF *ZBuffer );
void  SPDXResetVirtualZBuffer( VSCR *VScreen );
void  SPDXClearVirtualScreen( CHR Color, VSCR *VScreen );
void  SPDXResetVirtualScreen( CHR Color, VSCR *VScreen );
void  SPDXSetVirtualPixel( UINT X, UINT Y, CHR Color, VSCR *VScreen );
UINT  SPDXGetVirtualPixel( UINT X, UINT Y, VSCR *VScreen );
void  SPDXDrawVirtualLine( INT X1, INT Y1, INT X2, INT Y2, CHR Color, VSCR *VScreen );
void  SPDXDrawVirtualText( VFONT *Font, CHR *String, INT X, INT Y, VSCR *VScreen );
void  SPDXSetVirtualScaleFactors( FLT ScaleX, FLT ScaleY, VSCR *VScreen );
void  SPDXGetVirtualScaleFactors( FLT *ScaleX, FLT *ScaleY, VSCR *VScreen );
void  SPDXSetVirtualMaxPolysToRender( INT Max, VSCR *VScreen );
INT   SPDXGetVirtualMaxPolysToRender( VSCR *VScreen );
void  SPDXSetVirtualLense( FLT Lense, VSCR *VScreen );
void  SPDXCorrectVirtualAspect( FLT XAspect, FLT YAspect, VSCR *VScreen );
void  SPDXVirtualAntiAlias( VSCR *Dest, VSCR *Src );
void  SPDXVirtualSmoothFrame( VSCR *VScreen );

/*----------------------------------------------------------------------------
  -   [SDXDECL.H   ] - End Of File                                           -
  ----------------------------------------------------------------------------*/
