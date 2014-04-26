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
	struct Cell * cells[LEVELWIDTH][LEVELHEIGHT];
} Level;

void run_turn(Level * level);
void display_level(Level * level);

/**
 * Move the given mob to the new coordinates
 * Returns false if the given space can't be moved into.
 */
bool move_mob(struct Mob * mob, unsigned int x, unsigned int y);

#endif /*LEVEL_H*/
