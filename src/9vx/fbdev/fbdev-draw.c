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
#include "fbdev-inc.h"

void
fbput(Memimage *m, Rectangle r) {
	int x, y;

	for (x = r.min.x; x < r.max.x; x++) for (y = r.min.y; y < r.min.y; y++){
		long fbloc = (x+_fb.vinfo.xoffset) * (_fb.vinfo.bits_per_pixel/8) + (y+_fb.vinfo.yoffset) * _fb.finfo.line_length;
		uint32 pixel = *((uint32*)(m->data->bdata + (y*m->width) + (x*4)));

		*((uint32*)(_fb.fbp + fbloc)) = 0x000000FF; //pixel;
	}
}

/*
 * Replacements for libmemdraw routines.
 * (They've been underscored.)
 */
Memimage*
allocmemimage(Rectangle r, uint32 chan)
{
	return _allocmemimage(r, chan);
}

void
freememimage(Memimage *m)
{
	if (m == nil)
		return;

	_freememimage(m);
}


int
cloadmemimage(Memimage *i, Rectangle r, uchar *data, int ndata)
{
	int n;

	n = _cloadmemimage(i, r, data, ndata);
	if(n > 0)
		fbput(i, r);
	return n;
}

void
memimagedraw(Memimage *dst, Rectangle r, Memimage *src, Point sp,
	Memimage *mask, Point mp, int op)
{
	Memdrawparam *par;

	if((par = _memimagedrawsetup(dst, r, src, sp, mask, mp, op)) == nil)
		return;

	/* now can run memimagedraw on the in-memory bits */
	_memimagedraw(par);

	fbput(par->dst, par->r);
}

void
memfillcolor(Memimage *m, uint32 val)
{
	_memfillcolor(m, val);

	fbput(m, m->r);
}

static void
addrect(Rectangle *rp, Rectangle r)
{
	if(rp->min.x >= rp->max.x)
		*rp = r;
	else
		combinerect(rp, r);
}

int
loadmemimage(Memimage *i, Rectangle r, uchar *data, int ndata)
{
	return _loadmemimage(i, r, data, ndata);
}

int
unloadmemimage(Memimage *i, Rectangle r, uchar *data, int ndata)
{
	return _unloadmemimage(i, r, data, ndata);
}

uint32
pixelbits(Memimage *m, Point p)
{
	return _pixelbits(m, p);
}
