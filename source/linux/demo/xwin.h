// Originally released under a custom license.
// This historical re-release is provided under the MIT License.
// See the LICENSE file in the repo root for details.
//
// https://github.com/nettlep



class	cXWin
{
private:
	Display		*display;
	Window		window;
	bool		initOK;
	int		screen, depth;

public:
			cXWin();
			~cXWin();
	bool		init();
	void		uninit();
	bool		verifyInit();
	bool		create(const int x = 0, const int y = 0, const unsigned int w = 320, const unsigned int h = 200, const int border = 3, char *windowName = "untitled");
	void		getGeometry(int &x, int &y, unsigned int &width, unsigned int &height) const;
	int		getX() const;
	int		getY() const;
	unsigned int	getWidth() const;
	unsigned int	getHeight() const;
	bool		preparePalette(char *palette);
	bool		showImage(char *data, char *palette);
};
