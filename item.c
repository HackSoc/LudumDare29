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
	char ** out = xcalloc(length(inventory) + 1, char *);

	unsigned int i = 0;
	for(List * list = inventory; list != NULL; list = list->next) {
		Item * theitem = fromlist(Item, inventory, list);
		out[i] = xcalloc(strlen(theitem->name) + 7 + 1, char);
		snprintf(out[i], strlen(theitem->name) + 7 + 1,
		         "%s [x%03i]", theitem->name, theitem->count);
		i ++;
	}

	out[i] = NULL;

	return (const char**)out;
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
	for (int i = 0; names[i] != NULL; i++) {
		xfree(names[i]);
	}
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
	for (int i = 0; names[i] != NULL; i++) {
		xfree(names[i]);
	}
	xfree(names);
	xfree(items);
	return out;
}

/**
 * Choose an item by type
 * @param inventory The inventory to choose from
 * @param type The type of the item
 * @param prompt The prompt
 * @param no_equipped Don't include equipped things
 * @return NULL if nothing was selected, otherwise a pointer to the choice.
 */
Item * choose_item_by_type(List * inventory,
                           enum ItemType type,
                           const char * prompt,
                           bool no_equipped) {
	const char ** names = xcalloc(length(inventory) + 1, char *);
	const void ** items = xcalloc(length(inventory) + 1, void *);

	unsigned int i = 0;
	for(List * list = inventory; list != NULL; list = list->next) {
		Item * item = fromlist(Item, inventory, list);
		if(item->type == type &&
		   ((no_equipped && !item->equipped) || !no_equipped)) {
			names[i] = item->name;
			items[i] = item;
			i ++;
		}
	}
	names[i] = NULL;
	items[i] = NULL;

	const void ** res = list_choice(false,
	                                prompt, prompt,
	                                false, true,
	                                names, items);

	xfree(names);
	xfree(items);
	if(res == NULL) {
		return NULL;
	} else {
		Item * out = (Item *) res[0];
		xfree(res);
		return out;
	}
}
