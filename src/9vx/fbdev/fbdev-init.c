#include "u.h"

#include <sys/ioctl.h>
#include <sys/mman.h>

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

static int parsewinsize(char *s, Rectangle *r, int *havemin);
static void plan9cmap(void);

FBprivate _fb;

void
fbput(Memimage *m, Rectangle r) {
	int x, y;

	for (x = r.min.x; x < r.max.x; x++) for (y = r.min.y; y < r.max.y; y++){
//		long fbloc = (x+_fb.vinfo.xoffset) + (y+_fb.vinfo.yoffset) * _fb.finfo.line_length;
		long fbloc = (x+_fb.vinfo.xoffset) * (_fb.vinfo.bits_per_pixel/8) + (y+_fb.vinfo.yoffset) * _fb.finfo.line_length;
		uint32 pixel = *((uint32*)(m->data->bdata + ((y*m->width) + x)*4));

		*((uint32*)(_fb.fbp + fbloc)) = pixel;
	}
}

Memimage*
_fbattach(char *label, char *winsize)
{
	Rectangle r;
	uint32 width, height;
	long screensize;

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
	screensize = _fb.vinfo.yres_virtual * _fb.finfo.line_length;
	if ((_fb.fbp = mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, _fb.fd, (off_t)0)) < 0)
		goto err;
	/*
	 * Figure out underlying screen format.
	 */
	r = Rect(0, 0, _fb.vinfo.xres_virtual, _fb.vinfo.yres_virtual);

	_fb.screenimage = allocmemimage(r, XRGB32);
	termreplacescreenimage(_fb.screenimage);
	fbput(_fb.screenimage, r);
	return _fb.screenimage;

err:
	return nil;
}

/*
 * Initialize map with the Plan 9 rgbv color map.
 */
static void
plan9cmap(void)
{
	int r, g, b, cr, cg, cb, v, num, den, idx, v7, idx7;
	static int once;

	if(once)
		return;
	once = 1;

	for(r=0; r!=4; r++)
	for(g = 0; g != 4; g++)
	for(b = 0; b!=4; b++)
	for(v = 0; v!=4; v++){
		den=r;
		if(g > den)
			den=g;
		if(b > den)
			den=b;
		/* divide check -- pick grey shades */
		if(den==0)
			cr=cg=cb=v*17;
		else {
			num=17*(4*den+v);
			cr=r*num/den;
			cg=g*num/den;
			cb=b*num/den;
		}
		idx = r*64 + v*16 + ((g*4 + b + v - r) & 15);
/*		_fb.map[idx].red = cr*0x0101;
		_fb.map[idx].green = cg*0x0101;
		_fb.map[idx].blue = cb*0x0101;
		_fb.map[idx].pixel = idx;
		_fb.map[idx].flags = DoRed|DoGreen|DoBlue;*/

		v7 = v >> 1;
		idx7 = r*32 + v7*16 + g*4 + b;
		if((v & 1) == v7){
//			_fb.map7to8[idx7][0] = idx;
			if(den == 0) { 		/* divide check -- pick grey shades */
				cr = ((255.0/7.0)*v7)+0.5;
				cg = cr;
				cb = cr;
			}
			else {
				num=17*15*(4*den+v7*2)/14;
				cr=r*num/den;
				cg=g*num/den;
				cb=b*num/den;
			}
/*			_fb.map7[idx7].red = cr*0x0101;
			_fb.map7[idx7].green = cg*0x0101;
			_fb.map7[idx7].blue = cb*0x0101;
			_fb.map7[idx7].pixel = idx7;
			_fb.map7[idx7].flags = DoRed|DoGreen|DoBlue;*/
		}
//		else
//			_fb.map7to8[idx7][1] = idx;
	}
}

void
flushmemscreen(Rectangle r)
{
	fbput(_fb.screenimage, r);
}

