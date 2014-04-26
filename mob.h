#ifndef MOB_H
#define MOB_H

struct Item;
struct Level;

#include "item.h"
#include "level.h"

typedef struct Mob {
	struct Level * level;
	int xpos, ypos;
	char symbol;
	struct Mob * next;
	struct Mob * prev;
	struct Item * items;
	void (*turn_action)(struct Mob *);
	void (*death_action)(struct Mob *);

	/* A mob is hostile if the player can damage it */
	bool hostile;

	/* Statistics - can be NULL/0/whatever for mobs where these are
	   not relevent. */
	char* name;
	char* race;
	char* profession;
	unsigned int health;
	unsigned int max_health;
} Mob;

#endif /*MOB_H*/
