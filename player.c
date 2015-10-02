#include <stdbool.h>
#include <curses.h>
#include <string.h>
#include <time.h>

#include "autoplay.h"
#include "mob.h"
#include "level.h"
#include "utils.h"
#include "player.h"
#include "effect.h"
#include "status.h"
#include "list.h"

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

#ifndef AUTOPLAY
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
#endif // AUTOPLAY

/**
 * Apply race stats to the player.
 * @param player The player
 */
static void apply_race(Mob * player) {
	if(strcmp(player->race, "Human") == 0) {
		player->attack     = 3;
		player->defense    = 2;
		player->max_health = 100;
		player->con        = 3;
	} else if(strcmp(player->race, "Dutch") == 0) {
		player->attack     = 2;
		player->defense    = 3;
		player->max_health = 100;
		player->con        = 3;
	} else if(strcmp(player->race, "Elf") == 0) {
		player->attack     = 5;
		player->defense    = 1;
		player->max_health = 125;
		player->con        = 5;
	} else if(strcmp(player->race, "Dwarf") == 0) {
		player->attack     = 3;
		player->defense    = 5;
		player->max_health = 100;
		player->con        = 2;
	} else if(strcmp(player->race, "Halfling") == 0) {
		player->attack     = 2;
		player->defense    = 2;
		player->max_health = 150;
		player->con        = 2;
	} else if(strcmp(player->race, "Quarterling") == 0) {
		player->attack     = 1;
		player->defense    = 1;
		player->max_health = 175;
		player->con        = 1;
	}

	player->health = player->max_health;
}

/**
 * Apply profession stats to the player.
 * @param player The player
 */
static void apply_profession(Mob * player) {
	Item * weapon = NULL;
	Item * armour = NULL;

	if(strcmp(player->profession, "Miner") == 0) {
		weapon = clone_item(PICKAXE);
		armour = clone_item(M_HELMET);
	} else if(strcmp(player->profession, "Attorney") == 0) {
		weapon = clone_item(BOOK_TAX);
		armour = clone_item(LAW_SUIT);
	} else if(strcmp(player->profession, "Clog Maker") == 0) {
		weapon = clone_item(CLOG);
		weapon->count = 4;
		armour = clone_item(CLOG_APRON);
	} else if(strcmp(player->profession, "Huntsman") == 0) {
		weapon = clone_item(HUNT_KNIFE);
		armour = clone_item(CLOAK);
	} else if(strcmp(player->profession, "Chef") == 0) {
		weapon = clone_item(FRY_PAN);
		armour = clone_item(APRON);
	} else if(strcmp(player->profession, "Tourist") == 0) {
		weapon = clone_item(CAMERA);
		armour = clone_item(HAWAII);
	} else if(strcmp(player->profession, "Dog") == 0) {
		weapon = clone_item(BONE);
		armour = clone_item(FURSUIT);
	}

	player->inventory = insert(player->inventory, &weapon->inventory);
	player->inventory = insert(player->inventory, &armour->inventory);
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
	player->turn_action = &player_turn;
	player->death_action = &player_death;

	/* Initialise the terrain knowledge to nothing */
	PlayerData * playerdata = xalloc(PlayerData);
	player->data = (void *)playerdata;
	playerdata->terrain = xalloc(Terrain);
	memset(playerdata->terrain->symbols, ' ', LEVELWIDTH * LEVELHEIGHT);


	Item * lantern = clone_item(LANTERN);

	Item * potion = clone_item(C_POISON_POTION);

	player->inventory = insert(player->inventory, &lantern->inventory);
	player->inventory = insert(player->inventory, &potion->inventory);

	/* Pick the name, race, and profession */
	clear();
#ifdef AUTOPLAY
	randomise_player(player);
#else
	mvaddprintf(9, 10, "Do you want to randomly generate your player? ");
	while(true) {
		int choice = getch();
		if(choice == 'y' || choice == 'Y') {
			randomise_player(player);

			/* Yay, special case jokes */
			if(strcmp(player->race, "Human") == 0 &&
			   strcmp(player->profession, "Attorney") == 0) {
				xfree(player->name);
				player->name = strdup("Javert");
			}
			break;
		} else if(choice == 'n' || choice == 'N') {
			design_player(player);
			break;
		}
	}
#endif // AUTOPLAY

	apply_race(player);
	apply_profession(player);

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
 * Wait for a direction. Returns the direction and the character.
 * @param allow_nop Allow a null move.
 */
static Direction select_direction(Mob * player, bool allow_nop) {
#ifdef AUTOPLAY
        return autoplay_select_direction(player);
	(void) allow_nop;
#else
	int ch = getch();
	Direction out = {.dx = 0, .dy = 0, .ch = 0};

	switch(ch) {
		case 'k':
		case '8':
		case KEY_UP:
			out.dy = -1;
			break;

		case 'j':
		case '2':
		case KEY_DOWN:
			out.dy = 1;
			break;

		case 'h':
		case '4':
		case KEY_LEFT:
			out.dx = -1;
			break;

		case 'l':
		case '6':
		case KEY_RIGHT:
			out.dx = 1;
			break;

		case 'y':
		case '7':
			out.dx = -1;
			out.dy = -1;
			break;

		case 'u':
		case '9':
			out.dx = 1;
			out.dy = -1;
			break;

		case 'n':
		case '3':
			out.dx = 1;
			out.dy = 1;
			break;

		case 'b':
		case '1':
			out.dx = -1;
			out.dy = 1;
			break;

		case '5':
		case '.':
			if(!allow_nop) {
				out.ch = ch;
			}
			break;

		default:
			out.ch = ch;
			break;
	}

	return out;
#endif // AUTOPLAY
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
#ifdef AUTOPLAY
		// delay to show the current level to the player
		for(long x = 0; x < 100; x++)
		  display_level(player->level);
#endif // AUTOPLAY
		
		Direction dir = select_direction(player, true);

		/* Movement in a level */
		if(dir.ch == 0) {
			move = true;
			xdiff = dir.dx;
			ydiff = dir.dy;
			break;
		}

		switch (dir.ch) {
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
				status_push("You drop a %s.", item->name);
			}

			drop_items(player, items);
			xfree(items);
			break;

		case ',':
			items = choose_items(current_cell->items, "Select items to pick up:");

			for(unsigned int i = 0; items[i] != NULL; i++) {
				Item * item = fromlist(Item, inventory, items[i]);
				status_push("You pick up a %s.", item->name);
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
				status_push("You wield a %s.", item->name);
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
				status_push("You eat a %s.", item->name);
				consume_item(player, item);
			}
			break;

		case 'q':
			item = choose_item_by_type(player->inventory,
			                           DRINK,
			                           "Select a drink",
			                           false);
			if(item != NULL) {
				status_push("You quaff a %s.", item->name);
				consume_item(player, item);
			}
			break;

			/* Ranged combat */
		case 'f':
			if (player->weapon == NULL || player->weapon->ranged == false) {
				status_push("You do not have a ranged weapon equipped!");
				break;
			}
			dir = select_direction(player, false);
			int curx = player->xpos;
			int cury = player->ypos;

			if (dir.dx != 0 || dir.dy != 0) {
				curx += dir.dx;
				cury += dir.dy;
				while(player->level->cells[curx][cury]->solid == false) {
					if (player->level->cells[curx][cury]->occupant != NULL) {
						int tmpx, tmpy;
						attack_mob(player, player->level->cells[curx][cury]->occupant);
						tmpx = player->xpos, tmpy = player->ypos;
						player->xpos = curx, player->ypos = cury;
						drop_item(player, player->weapon);
						player->xpos = tmpx, player->ypos = tmpy;
						break;
					}
					curx += dir.dx;
					cury += dir.dy;
				}
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

	for(List * list = player->inventory; list != NULL; list = list->next) {
		Item * item = fromlist(Item, inventory, list);
		if (item->type == VALUABLE) {
			player->score += item->value;
		}
	}

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
