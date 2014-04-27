#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <curses.h>
#include <string.h>

#include "item.h"
#include "utils.h"
#include "list.h"

/**
 * Convert an inventory doubly-linked list into an array of pointers
 * to names of members of the inventory.
 * @param inventory The inventory to convert
 */
static const char ** inventory_name_array(List * inventory) {
	const char ** out = xcalloc(length(inventory) + 1, char *);

	unsigned int i = 0;
	for(List * list = inventory; list != NULL; list = list->next) {
		Item * theitem = fromlist(Item, inventory, list);
		out[i] = theitem->name;
		i ++;
	}

	out[i] = NULL;

	return out;
}

/**
 * Convert an inventory doubly-linked list into an array of pointers
 * to members of the inventory.
 * @param inventory The inventory to convert
 */
static const List ** inventory_array(List * inventory) {
	const List ** out = xcalloc(length(inventory) + 1, List *);

	unsigned int i = 0;
	for(List * list = inventory; list != NULL; list = list->next) {
		out[i] = list;
		i ++;
	}

	out[i] = NULL;

	return out;
}

/**
 * Display an inventory
 * @param inventory The inventory to display
 * @param title The title to display
 */
void display_inventory(List * inventory, const char * title) {
	const char ** names = inventory_name_array(inventory);
	list_choice(true,
	            title, NULL,
	            false, false,
	            names, NULL);
	xfree(names);
}

/**
 * Choose some items from an inventory
 * @param inventory The inventory to choose from
 * @param prompt The prompt to display
 */
List ** choose_items(List * inventory, const char * prompt){
	const char ** names = inventory_name_array(inventory);
	const List ** items = inventory_array(inventory);
	List ** out = (List **)list_choice(false,
	                                   prompt, prompt,
	                                   true, true,
	                                   names, (const void **)items);
	xfree(names);
	xfree(items);
	return out;
}

/**
 * Choose a piece of equipment
 * @param inventory The inventory to choose from
 * @param type The type of the equipment
 * @param prompt The prompt
 * @return NULL if nothing was selected, otherwise a pointer to the choice.
 */
Equipment * choose_equipment(List * inventory,
                             enum EquipmentType type,
                             const char * prompt) {

	const char ** names = xcalloc(length(inventory) + 1, char *);
	const void ** equipment = xcalloc(length(inventory) + 1, void *);

	unsigned int i = 0;
	for(List * list = inventory; list != NULL; list = list->next) {
		Item * item = fromlist(Item, inventory, list);
		if(item->equipment != NULL && item->equipment->type == type) {
			names[i] = item->name;
			equipment[i] = item->equipment;
			i ++;
		}
	}
	names[i] = NULL;
	equipment[i] = NULL;

	const void ** res = list_choice(false,
	                                prompt, prompt,
	                                false, true,
	                                names, equipment);

	xfree(names);
	xfree(equipment);
	if(res == NULL) {
		return NULL;
	} else {
		Equipment * out = (Equipment *) res[0];
		xfree(res);
		return out;
	}
}
