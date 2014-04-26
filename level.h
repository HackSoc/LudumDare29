#ifndef LEVEL_H
#define LEVEL_H

#include <stdbool.h>

struct Mob;
struct Item;

#include "mob.h"
#include "item.h"

#define LEVELWIDTH  80
#define LEVELHEIGHT 20

typedef struct Cell{
	char baseSymbol;
	bool solid;
	struct Mob * occupant;
	struct Item * items;
} Cell;

typedef struct Level{
	struct Level * next;
	struct Level * prev;
	struct Mob * mobs;
	struct Mob * player;
	struct Cell * cells[LEVELWIDTH][LEVELHEIGHT];
} Level;

void run_turn(Level * level);
void display_level(Level * level);

/**
 * Move the given mob to the new coordinates
 * Returns false if the given space can't be moved into.
 */
bool move_mob(struct Mob * mob, unsigned int x, unsigned int y);

/**
 * Move a mob by a relative position
 */
bool move_mob_relative(struct Mob * mob, int xdiff, int ydiff);

/**
 * Damage a mob
 * Returns true if this killed the mob.
 */
bool damage_mob(struct Mob * mob, unsigned int amount);

/**
 * Move and attack at the same time - if a mob is in the target cell,
 * damage it, but don't move.
 * Should only be usef by the player, as this checks `hostile`.
 */
bool attackmove(struct Mob * player, unsigned int xdiff, unsigned int ydiff,
				unsigned int damage);

/**
 * Like attackmove, but relative position
 */
bool attackmove_relative(struct Mob * player, int xdiff, int ydiff,
						 unsigned int damage);
#endif /*LEVEL_H*/
