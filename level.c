#include <curses.h>
#include <stdlib.h>

#include "level.h"
#include "mob.h"
#include "utils.h"

extern bool quit;

/**
 * Initialises a level.
 * @param level Level to initialise.
 */
void build_level(Level * level) {
	for(unsigned int y = 0; y < LEVELHEIGHT; y++) {
		for(unsigned int x = 0; x < LEVELWIDTH; x++) {
			level->cells[x][y] = xalloc(Cell);
			if(y == 0 || y == LEVELHEIGHT - 1) {
				level->cells[x][y]->baseSymbol = '-';
				level->cells[x][y]->solid = true;
			} else if (x == 0 || x == LEVELWIDTH - 1) {
				level->cells[x][y]->baseSymbol = '|';
				level->cells[x][y]->solid = true;
			} else {
				level->cells[x][y]->baseSymbol = '.';
			}
		}
	}
}

/**
 * A "turn" consists of all of the mobs acting once, possibly followed
 * by some constant effect on the mob. As the player is a turn, this
 * is (indirectly) where blocking for input happens.
 */
void run_turn(Level * level) {
	for(Mob * mob = level->mobs; mob != NULL && !quit; mob = mob->next) {
		if(mob->turn_action != NULL) {
			mob->turn_action(mob);
		}
		//every turn effects on the mob go here
	}
}

/**
 * Render the level to the screen. The symbol for a level is picked
 * according to the following priorities: occupant > top item > base.
 */
void display_level(Level * level) {
	for(unsigned int x = 0; x < LEVELWIDTH; x++) {
		for(unsigned int y = 0; y < LEVELHEIGHT; y++) {
			if(level->cells[x][y]->occupant != NULL) {
				mvaddch(y, x, level->cells[x][y]->occupant->symbol);
			} else if(level->cells[x][y]->items != NULL) {
				mvaddch(y, x, level->cells[x][y]->items->symbol);
			} else {
				mvaddch(y, x, level->cells[x][y]->baseSymbol);
			}
		}
	}
}
