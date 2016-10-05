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

#include <termios.h>

extern Mouse fbmouse;

void
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

void
fbputc(int c)
{
	static int escaped = 0;
	static int number;
	char digit[2] = { 0, 0 };

	if (escaped == 1) {
		number = 0;

		if (c == '[') {
			escaped = 2;
			return;
		} else {
			escaped = 0;
			kbdputc(kbdq, 27);
		}
	} else if (escaped == 2) {
		if (isdigit(c)) {
			digit[0] = c;
			number *= 10;
			number += atoi(digit);
			return;
		}

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
		case '~':
			switch (number) {
			case 1:
				c = Khome;
				break;
			case 2:
				c = Kins;
				break;
			case 3:
				c = Kdel;
				break;
			case 4:
				c = Kend;
				break;
			case 5:
				c = Kpgup;
				break;
			case 6:
				c = Kpgdown;
				break;
			case 0:
			default:
				escaped = 0;
				return;
			}
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
		case Kdel:
			if (number == 3) {
				number = 0;
				break;
			}
			c = Kbs;
			break;
		default:
			break;
	}

	kbdputc(kbdq, c);
}

void
ctrlc(int sig) {
	latin1putc(3, fbputc);
}
