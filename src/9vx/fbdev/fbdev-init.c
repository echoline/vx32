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

#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

static int parsewinsize(char *s, Rectangle *r, int *havemin);
static void plan9cmap(void);

FBprivate _fb;

Memimage*
_fbattach(char *label, char *winsize)
{
	Rectangle r;
	struct fb_fix_screeninfo finfo;
	struct fb_var_screeninfo vinfo;
	uint32 width, height;
	long screensize;

	/*
	 * Connect to /dev/fb0
	 */
	if (_fb.fd = open("/dev/fb0", O_RDWR) < 0)
		goto err;

	if (ioctl(_fb.fd, FBIOGET_VSCREENINFO, &vinfo) < 0)
		goto err;
	vinfo.grayscale=0;
	vinfo.bits_per_pixel=32;
	if (ioctl(_fb.fd, FBIOPUT_VSCREENINFO, &vinfo) < 0)
		goto err;

	if (ioctl(_fb.fd, FBIOGET_FSCREENINFO, &finfo) < 0)
		goto err;
	screensize = vinfo.yres_virtual * finfo.line_length;
	if (_fb.fbp = mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, _fb.fd, (off_t)0) < 0)
		goto err;
	/*
	 * Figure out underlying screen format.
	 */
	r = Rect(0, 0, vinfo.xres_virtual, vinfo.yres_virtual);

	/*
	 * _fb.depth is only the number of significant pixel bits,
	 * not the total number of pixel bits.  We need to walk the
	 * display list to find how many actual bits are used
	 * per pixel.
	 */

	/*
	 * Set up color map if necessary.
	 */

	/*
	 * We get to choose the initial rectangle size.
	 * This is arbitrary.  In theory we should read the
	 * command line and allow the traditional X options.
	 */

	/*
	 * Label and other properties required by ICCCCM.
	 */

	/*
	 * Look up clipboard atom.
	 */

	/*
	 * Put the window on the screen, check to see what size we actually got.
	 */

	/*
	 * Allocate our local backing store.
	 */

	/*
	 * Figure out physical window location.
	 */

	/*
	 * Allocate some useful graphics contexts for the future.
	 */
	
	for(int x = 0; x < vinfo.xres; x++) for (int y = 0; y < vinfo.yres; y++) {
		long location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) + (y+vinfo.yoffset) * finfo.line_length;
		*((uint32*)(_fb.fbp + location)) = 0x00FFFF00;
	}

	return allocmemimage(r, XRGB32);

err:
	oserror();
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
	if(r.min.x >= r.max.x || r.min.y >= r.max.y)
		return;
}

static int
parsewinsize(char *s, Rectangle *r, int *havemin)
{
	char c, *os;
	int i, j, k, l;

	os = s;
	*havemin = 0;
	*r = Rect(0,0,0,0);
	if(!isdigit((uchar)*s))
		goto oops;
	i = strtol(s, &s, 0);
	if(*s == 'x'){
		s++;
		if(!isdigit((uchar)*s))
			goto oops;
		j = strtol(s, &s, 0);
		r->max.x = i;
		r->max.y = j;
		if(*s == 0)
			return 0;
		if(*s != '@')
			goto oops;

		s++;
		if(!isdigit((uchar)*s))
			goto oops;
		i = strtol(s, &s, 0);
		if(*s != ',' && *s != ' ')
			goto oops;
		s++;
		if(!isdigit((uchar)*s))
			goto oops;
		j = strtol(s, &s, 0);
		if(*s != 0)
			goto oops;
		*r = rectaddpt(*r, Pt(i,j));
		*havemin = 1;
		return 0;
	}

	c = *s;
	if(c != ' ' && c != ',')
		goto oops;
	s++;
	if(!isdigit((uchar)*s))
		goto oops;
	j = strtol(s, &s, 0);
	if(*s != c)
		goto oops;
	s++;
	if(!isdigit((uchar)*s))
		goto oops;
	k = strtol(s, &s, 0);
	if(*s != c)
		goto oops;
	s++;
	if(!isdigit((uchar)*s))
		goto oops;
	l = strtol(s, &s, 0);
	if(*s != 0)
		goto oops;
	*r = Rect(i,j,k,l);
	*havemin = 1;
	return 0;

oops:
	werrstr("bad syntax in window size '%s'", os);
	return -1;
}
