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
 * Wrapper macro for _xalloc to add the sizeof
 */
#define xalloc(T) _xalloc(sizeof(T))

/**
 * Handy calloc-like function using _xalloc
 */
#define xcalloc(S,N) _xalloc(S * sizeof(N));

/**
 * Wrapper macro for _xfree to add the extra indirection
 */
#define xfree(P) _xfree(&P)

/**
 * printf the given string at the given position
 */
void mvaddprintf(unsigned int y, unsigned int x, const char * fmt, ...);

/**
 * printf the given string at the current position of the cursor
 */
void addprintf(const char * fmt, ...);

/**
 * Allocate (and zero) memory and immediately bail out if it fails.
 * Use the xalloc macro.
 */
void* _xalloc(size_t size);

/**
 * Free a non-NULL pointer, and set it to NULL.
 * Use the xfree macro.
 */
void _xfree(void** ptr);

#endif
