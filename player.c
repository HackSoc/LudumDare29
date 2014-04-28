#include <stdbool.h>
#include <curses.h>
#include <string.h>

#include "mob.h"
#include "level.h"
#include "utils.h"
#include "player.h"
#include "effect.h"
#include "status.h"

const char * names[] = {"Colin",
                        NULL};

const char * races[] = {"Human",
                        "Dutch",
                        "Elf",
                        "Dwarf",
                        "Halfling",
                        "Quarterling",
                        NULL};

const char * professions[] = {"Miner",
                              "Attorney",
                              "Clog Maker",
                              "Huntsman",
                              "Chef",
                              "Tourist",
                              "Dog",
                              NULL};

extern bool quit;

/**
 * Randomise a player's name, race, and profession.
 * @param player The player
 */
static void randomise_player(Mob * player) {
	player->name = strdup((char *) random_choice((const void**) names));
	player->race = strdup((char *) random_choice((const void**) races));
	player->profession = strdup((char *) random_choice((const void**) professions));
}

/**
 * Have the human player enter the character player's name, race, and
 * profession (the opposite of randomise_player).
 * @param player The player
 */
static void design_player(Mob * player) {
	clear();
	echo();
	mvaddprintf(9, 10, "Enter your player's name: ");
	char buf[80];
	getnstr(buf, 79);
	player->name = strdup(buf);
	noecho();

	const void ** race = list_choice(false,
	                                 "What is your race?",
	                                 "Don't be silly, choose a proper race.",
	                                 false,
	                                 false,
	                                 races,
	                                 (const void **)races);
	player->race = strdup((char *) race[0]);

	const void ** profession = list_choice(false,
	                                       "And finally, what is your profession?",
	                                       "That's not a real job!",
	                                       false,
	                                       false,
	                                       professions,
	                                       (const void **)professions);
	player->profession = strdup((char*) profession[0]);
}

/**
 * Create and return a new player. This prompts the user for stuff,
 * and clears the screen when it is done.
 */
Mob * create_player() {
	Mob * player = xalloc(Mob);
	player->symbol = '@';
	player->colour = COLOR_WHITE;
	player->is_bold = true;
	player->hostile = false;
	player->turn_action = &player_turn;
	player->death_action = &player_death;
	player->health = 100;
	player->max_health = 100;
	player->con = 3;
	player->score = 0;
	player->darksight = false;

	/* Initialise the terrain knowledge to nothing */
	PlayerData * playerdata = xalloc(PlayerData);
	player->data = (void *)playerdata;
	playerdata->terrain = xalloc(Terrain);
	memset(playerdata->terrain->symbols, ' ', LEVELWIDTH * LEVELHEIGHT);

	Item * stone = xalloc(Item);
	stone->symbol = '*';
	stone->name = "Stone";

	Item * coin = xalloc(Item);
	coin->symbol = '$';
	coin->name = "Coin";

	Item * sword = xalloc(Item);
	sword->symbol = '/';
	sword->name = "Sword";
	sword->type = WEAPON;
	sword->value = 10;

	Item * lantern = xalloc(Item);
	lantern->symbol = '^';
	lantern->name = "Lantern";
	lantern->type = WEAPON;
	lantern->luminous = true;
	lantern->value = 1;

	Item * potion = xalloc(Item);
	potion->symbol = '-';
	potion->name = "Potion of Cure Poison";
	potion->type = DRINK;
	potion->effect = &cure_poison;
   
	Item * pickaxe = xalloc(Item);
	pickaxe->symbol = '/';
	pickaxe->name = "Pickaxe";
	pickaxe->type = WEAPON;
	pickaxe->can_dig = true;
	pickaxe->value = 5;

	player->inventory = insert(player->inventory, &stone->inventory);
	player->inventory = insert(player->inventory, &coin->inventory);
	player->inventory = insert(player->inventory, &sword->inventory);
	player->inventory = insert(player->inventory, &lantern->inventory);
	player->inventory = insert(player->inventory, &potion->inventory);
	player->inventory = insert(player->inventory, &pickaxe->inventory);

	clear();
	mvaddprintf(9, 10, "Do you want to randomly generate your player? ");
	while(true) {
		int choice = getch();
		if(choice == 'y' || choice == 'Y') {
			randomise_player(player);
			break;
		} else if(choice == 'n' || choice == 'N') {
			design_player(player);
			break;
		}
	}

	clear();
	return player;
}

/**
 * Move and attack at the same time - if a mob is in the target cell,
 * damage it, but don't move.
 * @param player Entity representing the player.
 * @param x x-coordinate to move to.
 * @param y y-coordinate to move to.
 * @return If the player damaged a mob.
 */
bool attackmove(Mob * player, unsigned int x, unsigned int y) {
	Mob * mob = player->level->cells[x][y]->occupant;

	if(!move_mob(player, x, y) && mob != NULL && mob->hostile) {
		attack_mob(player, mob);
		return true;
	}
	return false;
}

/**
 * Like attackmove, but relative position.
 * @param player Entity representing the player.
 * @param xdiff Difference in the x-axis to move.
 * @param ydiff Difference in the y-axis to move.
 * @return If the player damaged a mob.
 */
bool attackmove_relative(Mob * player, int xdiff, int ydiff) {
	unsigned int x = player->xpos + xdiff;
	unsigned int y = player->ypos + ydiff;

	return attackmove(player, x, y);
}

/**
 * Wait for user input, and then act accordingly.
 * @param player Player entity.
 */
void player_turn(Mob * player) {
	List ** items;
	Item * item;
	Cell * current_cell = player->level->cells[player->xpos][player->ypos];
	bool done = false;

	bool move = false;
	int xdiff = 0;
	int ydiff = 0;

	while(!done && !quit) {
		display_level(player->level);

		int ch = getch();
		switch (ch) {
			/* Movement in a level */
		case 'k':
		case '8':
		case KEY_UP:
			move = true;
			done = true;
			xdiff =  0; ydiff = -1;
			break;

		case 'j':
		case '2':
		case KEY_DOWN:
			move = true;
			done = true;
			xdiff =  0; ydiff =  1;
			break;

		case 'h':
		case '4':
		case KEY_LEFT:
			move = true;
			done = true;
			xdiff = -1; ydiff =  0;
			break;

		case 'l':
		case '6':
		case KEY_RIGHT:
			move = true;
			done = true;
			xdiff =  1; ydiff =  0;
			break;

		case 'y':
		case '7':
			move = true;
			done = true;
			xdiff = -1; ydiff = -1;
			break;

		case 'u':
		case '9':
			move = true;
			done = true;
			xdiff =  1; ydiff = -1;
			break;

		case 'n':
		case '3':
			move = true;
			done = true;
			xdiff =  1; ydiff =  1;
			break;

		case 'b':
		case '1':
			move = true;
			done = true;
			xdiff = -1; ydiff =  1;
			break;

		/* Movement between levels */
		case '>':
			if (current_cell->baseSymbol == '>'){
				status_push("You descend deeper into the caves.");
				move_mob_level(player, false);
			}
			done = true;
			break;

		case '<':
			if (current_cell->baseSymbol == '<'){
				status_push("You ascend towards the fresh air.");
				move_mob_level(player, true);
			}
			done = true;
			break;

			/* Inventory management */
		case 'i':
			display_inventory(player->inventory, "Inventory Contents:");
			break;

		case 'd':
			items = choose_items(player->inventory, "Select items to drop:");

			/* Update the status */
			for(unsigned int i = 0; items[i] != NULL; i++) {
				Item * item = fromlist(Item, inventory, items[i]);
				status_push("You drop the %s.", item->name);
			}

			drop_items(player, items);
			xfree(items);
			break;

		case ',':
			items = choose_items(current_cell->items, "Select items to pick up:");

			for(unsigned int i = 0; items[i] != NULL; i++) {
				Item * item = fromlist(Item, inventory, items[i]);
				status_push("You pick up the %s.", item->name);
			}

			pickup_items(player, items);
			xfree(items);
			break;

		case 'w':
			item = choose_item_by_type(player->inventory,
			                           WEAPON,
			                           "Select a weapon to equip",
			                           true);
			if(item != NULL) {
				status_push("You wield the %s.", item->name);
				wield_item(player, item);
			}
			break;

		case 'x':
			item = player->weapon;
			player->weapon = player->offhand;
			player->offhand = item;

			if(player->weapon != NULL && player->offhand != NULL) {
				status_push("You hold a %s and a %s.",
				            player->weapon->name,
				            player->offhand->name);
			} else if(player->weapon != NULL) {
				status_push("You hold a %s.",
				            player->weapon->name);
			} else if(player->offhand != NULL) {
				status_push("You hold a %s in your off-hand.",
				            player->offhand->name);
			}

			break;

		case 'W':
			item = choose_item_by_type(player->inventory,
			                           ARMOUR,
			                           "Select some armour to wear",
			                           false);
			if(item != NULL) {
				status_push("You put on the %s.", item->name);
				wield_item(player, item);
			}
			break;

		/* Food and drink */
		case 'e':
			item = choose_item_by_type(player->inventory,
			                           FOOD,
			                           "Select some food to eat",
			                           false);
			if(item != NULL) {
				status_push("You eat the %s.", item->name);
				consume_item(player, item);
			}
			break;

		case 'q':
			item = choose_item_by_type(player->inventory,
			                           DRINK,
			                           "Select a drink",
			                           false);
			if(item != NULL) {
				status_push("You quaff the %s.", item->name);
				consume_item(player, item);
			}
			break;

		/* Misc */
		case '?':
			show_help();
			break;

		default:
			break;
		}
	}

	if (move && attackmove_relative(player, xdiff, ydiff)) {
		player->score += 5;
	}
}

/**
 * Say that the player is dead, and do stuff.
 * @param player Player that died.
 */
void player_death(Mob * player) {
	clear();

	/* Free ALL the things! */
	PlayerData * playerdata = (PlayerData *)player->data;
	Terrain * terrain = fromlist(Terrain, levels, gethead(&playerdata->terrain->levels));
	while(terrain != NULL) {
		Terrain * next = fromlist(Terrain, levels, terrain->levels.next);
		xfree(terrain);
		terrain = next;
	}
	xfree(playerdata);

	/* Free the stats */
	xfree(player->name);
	xfree(player->profession);
	xfree(player->race);

	mvaddprintf(9, 10, "Oh dear, you died. :(");
	mvaddprintf(10, 10, "Score: %i", player->score);
	mvaddprintf(20, 10, "Press any key to exit");

	quit = true;

	getch();
}
