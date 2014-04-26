#ifndef LEVEL_H
#define LEVEL_H

struct Mob;
struct Item;

#include "mob.h"
#include "item.h"

#define LEVELWIDTH  80
#define LEVELHEIGHT 20

typedef struct Cell{
	char baseSymbol;
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

#endif /*LEVEL_H*/
