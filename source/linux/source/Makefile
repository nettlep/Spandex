#                        Makefile for The SPANDEX Library

U_D     = n                    # Debug options?
U_R     = y                    # Release flag?
U_P     = y                    # Perspective Correct flag?
U_Z     = n                    # Z-Buffer flag?
U_E     = n                    # Evaluation flag?
DEV_KIT = ../dev_kit           # This is where everything goes

AR = ar cr
CC = g++ -O3 -fomit-frame-pointer -fstrength-reduce -ffast-math -Wall -DU_R -c
#CC = g++ -O0 -Wall -DU_R -c

HEADERS = spandex.h sdxdecl.h sdxdefs.h sdxextrn.h sdxmacro.h sdxtypes.h sdxint86.h sdxfixed.h sdxmisc.h

SHAREDSUFFIX = a
LIBNAME = libspandex
LIBS = 	clip cull decoder draw encoder errors fileio globals image lights loadimg maps material math mem mouse neuquant objects octree orient palette points polys prims project readusd render saveimg shade sort spandex stuff trans tris vertices vscreen

INCLUDE_FILES = $(foreach libfile,$(LIBS),$(libfile).h)
OBJECT_FILES = $(foreach libfile,$(LIBS),$(libfile).o)
SRC_FILES = $(foreach libfile,$(LIBS),$(libfile).cpp)

$(LIBNAME).$(SHAREDSUFFIX): $(OBJECT_FILES)
	rm -f $@
	$(AR) $@ $^
	cp -f $(LIBNAME).$(SHAREDSUFFIX) $(DEV_KIT)
	cp *h $(DEV_KIT)

clean:
	-rm -f *.o *.$(SHAREDSUFFIX)

clip.o: clip.cpp
	$(CC) $^

cull.o: cull.cpp
	$(CC) $^

decoder.o: decoder.cpp
	$(CC) $^

draw.o: draw.cpp
	$(CC) $^

encoder.o: encoder.cpp
	$(CC) $^

errors.o: errors.cpp
	$(CC) $^

fileio.o: fileio.cpp
	$(CC) $^

globals.o: globals.cpp
	$(CC) $^

image.o: image.cpp
	$(CC) $^

int.o: int.cpp
	$(CC) $^

int86.o: int86.cpp
	$(CC) $^

lights.o: lights.cpp
	$(CC) $^

loadimg.o: loadimg.cpp
	$(CC) $^

maps.o: maps.cpp
	$(CC) $^

material.o: material.cpp
	$(CC) $^

math.o: math.cpp
	$(CC) $^

mem.o: mem.cpp
	$(CC) $^

mouse.o: mouse.cpp
	$(CC) $^

multi.o: multi.cpp
	$(CC) $^

neuquant.o: neuquant.cpp
	$(CC) $^

objects.o: objects.cpp
	$(CC) $^

octree.o: octree.cpp
	$(CC) $^

orient.o: orient.cpp
	$(CC) $^

palette.o: palette.cpp
	$(CC) $^

points.o: points.cpp
	$(CC) $^

polys.o: polys.cpp
	$(CC) $^

prims.o: prims.cpp
	$(CC) $^

project.o: project.cpp
	$(CC) $^

readusd.o: readusd.cpp
	$(CC) $^

render.o: render.cpp
	$(CC) $^

saveimg.o: saveimg.cpp
	$(CC) $^

shade.o: shade.cpp
	$(CC) $^

sort.o: sort.cpp
	$(CC) $^

spandex.o: spandex.cpp
	$(CC) $^

stuff.o: stuff.cpp
	$(CC) $^

trans.o: trans.cpp
	$(CC) $^

tris.o: tris.cpp
	$(CC) $^

vertices.o: vertices.cpp
	$(CC) $^

vscreen.o: vscreen.cpp
	$(CC) $^

