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

#include <linux/input.h>
#include <linux/keyboard.h>

int
__mouse(struct input_event *event)
{
	struct timeval tv;
	Rectangle old, new;
	static Point startpt;
	static Point startmousept;
	static char touched;
	static Point coord;

	gettimeofday(&tv, nil);
	_fb.mouse.msec = tv.tv_usec * 1000000000LL + tv.tv_sec * 1000;

	old.min = _fb.mouse.xy;
	old.max = addpt(old.min, Pt(16, 16));

	_fb.mouse.buttons &= ~0x18; // clear mosuewheel

	switch (event->type) {
	case 3:
		//printf("%x %x %x\n", event->type, event->code, event->value);
		switch (event->code) {
		case 0:	
			coord.x = event->value;
			break;
		case 1:
			coord.y = event->value;
			break;
		case 0x18:
		case 0x1c:
			if (event->value == 0)
				touched = 0;
			else if (event->value > 24){
				touched = 1;
				startmousept = coord;
				startpt = _fb.mouse.xy;
			}
			break;
		default:
			return -1;
		}
		if (touched)
			_fb.mouse.xy = addpt(startpt, divpt(subpt(coord, startmousept), 4));
		break;
	case 2:
		switch (event->code) {
		case 0:
			_fb.mouse.xy.x += event->value;
			break;
		case 1:
			_fb.mouse.xy.y += event->value;
			break;
		case 8:
			_fb.mouse.buttons |= (event->value == 1? 8: 16); // wheel
			break;
		default:
			return -1;
		}
		break;
	case 1:
		switch (event->code) {
		case 0x110:
			if (event->value == 1)
				_fb.mouse.buttons |= 1;
			else
				_fb.mouse.buttons &= ~1;
			break;
		case 0x111:
			if (event->value == 1)
				_fb.mouse.buttons |= (_fb.shift_state & (1 << KG_SHIFT)? 2: 4);
			else
				_fb.mouse.buttons &= ~(_fb.shift_state & (1 << KG_SHIFT)? 2: 4);
			break;
		case 0x112:
			if (event->value == 1)
				_fb.mouse.buttons |= 2;
			else
				_fb.mouse.buttons &= ~2;
			break;
		default:
			return -1;
		}
		break;
	default:
		return -1;
	}

	if (_fb.mouse.xy.x < _fb.screenimage->r.min.x)
		_fb.mouse.xy.x = _fb.screenimage->r.min.x;
	if (_fb.mouse.xy.y < _fb.screenimage->r.min.y)
		_fb.mouse.xy.y = _fb.screenimage->r.min.y;
	if (_fb.mouse.xy.x > _fb.screenimage->r.max.x)
		_fb.mouse.xy.x = _fb.screenimage->r.max.x;
	if (_fb.mouse.xy.y > _fb.screenimage->r.max.y)
		_fb.mouse.xy.y = _fb.screenimage->r.max.y;
	
	new.min = _fb.mouse.xy;
	new.max = addpt(new.min, Pt(16, 16)); // size of cursor bitmap

	combinerect(&new, old);
	new.min = subpt(new.min, Pt(16, 16)); // to encompass any _fb.cursor->offset

	rectclip(&new, _fb.screenimage->r);

	mousetrack(_fb.mouse.xy.x, _fb.mouse.xy.y, _fb.mouse.buttons, _fb.mouse.msec);

	flushmemscreen(new);

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
	_fb.mouse.xy = p;
	flushmemscreen(_fb.screenimage->r);
}

void
setcursor(struct Cursor *c)
{
	drawqlock();
	_fb.cursor = c;
	drawqunlock();
}

