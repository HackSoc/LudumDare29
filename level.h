#ifndef LEVEL_H
#define LEVEL_H

#include <stdbool.h>

struct Mob;
struct Item;

#include "mob.h"
#include "item.h"

/**
 * The width of a level in characters
 */
#define LEVELWIDTH  80

/**
 * The height of a level in characters
 */
#define LEVELHEIGHT 20

/**
 * A cell is an individual space in a level, they have a base symbol,
 * may be solid, may contain at most one occupant mob, and a list of
 * items.
 */
typedef struct Cell{
	char baseSymbol; /**< The symbol of the cell (floor, wall, etc) */
	bool solid; /**< Whether the cell is solid (impassible) or not */

	struct Mob * occupant; /**< The occpuant (may be NULL) */
	struct Item * items; /**< The list of items (may be NULL) */
} Cell;

/**
 * A level is the current part of the game which is active, it gets
 * rendered to the screen, has a bunch of mobs, and a single
 * player. Levels form a doubly-linked list.
 */
typedef struct Level{
	struct Level * next; /**< The level after this one */
	struct Level * prev; /**< The level before this one */

	struct Mob * mobs; /**< Doubly-linked list of mobs in the level */
	struct Mob * player; /**< The player mob (must also be in mobs) */

	struct Cell * cells[LEVELWIDTH][LEVELHEIGHT]; /**< The map */
} Level;

void build_level(Level * level);

/**
 * A "turn" consists of all of the mobs acting once, possibly followed
 * by some constant effect on the mob. As the player is a turn, this
 * is (indirectly) where blocking for input happens.
 */
void run_turn(Level * level);

/**
 * Render the level to the screen. The symbol for a level is picked
 * according to the following priorities: occupant > top item > base.
 */
void display_level(Level * level);

#endif /*LEVEL_H*/
