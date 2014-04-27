#ifndef ITEM_H
#define ITEM_H

#include <stdbool.h>
#include "list.h"

struct Equipment;
struct Item;

/**
 * Used to determine the type of some equipment.
 */
enum EquipmentType { NONE, WEAPON, ARMOUR };

/**
 * Some items are equipment, and so have a pointer to an equipment
 * struct, which defines its stats.
 */
typedef struct Equipment {
	struct Item * item; /**< The item with which this equipment is
	                       associated */

	enum EquipmentType type; /**< The type of the equiment. */
	unsigned int attack; /**< The attack damage of the item. */
	unsigned int defense; /**< The defense value of the item. */
} Equipment;

/**
 * Items are things that mobs can carry around, and possibly
 * equip. They live in inventories.
 */
typedef struct Item {
	char symbol; /**< The symbol to display when dropped */
	char * name; /**< The name to display when examined */

	List inventory; /**< The inventory to which this item belongs. */

	Equipment * equipment; /**< Pointer to the equipment stats for the
	                          item, will be NULL if the item is not
	                          equipment. */
} Item;

void display_inventory(List * inventory, const char * title);
List ** choose_items(List * inventory, const char * prompt);
Equipment * choose_equipment(List * inventory,
                             enum EquipmentType type,
                             const char * prompt);

#endif /*ITEM_H*/
