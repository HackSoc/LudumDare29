#include <stdlib.h>

#include "mob.h"
#include "level.h"
#include "utils.h"
#include "effect.h"

/**
 * Move the given mob to the new coordinates.
 * @param mob Entity to move.
 * @param x Target x position.
 * @param y Target y position.
 * @return false if the given space can't be moved into.
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

	/* Check for poison water - this should not be in move, but it
	   works for now. */	
	if(target->baseSymbol == '~' && mob->effect_action != &effect_poison) {
		afflict(mob, &effect_poison, 5);
	}

	return true;
}

/**
 * Move a mob by a relative position
 * @param mob Entity to move.
 * @param xdiff x-coordinate difference.
 * @param ydiff y-coordinate difference.
 * @return If the mob was moved successfully.
 */
bool move_mob_relative(Mob * mob, int xdiff, int ydiff) {
	return move_mob(mob, mob->xpos + xdiff, mob->ypos + ydiff);
}

/**
 * Damage a mob.
 * @param mob Entity to damage.
 * @param damage Amount of damage to apply to the mob.
 * @return true if this killed the mob, a mob is dead if its health
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

		if(mob->next != NULL) {
			mob->next->prev = mob->prev;
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
 * A very simple enemy: move towards the player, and damage them if adjacent.
 * @param enemy Entity to move.
 */
void simple_enemy_turn(Mob * enemy) {
	Mob * player = enemy->level->player;

	int xdiff = enemy->xpos - player->xpos;
	int ydiff = enemy->ypos - player->ypos;

	/* If adjacent to the player, damage them */
	if((abs(xdiff) == 1 && ydiff == 0) || (xdiff == 0 && abs(ydiff) == 1)) {
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

/**
 * Moves the given mod to the next or previous level.
 * @param mob Mob to move.
 * @param toprev Determines which direction the movement is in. If true, the movement is to the previous level, otherwise to the next.
 * @return If the mob moved sucessfully.
 */
bool move_mob_level(Mob * mob, bool toprev) {
	Level * level = mob->level;
	int newx, newy;
	Level * newlevel;

	if (toprev) {
		if (level->prev == NULL) {
			/* top of the cave so no previous level */
			return false;
		}
		newlevel = level->prev;
		newx = newlevel->endx;
		newy = newlevel->endy;
	} else {
		if (level->next == NULL) {
			/* no next level so make one */
			level->next = xalloc(Level);
			build_level(level->next);
			level->next->prev = level;
			level->next->depth = level->depth + 1;
		}
		newlevel = level->next;
		newx = newlevel->startx;
		newy = newlevel->starty;
	}

	/* remove the mob from the current level */
	if(mob->level->mobs == mob) {
		mob->level->mobs = mob->next;
	}

	if (mob->prev != NULL) {
		mob->prev->next = mob->next;
	}
	if (mob->next != NULL) {
		mob->next->prev = mob->prev;
	}
	mob->next = NULL;
	mob->prev = NULL;
	level->cells[mob->xpos][mob->ypos]->occupant = NULL;

	/*put the mob in the new level */
	mob->level = newlevel;
	mob->next = newlevel->mobs;
	newlevel->mobs = mob;
	newlevel->cells[newx][newy]->occupant = mob;
	if (mob->next != NULL) {
		mob->next->prev = mob;
	}
	mob->xpos = newx;
	mob->ypos = newy;

	if (mob == level->player) {
		level->player = NULL;
		newlevel->player = mob;
	}

	return true;
}
