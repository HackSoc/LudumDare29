#ifndef STATUS_H
#define STATUS_H

/**
 * The top of the status box
 */
#define STATUS_TOP 25

/**
 * The number of lines in the status box
 */
#define STATUS_LINES 7

/**
 * The X coordinate of the status box
 */
#define STATUS_X 5

/**
 * Print the status box
 */
void display_status(void);

/**
 * Print a line to the status box
 */
void status_push(const char * fmt, ...);


#endif
