#include <curses.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

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
 * Duplicate a string
 * @param str String to duplicate
 */
char * strdup(const char * str) {
	char * out = xcalloc(strlen(str) + 1, char);
	strcpy(out, str);
	return out;
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
 * Select from a list of things, berate the player if they enter a bad
 * choice. This clears the screen before and after running.
 * @param nochoice This isn't a choice, just a list.
 * @param prompt The initial question.
 * @param prompt2 The prompt to use after a bad choice.
 * @param multi Allow multiple selections.
 * @param empty Allow an empty selection.
 * @param choices NULL-terminated list of choice names.
 * @param results NULL-terminated list of actual results.
 * @return The results corresponding to the choices, or NULL.
 */
const void ** list_choice(bool nochoice,
                          const char * prompt,
                          const char * prompt2,
                          bool multi,
                          bool empty,
                          const char * choices[],
                          const void * results[]) {
	clear();

	/* Build the list of choices, and count how many there are */
	unsigned int num_choices;
	for(num_choices = 0; choices[num_choices] != NULL; num_choices ++) {
		mvaddprintf(2 + num_choices, 1,
		            "%c - %s", 'a' + num_choices, choices[num_choices]);
	}

	mvaddstr(1, 1, prompt);
	addch(' ');

	/* If this is not a choice, end here */
	if(nochoice) {
		getch();
		clear();
		return NULL;
	}

	/* If there are no items, return an empty list - but this is still a 'choice',
	   so don't return nothing. */
	if(num_choices == 0) {
		getch();
		clear();
		const void ** out = xcalloc(1, void *);
		return out;
	}

	/* Get a list of choices */
	assert(num_choices != 0);
	bool * chosen = xcalloc(num_choices, bool);
	unsigned int num_chosen = 0;

	while(true) {
		unsigned int ch = getch();
		if('a' <= ch && 'a' + num_choices > ch) {
			/* If multiple choice isn't allowed, and a choice has been
			   made, just skip this iteration if they tried to choose
			   another thing */
			if(!multi && num_chosen != 0 && !chosen[ch - 'a']) {
				continue;
			}

			/* Toggle the chosen state of the choice and update the
			   list */
			chosen[ch - 'a'] = !chosen[ch - 'a'];
			if (chosen[ch - 'a']) {
				num_chosen ++;
				mvaddprintf(2 + ch - 'a', 3, "+");
			} else {
				num_chosen --;
				mvaddprintf(2 + ch - 'a', 3, "-");
			}
		} else {
			if(num_chosen > 0 || empty) {
				/* If there are choices, or we allow an empty choice,
				   terminate */
				break;
			} else {
				/* Otherwise, berate the user */
				move(1, 1);
				clrtoeol();
				addstr(prompt2);
			}
		}
	}

	/* Construct the return list */
	const void ** selected = xcalloc(num_chosen + 1, void *);
	unsigned int j = 0;
	for(unsigned int i = 0; i < num_choices; i++) {
		if(chosen[i]) {
			selected[j] = results[i];
			j++;
		}
	}
	selected[j] = NULL;

	/* Clean up and return */
	clear();
	xfree(chosen);

	return selected;
}

/**
 * Select a random value from a list
 * @param choices NULL-terminates list of choices.
 */
const void * random_choice(const void * choices[]) {
	int num_choices;
	for(num_choices = 0; choices[num_choices] != NULL; num_choices ++);
	assert(num_choices != 0);
	return choices[rand() % num_choices];
}

/**
 * Show some help text to the player.
 */
void show_help() {
	clear();

	mvaddprintf( 5, 5, "You lost?");

	mvaddprintf( 7, 5, "You are @");
	mvaddprintf( 8, 5, "S and H are bad");
	mvaddprintf( 9, 5, "Use > to go further down into the deep");
	mvaddprintf(10, 5, "Use < to reach the light, if you can");

	mvaddprintf(20, 5, "Good luck.");

	getch();
	clear();
}

/**
 * Allocate (and zero) memory and immediately bail out if it fails.
 * @param size Amount of memory to allocate (in bytes).
 * @return Allocated memory.
 * @note Do not use this directly, use the xalloc macro instead.
 */
void * _xalloc(size_t size) {
	if (size == 0) return NULL;
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

/**
 * A random distribution biased towards the end
 * @param max Maximum random value
 */
int biased_rand(int max) {
	int x = rand() % max;
	return (int)(sqrt(x) * sqrt(max));
}
