#include "u.h"
#include "lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "error.h"
#define Image IMAGE	/* kernel has its own Image */
#include <draw.h>
#include <memdraw.h>
#include <keyboard.h>
#include <cursor.h>
#include "screen.h"
#include "mouse.h"
#include "fbdev-inc.h"

#include <gpm.h>

#define MouseMask (\
	ButtonPressMask|\
	ButtonReleaseMask|\
	PointerMotionMask|\
	Button1MotionMask|\
	Button2MotionMask|\
	Button3MotionMask)

Rectangle windowrect;
Rectangle screenrect;
int fullscreen;

static void
_fbproc(void *v)
{
	int c;

	for(;;){
		while((c = Gpm_Getc(stdin)) != EOF)
			kbdputc(kbdq, c);
	}
}

void
screeninit(void)
{
}

int fbgpmhandler(Gpm_Event *event, void *data)
{
//	printf("Event Type : %d at x=%d y=%d\n", event->type, event->x, event->y);
	return 0;       
}

uchar*
attachscreen(Rectangle *r, ulong *chan, int *depth,
	int *width, int *softscreen, void **X)
{
	Memimage *m;
	Gpm_Connect conn;

	if(_fb.screenimage == nil){
		_memimageinit();
		if(_fbattach("9vx", nil) == nil)
			panic("cannot connect to framebuffer: %r");

		conn.eventMask = ~0;
		conn.defaultMask = 0;
		conn.minMod = 0;
		conn.maxMod= ~0;

		if (Gpm_Open(&conn, 0) == -1)
			panic("cannot connect to gpm: %r");

		gpm_handler = fbgpmhandler;

		kproc("*fbdev*", _fbproc, nil);
	}
	m = _fb.screenimage;
	*r = m->r;
	*chan = m->chan;
	*depth = m->depth;
	*width = m->width;
	*X = m->x;
	*softscreen = 1;
	return m->data->bdata;
}

int
hwdraw(Memdrawparam *p)
{
	return 0;
}

void
getcolor(ulong i, ulong *r, ulong *g, ulong *b)
{
	ulong v;
	
	v = cmap2rgb(i);
	*r = (v>>16)&0xFF;
	*g = (v>>8)&0xFF;
	*b = v&0xFF;
}

int
setcolor(ulong i, ulong r, ulong g, ulong b)
{
	/* no-op */
	return 0;
}

char*
getsnarf(void)
{
	return _fb.snarfbuf? _fb.snarfbuf: "";
}

void
putsnarf(char *data)
{
	if (data) {
		if (_fb.snarfbuf)
			free(_fb.snarfbuf);
		_fb.snarfbuf = strdup(data);
	}
}

void
setmouse(Point p)
{
	drawqlock();
	drawqunlock();
}

void
setcursor(struct Cursor *c)
{
	drawqlock();
	drawqunlock();
}

