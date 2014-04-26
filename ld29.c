#include <curses.h>
#include <stdbool.h>
#include <stdlib.h>

#include "curses.h"
#include "level.h"
#include "mob.h"
#include "item.h"

bool quit = false;

void player_action(Mob *);

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
				  .items = NULL,
				  .action = &player_action,
				  .name = "Elrond",
				  .race = "Half-Elf",
				  .profession = "Orc Slayer",
				  .health = 100,
				  .max_health = 100};
	
	Level level = {.next = NULL,
				   .prev = NULL,
				   .mobs = &player};
	
	player.level = &level;
	
	for(unsigned int y = 0; y < LEVELHEIGHT; y++) {
		for(unsigned int x = 0; x < LEVELWIDTH; x++) {
			level.cells[x][y] = calloc(1, sizeof(Cell));
			if(y == 0 || y == LEVELHEIGHT - 1) {
				level.cells[x][y]->baseSymbol = '-';
			} else if (x == 0 || x == LEVELWIDTH - 1) {
				level.cells[x][y]->baseSymbol = '|';
			} else {
				level.cells[x][y]->baseSymbol = '.';
			}
		}
	}
	
	level.cells[15][10]->occupant = &player;
	player.xpos = 15;
	player.ypos = 10;

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
	while(!quit) {
	/* Update mobs */
	run_turn(current_level);
	
	/* Render level */
	clear();
	display_level(current_level);
	
	/* Display player stats */
	mvaddprintf(21, 5, "%s, the %s %s", player.name, player.race, player.profession);
	mvaddprintf(22, 5, "HP: %d/%d", player.health, player.max_health);
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
