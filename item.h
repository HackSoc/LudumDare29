#ifndef ITEM_H
#define ITEM_H

#include <stdbool.h>

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

	struct Item * next; /**< The next item in the inventory */
	struct Item * prev; /**< The last item in the inventory */

	Equipment * equipment; /**< Pointer to the equipment stats for the
	                          item, will be NULL if the item is not
	                          equipment. */
} Item;

void display_inventory(Item * inventory, const char * title);
Item ** choose_items(Item * inventory, const char * prompt);
Equipment * choose_equipment(Item * inventory,
                             enum EquipmentType type,
                             const char * prompt);
Item * remove_items(Item * inventory, Item ** items);
Item * add_items(Item * inventory, Item ** items);

#endif /*ITEM_H*/
