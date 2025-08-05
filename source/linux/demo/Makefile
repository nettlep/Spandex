SPANDEX = ../dev_kit
TOOLS = ../tools

CC = g++ -O2 -Wall -I$(SPANDEX)
X11_DIR = /usr/X11R6

demo: demo.o xwin.o $(SPANDEX)/libspandex.a
	g++ demo.o xwin.o -L$(SPANDEX) -lspandex -o demo -L$(X11_DIR)/lib -lX11 -lXext

clean:
	rm -f demo *.o core *.tga *.map demo.usd *.dbg *.log *.stf

demo.map: default.tga
	$(TOOLS)/makemap demo.map default.tga -S0.75 -I0.4

demo.o: demo.cpp
	$(CC) -c demo.cpp -DU_R -I$(X11_DIR)/include 

xwin.o: xwin.cpp xwin.h
	$(CC) -c xwin.cpp -DU_R -I$(X11_DIR)/include
