#include "level.h"

void run_turn(Level * level) {
	Mob * mob;
	for(mob = level->mobs; mob != NULL; mob = mob->next) {
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
				mvaddch(y, x, level->cells[x][y]-baseSymbol);
			}
		}
	}
}
