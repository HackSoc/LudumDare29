#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "mob.h"
#include "level.h"
#include "utils.h"
#include "effect.h"
#include "player.h"

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
 * Attack a mob, modified by the weapon of the attacker and the armour
 * of the defender.
 * @param attacker The mob doing the attacking.
 * @param defender The mob being attacked.
 */
void attack_mob(Mob * attacker, Mob * defender) {
	int damage = attacker->attack;

	if(attacker->weapon != NULL) {
		damage += attacker->weapon->attack;
	}

	if(defender->armour != NULL) {
		damage -= defender->armour->defense;
	}

	/* Can always do at least 1 damage */
	if(damage < 1) {
		damage = 1;
	}

	damage_mob(defender, (unsigned int) damage);
}

/**
 * Damage a mob.
 * @param mob Entity to damage.
 * @param damage Amount of damage to apply to the mob.
 * @return If this killed the mob, a mob is dead if its health
 * drops to zero or below.
 */
bool damage_mob(Mob * mob, unsigned int damage) {
	mob->health -= damage;

	return (mob->health <= 0);
}

/**
 * Kill a mob - free it, and remove it from the lists.
 * @param mob The mob to kill
 * @return The next mob in the list, or NULL
 */
Mob * kill_mob(Mob * mob) {
	if(mob->death_action != NULL) {
		mob->death_action(mob);
	}

	Level * level = mob->level;
	Cell * cell = level->cells[mob->xpos][mob->ypos];
	Mob * next = fromlist(Mob, moblist, mob->moblist.next);

	/* Remove it from the cell */
	cell->occupant = NULL;

	/* Remove from the mob list */
	level->mobs = drop(&mob->moblist);

	/* Drop its items */
	cell->items = append(cell->items, mob->inventory);

	/* Only the player needs to have its strings free'd */
	if (mob->level->player == mob) {
		xfree(mob->name);
		xfree(mob->profession);
		xfree(mob->race);
	}
	xfree(mob);

	return next;
}

/**
 * Determine if a mob can see the given point. All points are visible
 * unless there is a wall in the way. This uses Bresenham's line
 * algorithm to determine line-of-sight.
 *
 * Note: as this will be primarily used to render the level, a better
 * version might be to operate on a 2d array of three-state variables
 * ("visible", "blocked", and "unknown"), and just iterate the
 * algorithm with different starting points until every point is
 * known. This would avoid the need to check each individual point,
 * possibly duplicating work.
 *
 * @param mob The mob
 * @param x The target X coordinate
 * @param y The target Y coordinate
 */
bool can_see(Mob * mob, unsigned int x, unsigned int y) {
	Level * level = mob->level;

	unsigned int x0 = mob->xpos;
	unsigned int y0 = mob->ypos;

	int dx = abs(x0 - x);
	int dy = abs(y0 - y) ;

	int sx = (x0 < x) ? 1 : -1;
	int sy = (y0 < y) ? 1 : -1;

	int err = dx - dy;

	while(x0 != x || y0 != y) {
		if(level->cells[x0][y0]->baseSymbol == '#') {
			return false;
		}

		int e2 = err * 2;

		if(e2 > -dy) {
			err -= dy;
			x0 += sx;
		}

		if(e2 < dx) {
			err += dx;
			y0 += sy;
		}
	}

	return true;
}

/**
 * Wrapper for can_see, to determine if a mob can see another mob.
 * @param moba One of the mobs
 * @param mobb The other. It really doesn't matter which way around they are.
 */
bool can_see_other(Mob * moba, Mob * mobb) {
	return can_see(moba, mobb->xpos, mobb->ypos);
}

/**
 * A very simple enemy: move towards the player, and damage them if adjacent.
 * @param enemy Entity to move.
 */
void simple_enemy_turn(Mob * enemy) {
	Mob * player = enemy->level->player;

	/* If we can't see the player, move randomly */
	if(!can_see_other(enemy, player)) {
		if(rand() % 2 == 0) {
			move_mob_relative(enemy, (rand() % 2 == 0) ? 1 : -1, 0);
		} else {
			move_mob_relative(enemy, 0, (rand() % 2 == 0) ? 1 : -1);
		}
		return;
	}

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
		if (level->levels.prev == NULL) {
			/* top of the cave so no previous level */
			return false;
		}
		newlevel = fromlist(Level, levels, level->levels.prev);
		newx = newlevel->endx;
		newy = newlevel->endy;
	} else {
		if (level->levels.next == NULL) {
			/* no next level so make one */
			Level * nextlevel = xalloc(Level);
			build_level(nextlevel);
			nextlevel->levels.prev = &level->levels;
			nextlevel->depth = level->depth + 1;
			level->levels.next = &nextlevel->levels;
			/* Assumes only the player can create levels */
			mob->score += 25; // arbitrary value
		}
		newlevel = fromlist(Level, levels, level->levels.next);
		newx = newlevel->startx;
		newy = newlevel->starty;
	}

	/* remove the mob from the current level */
	mob->level->mobs = drop(&mob->moblist);
	level->cells[mob->xpos][mob->ypos]->occupant = NULL;

	/* Puts the mob in the new level, inserting
	   it at the front of the list of mobs */
	mob->level = newlevel;
	newlevel->mobs = insert(newlevel->mobs, &mob->moblist);
	newlevel->cells[newx][newy]->occupant = mob;
	mob->xpos = newx;
	mob->ypos = newy;

	if (mob == level->player) {
		PlayerData * playerdata = (PlayerData *)level->player->data;
		
		level->player = NULL;
		newlevel->player = mob;

		if(toprev) {
			playerdata->terrain = fromlist(Terrain, levels,
			                               playerdata->terrain->levels.prev);
		} else {
			List * next = playerdata->terrain->levels.next;
			if(next == NULL) {
				Terrain * nextlevel = xalloc(Terrain);
				memset(nextlevel->symbols, ' ', LEVELWIDTH * LEVELHEIGHT);
				playerdata->terrain->levels.next = &nextlevel->levels;
				nextlevel->levels.prev = &playerdata->terrain->levels;
				playerdata->terrain = nextlevel;
			} else {
				playerdata->terrain = fromlist(Terrain, levels, next);
			}
		}
	}

	return true;
}
