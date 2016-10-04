#include <linux/fb.h>

typedef struct FBprivate FBprivate;
struct FBprivate {
	uint32		chan;
	int		fd;	/* of display */
	int		mousefd;
	uchar*		fbp;
	int		depth;				/* of screen */
	Rectangle	newscreenr;
	Memimage*	screenimage;
	Memimage*	backbuf;
	Rectangle	screenr;
	uint		putsnarf;
	char*		snarfbuf;
	uint		assertsnarf;
	int		destroyed;
	struct fb_fix_screeninfo finfo;
	struct fb_var_screeninfo vinfo;
	long		screensize;
	struct Cursor*	cursor;
	char		shift_state;
};

extern FBprivate _fb;

extern Memimage*	_fbattach(char*, char*);

