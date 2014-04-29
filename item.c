#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <curses.h>
#include <string.h>

#include "item.h"
#include "utils.h"
#include "list.h"
#include "effect.h"

/** Definitions of special items. */
#define ITEM(sym, n, t, val, dig, lit, range, eff, atkeff) {	  \
		.count = 1, .symbol = (sym), .name = (n), .type = (t),\
        .value = (val), .can_dig = (dig), .luminous = (lit),\
		.ranged = (range), .effect = (eff), .fight_effect = (atkeff)}
#define ITEM_D(sym, n, t, val) ITEM(sym, n, t, val, true, false, false, NULL, NULL)
#define ITEM_L(sym, n, t, val) ITEM(sym, n, t, val, false, true, false, NULL, NULL)
#define ITEM_N(sym, n, t, val) ITEM(sym, n, t, val, false, false, false, NULL, NULL)
#define ITEM_F(sym, n, t, val, atkeff) ITEM(sym, n, t, val, false, false, false, NULL, atkeff)
#define ITEM_R(sym, n, t, val) ITEM(sym, n, t, val, false, false, true, NULL, NULL)
#define ITEM_E(sym, n, t, val, eff) ITEM(sym, n, t, val, false, false, false, eff, NULL)

/* Should keep the same structure as DefaultItem in item.h. */
const struct Item default_items[] = {
	ITEM_D('/', "Pickaxe",                WEAPON,  5),
	ITEM_L('^', "Lantern",                WEAPON,  1),
	ITEM_N('/', "Orcish Sword",           WEAPON,  5),
	ITEM_N(']', "Helmet",                 ARMOUR,  3),
	ITEM_N('/', "Sword",                  WEAPON, 10),
	ITEM_N(']', "Chain Mail",             ARMOUR,  7),
	ITEM_N(']', "Dragon Scale Mail",      ARMOUR, 15),
	ITEM_F('/', "Flaming Sword of Fire", WEAPON, 10, &inflict_fire),
	ITEM_N('%', "Food Ration",            FOOD,    5),
	ITEM_N('%', "Nourishing Food Ration", FOOD,    7),
	ITEM_N('%', "Manna",                  FOOD,   50),
	ITEM_L('n', "Mining Helmet",          ARMOUR,  5),
	ITEM_N('v', "Book of Tax Code",       WEAPON,  2),
	ITEM_F('r', "Law Suit",               ARMOUR,  8, &reflect_damage),
	ITEM_R('c', "Clog",                   WEAPON,  2),
	ITEM_N('y', "Clogging Apron",         ARMOUR,  8),
	ITEM_N('\'', "Hunting Knife",         WEAPON,  8),
	ITEM_N('h', "Cloak",                  ARMOUR,  2),
	ITEM_N('q', "Frying Pan",             WEAPON,  6),
	ITEM_N('y', "Apron",                  ARMOUR,  4),
	ITEM_N('k', "Camera",                 WEAPON,  1),
	ITEM_N('t', "Hawaiian Shirt",         ARMOUR,  9),
	ITEM_N('l', "Bone",                   WEAPON,  3),
	ITEM_N('d', "Fursuit",                ARMOUR,  7),
	ITEM_E('-', "Potion of Cure Poison",  DRINK,   0, &cure_poison),
	ITEM_N('$', "Gold",                  VALUABLE, 5),
	ITEM_N('*', "Stone",                  NONE,    0),
	ITEM_E('%', "Corpse",                 FOOD,    4, &corpse_effect),
	ITEM_N('/', "Advanced Mining Pickaxe", WEAPON, 18),
	ITEM_N('v', "Book of Tax Code Bound in Human Flesh", WEAPON, 15),
	ITEM_R('c', "Wooden Boot",            WEAPON, 15),
};

#undef ITEM_N
#undef ITEM_L
#undef ITEM_D
#undef ITEM

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
		out[i] = xcalloc(strlen(theitem->name) + 7 + 1, char);
		snprintf((char *)out[i], strlen(theitem->name) + 7 + 1,
		         "%s [x%03i]", theitem->name, theitem->count);
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
			names[i] = xcalloc(strlen(item->name) + 7 + 1, char);
			snprintf((char *)names[i], strlen(item->name) + 7 + 1,
					 "%s [x%03i]", item->name, item->count);
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

	for (int i = 0; names[i] != NULL; i++) {
		xfree(names[i]);
	}
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
