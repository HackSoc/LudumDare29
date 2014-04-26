#include <curses.h>
#include <stdlib.h>

#include "level.h"
#include "mob.h"

extern bool quit;

void run_turn(Level * level) {
	Mob * mob;
	for(mob = level->mobs; mob != NULL && !quit; mob = mob->next) {
		if(mob->turn_action != NULL) {
			mob->turn_action(mob);
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

/**
 * Move a mob by a relative position
 */
bool move_mob_relative(Mob * mob, int xdiff, int ydiff) {
	return move_mob(mob, mob->xpos + xdiff, mob->ypos + ydiff);
}

/**
 * Damage a mob
 * Returns true if this killed the mob.
 */
bool damage_mob(Mob * mob, unsigned int damage) {
	mob->health -= damage;

	if(mob->health <= 0) {
		if(mob->death_action != NULL) {
			mob->death_action(mob);
		}

		mob->level->cells[mob->xpos][mob->ypos]->occupant = NULL;

		/* Remove it from the world */
		if(mob->level->mobs == mob) {
			mob->level->mobs = mob->next;
		}

		if(mob->prev != NULL) {
			mob->prev->next = mob->next;
		}
		
		return true;
	}

	return false;
}

/**
 * Move and attack at the same time - if a mob is in the target cell,
 * damage it, but don't move.
 * Should only be usef by the player, as this checks `hostile`.
 */
bool attackmove(Mob * player, unsigned int x, unsigned int y,
				unsigned int damage) {
	Mob * mob = player->level->cells[x][y]->occupant;
	bool unoccupied = move_mob(player, x, y);

	if(!unoccupied && mob->hostile) {
		damage_mob(mob, damage);
	}

	return unoccupied;
}

/**
 * Like attackmove, but relative position
 */
bool attackmove_relative(Mob * player, int xdiff, int ydiff,
						 unsigned int damage) {
	unsigned int x = player->xpos + xdiff;
	unsigned int y = player->ypos + ydiff;

	return attackmove(player, x, y, damage);
}
