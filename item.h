#ifndef ITEM_H
#define ITEM_H

#include <stdbool.h>

typedef struct Item {
	char symbol;
	char * name;
	struct Item * next;
	struct Item * prev;
} Item;

Item * display_items(Item * items, bool isChoice, char * prompt);

#endif /*ITEM_H*/
