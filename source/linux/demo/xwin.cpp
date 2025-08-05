// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep


#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "xwin.h"

#define	LOG_INFO

// ----------------------------------------------------------------------------

		cXWin::cXWin()
{
	display = NULL;
	screen = 0;
	initOK = false;

	init();
}

// ----------------------------------------------------------------------------

		cXWin::~cXWin()
{
	uninit();
}

// ----------------------------------------------------------------------------

bool		cXWin::init()
{
	initOK = false;

	display = XOpenDisplay(NULL);

	if (!display)
	{
		printf( "XOpenDisplay failed to connect to '%s'\n", XDisplayName(NULL) );
		return false; 
	}

	screen = DefaultScreen( display );
	depth  = DefaultDepth( display, screen );

	#ifdef	LOG_INFO
	int	width  = DisplayWidth( display, screen );
	int	height = DisplayHeight( display, screen );

	printf( "Display (%s) is %dx%d, %d bits\n", XDisplayName(NULL), width, height, depth );
	#endif

	if (depth < 8)
	{
		printf( "Not enough color depth (%d), 8 bits required\n", depth );
		return false;
	}

	initOK = true;
	return true;
}

// ----------------------------------------------------------------------------

void		cXWin::uninit()
{
	if (!initOK) return;

	if (window)
	{
		XDestroyWindow(display, window);
		window = 0;
	}

	if (display)
	{
		XCloseDisplay(display);
		display = NULL;
	}

	screen = 0;

	initOK = false;
}

// ----------------------------------------------------------------------------

bool		cXWin::verifyInit()
{
	return initOK;
}

// ----------------------------------------------------------------------------

bool		cXWin::create(const int x, const int y, const unsigned int w, const unsigned int h, const int border, char *windowName)
{
	Window			parent = RootWindow(display, screen);
	XSetWindowAttributes	winAttr;
	unsigned long		winMask;

	winAttr.border_pixel = BlackPixel(display, screen);
	winAttr.background_pixel = BlackPixel(display, screen);
	winAttr.override_redirect = True;
	//winMask = (CWBackPixel | CWBorderPixel | CWOverrideRedirect);
	winMask = (CWBackPixel | CWBorderPixel);

	window = XCreateWindow(display, parent, x, y, w, h, border, depth, InputOutput, CopyFromParent, winMask, &winAttr );

	if (!window)
	{
		printf( "Unable to create the window\n" );
		return false;
	}

	#ifdef LOG_INFO
	printf("window #%ld\n", window);
	#endif

	XWMHints	*wmHints = XAllocWMHints();
	wmHints->initial_state = NormalState;
	wmHints->flags = StateHint;
	XSetWMHints(display, window, wmHints );
	XFree(wmHints);

	XSizeHints	*sHints = XAllocSizeHints();
	sHints->flags = PPosition | PSize | PBaseSize | PMinSize;
	sHints->x = x;
	sHints->y = y;
	sHints->width = w;
	sHints->height = h;
	sHints->min_width = 10;
	sHints->min_height = 10;
	sHints->base_width = w;
	sHints->base_height = h;
	XSetWMNormalHints(display, window, sHints);
	XFree(sHints);

	XClassHint	*cHints = XAllocClassHint();
	cHints->res_name = windowName;
	cHints->res_class = "XLogo";
	XSetClassHint(display, window, cHints);
	XFree(cHints);

	XStoreName(display, window, windowName);

	XMapRaised(display, window);
	XMapSubwindows(display, window);
	XFlush(display);

	return true;
}

// ----------------------------------------------------------------------------

void		cXWin::getGeometry(int &x, int &y, unsigned int &width, unsigned int &height) const
{
	Window		root;
	unsigned int	border, depth;
	XGetGeometry(display, window, &root, &x, &y, &width, &height, &border, &depth);
}

// ----------------------------------------------------------------------------

int		cXWin::getX() const
{
	int		x, y;
	unsigned int	w, h;
	getGeometry(x, y, w, h);
	return x;
}

// ----------------------------------------------------------------------------

int		cXWin::getY() const
{
	int		x, y;
	unsigned int	w, h;
	getGeometry(x, y, w, h);
	return y;
}

// ----------------------------------------------------------------------------

unsigned int	cXWin::getWidth() const
{
	int		x, y;
	unsigned int	w, h;
	getGeometry(x, y, w, h);
	return w;
}

// ----------------------------------------------------------------------------

unsigned int	cXWin::getHeight() const
{
	int		x, y;
	unsigned int	w, h;
	getGeometry(x, y, w, h);
	return h;
}

// ----------------------------------------------------------------------------

bool		cXWin::preparePalette(char *palette)
{
	switch(depth)
	{
		case 15:
			for (int i = 0; i < 256; i++)
			{
				palette[i*3+0] >>= 1;
				palette[i*3+1] >>= 1;
				palette[i*3+2] >>= 1;
			}
			break;

		case 16:
			for (int i = 0; i < 256; i++)
			{
				palette[i*3+0] >>= 1;
				palette[i*3+2] >>= 1;
			}
			break;

		case 24:
		case 32:
			for (int i = 0; i < 256; i++)
			{
				palette[i*3+0] <<= 2;
				palette[i*3+1] <<= 2;
				palette[i*3+2] <<= 2;
			}
			break;

		default:
			printf( "Invalid bit depth for cXWin::preparePalette():  %d\n", depth);
			return false;
	}

	return true;
}

// ----------------------------------------------------------------------------

bool		cXWin::showImage(char *data, char *palette)
{
	unsigned int	width = getWidth();
	unsigned int	height = getHeight();
	XImage		*image = NULL;

	if (depth == 8)
	{
		unsigned int	size = width * height;
		unsigned char	*dest = new unsigned char[size];
		unsigned char	*d = dest;
		unsigned char	*s = (unsigned char *) data;

		for (unsigned int i = 0; i < size; i++)
		{
			int	c = *(s++);
			*(d++) = palette[c+c+c];
		}

		image = XCreateImage(display, CopyFromParent, depth, ZPixmap, 0, (char *) dest, width, height, 8, width);

		if (!image)
		{
			delete dest;
			printf( "Unable to create image\n" );
			return false;
		}
	}
	else if (depth == 16 || depth == 15)
	{
		unsigned int	size = width * height;
		unsigned short	*dest = new unsigned short[size];
		unsigned short	*d = dest;
		unsigned char	*s = (unsigned char *) data;

		for (unsigned int i = 0; i < size; i++)
		{
			int	c = *(s++);
			c += c + c;
			*(d++) = (palette[c+0]<<11) + (palette[c+1]<<5) + palette[c+2];
		}
	
		image = XCreateImage(display, CopyFromParent, depth, ZPixmap, 0, (char *) dest, width, height, 16, width * 2);

		if (!image)
		{
			delete dest;
			printf( "Unable to create image\n" );
			return false;
		}
	}
	else if (depth == 24 || depth == 32)
	{
		unsigned int	size = width * height;
		unsigned char	*dest = new unsigned char[size * 4];
		unsigned char	*d = dest;
		unsigned char	*s = (unsigned char *) data;

		for (unsigned int i = 0; i < size; i++)
		{
			int	c = *(s++);
			c += c + c;
			*(d++) = palette[c+0];
			*(d++) = palette[c+1];
			*(d++) = palette[c+2];
			d++;
		}

		image = XCreateImage(display, CopyFromParent, depth, ZPixmap, 0, (char *) dest, width, height, 8, width * 4);

		if (!image)
		{
			delete dest;
			printf( "Unable to create image\n" );
			return false;
		}
	}
	else
	{
		printf( "Invalid bit depth for cXWin::showImage(): %d\n", depth);
		return false;
	}


	XGCValues	gcval;
	gcval.foreground = 0;
	gcval.background = 0;
	GC		gc = XCreateGC(display, window, GCForeground | GCBackground, &gcval);

	if (!gc)
	{
		XDestroyImage(image);
		printf( "Unable to create GC\n");
		return false;
	}

	XPutImage(display, window, gc, image, 0, 0, 0, 0, width, height);
	XFlush(display);
	XFreeGC(display, gc);
	XDestroyImage(image);

	return true;
}

// ----------------------------------------------------------------------------
