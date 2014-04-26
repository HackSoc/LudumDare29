#include <curses.h>
#include <stdlib.h>

#include "level.h"
#include "mob.h"

extern bool quit;

void run_turn(Level * level) {
	Mob * mob;
	for(mob = level->mobs; mob != NULL && !quit; mob = mob->next) {
		if(mob->action != NULL) {
			mob->action(mob);
		}
		//every turn effects on the mob go here
	}
}

void display_level(Level * level) {
	int x, y;
	for(x = 0; x < LEVELWIDTH; x++) {
		for(y = 0; y < LEVELHEIGHT; y++) {
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

/**
 * Move the given mob to the new coordinates
 * Returns false if the given space can't be moved into.
 */
bool move_mob(Mob * mob, unsigned int x, unsigned int y) {
	Level * level = mob->level;
	Cell * source = level->cells[mob->xpos][mob->ypos];
	Cell * target = level->cells[x][y];

	if(source == target)
		return true;

	if(target->solid == true || target->occupant != NULL) {
		return false;
	}

	source->occupant = NULL;
	target->occupant = mob;
	mob->xpos = x;
	mob->ypos = y;

	return true;
}
