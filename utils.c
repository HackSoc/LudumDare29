#include <curses.h>
#include <stdlib.h>
#include <assert.h>
#include "utils.h"

/**
 * printf the given string at the given position.
 * @param y y-coordinate of the screen.
 * @param x x-coordinate of the screen.
 * @param fmt Format of the string.
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
 * printf the given string at the current position of the cursor.
 * @param fmt Format of the string.
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
 * @param size Amount of memory to allocate (in bytes).
 * @return Allocated memory.
 * @note Do not use this directly, use the xalloc macro instead.
 */
void * _xalloc(size_t size) {
	void * mem = calloc(1, size);
	assert(mem != NULL);
	return mem;
}

/**
 * Free a non-NULL pointer.
 * @param ptr Pointer to memory to free.
 * @note Do not use this directly, use the xfree macro instead.
 */
void _xfree(void ** ptr) {
	if(*ptr != NULL) {
		free(*ptr);
		*ptr = NULL;
	}
}
