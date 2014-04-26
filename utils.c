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
 * Render the given char at the given position with the given colour
 * pair.
 * @param y The Y position
 * @param x The X position
 * @param chr The character to render
 * @param fg The foreground colour
 * @param bg The background colour
 * @param bold Whether to bold or not
 */
void mvaddchcol(unsigned int y, unsigned int x,
				char chr,
				int fg, int bg,
				bool bold) {
	init_pair(fg << 3 | bg, fg, bg);
	attron(COLOR_PAIR(fg << 3 | bg));
	if(bold) {
		attron(A_BOLD);
	}

	mvaddch(y, x, chr);

	if(bold) {
		attroff(A_BOLD);
	}
	attroff(COLOR_PAIR(fg << 3 | bg));
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
