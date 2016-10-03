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
	Memimage *m = _fb.screenimage;
	Rectangle r = m->r;
	static Lock flushlock;
	int x, y;

	for(;;){
		lock(&flushlock);
		for (x = r.min.x; x < r.max.x; x++) for (y = r.min.y; y < r.min.y; y++){
			long fbloc = (x+_fb.vinfo.xoffset) * (_fb.vinfo.bits_per_pixel/8) + (y+_fb.vinfo.yoffset) * _fb.finfo.line_length;
			long miloc = (y*m->width) + x*4;

			*((uint32*)(_fb.fbp + fbloc)) = 0x0000FF00; //*((uint32*)(m->data->bdata + miloc));
		}
		unlock(&flushlock);
		usleep(100);
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

	if(_fb.screenimage == nil){
		_memimageinit();
		if(_fbattach("9vx", nil) == nil)
			panic("cannot connect to framebuffer: %r");
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

