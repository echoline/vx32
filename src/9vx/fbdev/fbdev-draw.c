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

/*
 * Replacements for libmemdraw routines.
 * (They've been underscored.)
 * TODO is this where acceleration would go?
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
	return _cloadmemimage(i, r, data, ndata);
}

void
memimagedraw(Memimage *dst, Rectangle r, Memimage *src, Point sp,
	Memimage *mask, Point mp, int op)
{
	Memdrawparam *par;

	if((par = _memimagedrawsetup(dst, r, src, sp, mask, mp, op)) == nil)
		return;

	_memimagedraw(par);

}

void
memfillcolor(Memimage *m, uint32 val)
{
	_memfillcolor(m, val);

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

