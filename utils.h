#ifndef _UTILS_H
#define _UTILS_H

/** The width of the screen, in characters. */
#define SCREENWIDTH 80

/** The height of the screen, in characters. */
#define SCREENHEIGHT 25

/** Wrapper macro for _xalloc to add the sizeof. */
#define xalloc(T) _xalloc(sizeof(T))

/** Handy calloc-like function using _xalloc. */
#define xcalloc(S,T) _xalloc(S * sizeof(T));

/** Wrapper macro for _xfree to add the extra indirection. */
#define xfree(P) _xfree(&P)

void mvaddprintf(unsigned int y, unsigned int x, const char * fmt, ...);
void addprintf(const char * fmt, ...);
void mvaddchcol(unsigned int y, unsigned int x,
				char chr,
				int fg, int bg,
				bool bold);
char * list_choice(unsigned int y, unsigned int x,
				   const char * prompt,
				   const char * prompt2,
				   const char * choices[]);
void * _xalloc(size_t size);
void _xfree(void ** ptr);

#endif
