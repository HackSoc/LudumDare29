#ifndef MOB_H
#define MOB_H

struct Item;
struct Level;

#include "item.h"
#include "level.h"
#include "utils.h"
#include "list.h"

/**
 * A mob is something which roams around the world, they are tied to a
 * level, and all the mobs in one level form a doubly-linked
 * list. There are a couple of callbacks associated with them to
 * determine what happens in certain situations.
 */
typedef struct Mob {
	List moblist; /**< The list of mobs to which this belongs */

	struct Level * level; /**< The level the mob is in. */
	unsigned int xpos;    /**< The X position. */
	unsigned int ypos;    /**< The Y position. */

	char symbol;  /**< The symbol to use to render the mob. */
	int colour;   /**< The colour to use to render the mob. */
	bool is_bold; /**< Whether to render the mob bold. */

	List * inventory; /**< List of items the mob is holding. */
	struct Item * weapon; /**< The weapon of the mob. */
	struct Item * offhand; /**< The offhand weapon of the mob. */
	struct Item * armour; /**< The armour of the mob. */

	void (*turn_action)(struct Mob *);  /**< What to do every turn. */
	void (*death_action)(struct Mob *); /**< What to do on death. */

	int effect_duration; /**< How long the current efefct will last. */
	void (*effect_action)(struct Mob *); /**< The effect to apply every turn. */

	bool hostile; /**< A mob is hostile if the player can damage it. */

	char* name;       /**< The name of the mob (may be NULL for NPC). */
	char* race;       /**< The race of the mob (may be NULL for NPC). */
	char* profession; /**< The job of the mob (may be NULL for NPC). */
	int score;        /**< The score of the mob (ignored for NPCs). */

	unsigned int attack; /**< The unarmed attack strength of the mob. */
	unsigned int defense; /**< The unarmoured defense strength of the mob. */

	int health; /**< The current health, signed to prevent underflow. */
	unsigned int max_health; /**< The maximum health. */
	unsigned int con; /**< The constitution of the mob. */

	bool darksight; /**< Whether the mob can see in the dark or not. */
	unsigned int luminosity; /**< Number of light sources the mob is holding. */

	unsigned int min_depth; /**< The minimum depth for the mob to appear at,
	                         * ignored for the player */

	void * data; /**< Mob type specific data, eg PlayerData */
} Mob;

bool move_mob(struct Mob * mob, unsigned int x, unsigned int y);
bool move_mob_relative(struct Mob * mob, int xdiff, int ydiff);
bool move_mob_level(Mob * mob, bool toprev);
bool damage_mob(struct Mob * mob, unsigned int amount);
void attack_mob(Mob * attacker, Mob * defender);
struct Mob * kill_mob(struct Mob * mob);
bool can_see_point(struct Level * level,
                   unsigned int x0, unsigned int y0,
                   unsigned int x, unsigned int y);
bool can_see(struct Mob * mob, unsigned int x, unsigned int y);
bool can_see_other(struct Mob * moba, struct Mob * mobb);
void simple_enemy_turn(struct Mob * enemy);
void drop_corpse(struct Mob * mob);
void drop_item(struct Mob * mob, struct Item * item);
void drop_items(struct Mob * mob, struct List ** items);
void pickup_item(struct Mob * mob, struct Item * item);
void pickup_items(struct Mob * mob, struct List ** items);
void wield_item(struct Mob * mob, struct Item * item);
void unwield_item(struct Mob * mob, struct Item * item);
void heal_mob(struct Mob * mob, unsigned int amount);
void consume_item(struct Mob * mob, struct Item * item);

#endif /*MOB_H*/
