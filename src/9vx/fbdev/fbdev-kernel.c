#include "u.h"
#include "lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"
#include "error.h"
#define Image IMAGE
#include <draw.h>
#include <memdraw.h>
#include <keyboard.h>
#include <cursor.h>
#include "screen.h"
#include "mouse.h"
#include "fbdev-inc.h"

#include <gpm.h>

Mouse fbmouse;

static void
fbputc(int c)
{
	kbdputc(kbdq, c);
}

static void
_fbproc(void *v)
{
	int c;

	for(;;) {
		c = Gpm_Getc(stdin);
		latin1putc(c, fbputc);
	}
}

void
screeninit(void)
{
}

int fbgpmhandler(Gpm_Event *event, void *data)
{
	struct timeval tv;
	Rectangle old, new;

	gettimeofday(&tv, nil);
	fbmouse.msec = tv.tv_usec / 1000 + tv.tv_sec * 1000;

	old.min.x = fbmouse.xy.x;
	old.min.y = fbmouse.xy.y;
	old.max.x = old.min.x + 16;
	old.max.y = old.min.y + 16;

	fbmouse.xy.x += event->dx * 3;
	fbmouse.xy.y += event->dy * 3;
	if (fbmouse.xy.x < 0)
		fbmouse.xy.x = 0;
	if (fbmouse.xy.y < 0)
		fbmouse.xy.y = 0;
	if (fbmouse.xy.x > _fb.screenimage->r.max.x)
		fbmouse.xy.x = _fb.screenimage->r.max.x;
	if (fbmouse.xy.y > _fb.screenimage->r.max.y)
		fbmouse.xy.y = _fb.screenimage->r.max.y;
	
	new.min.x = fbmouse.xy.x;
	new.min.y = fbmouse.xy.y;
	new.max.x = new.min.x + 16;
	new.max.y = new.min.y + 16;

	fbmouse.buttons = 0;
	if (fbmouse.buttons & 4)
		fbmouse.buttons |= 1;
	if (fbmouse.buttons & 2)
		fbmouse.buttons |= 2;
	if (fbmouse.buttons & 1)
		fbmouse.buttons |= 4;

	mousetrack(fbmouse.xy.x, fbmouse.xy.y, fbmouse.buttons, fbmouse.msec);

	combinerect(&new, old);
	if (new.min.x < 0)
		new.min.x = 0;
	if (new.min.y < 0)
		new.min.y = 0;
	if (new.max.x > _fb.screenimage->r.max.x)
		new.max.x = _fb.screenimage->r.max.x;
	if (new.max.y > _fb.screenimage->r.max.y)
		new.max.y = _fb.screenimage->r.max.y;

	flushmemscreen(new);

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

	printf("\x1b[?17;0;0c\n");

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
	fbmouse.xy = p;
}

void
setcursor(struct Cursor *c)
{
	_fb.cursor = c;
}

