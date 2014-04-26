#include <curses.h>
#include <stdbool.h>
#include <stdlib.h>

#include "utils.h"
#include "level.h"
#include "mob.h"
#include "item.h"
#include "player.h"

/** Whether to quit the game or not. */
bool quit = false;

/** Entry point. */
int main() {
	/* Simple static level for rendering */
	Mob player = {
		.level = NULL,
		.symbol = '@',
		.next = NULL,
		.prev = NULL,
		.items = NULL,
		.hostile = false,
		.turn_action = &player_turn,
		.death_action = &player_death,
		.name = "Elrond",
		.race = "Half-Elf",
		.profession = "Orc Slayer",
		.health = 100,
		.max_health = 100,
		.xpos = 15,
		.ypos = 10
	};

	Mob hedgehog = {
		.level = NULL,
		.symbol = 'H',
		.next = NULL,
		.prev = &player,
		.items = NULL,
		.hostile = true,
		.turn_action = &simple_enemy_turn,
		.death_action = NULL,
		.health = 5,
		.max_health = 5,
		.xpos = 20,
		.ypos = 3
	};

	Level level = {
		.next = NULL,
		.prev = NULL,
		.mobs = &player,
		.player = &player
	};

	player.level = &level;
	hedgehog.level = &level;
	player.next = &hedgehog;

	build_level(&level);

	level.cells[player.xpos][player.ypos]->occupant = &player;
	level.cells[hedgehog.xpos][hedgehog.ypos]->occupant = &hedgehog;

	/* Initialise curses */
	initscr();
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	curs_set(0);

	/* Intro text */
	mvaddprintf(10, 10, "You enter a cave.");
	mvaddprintf(11, 10, "It's beneath the surface.");
	mvaddprintf(19, 44, "A game for Ludum Dare 29 by HackSoc.");
	getch();

	/* Game loop */
	Level * current_level = &level;
	while(!quit) {
		/* Render level */
		display_level(current_level);

		/* Display player stats */
		mvaddprintf(21, 5, "%s, the %s %s", player.name, player.race, player.profession);
		mvaddprintf(22, 5, "HP: %d/%d", player.health, player.max_health);

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
