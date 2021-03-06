#include <linux/fb.h>
#include <linux/input.h>

typedef struct FBprivate FBprivate;
struct FBprivate {
	uint32		chan;
	int		fd;	/* of display */
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
	int		mousefd;
	Mouse		mouse;
	struct Cursor*	cursor;
	char		shift_state;
};

extern FBprivate _fb;

extern Memimage*	fbattach(int);
extern int		mouseattach(int);
extern int		mouseevent(struct input_event*);
extern void		ctrlc(int sig);
extern void		fbputc(int c);
extern void		termctl(uint32 o, int);
