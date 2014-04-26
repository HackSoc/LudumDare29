#ifndef MOB_H
#define MOB_H

struct Item;
struct Level;

#include "item.h"
#include "level.h"

typedef struct Mob {
	struct Level * level;
	char symbol;
	struct Mob * next;
	struct Mob * prev;
	struct Item * items;
	void (*action)(struct Mob *);

	/* Statistics - can be NULL/0/whatever for mobs where these are
	   not relevent. */
	char* name;
	char* race;
	char* profession;
	unsigned int health;
	unsigned int max_health;
} Mob;

#endif /*MOB_H*/
