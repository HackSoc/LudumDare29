#include <stdlib.h>

#include "mob.h"
#include "level.h"

/**
 * A very simple enemy: move towards the player, and damage them if
 * adjacent
 */
void simple_enemy_turn(Mob * enemy) {
	Mob * player = enemy->level->player;
	
	int xdiff = enemy->xpos - player->xpos;
	int ydiff = enemy->ypos - player->ypos;

	/* If adjacent to the player, damage them */
	if((abs(xdiff) == 1 && ydiff == 0) ||
	   (xdiff == 0 && abs(ydiff) == 1)) {
		damage_mob(player, 2);
		return;
	}

	/* Move along the axis furthest away from the player */
	if(abs(xdiff) > abs(ydiff)) {
		bool res = move_mob_relative(enemy, (xdiff < 0) ? 1 : -1, 0);

		/* If there was something solid in the way, try moving y */
		if(!res) {
			move_mob_relative(enemy, 0, (ydiff < 0) ? 1 : -1);
		}
	} else {
		bool res = move_mob_relative(enemy, 0, (ydiff < 0) ? 1 : -1);

		if(!res) {
			move_mob_relative(enemy, (xdiff < 0) ? 1 : -1, 0);
		}
	}
}
