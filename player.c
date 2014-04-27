#include <stdbool.h>
#include <curses.h>

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

	player->race = strdup(list_choice(9, 10,
									  "What is your race?",
									  "Don't be silly, choose a proper race.",
									  races));

	player->profession = strdup(list_choice(9, 10,
											"And finally, what is your profession?",
											"That's not a real job!",
											professions));
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
	
	player->items = xalloc(Item);
	player->items->symbol = '*';
	player->items->name = "A Stone";

	player->items->next = xalloc(Item);
	player->items->next->prev = player->items;
	player->items->next->symbol = '$';
	player->items->next->name = "A Coin";

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
 * @param damage Amount of damage to inflict on any mobs, if present.
 * @return If the player moved (no damage done).
 */
bool attackmove(Mob * player, unsigned int x, unsigned int y,
				unsigned int damage) {
	Mob * mob = player->level->cells[x][y]->occupant;
	bool unoccupied = move_mob(player, x, y);

	if(!unoccupied && mob != NULL && mob->hostile) {
		damage_mob(mob, damage);
	}

	return unoccupied;
}

/**
 * Like attackmove, but relative position.
 * @param player Entity representing the player.
 * @param xdiff Difference in the x-axis to move.
 * @param ydiff Difference in the y-axis to move.
 * @param damage Damage to do to any mobs, if present.
 * @return If the player moved (no damage done).
 */
bool attackmove_relative(Mob * player, int xdiff, int ydiff,
						 unsigned int damage) {
	unsigned int x = player->xpos + xdiff;
	unsigned int y = player->ypos + ydiff;

	return attackmove(player, x, y, damage);
}

/**
 * Wait for user input, and then act accordingly.
 * @param player Player entity.
 */
void player_turn(Mob * player) {
	int ch;

	display_level(player->level);
	ch = getch();
	switch (ch) {
	case 'k':
	case '8':
	case KEY_UP:
			attackmove_relative(player, 0, -1, 5);
			break;
	case 'j':
	case '2':
	case KEY_DOWN:
			attackmove_relative(player, 0, 1, 5);
			break;
	case 'h':
	case '4':
	case KEY_LEFT:
		attackmove_relative(player, -1, 0, 5);
		break;
	case 'l':
	case '6':
	case KEY_RIGHT:
			attackmove_relative(player, 1, 0, 5);
			break;
	case 'y':
	case '7':
		attackmove_relative(player, -1, -1, 5);
		break;
	case 'u':
	case '9':
		attackmove_relative(player, 1, -1, 5);
		break;
	case 'n':
	case '3':
		attackmove_relative(player, 1, 1, 5);
		break;
	case 'b':
	case '1':
		attackmove_relative(player, -1, 1, 5);
		break;
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
	case 'i':
		display_items(player->items, false, "Inventory Contents:");
		break;
	case 'q':
		quit = true;
	}
}

/**
 * Say that the player is dead, and do stuff.
 * @param player Player that died.
 * @todo Implement properly.
 */
void player_death(Mob * player) {
	// Just quit
	quit = true;
}
