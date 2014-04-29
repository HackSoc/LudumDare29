#ifndef ITEM_H
#define ITEM_H

struct Mob;

#include <stdbool.h>
#include "list.h"
#include "mob.h"

/**
 * Used to determine the type of an item
 */
enum ItemType { NONE, WEAPON, ARMOUR, FOOD, DRINK, VALUABLE };

enum DefaultItem {
	PICKAXE,
	LANTURN,
	O_SWORD,
	HELMET,
	SWORD,
	CUTLASS,
	C_MAIL,
	D_MAIL,
	F_SWORD,
	FOOD_RATION,
	HARD_TACK,
	N_FOOD_RATION,
	MANNA,
	M_HELMET,
	BOOK_TAX,
	LAW_SUIT,
	CLOG,
	CLOG_APRON,
	HUNT_KNIFE,
	CLOAK,
	FRY_PAN,
	APRON,
	CAMERA,
	HAWAII,
	BONE,
	FURSUIT,
	C_POISON_POTION,
	GOLD,
	STONE,
	CORPSE,
	A_PICKAXE,
	FLESHBOOK,
	W_BOOT
};

/**
 * Items are things that mobs can carry around, and possibly
 * equip. They live in inventories.
 * Note: when a mob picks up or drops a luminous item, its (the mob)
 * luminosity should change by 1.
 */
typedef struct Item {
	enum ItemType type; /**< The type of the item. */
	int count; /**< The number of that item currently held. */

	char symbol; /**< The symbol to display when dropped */
	char * name; /**< The name to display when examined */

	bool luminous; /**< Whether the item is luminous or not */
	bool can_dig; /**< Whether the item is capable of digging through rock */
	bool ranged; /**< In the case of a weapon, whether it can be used for ranged combat */

	List inventory; /**< The inventory to which this item belongs. */

	bool equipped; /**< Whether the item is equipped or not */

	int value; /**< Some type-dependent value */
	void (*effect)(struct Mob *); /**< Some type-dependent effect */
	void (*fight_effect)(struct Mob *, struct Item *, struct Mob *, struct Mob *, unsigned int); /**< Called if the item is an equipped weapon or piece of armour in a fight */
} Item;

void display_inventory(List * inventory, const char * title);
List ** choose_items(List * inventory, const char * prompt);
Item * choose_item_by_type(List * inventory,
                           enum ItemType type,
                           const char * prompt,
                           bool no_equipped);

#endif /*ITEM_H*/
