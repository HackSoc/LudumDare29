#include <stdlib.h>
#include <stdbool.h>
#include <curses.h>
#include <string.h>

#include "item.h"
#include "utils.h"

/**
 * Display the list of items given, allowing for items to be chosen from the list.
 * @param items The items to display, as a doubly linked list.
 * @param isChoice Determines whether the player is allowed to choose items from the list, which are returned from the function.
 * @param prompt The prompt to be written at the start of the list.
 * @return The list of items chosen, or NULL if isChoice is false or no items were chosen.
 */
Item ** display_items(Item * items, bool isChoice, char * prompt) {
	Item ** item_array;
	bool * chosen;
	int num_items = 0;
	Item * item = NULL;
	int i, ch;
	unsigned int num_chosen = 0;

	/* count the items */
	for (item = items; item != NULL; item = item->next) {
		num_items++;
	}

	if(num_items == 0) {
		/* No items, so this is not a choice */
		isChoice = false;
	}

	item_array = xcalloc(num_items, Item *);
	chosen = xcalloc(num_items, bool);

	i = 0;
	for (item = items; item != NULL; item = item->next) {
		item_array[i] = item;
		i++;
	}

	clear();
	mvaddprintf(0, 0, "%s", prompt);
	for (i = 0; i < num_items; i++) {
		mvaddprintf(i+1, 0, "%c", 'a' + i);
		addprintf(" - ");
		addprintf("%s", item_array[i]->name);
	}

	if (!isChoice) {
		getch();

		xfree(item_array);
		xfree(chosen);

		return NULL;
	}

	while (true) {
		ch = getch();
		
		if ('a' <= ch && 'a' + num_items > ch) {
			chosen[ch-'a'] = !chosen[ch-'a'];
			if (chosen[ch-'a']) {
				num_chosen ++;
				mvaddprintf(1 + ch - 'a', 2, "+");
			} else {
				num_chosen --;
				mvaddprintf(1 + ch - 'a', 2, "-");
			}
		}
		if (ch <= 32 || ch >= 126) { 
			/* if ch is a non-printable ASCII character */
			break;
		} 
	} 
	
	/* construct the return list */
	Item ** selected = xcalloc(num_chosen + 1, Item *);
	unsigned int j = 0;
	for(i = 0; i < num_items; i++) {
		if(chosen[i]) {
			selected[j] = item_array[i];
			j++;
		}
	}
	selected[j] = NULL;
	
	clear();

	xfree(item_array);
	xfree(chosen);

	return selected;
}

/**
 * Choose a piece of equipment
 * @param inventory The inventory to choose from
 * @param type The type of the equipment
 * @param prompt The prompt
 * @return NULL if nothing was selected, otherwise a pointer to the choice.
 */
Item * choose_equipment(Item * inventory,
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

	char ** choices = xcalloc(num_pieces, char*);

	unsigned int i = 0;
	for(Item * item = inventory; item != NULL; item = item->next) {
		if(item->equipment != NULL && item->equipment->type == type) {
			choices[i] = item->name;
			i ++;
		}
	}

	char * choice = list_choice(1, 0, prompt, prompt, choices);
	for(Item * item = inventory; item != NULL; item = item->next) {
		if(strcmp(choice, item->name) == 0) {
			return item;
		}
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
