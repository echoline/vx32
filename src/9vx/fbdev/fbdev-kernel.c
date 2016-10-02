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

#define Mask MouseMask|ExposureMask|StructureNotifyMask|KeyPressMask|EnterWindowMask|LeaveWindowMask

Rectangle windowrect;
Rectangle screenrect;
int fullscreen;

static void
_xproc(void *v)
{
	for(;;){
	}
}

void
screeninit(void)
{
}

uchar*
attachscreen(Rectangle *r, ulong *chan, int *depth,
	int *width, int *softscreen, void **X)
{
	Memimage *m;

	if(0){
		_memimageinit();
		if(_xattach("9vx", nil) == nil)
			panic("cannot connect to framebuffer: %r");
		kproc("*fbdev*", _xproc, nil);
	}
	m = _xattach("9vx", "640x480");
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
	return "";
}

void
putsnarf(char *data)
{
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

