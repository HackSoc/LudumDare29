#include <curses.h>
#include <stdlib.h>
#include <assert.h>
#include "utils.h"

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

/**
 * Allocate (and zero) memory and immediately bail out if it fails.
 * Use the xalloc macro.
 */
void* _xalloc(size_t size) {
	void* mem = calloc(1, size);
	assert(mem != NULL);
	return mem;
}

/**
 * Free a non-NULL pointer.
 * Use the xfree macro.
 */
void _xfree(void** ptr) {
	if(*ptr != NULL) {
		free(*ptr);
		*ptr = NULL;
	}
}
