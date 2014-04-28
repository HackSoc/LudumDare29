#include <curses.h>
#include <string.h>
#include "status.h"
#include "utils.h"

/**
 * The status text, in a cyclic buffer.
 */
static char status_box[STATUS_LINES][SCREENWIDTH - STATUS_X];

/**
 * The index into the status buffer
 */
static unsigned int head = 0;

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
	vsnprintf(status_box[head], SCREENWIDTH - STATUS_X, fmt, ap);
	va_end(ap);

	head = (head + 1) % STATUS_LINES;
	
	if(head == 0) {
		full = true;
	}
}

/**
 * Print the status box
 */
void display_status() {
	if(full) {
		unsigned int i = head;
		unsigned int j = 0;
		do {
			mvaddstr(STATUS_TOP + j, STATUS_X, status_box[i]);
			i = (i + 1) % STATUS_LINES;
			j++;
		} while(i != head);
	} else {
		for(unsigned int i = 0; i < head; i++) {
			mvaddstr(STATUS_TOP + i, STATUS_X, status_box[i]);
		}
	}
}
