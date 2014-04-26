#include <curses.h>
#include <stdlib.h>

#include "level.h"
#include "mob.h"
#include "utils.h"
#include "effect.h"

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
 * @param to_place The cell to place.
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

	/* Mine out passageways */
	Cell floor = {
		.baseSymbol = '.',
		.solid = false,
		.occupant = NULL,
		.items = NULL};

	mine_level(level,
			   NMINERS, SPREAD, ITERATIONS,
			   level->startx, level->starty,
			   &floor, true);

	/* Mine out lakes */
	Cell poison_lake = {
		.baseSymbol = '~',
		.solid = false,
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
}

/**
 * Run afflicated routines on a mob.
 * @param mob Afflicted mob.
 */
void do_affliction(Mob * mob) {
	if(!is_afflicted(mob)) return;

	mob->effect_action(mob);

	if(mob->effect_duration == 1) {
		mob->effect_action = NULL;
	} else if(mob->effect_duration > 1) {
		mob->effect_duration --;
	}
}

/**
 * A "turn" consists of all of the mobs acting once, possibly followed
 * by some constant effect on the mob. As the player is a turn, this
 * is (indirectly) where blocking for input happens.
 * @param level The level grid to run the turn on.
 */
void run_turn(Level * level) {
	for(Mob * mob = level->mobs; mob != NULL && !quit; mob = mob->next) {
		if(mob->turn_action != NULL) {
			mob->turn_action(mob);
		}
		do_affliction(mob);
	}
	/* Player is special, and goes last */
	level->player->turn_action(level->player);
	do_affliction(level->player);
}

/**
 * Render the level to the screen. The symbol for a level is picked
 * according to the following priorities: occupant > top item > base.
 * @param level Grid to display.
 */
void display_level(Level * level) {
	Mob * player = level->player;
	for(unsigned int x = 0; x < LEVELWIDTH; x++) {
		for(unsigned int y = 0; y < LEVELHEIGHT; y++) {
			if(level->cells[x][y]->occupant != NULL) {
				mvaddch(y, x, level->cells[x][y]->occupant->symbol);
			} else if(level->cells[x][y]->items != NULL) {
				mvaddch(y, x, level->cells[x][y]->items->symbol);
			} else {
				mvaddch(y, x, level->cells[x][y]->baseSymbol);
			}
		}
	}
	
	/* Display player stats */
	mvaddprintf(21, 5, "%s, the %s %s", player->name, player->race, player->profession);
	mvaddprintf(22, 5, "HP: %d/%d", player->health, player->max_health);
}
