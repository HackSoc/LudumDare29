#ifndef STATUS_H
#define STATUS_H

/**
 * The top of the status box
 */
#define STATUS_TOP 21

/**
 * The number of lines in the status box
 */
#define STATUS_LINES 4

/**
 * The X coordinate of the status box
 */
#define STATUS_X 40

/**
 * Print the status box
 */
void display_status(void);

/**
 * Print a line to the status box
 */
void status_push(const char * fmt, ...);


#endif
