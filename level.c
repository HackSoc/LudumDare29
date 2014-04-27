#include <curses.h>
#include <stdlib.h>

#include "level.h"
#include "mob.h"
#include "utils.h"
#include "effect.h"
#include "list.h"
#include "player.h"

extern bool quit;

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

#define DEF_MOB(sym, n, col, hlth) {.symbol = (sym), .colour = (col), .name = (n), \
			.is_bold = false, .hostile = true, .health = (hlth), .max_health = (hlth), \
			.level = NULL, .moblist = {.next = NULL, .prev=NULL}, .turn_action = NULL, \
		    .xpos = 0, .ypos = 0, .score = 0, .darksight = true, .luminosity = 0}

static const struct Mob default_mobs[] = {
	DEF_MOB('H', "Hedgehog", COLOR_YELLOW, 5),
	DEF_MOB('S', "Squirrel", COLOR_YELLOW, 10)
};

#undef DEF_MOB

/**
 * Add a mob to a level.
 * @param level The level to add to.
 */
static void add_mob(Level * level) {
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

	Mob * new = xalloc(Mob);
	*new = default_mobs[rand() % lengthof(default_mobs)];
	new->level = level;
	new->turn_action = &simple_enemy_turn;
	new->xpos = x;
	new->ypos = y;
	level->mobs = insert(level->mobs, &new->moblist);
	level->cells[x][y]->occupant = new;
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
				}
			}
		}
	}

	/* generate starting coordinates near the middle */
	level->startx = 39 + (rand() % 20);
	level->starty = 4  + (rand() % 10);

	/* assign depth number. if this isn't the first level,
	   this will be corrected pretty darn swiftly. */
	level->depth = 0;

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
	for (int i = 0; i < 5; i++) {
		add_mob(level);
	}
}

/**
 * Run afflicated routines on a mob.
 * @param mob Afflicted mob.
 */
void do_affliction(Mob * mob) {
	if(!is_afflicted(mob)) return;

	if(mob->effect_duration == 0) {
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
	mvaddprintf(22, 5, "HP: %d/%d", player->health, player->max_health);

	/* Display what level we are on */
	mvaddprintf(21, 67, "Depth: %d", level->depth);
}
