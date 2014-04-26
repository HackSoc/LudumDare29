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
	srand(time(NULL));

	/* Simple static level for rendering */
	Mob player = {
		.level = NULL,
		.symbol = '@',
		.colour = COLOR_WHITE,
		.is_bold = true,
		.next = NULL,
		.prev = NULL,
		.items = NULL,
		.hostile = false,
		.turn_action = &player_turn,
		.death_action = &player_death,
		.effect_action = NULL,
		.name = "Elrond",
		.race = "Half-Elf",
		.profession = "Orc Slayer",
		.health = 100,
		.max_health = 100,
		.xpos = 15,
		.ypos = 10
	};

	Level level = {
		.next = NULL,
		.prev = NULL,
		.mobs = &player,
		.player = &player
	};

	player.level = &level;

	build_level(&level);
	player.xpos = level.startx;
	player.ypos = level.starty;

	level.cells[player.xpos][player.ypos]->occupant = &player;

	/* Initialise curses */
	initscr();
	start_color();
	cbreak();
	noecho();
	nonl();

	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	//start_color();
	curs_set(0);

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
