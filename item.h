#ifndef ITEM_H
#define ITEM_H

#include <stdbool.h>

typedef struct Item {
	char symbol;
	char * name;
	struct Item * next;
	struct Item * prev;
} Item;

Item ** display_items(Item * items, bool isChoice, char * prompt);
Item * remove_items(Item * inventory, Item ** items);
Item * add_items(Item * inventory, Item ** items);

#endif /*ITEM_H*/
