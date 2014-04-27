#include <stdlib.h>
#include <stdbool.h>
#include <curses.h>
#include <string.h>

#include "item.h"
#include "utils.h"

/**
 * Convert an inventory doubly-linked list into an array of pointers
 * to names of members of the inventory.
 * @param inventory The inventory to convert
 */
static const char ** inventory_name_array(Item * inventory) {
	unsigned int num_items = 0;
	for(Item * item = inventory; item != NULL; item = item->next) {
		num_items ++;
	}

	const char ** out = xcalloc(num_items + 1, char *);

	unsigned int i = 0;
	for(Item * item = inventory; item != NULL; item = item->next) {
		out[i] = item->name;
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
static const Item ** inventory_array(Item * inventory) {
	unsigned int num_items = 0;
	for(Item * item = inventory; item != NULL; item = item->next) {
		num_items ++;
	}

	const Item ** out = xcalloc(num_items + 1, Item *);

	unsigned int i = 0;
	for(Item * item = inventory; item != NULL; item = item->next) {
		out[i] = item;
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
void display_inventory(Item * inventory, const char * title) {
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
Item ** choose_items(Item * inventory, const char * prompt){
	const char ** names = inventory_name_array(inventory);
	const Item ** items = inventory_array(inventory);
	Item ** out = (Item **)list_choice(false,
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
Equipment * choose_equipment(Item * inventory,
							 enum EquipmentType type,
							 const char * prompt) {
	unsigned int num_pieces = 0;

	for(Item * item = inventory; item != NULL; item = item->next) {
		if(item->equipment != NULL && item->equipment->type == type) {
			num_pieces ++;
		}
	}

	if(num_pieces == 0) {
		return NULL;
	}

	const char ** names = xcalloc(num_pieces, char *);
	const void ** equipment = xcalloc(num_pieces, void *);

	unsigned int i = 0;
	for(Item * item = inventory; item != NULL; item = item->next) {
		if(item->equipment != NULL && item->equipment->type == type) {
			names[i] = item->name;
			equipment[i] = item->equipment;
			i ++;
		}
	}

	const void ** res = list_choice(false,
									prompt, prompt,
									false, true,
									names, equipment);

	if(res == NULL) {
		return NULL;
	} else {
		Equipment * out = (Equipment *) res[0];
		xfree(res);
		return out;
	}
}

/**
 * Remove a list of items from an inventory. Returns the (possibly
 * new) head of the inventory.
 * @param inventory The inentory
 * @param items The items to remove
 */
Item * remove_items(Item * inventory, Item ** items) {
	Item * head = inventory;
	Item * cur = head;

	while(cur != NULL) {
		for(unsigned int i = 0; items[i] != NULL; i++) {
			if(cur == items[i]) {
				if(cur == head) {
					head = head->next;
					if(head != NULL) {
						head->prev = NULL;
					}
				}

				if(cur->next != NULL) {
					cur->next->prev = cur->prev;
				}
				if(cur->prev != NULL) {
					cur->prev->next = cur->next;
				}

				cur = cur->next;
				break;
			}
		}
		if(cur != NULL) {
			cur = cur->next;
		}
	}

	return head;
}

/**
 * Add a list of items to an inventory. Returns the head of the inventory.
 * @param inventory The inventory
 * @param items The items to add
 */
Item * add_items(Item * inventory, Item ** items) {
	Item * head = NULL;
	Item * cur = NULL;

	for(unsigned int i = 0; items[i] != NULL; i ++) {
		items[i]->prev = cur;

		if(cur != NULL) {
			cur->next = items[i];
		}

		cur = items[i];

		if(head == NULL) {
			head = cur;
		}
	}

	if(head == NULL) {
		return inventory;
	} else {
		cur->next = inventory;

		if(inventory != NULL) {
			inventory->prev = cur;
		}

		return head;
	}
}
