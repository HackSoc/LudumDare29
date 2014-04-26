#include <curses.h>
#include <stdlib.h>

#include "level.h"
#include "mob.h"
#include "utils.h"

extern bool quit;

/**
 * Initialises a level.
 * @param level Level to initialise.
 */
void build_level(Level * level) {
	const int NMINERS = 7;
	const int SPREAD = 100;
	const int ITERATIONS = 100;
	int i, m;
	int minersx[NMINERS], minersy[NMINERS];

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
	
	/* generate starting coordinates near the middle and place the stairs */
	level->startx = 39 + (rand() % 20);
	level->starty = 4  + (rand() % 10);
	level->cells[level->startx][level->starty]->baseSymbol = '<';
	level->cells[level->startx][level->starty]->solid = false;

	/* have the miners start where the player starts */
	for(m = 0; m < NMINERS; m++) {
		minersx[m] = level->startx;
		minersy[m] = level->starty;
	}
	/* spread the miners out a little */
	for(m = 0; m < NMINERS; m++) {
		int dx = (m % 2) ? -1 : 1;
		int totalspread = rand() % SPREAD;
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

			level->cells[minersx[m]][minersy[m]]->baseSymbol = '.';
			level->cells[minersx[m]][minersy[m]]->solid = false;
			level->cells[minersx[m]-dx][minersy[m]]->baseSymbol = '.';
			level->cells[minersx[m]-dx][minersy[m]]->solid = false;
			level->cells[minersx[m]][minersy[m]-dy]->baseSymbol = '.';
			level->cells[minersx[m]][minersy[m]-dy]->solid = false;
		}
	}
	for(i = 0; i < ITERATIONS; i++) {
		for(m = 0; m < NMINERS; m++) {
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

			level->cells[minersx[m]][minersy[m]]->baseSymbol = '.';
			level->cells[minersx[m]][minersy[m]]->solid = false;
			level->cells[minersx[m]-dx][minersy[m]]->baseSymbol = '.';
			level->cells[minersx[m]-dx][minersy[m]]->solid = false;
			level->cells[minersx[m]][minersy[m]-dy]->baseSymbol = '.';
			level->cells[minersx[m]][minersy[m]-dy]->solid = false;
		}
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
		//every turn effects on the mob go here
	}
}

/**
 * Render the level to the screen. The symbol for a level is picked
 * according to the following priorities: occupant > top item > base.
 * @param level Grid to display.
 */
void display_level(Level * level) {
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
}
