#ifndef MOB_H
#define MOB_H

struct Item;
struct Level;

#include "item.h"
#include "level.h"

/**
 * A mob is something which roams around the world, they are tied to a
 * level, and all the mobs in one level form a doubly-linked
 * list. There are a couple of callbacks associated with them to
 * determine what happens in certain situations.
 */
typedef struct Mob {
	struct Mob * next; /**< The next mob in the list. */
	struct Mob * prev; /**< The prior mob in the list. */

	struct Level * level; /**< The level the mob is in. */
	unsigned int xpos;    /**< The X position. */
	unsigned int ypos;    /**< The Y position. */

	char symbol; /**< The symbol to use to render the mob. */

	struct Item * items; /**< List of items the mob is holding. */

	void (*turn_action)(struct Mob *);  /**< What to do every turn. */
	void (*death_action)(struct Mob *); /**< What to do on death. */

	bool hostile; /**< A mob is hostile if the player can damage it. */

	char* name; /**< The name of the mob (may be NULL for NPC). */
	char* race; /**< The race of the mob (may be NULL for NPC). */
	char* profession; /**< The job of the mob (may be NULL for NPC). */

	int health; /**< The current health, signed to prevent underflow. */
	unsigned int max_health; /**< The maximum health. */
} Mob;

bool move_mob(struct Mob * mob, unsigned int x, unsigned int y);
bool move_mob_relative(struct Mob * mob, int xdiff, int ydiff);
bool move_mob_level(Mob * mob, bool toprev);
bool damage_mob(struct Mob * mob, unsigned int amount);
void simple_enemy_turn(Mob * enemy);

#endif /*MOB_H*/
