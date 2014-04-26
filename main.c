#include <curses.h>
#include <stdbool.h>
#include <stdlib.h>

#include "utils.h"
#include "level.h"
#include "mob.h"
#include "item.h"
#include "player.h"

/**
 * Whether to quit the game or not
 */
bool quit = false;

int main() {
	/* Simple static level for rendering */
	Mob player = {.level = NULL,
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
				  .max_health = 100};

	Mob hedgehog = {.level = NULL,
					.symbol = 'H',
					.next = NULL,
					.prev = &player,
					.items = NULL,
					.hostile = true,
					.turn_action = &simple_enemy_turn,
					.death_action = NULL,
					.health = 5,
					.max_health = 5};

	Level level = {.next = NULL,
				   .prev = NULL,
				   .mobs = &player,
				   .player = &player};
	
	player.level = &level;
	hedgehog.level = &level;
	player.next = &hedgehog;
	
	for(unsigned int y = 0; y < LEVELHEIGHT; y++) {
		for(unsigned int x = 0; x < LEVELWIDTH; x++) {
			level.cells[x][y] = calloc(1, sizeof(Cell));
			if(y == 0 || y == LEVELHEIGHT - 1) {
				level.cells[x][y]->baseSymbol = '-';
				level.cells[x][y]->solid = true;
			} else if (x == 0 || x == LEVELWIDTH - 1) {
				level.cells[x][y]->baseSymbol = '|';
				level.cells[x][y]->solid = true;
			} else {
				level.cells[x][y]->baseSymbol = '.';
			}
		}
	}
	
	level.cells[15][10]->occupant = &player;
	player.xpos = 15;
	player.ypos = 10;

	level.cells[20][3]->occupant = &hedgehog;
	hedgehog.xpos = 20;
	hedgehog.ypos = 3;

	/* Initialise curses */
	initscr();
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	curs_set(0);
	
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
