#ifndef LEVEL_H
#define LEVEL_H

#include <stdbool.h>

#include "mob.h"
#include "item.h"
#include "list.h"

/** The width of a level in characters. */
#define LEVELWIDTH  80

/** The height of a level in characters. */
#define LEVELHEIGHT 20

/**
 * A cell is an individual space in a level, they have a base symbol,
 * may be solid, may contain at most one occupant mob, and a list of
 * items.
 */
typedef struct Cell {
	char baseSymbol; /**< The symbol of the cell (floor, wall, etc). */
	int colour; /**< The colour to use to render the cell (if unoccupied). */

	bool solid;      /**< Whether the cell is solid (impassible) or not. */
	bool illuminated; /**< Whether the cell is lit by a light or not. */
	unsigned int luminosity; /**< Number of light sources in the cell */

	struct Mob * occupant; /**< The occpuant (may be NULL). */
	struct List * items;   /**< The list of items (may be NULL). */
} Cell;

/**
 * A level is the current part of the game which is active, it gets rendered
 * to the screen, has a bunch of mobs, and a single player.
 * Levels form a doubly-linked list.
 */
typedef struct Level {
	List levels; /**< The list of levels to which this belongs */

	List * mobs; /**< The list of mobs in the level. */
	struct Mob * player; /**< The player mob (must also be in mobs). */

	unsigned int depth; /**< The depth of the level.*/

	int startx, starty; /**< The x and y positions of the stairs from the previous level. */
	int endx, endy; /**< The x and y positions of the stairs to the next level. */

	struct Cell * cells[LEVELWIDTH][LEVELHEIGHT]; /**< The map. */
} Level;

void build_level(Level * level);
void run_turn(Level * level);
void display_level(Level * level);

#endif /* LEVEL_H */
