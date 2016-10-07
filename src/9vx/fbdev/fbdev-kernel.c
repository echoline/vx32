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

void
screeninit(void)
{
}

static void
fbproc(void *v)
{
	struct input_event data;
	unsigned char c;
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
			if (read(_fb.mousefd, &data, sizeof(data)) != sizeof(data))
				panic("mousefd read: %r");

			mouseevent(&data);
		}
		if (pfd[1].revents & POLLIN) {
			if (read(0, &c, 1) != 1)
				panic("stdio read: %r");
			latin1putc(c, fbputc);
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
	printf("%cc", Kesc);
	printf("%c[?17;0;0c", Kesc);
	termctl(~(ICANON|ECHO), 0);
	signal(SIGINT, ctrlc);

	if(_fb.backbuf == nil){
		_memimageinit();
		if(fbattach(0) == nil)
			panic("cannot open framebuffer: %r");

		if(mouseattach(-1) < 0)
			panic("cannot open mouse: %r");

		kproc("*fbdev*", fbproc, nil);
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
