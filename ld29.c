#include <curses.h>
#include <stdbool.h>
#include <stdlib.h>

#include "level.h"
#include "mob.h"
#include "item.h"

int main() {
	/* Simple static level for rendering */
	Cell hwall = {.baseSymbol = '-',
				  .occupant   = NULL,
				  .items      = NULL};
	
	Cell vwall = {.baseSymbol = '|',
				  .occupant   = NULL,
				  .items      = NULL};
	
	Cell floor = {.baseSymbol = '.',
				  .occupant   = NULL,
				  .items      = NULL};
	
	Cell floor2 = {.baseSymbol = '.',
				   .occupant   = NULL,
				   .items      = NULL};
	
	Mob player = {.level = NULL,
				  .symbol = '@',
				  .next = NULL,
				  .prev = NULL,
				  .items = NULL};
	
	Level level = {.next = NULL,
				   .prev = NULL,
				   .mobs = &player};
	
	player.level = &level;
	
	for(unsigned int y = 0; y < LEVELHEIGHT; y++) {
		for(unsigned int x = 0; x < LEVELWIDTH; x++) {
			if(y == 0 || y == LEVELHEIGHT - 1) {
				level.cells[x][y] = &hwall;
			} else if (x == 0 || x == LEVELWIDTH - 1) {
				level.cells[x][y] = &vwall;
			} else {
				level.cells[x][y] = &floor;
			}
		}
	}
	
	level.cells[15][10] = &floor2;
	level.cells[15][10]->occupant = &player;

	/* Initialise curses */
	initscr();
	cbreak();
	noecho();
	nonl();
	intrflush(stdscr, FALSE);
	keypad(stdscr, TRUE);
	curs_set(0);
	
	/* Game loop */
	Level * current_level = &level;
	//while(true) {
	/* Update mobs */
	for(Mob * mob = current_level->mobs; mob != NULL; mob = mob->next) {
		//mob update
	}
	
	/* Render level */
	clear();
	for(unsigned int y = 0; y < LEVELHEIGHT; y ++) {
		for(unsigned int x = 0; x < LEVELWIDTH; x++) {
			if(current_level->cells[x][y]->occupant != NULL) {
				mvaddch(y, x, current_level->cells[x][y]->occupant->symbol);
			} else {
				mvaddch(y, x, current_level->cells[x][y]->baseSymbol);
			}
		}
	}
	
	/* Hang for output */
	getch();
	
	/* Deinitialise curses */
	curs_set(1);
	nl();
	echo();
	nocbreak();
	endwin();
}
