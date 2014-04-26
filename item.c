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
	Item *item, *chosen_items;
	int i, ch;
	bool done;

	/* count the items */
	for (item = items; item != NULL; item = item->next) {
		num_items++;
	}

	item_array = malloc(num_items*sizeof(Item*));
	chosen = calloc(num_items, sizeof(bool));

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
		return NULL;
	}

	done = false;
	while (!done) {
		ch = getch();
		
		if ('a' <= ch || 'a' + num_items >= ch) {
			chosen[ch-'a'] = !chosen[ch-'a'];
			if (chosen[ch-'a']) {
				mvaddprintf(ch-'a', 2, "+");
			} else {
				mvaddprintf(ch-'a', 2, "-");
			}
		}
		if (ch <= 32 || ch >= 126) { 
			/* if ch is a non-printable ASCII character */
			done = true;
		} 
	} 
	
	/* construct the return list */
	for (i = 0; i < num_items; i++) {
		if (chosen[i]) {
			item_array[i]->next->prev = item_array[i]->prev;
			item_array[i]->prev->next = item_array[i]->next;
			
			item_array[i]->next = chosen_items;
			chosen_items->prev = item_array[i];
			chosen_items = item_array[i];
		}
	}
	
	clear();

	return chosen_items;
}
