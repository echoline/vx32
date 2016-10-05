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

#include <fcntl.h>
#include <linux/input.h>
#include <termios.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <linux/keyboard.h>

#define MOUSEFILE "/dev/input/mice"

Mouse fbmouse;

static void
termctl(uint32 o, int or)
{
	struct termios t;

	tcgetattr(0, &t);
	if (or)
		t.c_lflag |= o;
	else
		t.c_lflag &= o;
	tcsetattr(0, TCSANOW, &t);
}

static void
fbputc(int c)
{
	static int escaped = 0;

	if (escaped == 1) {
		if (c == '[') {
			escaped = 2;
			return;
		} else {
			escaped = 0;
			kbdputc(kbdq, 27);
		}
	} else if (escaped == 2) {
		switch (c) {
			case 'A':
				c = Kup;
				break;
			case 'B':
				c = Kdown;
				break;
			case 'C':
				c = Kright;
				break;
			case 'D':
				c = Kleft;
				break;
			default:
				kbdputc(kbdq, 27);
				kbdputc(kbdq, '[');
				break;
		}
		escaped = 0;
	}

	switch (c) {
		case 27:
			escaped = 1;
			return;
		default:
			break;
	}

	kbdputc(kbdq, c);
}

static void
ctrlc(int sig) {
	latin1putc(3, fbputc);
}

void
screeninit(void)
{
}

int
__mouse(unsigned char *data)
{
	struct timeval tv;
	Rectangle old, new;

	gettimeofday(&tv, nil);
	fbmouse.msec = tv.tv_usec / 1000 + tv.tv_sec * 1000;

	old.min.x = fbmouse.xy.x;
	old.min.y = fbmouse.xy.y;
	old.max.x = old.min.x + 16;
	old.max.y = old.min.y + 16;

	fbmouse.xy.x += (char)data[1];
	fbmouse.xy.y -= (char)data[2];
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
	new.max.x = new.min.x + 16; // size of cursor bitmap
	new.max.y = new.min.y + 16;

	fbmouse.buttons = (data[0] & 1? 1: 0)	// LEFT
			| (data[0] & 2? (_fb.shift_state & (1 << KG_SHIFT)? 2: 4): 0) // RIGHT
			| (data[0] & 4? 2: 0);	// MIDDLE

	mousetrack(fbmouse.xy.x, fbmouse.xy.y, fbmouse.buttons, fbmouse.msec);

	combinerect(&new, old);
	new.min.x -= 16; // to encompass any _fb.cursor->offset
	new.min.y -= 16;

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

static void
_fbproc(void *v)
{
	unsigned char data[3];
	struct pollfd pfd[2];
	int r;

	pfd[0].fd = _fb.mousefd;
	pfd[1].fd = 0;
	pfd[0].events = pfd[1].events = POLLIN;

	for(;;) {
		_fb.shift_state = 6;
		if (ioctl(0, TIOCLINUX, &_fb.shift_state) < 0)
			panic("ioctl TIOCLINUX 6: %r");

		r = poll(pfd, 2, -1);
		if (r < 0)
			oserror();
		if (pfd[0].revents & POLLIN) {
			if (read(_fb.mousefd, data, 3) != 3)
				panic("mousefd read: %r");

			__mouse(data);
		}
		if (pfd[1].revents & POLLIN) {
			if (read(0, data, 1) != 1)
				panic("stdio read: %r");
			latin1putc(data[0], fbputc);
		}
	}

	termctl(ECHO, 1);
}

uchar*
attachscreen(Rectangle *r, ulong *chan, int *depth,
	int *width, int *softscreen, void **X)
{
	Memimage *m;

	// set up terminal
	printf("\x1b[?17;0;0c\n");
	termctl(~(ICANON|ECHO), 0);
	signal(SIGINT, ctrlc);

	if(_fb.backbuf == nil){
		_memimageinit();
		if(_fbattach("9vx", nil) == nil)
			panic("cannot open framebuffer: %r");

		if ((_fb.mousefd = open(MOUSEFILE, O_RDONLY)) == -1)
			panic("cannot open mouse: %r");

		kproc("*fbdev*", _fbproc, nil);
	}
	m = _fb.backbuf;
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
	return strdup(_fb.snarfbuf? _fb.snarfbuf: "");
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

