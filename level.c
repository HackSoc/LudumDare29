#include <curses.h>
#include <stdlib.h>

#include "level.h"
#include "mob.h"
#include "utils.h"
#include "effect.h"
#include "list.h"
#include "player.h"
#include "status.h"
#include "enemy.h"

extern bool quit;
extern const struct Mob default_enemies[];

/**
 * (Shallow) Clone a cell and place it in the given position.
 * Mobs and items in the cell are NOT cloned, those pointers get shared.
 * @param level The level to place the cell in.
 * @param x The X coordinate.
 * @param y The Y coordinate.
 * @param to_place The cell to clone.
 * @param careful Only place if the space is occuped by a wall or floor.
 */
static void place_cell(Level * level,
                       unsigned int x,
                       unsigned int y,
                       Cell * to_place,
                       bool careful) {

	if(careful) {
		switch(level->cells[x][y]->baseSymbol) {
		case '#':
		case '.':
			break;
		default:
			return;
		}
	}

	level->cells[x][y]->baseSymbol = to_place->baseSymbol;
	level->cells[x][y]->colour     = to_place->colour;
	level->cells[x][y]->solid      = to_place->solid;
	level->cells[x][y]->occupant   = to_place->occupant;
	level->cells[x][y]->items      = to_place->items;
	level->cells[x][y]->luminosity = to_place->luminosity;
}

/**
 * Mine out the level. All miners get placed in the same starting
 * coordinates, and then are spread out, before being free to wander
 * around as they see fit.
 * @param level The level to mine
 * @param num_miners The number of miners to place.
 * @param spread The maximum distance to spread out the miners.
 * @param iterations The number of steps to mine.
 * @param startx The X coordinate to position the miners (before spreading).
 * @param starty The Y coordinate to position the miners (before spreading).
 * @param to_place The cell to place.
 * @param make_stairs If true, place the stairs at one of the miners randomly.
 */
static void mine_level(Level * level,
                       unsigned int num_miners,
                       unsigned int spread,
                       unsigned int iterations,
                       unsigned int startx, unsigned int starty,
                       Cell * to_place,
                       bool make_stairs) {
	unsigned int minersx[num_miners], minersy[num_miners];
	unsigned int i, m;

	for(m = 0; m < num_miners; m++) {
		minersx[m] = startx;
		minersy[m] = starty;
	}

	/* spread the miners out a little */
	for(m = 0; m < num_miners; m++) {
		int dx = (m % 2) ? -1 : 1;
		unsigned int totalspread = rand() % spread;
		for(i = 0; i < totalspread; i++) {
			int dy = (rand() % 2) ? -1 : 1;
			minersx[m] += dx;
			minersy[m] += dy;

			/* make sure we don't destroy the border or segfault */
			if(minersx[m] <= 0){
				minersx[m] = 1;
			}
			if(minersx[m] >= LEVELWIDTH - 1){
				minersx[m] = LEVELWIDTH - 2;
			}
			if(minersy[m] <= 0){
				minersy[m] = 1;
			}
			if(minersy[m] >= LEVELHEIGHT - 1){
				minersy[m] = LEVELHEIGHT - 2;
			}

			/* Place a cell, making sure we don't overwrite any features */
			place_cell(level, minersx[m], minersy[m], to_place, true);
			place_cell(level, minersx[m]-dx, minersy[m], to_place, true);
			place_cell(level, minersx[m], minersy[m]-dy, to_place, true);
		}
	}
	for(i = 0; i < iterations; i++) {
		for(m = 0; m < num_miners; m++) {
			/* pick a random direction */
			int dx = (rand() % 2) ? -1 : 1;
			int dy = (rand() % 2) ? -1 : 1;
			minersx[m] += dx;
			minersy[m] += dy;

			/* make sure we don't destroy the border or segfault */
			if(minersx[m] <= 0){
				minersx[m] = 1;
			}
			if(minersx[m] >= LEVELWIDTH - 1){
				minersx[m] = LEVELWIDTH - 2;
			}
			if(minersy[m] <= 0){
				minersy[m] = 1;
			}
			if(minersy[m] >= LEVELHEIGHT - 1){
				minersy[m] = LEVELHEIGHT - 2;
			}

			place_cell(level, minersx[m], minersy[m], to_place, true);
			place_cell(level, minersx[m]-dx, minersy[m], to_place, true);
			place_cell(level, minersx[m], minersy[m]-dy, to_place, true);
		}
	}

	/* drop the downstair at the position of a random miner */
	if (make_stairs) {
		int m = rand() % num_miners;
		level->cells[minersx[m]][minersy[m]]->baseSymbol = '>';
		level->cells[minersx[m]][minersy[m]]->solid = false;
		level->endx = minersx[m];
		level->endy = minersy[m];
	}
}

/**
 * Add a mob to a level at the specified location.
 * @param level The level to add to.
 * @param mob The mob to add.
 * @param x The x coordinate to add the mob at.
 * @param y The y coordinate to add the mob at.
 */
static void add_mob(Level * level, Mob * mob, int x, int y) {
	mob->level = level;
	mob->xpos = x;
	mob->ypos = y;
	level->mobs = insert(level->mobs, &mob->moblist);
	level->cells[x][y]->occupant = mob;
}

/**
 * Add a mob to a level at a random location.
 * @param level The level to add to.
 * @param mob The mob to add.
 */
static void add_mob_random(Level * level, Mob * mob) {
	/* Try 20 times to get a random clear tile. */
	int x, y;
	bool found = false;
	for (int i = 0; i < 20; i++) {
		x = rand() % (LEVELWIDTH - 1);
		y = rand() % (LEVELHEIGHT - 1);

		if (!level->cells[x][y]->solid && level->cells[x][y]->occupant == NULL) {
			found = true;
			break;
		}
	}
	if (!found) return;

	add_mob(level, mob, x, y);
}

/**
 * Randomly add a number of items to the inventory. Items are NOT placed on stairs.
 * @param level The level
 * @param item Item type to place
 * @param count Number of copies to place
 */
static void place_randomly(Level * level, enum DefaultItem item, unsigned int count) {
	if (item == NO_SUCH_ITEM) return;

	for (unsigned int i = 0; i < count; i++) {
		Item * to_place = clone_item(item);
		int x, y;

		do {
			x = 1 + (rand() % (LEVELWIDTH-2));
			y = 1 + (rand() % (LEVELHEIGHT-2));
		} while (level->cells[x][y]->baseSymbol == '<' || level->cells[x][y]->baseSymbol == '>');

		level->cells[x][y]->items = insert(level->cells[x][y]->items, &to_place->inventory);
	}
}

/**
 * Initialises a level.
 * @param level Level to initialise.
 */
void build_level(Level * level) {
	/* For mining passageways */
	const int NMINERS = 7;
	const int SPREAD = 100;
	const int ITERATIONS = 100;

	/* For "mining" lakes of poison water */
	const int NUMLAKES = 3;
	const int NLAKEMINERS = 1;
	const int LAKESPREAD = 100;
	const int LAKEITERATIONS = 5;

	for(unsigned int y = 0; y < LEVELHEIGHT; y++) {
		for(unsigned int x = 0; x < LEVELWIDTH; x++) {
			level->cells[x][y] = xalloc(Cell);
			if(y == 0 || y == LEVELHEIGHT - 1) {
				level->cells[x][y]->baseSymbol = '-';
				level->cells[x][y]->solid = true;
			} else if (x == 0 || x == LEVELWIDTH - 1) {
				level->cells[x][y]->baseSymbol = '|';
				level->cells[x][y]->solid = true;
			} else {
				/*fill 99% of the level with rocks*/
				if (rand() % 100 == 0) {
					level->cells[x][y]->baseSymbol = '.';
					level->cells[x][y]->solid = false;
				} else {
					level->cells[x][y]->baseSymbol = '#';
					level->cells[x][y]->solid = true;

					if(rand() % 200 == 0) {
						level->cells[x][y]->colour = COLOR_YELLOW;
						level->cells[x][y]->luminosity = 1;
					}
				}
			}
		}
	}

	/* generate starting coordinates near the middle */
	level->startx = 39 + (rand() % 20);
	level->starty = 4  + (rand() % 10);

	/* Mine out passageways */
	Cell floor = {
		.baseSymbol = '.',
		.colour = COLOR_WHITE,
		.solid = false,
		.illuminated = false,
		.luminosity = 0,
		.occupant = NULL,
		.items = NULL};

	mine_level(level,
	           NMINERS, SPREAD, ITERATIONS,
	           level->startx, level->starty,
	           &floor, true);

	/* Mine out lakes */
	Cell poison_lake = {
		.baseSymbol = '~',
		.colour = COLOR_GREEN,
		.solid = false,
		.illuminated = false,
		.luminosity = 0,
		.occupant = NULL,
		.items = NULL};

	int lakes = rand() % NUMLAKES;
	for(int lake = 0; lake < lakes; lake++) {
		int lx = rand() % LEVELWIDTH;
		int ly = rand() % LEVELHEIGHT;

		mine_level(level,
		           NLAKEMINERS, LAKESPREAD, LAKEITERATIONS,
		           lx, ly,
		           &poison_lake, false);
	}

	/* Place the stairs */
	level->cells[level->startx][level->starty]->baseSymbol = '<';
	level->cells[level->startx][level->starty]->solid = false;
	level->cells[level->startx][level->starty]->colour = COLOR_WHITE;

	/* Arbitrary number of mobs */
	unsigned int available_mobs;
	for(available_mobs = 0;
	    default_enemies[available_mobs].min_depth <= level->depth &&
		    available_mobs < NUM_ENEMY_TYPES;
	    available_mobs ++);

	for (int i = 0; i < 5; i++) {
		enum EnemyType mobtype = (enum EnemyType) biased_rand(available_mobs);
		Mob * mob = create_enemy(mobtype);

		add_mob_random(level, mob);

		/* 1. share hunter state
		   2. hunters always appear in 2s, (giving up to 10 enemies!) */
		if(mobtype == WOLFMAN  || mobtype == CAVE_PIRATE/* || mobtype == ... */) {
			Mob * mob2 = create_enemy(mobtype);
			add_mob_random(level, mob2);

			HunterState * state = xalloc(HunterState);
			state->refcount += 2;
			mob->data = state;
			mob2->data = state;
		}

		/* chasers are like hunters, but they don't share the state - so
		 * it's just a memory of the player. */
		if(mobtype == FALLEN_ANGEL /* || mobtype == ... */) {
			HunterState * state = xalloc(HunterState);
			state->refcount = 1;
			mob->data = state;
		}
	}

	/* add 5 gold for the player to find */
	place_randomly(level, GOLD, 5);

	/* add a regular item for the player to find */
	{
		enum DefaultItem item = NO_SUCH_ITEM;
		switch (rand() % 10) {
			case 0: case 1: case 2: case 3: case 4:
				item = FOOD_RATION;
				break;
			case 5: case 6: case 7: case 8:
				item = C_POISON_POTION;
				break;
			case 9:
				item = STONE;
				break;
		}
		place_randomly(level, item, 1);
	}

	/* possibly add a special item, level dependent */
	{
		enum DefaultItem item = NO_SUCH_ITEM;

		switch (rand() % 10) {
		case 0: case 1: case 2:
			item = PICKAXE;
			break;
		case 3: case 4: case 5:
			item = LANTERN;
			break;
		case 6:
			item = O_SWORD;
			break;
		case 7: case 8:
			item = HELMET;
			break;
		case 9:
			switch (rand() % 13) {
			case 0: case 1: case 2: case 3: case 4:
				if (level->depth > 5) {
					item = SWORD;
				}
				break;
			case 5: case 6: case 7:
				if (level->depth > 5) {
					item = C_MAIL;
				}
				break;
			case 8:
				if (level->depth > 10) {
					item = D_MAIL;
				}
				break;
			case 10:
				if (level->depth > 20) {
					item = A_PICKAXE;
				}
				break;
			case 11:
				if (level->depth > 20) {
					item = FLESHBOOK;
				}
				break;
			case 12:
				if (level->depth > 20) {
					item = W_BOOT;
				}
				break;
			}
		}
		place_randomly(level, item, 1);
	}
}

/**
 * Run afflicated routines on a mob.
 * @param mob Afflicted mob.
 */
void do_affliction(Mob * mob) {
	if(!is_afflicted(mob)) return;

	if(mob->effect_duration == 0) {
		if(mob == mob->level->player) {
			status_push("The effect wears off.");
		}

		mob->effect_action = NULL;
		return;
	} else if(mob->effect_duration > 0) {
		mob->effect_duration --;
	}

	mob->effect_action(mob);
}

/**
 * A "turn" consists of all of the mobs acting once, possibly followed
 * by some constant effect on the mob. As the player is a turn, this
 * is (indirectly) where blocking for input happens.
 * @param level The level grid to run the turn on.
 */
void run_turn(Level * level) {
	/* Process each mob's turn */
	for(List * moblist = level->mobs; moblist != NULL && !quit; moblist = moblist->next) {
		Mob * mob = fromlist(Mob, moblist, moblist);
		if(mob->health <= 0) {
			continue;
		}

		if(mob->turn_action != NULL) {
			mob->turn_action(mob);
		}
		do_affliction(mob);
	}

	/* Free dead mobs */
	List * moblist = level->mobs;
	while(moblist != NULL) {
		Mob * mob = fromlist(Mob, moblist, moblist);
		if(mob->health <= 0) {
			Mob * nextmob = kill_mob(mob);
			moblist = (nextmob == NULL) ? NULL : &nextmob->moblist;
		} else {
			moblist = moblist->next;
		}
	}
}

/**
 * Calculate which cells are illuminated or not
 * @param level The level to check
 */
static void calculate_illumination(Level * level) {
	/* Unset all illumination */
	for(unsigned int x = 0; x < LEVELWIDTH; x++) {
		for(unsigned int y = 0; y < LEVELHEIGHT; y++) {
			level->cells[x][y]->illuminated = false;
		}
	}

	/* Calculate illumination */
	for(unsigned int x = 0; x < LEVELWIDTH; x++) {
		for(unsigned int y = 0; y < LEVELHEIGHT; y++) {
			Cell * cell = level->cells[x][y];
			if(cell->luminosity > 0 ||
			   (cell->occupant != NULL && cell->occupant->luminosity > 0)) {
				/* Luminous cells are illuminated */
				level->cells[x][y]->illuminated = true;

				/* As is every cell they can see */
				for(unsigned int x2 = 0; x2 < LEVELWIDTH; x2++) {
					for(unsigned int y2 = 0; y2 < LEVELHEIGHT; y2++) {
						if(level->cells[x2][y2]->illuminated) {
							continue;
						}
						if(can_see_point(level, x, y, x2, y2)) {
							level->cells[x2][y2]->illuminated = true;
						}
					}
				}
			}
		}
	}
}

/**
 * Render the level to the screen. The symbol for a level is picked
 * according to the following priorities: occupant > top item > base.
 * @param level Grid to display.
 */
void display_level(Level * level) {
	Mob * player = level->player;
	PlayerData * playerdata = (PlayerData *)player->data;

	calculate_illumination(level);

	for(unsigned int x = 0; x < LEVELWIDTH; x++) {
		for(unsigned int y = 0; y < LEVELHEIGHT; y++) {
			if(!can_see(player, x, y)) {
				mvaddchcol(y, x,
				           playerdata->terrain->symbols[x][y],
				           COLOR_BLUE,
				           COLOR_BLACK,
				           false);
				continue;
			}

			playerdata->terrain->symbols[x][y] = level->cells[x][y]->baseSymbol;

			if(level->cells[x][y]->occupant != NULL &&
			   level->cells[x][y]->occupant->health > 0) {
				mvaddchcol(y, x,
				           level->cells[x][y]->occupant->symbol,
				           level->cells[x][y]->occupant->colour, COLOR_BLACK,
				           level->cells[x][y]->occupant->is_bold);
			} else if(level->cells[x][y]->items != NULL) {
				Item * item = fromlist(Item, inventory, level->cells[x][y]->items);
				mvaddch(y, x, item->symbol);
			} else {
				mvaddchcol(y, x,
				           level->cells[x][y]->baseSymbol,
				           level->cells[x][y]->colour, COLOR_BLACK,
				           false);
			}
		}
	}

	/* Display player stats */
	mvaddprintf(21, 5, "%s, the %s %s", player->name, player->race, player->profession);
	mvaddprintf(22, 5, "HP: %d/%d, Atk: %d (+%d), Def: %d (+%d), Con: %d",
	            player->health, player->max_health,
	            player->attack, (player->weapon == NULL) ? 0 : player->weapon->value,
	            player->defense, (player->armour == NULL) ? 0 : player->armour->value,
	            player->con);

	/* Display what level we are on */
	mvaddprintf(23, 5, "Depth: %d", level->depth);

	/* Display the status */
	display_status();
	refresh();
}
