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
	initscr();
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

	Level level = {
		.next = NULL,
		.prev = NULL,
		.mobs = player,
		.player = player
	};

	player->level = &level;

	build_level(&level);

	player->xpos = level.startx;
	player->ypos = level.starty;
	level.cells[player->xpos][player->ypos]->occupant = player;

	/* Intro text */
	mvaddprintf(10, 10, "You enter a cave.");
	mvaddprintf(11, 10, "It's beneath the surface.");
	mvaddprintf(19, 44, "A game for Ludum Dare 29 by HackSoc.");
	getch();

	/* Game loop */
	Level * current_level = &level;
	while(!quit) {
		/* Update mobs */
		run_turn(current_level);

		clear();
	}

	/* Deinitialise curses */
	curs_set(1);
	nl();
	echo();
	nocbreak();
	endwin();
}
