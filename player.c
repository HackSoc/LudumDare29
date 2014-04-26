#include <stdbool.h>
#include <curses.h>

#include "mob.h"
#include "level.h"

extern bool quit;

void player_action(Mob * player) {
	int ch;
	int x = player->xpos;
	int y = player->ypos;

	display_level(player->level);
	ch = getch();
	switch (ch) {
		case 'k':
		case '8':
		case KEY_UP:
			y--;
			break;
		case 'j':
		case '2':
		case KEY_DOWN:
			y++;
			break;
		case 'h':
		case '4':
		case KEY_LEFT:
			x--;
			break;
		case 'l':
		case '6':
		case KEY_RIGHT:
			x++;
			break;
		case 'q':
			quit = true;
		}

	move_mob(player, x, y);
}
