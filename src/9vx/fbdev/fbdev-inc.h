/*
 * Structure pointed to by X field of Memimage
 */
typedef struct Xmem Xmem;
struct Xmem
{
	Rectangle	r;	/* size of image */
};

typedef struct Xprivate Xprivate;
struct Xprivate {
	uint32		chan;
	int		fd;	/* of display */
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
	uint		assertsnarf;
	int		destroyed;
};

extern Xprivate _x;

extern Memimage*	_xattach(char*, char*);

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
