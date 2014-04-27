#include <stdbool.h>
#include <curses.h>
#include <string.h>

#include "mob.h"
#include "level.h"
#include "utils.h"
#include "player.h"

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
	player->score = 0;
	player->darksight = false;

	/* Initialise the terrain knowledge to nothing */
	PlayerData * playerdata = xalloc(PlayerData);
	player->data = (void *)playerdata;
	playerdata->terrain = xalloc(Terrain);
	memset(playerdata->terrain->symbols, ' ', LEVELWIDTH * LEVELHEIGHT);

	Item * stone = xalloc(Item);
	stone->symbol = '*';
	stone->name = "A Stone";

	Item * coin = xalloc(Item);
	coin->symbol = '$';
	coin->name = "A Coin";

	Item * sword = xalloc(Item);
	sword->symbol = '/';
	sword->name = "A Sword";
	sword->type = WEAPON;
	sword->equipment = xalloc(Equipment);
	sword->equipment->item = sword;
	sword->equipment->attack = 10;

	Item * lantern = xalloc(Item);
	lantern->symbol = '^';
	lantern->name = "Lantern";
	lantern->type = WEAPON;
	lantern->luminous = true;
	lantern->equipment = xalloc(Equipment);
	lantern->equipment->item = lantern;
	lantern->equipment->attack = 1;

	player->inventory = insert(player->inventory, &stone->inventory);
	player->inventory = insert(player->inventory, &coin->inventory);
	player->inventory = insert(player->inventory, &sword->inventory);
	player->inventory = insert(player->inventory, &lantern->inventory);

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
	Equipment * equipment;
	Cell * current_cell = player->level->cells[player->xpos][player->ypos];

	display_level(player->level);

	bool move = false;
	int xdiff = 0;
	int ydiff = 0;
	int ch = getch();
	switch (ch) {
		/* Movement in a level */
	case 'k':
	case '8':
	case KEY_UP:
		move = true;
		xdiff =  0; ydiff = -1;
		break;
	case 'j':
	case '2':
	case KEY_DOWN:
		move = true;
		xdiff =  0; ydiff =  1;
		break;
	case 'h':
	case '4':
	case KEY_LEFT:
		move = true;
		xdiff = -1; ydiff =  0;
		break;

	case 'l':
	case '6':
	case KEY_RIGHT:
		move = true;
		xdiff =  1; ydiff =  0;
		break;

	case 'y':
	case '7':
		move = true;
		xdiff = -1; ydiff = -1;
		break;

	case 'u':
	case '9':
		move = true;
		xdiff =  1; ydiff = -1;
		break;

	case 'n':
	case '3':
		move = true;
		xdiff =  1; ydiff =  1;
		break;

	case 'b':
	case '1':
		move = true;
		xdiff = -1; ydiff =  1;
		break;

		/* Movement between levels */
	case '>':
		if (player->level->cells[player->xpos][player->ypos]->baseSymbol == '>'){
			move_mob_level(player, false);
		}
		break;

	case '<':
		if (player->level->cells[player->xpos][player->ypos]->baseSymbol == '<'){
			move_mob_level(player, true);
		}
		break;

		/* Inventory management */
	case 'i':
		display_inventory(player->inventory, "Inventory Contents:");
		break;

	case 'd':
		items = choose_items(player->inventory, "Select items to drop:");
		if(items != NULL) {
			for(unsigned int i = 0; items[i] != NULL; i++) {
				Item * item = fromlist(Item, inventory, items[i]);
				/* Unequip if necessary */
				if(player->weapon != NULL &&
				   item == player->weapon->item) {
					if(item->luminous) {
						player->luminosity --;
					}
					player->weapon = NULL;
				} else if(player->armour != NULL &&
				          item == player->armour->item) {
					if(item->luminous) {
						player->luminosity --;
					}
					player->armour = NULL;
				} else if(player->offhand != NULL &&
				          item == player->offhand->item) {
					if(item->luminous) {
						player->luminosity --;
					}
					player->offhand = NULL;
				}
			}
			/* Remove from the inventory */
			player->inventory = dropall(items);

			/* Add to the cell */
			current_cell->items = insertall(current_cell->items, items);
		}
		xfree(items);
		break;

	case ',':
		items = choose_items(current_cell->items, "Select items to pick up:");
		if(items != NULL) {
			/* Remove from the cell */
			current_cell->items = dropall(items);

			/* Add to the player */
			player->inventory = insertall(player->inventory, items);
		}
		xfree(items);
		break;

	case 'w':
		equipment = choose_equipment(player->inventory,
		                             WEAPON,
		                             "Select a weapon to equip");
		if(equipment != NULL) {
			if(player->weapon != NULL) {
				player->weapon->equipped = false;
				if(player->weapon->item->luminous) {
					player->luminosity --;
				}
			}

			player->weapon = equipment;
			equipment->equipped = true;

			if(equipment->item->luminous) {
				player->luminosity ++;
			}
		}
		break;

	case 'x':
		equipment = player->weapon;
		player->weapon = player->offhand;
		player->offhand = equipment;
		break;

	case 'W':
		equipment = choose_equipment(player->inventory,
		                             ARMOUR,
		                             "Select some armour to wear");
		if(equipment != NULL) {
			if(player->armour != NULL) {
				player->armour->equipped = false;
				if(player->armour->item->luminous) {
					player->luminosity --;
				}
			}

			player->armour = equipment;
			equipment->equipped = true;

			if(equipment->item->luminous) {
				player->luminosity ++;
			}
		}
		break;

		/* Misc */
	case '?':
		show_help();
		break;

	case 'q':
		quit = true;
		break;
	default:
		break;
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

	mvaddprintf(9, 10, "Oh dear, you died. :(");
	mvaddprintf(10, 10, "Score: %i", player->score);
	mvaddprintf(20, 10, "Press any key to exit");

	quit = true;

	getch();
}
