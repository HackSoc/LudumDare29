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

#endif /*MOB_H*/
