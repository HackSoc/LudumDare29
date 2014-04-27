#include <stdlib.h>
#include <stdbool.h>
#include <curses.h>

#include "item.h"
#include "utils.h"

/**
 * Display the list of items given, allowing for items to be chosen from the list.
 * @param items The items to display, as a doubly linked list.
 * @param isChoice Determines whether the player is allowed to choose items from the list, which are returned from the function.
 * @param prompt The prompt to be written at the start of the list.
 * @return The list of items chosen, or NULL if isChoice is false or no items were chosen.
 */
Item * display_items(Item * items, bool isChoice, char * prompt) {
	Item ** item_array;
	bool * chosen;
	int num_items = 0;
	Item * item = NULL;
	Item * chosen_items = NULL;
	int i, ch;
	bool done;

	/* count the items */
	for (item = items; item != NULL; item = item->next) {
		num_items++;
	}

	if(num_items == 0) {
		/* No items, so this is not a choice */
		isChoice = false;
	}

	item_array = xcalloc(num_items, Item*);
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
				mvaddprintf(1 + ch - 'a', 2, "+");
			} else {
				mvaddprintf(1 + ch - 'a', 2, "-");
			}
		}
		if (ch <= 32 || ch >= 126) { 
			/* if ch is a non-printable ASCII character */
			break;
		} 
	} 
	
	/* construct the return list */
	for (i = 0; i < num_items; i++) {
		if (chosen[i]) {
			if(item_array[i]->next != NULL) {
				item_array[i]->next->prev = item_array[i]->prev;
			}
			if(item_array[i]->prev != NULL) {
				item_array[i]->prev->next = item_array[i]->next;
			}
			
			item_array[i]->next = chosen_items;
			if(chosen_items != NULL) {
				chosen_items->prev = item_array[i];
			}
			chosen_items = item_array[i];
		}
	}
	
	clear();

	xfree(item_array);
	xfree(chosen);

	return chosen_items;
}
