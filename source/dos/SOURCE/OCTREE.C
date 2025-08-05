// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±                                                                          ±
  ±   Copyright (c) 1997 Paul D. Nettle.  All Rights Reserved.               ±
  ±                                                                          ±
  ±   [OCTREE.C    ] - Octree Code                                           ±
  ±                                                                          ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <string.h>
#include <math.h>

#include "spandex.h"

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

#define MaxColormapSize 65535L
#define RGBColorspace   1
#define PseudoClass     2
#define GRAYColorspace  2
#define MaxTextLength   2048
#define color_number    number_colors
#define MaxRGB          255
#define MaxNodes        266817
#define MaxShift        8
#define MaxTreeDepth    8
#define NodesInAList    2048

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

typedef struct _RunlengthPacket
{
   UWRD  red, green, blue, length;
} RunlengthPacket;

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

typedef struct _ColorPacket
{
   UBYT  red, green, blue;
} ColorPacket;

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

typedef struct _Node
{
   struct   _Node *parent, *child[8];
   UBYT     id, level, census, mid_red, mid_green, mid_blue;
   ULNG     number_colors, number_unique, total_red, total_green, total_blue;
} Node;

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

typedef struct _Nodes
{
   Node        nodes[NodesInAList];
   struct      _Nodes *next;
} Nodes;

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

typedef struct _Cube
{
   Node        *root;
   ColorPacket color, *colormap;
   UINT        depth;
   ULNG        colors, pruning_threshold, next_pruning_threshold, distance, squares[MaxRGB+MaxRGB+1];
   UINT        shift[MaxTreeDepth+1], nodes, free_nodes, color_number;
   Node        *next_node;
   Nodes       *node_queue;
} Cube;

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static Cube cube;

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static void Assignment(IMAGE *image, UINT number_colors);
static void Classification(IMAGE *image);
static void Colormap(Node *node);
static void InitializeCube(UINT number_pixels, UINT number_colors, UINT tree_depth, UINT adjust);
static Node *InitializeNode(UINT id, UINT level, Node *parent, UINT mid_red, UINT mid_green, UINT mid_blue);
static void PruneChild(Node *node);
static void PruneLevel(Node *node);
static void Reduce(Node *node);
static void Reduction(UINT number_colors);
static void ClosestColor(Node *node);

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static void Assignment(IMAGE *image, UINT number_colors)
{
INT   i, ImageSize;
Node  *node;
CHR   *p, *q;
UINT  id;

   FUNC("Assignment");
   Assert( image );

   number_colors = number_colors; // bypass compiler warning

   // Allocate image colormap.

   if (cube.colormap != (ColorPacket *) NULL)
      SPDXFree((CHR *) cube.colormap);

   cube.colormap = (ColorPacket *) SPDXMalloc((UINT) cube.colors * sizeof(ColorPacket));

   if (cube.colormap == (ColorPacket *) NULL)
      SPDXFatalError(LE_NOMEM, "Unable to quantize image");

   cube.colors=0;
   Colormap(cube.root);

   // Create a reduced color image.

   p = q = image->Buffer;
   ImageSize = image->ResX * image->ResY;

   for (i = 0; i < ImageSize; i++, p += 3, q++)
   {
      // Identify the deepest node containing the pixel's color.
      node = cube.root;

      FOREVER
      {
         id = (p[0] > node->mid_red   ? 1 : 0) |
              (p[1] > node->mid_green ? 1 : 0) << 1 |
              (p[2] > node->mid_blue  ? 1 : 0) << 2;

         if ((node->census & (1 << id)) == 0)
            break;

         node=node->child[id];
      }

      // Find closest color among siblings and their children.

      cube.color.red   = p[0];
      cube.color.green = p[1];
      cube.color.blue  = p[2];

      cube.distance = (ULNG) (~0);

      ClosestColor(node->parent);

      *q = (CHR) cube.color_number;
   }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static void Classification(IMAGE *image)
{
INT   i, ImageSize;
Node  *node;
CHR   *p;
UINT  bisect, id, level;

   FUNC("Classification");
   Assert( image );

   p = image->Buffer;
   ImageSize = image->ResX * image->ResY;

   for (i = 0; i < ImageSize; i++, p += 3)
   {
      if (cube.nodes > MaxNodes)
      {
         // Prune one level if the color tree is too large.
         PruneLevel(cube.root);
         cube.depth--;
      }

      // Start at the root and descend the color cube tree.
      node=cube.root;

      for (level = 1; level <= cube.depth; level++)
      {
         id = (p[0] > node->mid_red   ? 1 : 0) |
              (p[1] > node->mid_green ? 1 : 0) << 1 |
              (p[2] > node->mid_blue  ? 1 : 0) << 2;

         if (node->child[id] == (Node *) NULL)
         {
            // Set colors of new node to contain pixel.

            node->census |= (UBYT) (1 << id);
            bisect=(UINT) (1 << (MaxTreeDepth-level)) >> 1;

            node->child[id] = InitializeNode(id,level,node,
               node->mid_red   + (id & 1 ? bisect : -bisect),
               node->mid_green + (id & 2 ? bisect : -bisect),
               node->mid_blue  + (id & 4 ? bisect : -bisect));

            if (node->child[id] == (Node *) NULL)
               SPDXFatalError(LE_NOMEM, "Unable to quantize image");

            if (level == cube.depth)
               cube.colors++;
         }

         // Record the number of colors represented by this node.  Shift by level
         // in the color description tree.

         node = node->child[id];
         node->number_colors += 1 << cube.shift[level];
      }

      // Increment unique color count and sum RGB values for this leaf for later
      // derivation of the mean cube color.

      node->number_unique++;
      node->total_red   += p[0];
      node->total_green += p[1];
      node->total_blue  += p[2];
   }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static void ClosestColor(Node *node)
{
UINT        id;
ColorPacket *color;
UINT        blue_distance, green_distance, red_distance;
ULNG       distance;

   FUNC("ClosestColor");
   Assert( node );

   // Traverse any children.

   if (node->census != 0)
      for (id = 0; id < 8; id++)
         if (node->census & (1 << id))
            ClosestColor(node->child[id]);

   if (node->number_unique != 0)
   {
      // Determine if this color is "closest".

      color = cube.colormap + node->color_number;
      red_distance = (INT) color->red - (INT) cube.color.red + MaxRGB;
      green_distance = (INT) color->green - (INT) cube.color.green + MaxRGB;
      blue_distance = (INT) color->blue - (INT) cube.color.blue + MaxRGB;
      distance = cube.squares[red_distance] + cube.squares[green_distance]+
      cube.squares[blue_distance];

      if (distance < cube.distance)
      {
         cube.distance = distance;
         cube.color_number = (UWRD) node->color_number;
      }
   }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static void Colormap(Node *node)
{
UINT  id;

   FUNC("Colormap");
   Assert( node );

   // Traverse any children.

   if (node->census != 0)
      for (id=0; id < 8; id++)
         if (node->census & (1 << id))
            Colormap(node->child[id]);

   if (node->number_unique != 0)
   {
      // Colormap entry is defined by the mean color in this cube.

      cube.colormap[cube.colors].red   = (UBYT)
      ((node->total_red   + (node->number_unique >> 1)) / node->number_unique);

      cube.colormap[cube.colors].green = (UBYT)
      ((node->total_green + (node->number_unique >> 1)) / node->number_unique);

      cube.colormap[cube.colors].blue  = (UBYT)
      ((node->total_blue  + (node->number_unique >> 1)) / node->number_unique);

      node->color_number = cube.colors++;
   }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static void InitializeCube(UINT number_pixels, UINT number_colors, UINT tree_depth, UINT adjust)
{
CHR   c;
INT   i;
UINT  bits, level, max_shift;

   FUNC("InitializeCube");
   // Initialize tree to describe color cube.  Depth is: Log4(colormap size)+2;

   cube.node_queue = (Nodes *) NULL;
   cube.nodes = 0;
   cube.free_nodes = 0;

   if (tree_depth == 0)
   {
      for (tree_depth = 1; number_colors != 0; tree_depth++)
         number_colors >>= 2;

      if (tree_depth > adjust)
         tree_depth -= adjust;
   }

   cube.depth = tree_depth;
   cube.depth = MIN(cube.depth, 8);
   cube.depth = MAX(cube.depth, 2);

   // Initialize the shift values.
   for(c = 1, bits = 0; c != (CHR) 0; bits++)
      c <<= 1;

   for(max_shift = sizeof(UINT) *bits; number_pixels != 0; max_shift--)
      number_pixels>>=1;

   for (level = 0; level <= cube.depth; level++)
   {
      cube.shift[level] = max_shift;

      if (max_shift != 0)
         max_shift--;
   }

   // Initialize root node.

   cube.root = InitializeNode(0, 0, (Node *) NULL, (MaxRGB+1) >> 1,
                              (MaxRGB+1) >> 1, (MaxRGB+1) >> 1);

   if (cube.root == (Node *) NULL)
      SPDXFatalError(LE_NOMEM, "Unable to quantize image");

   cube.root->parent = cube.root;
   cube.root->number_colors = (ULNG) (~0);
   cube.colors = 0;

   // Initialize the square values.

   for (i = (-MaxRGB); i <= MaxRGB; i++)
      cube.squares[i+MaxRGB] = i*i;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static Node *InitializeNode(UINT id, UINT level, Node *parent, UINT mid_red, UINT mid_green, UINT mid_blue)
{
INT   i;
Node  *node;
Nodes *nodes;

   FUNC("InitializeNode");
   if (cube.free_nodes == 0)
   {
      // Allocate a new nodes of nodes.

      nodes = (Nodes *) SPDXMalloc(sizeof(Nodes));

      if (nodes == (Nodes *) NULL)
         return NULL;

      nodes->next     = cube.node_queue;
      cube.node_queue = nodes;
      cube.next_node  = nodes->nodes;
      cube.free_nodes = NodesInAList;
   }

   cube.nodes++;
   cube.free_nodes--;
   node = cube.next_node++;
   node->parent = parent;

   for (i = 0; i < 8; i++)
      node->child[i] = (Node *) NULL;

   node->id = (UBYT) id;
   node->level = (UBYT) level;
   node->census = 0;
   node->mid_red = (UBYT) mid_red;
   node->mid_green = (UBYT) mid_green;
   node->mid_blue = (UBYT) mid_blue;
   node->number_colors = 0;
   node->number_unique = 0;
   node->total_red = 0;
   node->total_green = 0;
   node->total_blue = 0;
   return node;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static void PruneChild(Node *node)
{
Node  *parent;

   FUNC("PruneChild");
   Assert( node );

  // Merge color statistics into parent.

  parent = node->parent;
  parent->census &= (UBYT) (~(1 << node->id));
  parent->number_unique += node->number_unique;
  parent->total_red += node->total_red;
  parent->total_green += node->total_green;
  parent->total_blue += node->total_blue;
  cube.nodes--;
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static void PruneLevel(Node *node)
{
INT   id;

   FUNC("PruneLevel");
   Assert( node );

   // Traverse any children.
   if (node->census != 0)
      for (id = 0; id < 8; id++)
         if (node->census & (1 << id))
            PruneLevel(node->child[id]);

   if (node->level == cube.depth)
      PruneChild(node);
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static void Reduce(Node *node)
{
UINT  id;

   FUNC("Reduce");
   Assert( node );

   //  Traverse any children.

   if (node->census != 0)
      for (id = 0; id < 8; id++)
         if (node->census & (1 << id))
            Reduce(node->child[id]);

   if (node->number_colors <= cube.pruning_threshold)
   {
      PruneChild(node);
   }
   else
   {
      // Find minimum pruning threshold.

      if (node->number_unique > 0)
         cube.colors++;

      if (node->number_colors < cube.next_pruning_threshold)
         cube.next_pruning_threshold = node->number_colors;
   }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

static void Reduction(UINT number_colors)
{
   FUNC("Reduction");
   cube.next_pruning_threshold=1;

   while (cube.colors > number_colors)
   {
      cube.pruning_threshold = cube.next_pruning_threshold;
      cube.next_pruning_threshold = cube.root->number_colors-1;
      cube.colors = 0;
      Reduce(cube.root);
   }
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXQuantizeImage(IMAGE *image, UINT number_colors, UINT tree_depth, UBYT *Palette)
{
Nodes *nodes;

   FUNC("SPDXQuantizeImage");
   Assert( image );
   Assert( Palette );

   cube.colormap = (ColorPacket *) NULL;

   // Reduce the number of colors in the continuous tone image.

   if (number_colors > MaxColormapSize)
      number_colors = MaxColormapSize;

   InitializeCube(image->ResX * image->ResY, number_colors, tree_depth, 0);
   Classification(image);
   Reduction(number_colors);
   Assignment(image, number_colors);

   SPDXMemSetBYTE( Palette, 0, number_colors*3 );
   SPDXMemCopyBYTE( Palette, cube.colormap, cube.colors*3 );

   // Release color cube tree storage.
   do
   {
      nodes = cube.node_queue->next;
      (void) SPDXFree((CHR *) cube.node_queue);
      cube.node_queue = nodes;
   }
   while (cube.node_queue != (Nodes *) NULL);

   if (cube.colormap != (ColorPacket *) NULL)
      SPDXFree((CHR *) cube.colormap);
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  SPDXQuantizeImages(IMAGE **images, UINT AddImages, UINT TotalImages, UINT number_colors, UINT tree_depth, UBYT *Palette)
{
Nodes *nodes;
UINT  i;

   FUNC("SPDXQuantizeImages");
   Assert( images );
   Assert( Palette );

   cube.colormap = (ColorPacket *) NULL;

   // Reduce the number of colors in the continuous tone image sequence.

   InitializeCube((UINT) (~0),number_colors,tree_depth,2);

   for (i = 0; i < AddImages; i += MAX((AddImages+16) >> 5, 1))
       Classification(images[i]);
   
   Reduction(number_colors);

   for (i = 0; i < TotalImages; i++)
      Assignment(images[i], number_colors);

   SPDXMemSetBYTE( Palette, 0, number_colors*3 );
   SPDXMemCopyBYTE( Palette, cube.colormap, cube.colors*3 );

   // Release color cube tree storage.
   do
   {
      nodes = cube.node_queue->next;
      (void) SPDXFree((CHR *) cube.node_queue);
      cube.node_queue = nodes;
   }
   while (cube.node_queue != (Nodes *) NULL);

   if (cube.colormap != (ColorPacket *) NULL)
      SPDXFree((CHR *) cube.colormap);
}

/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/

void  _SPDXMakeTransTable(IMAGE *PaletteTable, IMAGE *TransTable, UINT number_colors, UINT tree_depth, UBYT *Palette, FLT Opacity)
{
INT   i, j;
CHR   *TempPtr;
Nodes *nodes;
FLT   red, green, blue;

   FUNC("_SPDXMakeTransTable");
   Assert( PaletteTable );
   Assert( TransTable );
   Assert( Palette );

   cube.colormap = (ColorPacket *) NULL;

   // Reduce the number of colors in the continuous tone image.

   if (number_colors > MaxColormapSize)
      number_colors = MaxColormapSize;

   InitializeCube((UINT) (~0),number_colors,tree_depth,2);

   Classification(PaletteTable);
   Classification(TransTable);

   Reduction(number_colors);

   Assignment(PaletteTable, number_colors);
   Assignment(TransTable, number_colors);

   SPDXMemSetBYTE( Palette, 0, number_colors*3 );
   SPDXMemCopyBYTE( Palette, cube.colormap, cube.colors*3 );

   TempPtr = TransTable->Buffer;

   // Make the trans table
   for (i = 0; i < number_colors; i++)
   {
      for( j = 0; j < number_colors; j++)
      {
         red   = (FLT) Palette[j*3+0]*(1.0-Opacity) +
                 (FLT) Palette[i*3+0]*Opacity;
         green = (FLT) Palette[j*3+1]*(1.0-Opacity) +
                 (FLT) Palette[i*3+1]*Opacity;
         blue  = (FLT) Palette[j*3+2]*(1.0-Opacity) +
                 (FLT) Palette[i*3+2]*Opacity;

         if ((UBYT) red > 63)
            TempPtr[i*number_colors*3+j*3+0] = 63;
         else
            TempPtr[i*number_colors*3+j*3+0] = ((UBYT) (red+0.5));

         if ((UBYT) red > 63)
            TempPtr[i*number_colors*3+j*3+1] = 63;
         else
            TempPtr[i*number_colors*3+j*3+1] = ((UBYT) (green+0.5));

         if ((UBYT) red > 63)
            TempPtr[i*number_colors*3+j*3+2] = 63;
         else
            TempPtr[i*number_colors*3+j*3+2] = ((UBYT) (blue+0.5));
      }
   }

   Assignment(TransTable, number_colors);

   // Release color cube tree storage.
   do
   {
      nodes = cube.node_queue->next;
      (void) SPDXFree((CHR *) cube.node_queue);
      cube.node_queue = nodes;
   }
   while (cube.node_queue != (Nodes *) NULL);

   if (cube.colormap != (ColorPacket *) NULL)
      SPDXFree((CHR *) cube.colormap);
}


/*±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
  ±   [OCTREE.C    ] - End Of File                                           ±
  ±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±*/
