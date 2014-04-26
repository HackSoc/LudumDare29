#ifndef _UTILS_H
#define _UTILS_H

/**
 * The width of the screen, in characters
 */
#define SCREENWIDTH 80

/**
 * The height of the screen, in characters
 */
#define SCREENHEIGHT 25

/**
 * printf the given string at the given position
 */
void mvaddprintf(unsigned int y, unsigned int x, const char * fmt, ...);

/**
 * printf the given string at the current position of the cursor
 */
void addprintf(const char * fmt, ...);

#endif
