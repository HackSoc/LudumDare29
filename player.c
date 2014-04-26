#include <stdbool.h>
#include <curses.h>

#include "mob.h"
#include "level.h"

extern bool quit;

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
