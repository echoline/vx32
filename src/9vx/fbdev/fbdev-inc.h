/*
 * Structure pointed to by FB field of Memimage
 */
typedef struct FBmem FBmem;
struct FBmem
{
	Rectangle	r;	/* size of image */
};

typedef struct FBprivate FBprivate;
struct FBprivate {
	uint32		chan;
	int		fd;	/* of display */
	uchar*		fbp;
	int		depth;				/* of screen */
	uint32		map[256];
	uint32		map7[128];
	uchar		map7to8[128][2];
	Rectangle	newscreenr;
	Memimage*	screenimage;
	Rectangle	screenr;
	int		toplan9[256];
	int		tox11[256];
	int		usetable;
	uint		putsnarf;
	char*		snarfbuf;
	uint		assertsnarf;
	int		destroyed;
};

extern FBprivate _fb;

extern Memimage*	_fbattach(char*, char*);

#define MouseMask (\
	ButtonPressMask|\
	ButtonReleaseMask|\
	PointerMotionMask|\
	Button1MotionMask|\
	Button2MotionMask|\
	Button3MotionMask)

extern Rectangle screenrect;
extern Rectangle windowrect;
extern int fullscreen;

typedef struct Cursor Cursor;

enum
{
	PMundef = ~0
};
