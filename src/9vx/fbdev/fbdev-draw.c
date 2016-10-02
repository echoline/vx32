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
	_freememimage(m);
}


int
cloadmemimage(Memimage *i, Rectangle r, uchar *data, int ndata)
{
	return _cloadmemimage(i, r, data, ndata);
}

static int xdraw(Memdrawparam*);

/*
 * The X acceleration doesn't fit into the standard hwaccel
 * model because we have the extra steps of pulling the image
 * data off the server and putting it back when we're done.
 */
void
memimagedraw(Memimage *dst, Rectangle r, Memimage *src, Point sp,
	Memimage *mask, Point mp, int op)
{
	Memdrawparam *par;

	if((par = _memimagedrawsetup(dst, r, src, sp, mask, mp, op)) == nil)
		return;

	/* now can run memimagedraw on the in-memory bits */
	_memimagedraw(par);

	if(xdraw(par))
		return;
}

static int
xdraw(Memdrawparam *par)
{
	uint state;
	Memimage *src, *dst, *mask;
	Rectangle r;

	if(par->dst->x == nil)
		return 0;

	dst   = par->dst;
	mask  = par->mask;
	r     = par->r;
	src   = par->src;
	state = par->state;

	/*
	 * Can't accelerate.
	 */
	return 0;
}


void
memfillcolor(Memimage *m, uint32 val)
{
	_memfillcolor(m, val);
	if(m->x == nil)
		return;
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
