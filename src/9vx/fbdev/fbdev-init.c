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

#include <sys/ioctl.h>
#include <sys/mman.h>

FBprivate _fb;

extern Mouse fbmouse;

void
_fbput(Memimage *m, Rectangle r) {
	int x, y;

	for (x = r.min.x; x < r.max.x; x++) for (y = r.min.y; y < r.max.y; y++){
		long fbloc = (x + y * m->width) * 4;
		uint32 pixel = *((uint32*)(m->data->bdata + fbloc));

		*((uint32*)(_fb.fbp + fbloc)) = pixel;
	}
}

Memimage*
_fbattach(char *label, char *winsize)
{
	Rectangle r;

	/*
	 * Connect to /dev/fb0
	 */
	if ((_fb.fd = open("/dev/fb0", O_RDWR)) < 0)
		goto err;

	if (ioctl(_fb.fd, FBIOGET_VSCREENINFO, &(_fb.vinfo)) < 0)
		goto err;
	_fb.vinfo.grayscale=0;
	_fb.vinfo.bits_per_pixel=32;
	if (ioctl(_fb.fd, FBIOPUT_VSCREENINFO, &(_fb.vinfo)) < 0)
		goto err;

	if (ioctl(_fb.fd, FBIOGET_FSCREENINFO, &(_fb.finfo)) < 0)
		goto err;
	_fb.screensize = _fb.vinfo.yres_virtual * _fb.finfo.line_length;
	if ((_fb.fbp = mmap(0, _fb.screensize, PROT_READ | PROT_WRITE, MAP_SHARED, _fb.fd, (off_t)0)) < 0)
		goto err;
	/*
	 * Figure out underlying screen format.
	 */
	r = Rect(0, 0, _fb.vinfo.xres_virtual, _fb.vinfo.yres_virtual);
	mouserect = r;

	_fb.screenimage = allocmemimage(r, XRGB32);
	_fb.backbuf = allocmemimage(r, XRGB32);
	termreplacescreenimage(_fb.backbuf);
	return _fb.backbuf;

err:
	return nil;
}

void
flushmemscreen(Rectangle r)
{
	int x, y, i;
	Point p;
	long fbloc;
	int x2, y2;

	memimagedraw(_fb.screenimage, r, _fb.backbuf, r.min, nil, r.min, S);

	if (_fb.cursor != nil) {
		p = fbmouse.xy;

		for (x = 0; x < 16; x++) {
			x2 = x + _fb.cursor->offset.x;

			if ((p.x + x2) < 0)
				continue;

			if ((p.x + x2) >= _fb.screenimage->r.max.x)
				break;

			for (y = 0; y < 16; y++) {
				y2 = y + _fb.cursor->offset.y;

				if ((p.y + y2) < 0)
					continue;

				if ((p.y + y2) >= _fb.screenimage->r.max.y)
					break;

				i = y * 2 + x / 8;
				fbloc = ((p.y+y2) * _fb.screenimage->r.max.x + (p.x+x2)) * 4;

				if (_fb.cursor->clr[i] & (128 >> (x % 8))) {
					*((uint32*)(_fb.screenimage->data->bdata + fbloc)) = 0xFFFFFFFF;
				}

				if (_fb.cursor->set[i] & (128 >> (x % 8))) {
					*((uint32*)(_fb.screenimage->data->bdata + fbloc)) = 0xFF000000;
				}
			}
		}
	}

	_fbput(_fb.screenimage, r);
}

