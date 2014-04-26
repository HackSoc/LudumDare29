#include <stdlib.h>

#include "mob.h"
#include "level.h"

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
 * Returns true if this killed the mob, a mob is dead if its health
 * drops to zero or below.
 */
bool damage_mob(Mob * mob, unsigned int damage) {
	mob->health -= damage;

	if(mob->health <= 0) {
		if(mob->death_action != NULL) {
			mob->death_action(mob);
		}

		Cell * cell = mob->level->cells[mob->xpos][mob->ypos];

		/* Remove it from the world */
		cell->occupant = NULL;

		if(mob->level->mobs == mob) {
			mob->level->mobs = mob->next;
		}

		if(mob->prev != NULL) {
			mob->prev->next = mob->next;
		}

		/* Drop its items */
		if(cell->items == NULL) {
			cell->items = mob->items;
		} else {
			Item * last;
			for(last = cell->items; last != NULL; last = last->next) {}
			last->next = mob->items;
			last->next->prev = last;
		}
		
		return true;
	}

	return false;
}

/**
 * A very simple enemy: move towards the player, and damage them if
 * adjacent
 */
void simple_enemy_turn(Mob * enemy) {
	Mob * player = enemy->level->player;
	
	int xdiff = enemy->xpos - player->xpos;
	int ydiff = enemy->ypos - player->ypos;

	/* If adjacent to the player, damage them */
	if((abs(xdiff) == 1 && ydiff == 0) ||
	   (xdiff == 0 && abs(ydiff) == 1)) {
		damage_mob(player, 2);
		return;
	}

	/* Move along the axis furthest away from the player */
	if(abs(xdiff) > abs(ydiff)) {
		bool res = move_mob_relative(enemy, (xdiff < 0) ? 1 : -1, 0);

		/* If there was something solid in the way, try moving y */
		if(!res) {
			move_mob_relative(enemy, 0, (ydiff < 0) ? 1 : -1);
		}
	} else {
		bool res = move_mob_relative(enemy, 0, (ydiff < 0) ? 1 : -1);

		if(!res) {
			move_mob_relative(enemy, (xdiff < 0) ? 1 : -1, 0);
		}
	}
}
