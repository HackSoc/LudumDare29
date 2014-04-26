#include <curses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include "utils.h"
#include "level.h"
#include "mob.h"
#include "item.h"
#include "player.h"

/** Whether to quit the game or not. */
bool quit = false;

/** Entry point. */
int main() {
	/* Initialise curses */
	WINDOW * mainwin = initscr();
	start_color();
	cbreak();
	noecho();
	nonl();

	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	start_color();
	curs_set(0);

	srand(time(NULL));

	Mob * player = create_player();

	Level * level_head = xalloc(Level);
	level_head->mobs = player;
	level_head->player = player;

	player->level = level_head;

	build_level(level_head);

	player->xpos = level_head->startx;
	player->ypos = level_head->starty;
	level_head->cells[player->xpos][player->ypos]->occupant = player;

	/* Intro text */
	mvaddprintf(10, 10, "You enter a cave.");
	mvaddprintf(11, 10, "It's beneath the surface.");
	mvaddprintf(19, 44, "A game for Ludum Dare 29 by HackSoc.");
	getch();

	/* Game loop */
	while(!quit) {
		/* Update mobs */
		run_turn(player->level);

		clear();
	}

	/* Free the things */
	while (level_head != NULL) {
		Level * level = level_head;
		level_head = level_head->next;

		Mob * m = level->mobs;
		while (m != NULL) {
			Mob * tmp = m;
			m = m->next;
			xfree(tmp->name);
			xfree(tmp->race);
			xfree(tmp->profession);
			xfree(tmp);
		}
		for (int x = 0; x < LEVELWIDTH; x++) {
			for (int y = 0; y < LEVELHEIGHT; y++) {
				xfree(level->cells[x][y]);
			}
			xfree(level->cells[x]);
		}
		xfree(level->cells);
		xfree(level);
	}

	/* Deinitialise curses */
	curs_set(1);
	nl();
	echo();
	nocbreak();
	delwin(mainwin);
	endwin();
}
