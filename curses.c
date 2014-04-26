#include <curses.h>
#include "curses.h"

/**
 * printf the given string at the given position
 */
void mvaddprintf(unsigned int y, unsigned int x, const char * fmt, ...) {
	char buf[SCREENWIDTH - x];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, SCREENWIDTH, fmt, ap);
	va_end(ap);
	mvaddstr(y, x, buf);
}

/**
 * printf the given string at the current position of the cursor
 */
void addprintf(const char * fmt, ...) {
	char buf[SCREENWIDTH];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, SCREENWIDTH, fmt, ap);
	va_end(ap);
	addstr(buf);
}
