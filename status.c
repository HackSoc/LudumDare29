#include <curses.h>
#include <string.h>
#include "status.h"
#include "utils.h"

/**
 * The status text, in a cyclic buffer.
 */
static char status_box[STATUS_LINES][SCREENWIDTH - STATUS_X];

/**
 * The indices into the status buffer
 */
static unsigned int status_tail = 0;
static unsigned int status_head = 0;

/**
 * Whether the buffer is full or not
 */
static bool full = false;

/**
 * Print a line to the status box
 */
void status_push(const char * fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(status_box[status_head], SCREENWIDTH - STATUS_X, fmt, ap);
	va_end(ap);

	status_head = (status_head + 1) % STATUS_LINES;
	
	if(status_head == status_tail) {
		status_tail = (status_tail + 1) % STATUS_LINES;
		full = true;
	}
}

/**
 * Print the status box
 */
void display_status() {
	for(unsigned int i = 0; (i + status_tail) % STATUS_LINES != status_head; i ++){
		mvaddstr(STATUS_TOP + i, STATUS_X, status_box[(i + status_tail) % STATUS_LINES]);
	}

	/* If the buffer is full, print the last line */
	if(full) {
		mvaddstr(STATUS_TOP + STATUS_LINES - 1, STATUS_X, status_box[status_head]);
	}
}
