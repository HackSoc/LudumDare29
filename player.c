#include <stdbool.h>
#include <curses.h>

#include "mob.h"
#include "level.h"

extern bool quit;

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
		case 'q':
			quit = true;
	}
}

void player_death(Mob * player) {
	// Just quit
	quit = true;
}
