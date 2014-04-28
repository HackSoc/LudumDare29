#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include "enemy.h"
#include "mob.h"

/**
 * Move an enemy randomly, not including diagonals.
 * @param enemy Enemy to move
 */
void random_move(Mob * enemy) {
	if(rand() % 2) {
		move_mob_relative(enemy, (rand() % 2) ? 1 : -1, 0);
	} else {
		move_mob_relative(enemy, 0, (rand() % 2) ? 1 : -1);
	}
}

/**
 * Move an enemy randomly, including diagonals.
 * @param enemy Enemy to move
 */
void random_move_diagonals(Mob * enemy) {
	move_mob_relative(enemy, (rand() % 3) - 1, (rand() % 3) - 1);
}

/**
 * Move towards a position. If the move fails, another axis will be
 * tried.
 * @param enemy Enemy to move
 * @param x Target X
 * @param y Target Y
 * @param diagonal Can move diagonally
 */
void move_towards(Mob * enemy,
                  unsigned int x, unsigned int y,
                  bool diagonal) {
	int xdiff = enemy->xpos - x;
	int ydiff = enemy->ypos - y;

	int dx = 0;
	int dy = 0;

	if(diagonal) {
		/* Move diagonally if we can, and are far enough away */
		if(xdiff > 5) {
			dx = -1;
		} else if(xdiff < -5) {
			dx = 1;
		}

		if(ydiff > 5) {
			dy = -1;
		} else if(ydiff < -5) {
			dy = 1;
		}
	}

	/* Move along the axis furthest away from the target */
	if(abs(xdiff) > abs(ydiff)) {
		bool res = move_mob_relative(enemy, (xdiff < 0) ? 1 : -1, dy);

		/* If there was something solid in the way, try moving y */
		if(!res) {
			move_mob_relative(enemy, dx, (ydiff < 0) ? 1 : -1);
		}
	} else {
		bool res = move_mob_relative(enemy, dx, (ydiff < 0) ? 1 : -1);

		if(!res) {
			move_mob_relative(enemy, (xdiff < 0) ? 1 : -1, dy);
		}
	}
}

/**
 * A very simple enemy: move towards the player, and damage them if adjacent.
 * @param enemy Entity to move.
 */
void simple_enemy_turn(Mob * enemy) {
	Mob * player = enemy->level->player;

	/* If we can't see the player, move randomly */
	if(!can_see_other(enemy, player)) {
		random_move(enemy);
		return;
	}

	int xdiff = enemy->xpos - player->xpos;
	int ydiff = enemy->ypos - player->ypos;

	if((abs(xdiff) == 1 && ydiff == 0)
	   || (xdiff == 0 && abs(ydiff) == 1)) {
		/* If (orthogonally) adjacent to the player, damage them */
		attack_mob(enemy, player);
	} else {
		/* Move along the axis furthest away from the player */
		move_towards(enemy, player->xpos, player->ypos, false);
	}
}

/**
 * A more sophisticated enemy: hunters share their data field, and use
 * it to instantly communicate the player position.  The behaviour is
 * as follows: wander randomly (including diagonals) if the player
 * hasn't been seen, and all home in on the player as soon as one sees
 * them.
 * @param enemy Entity to move
 */
void hunter_turn(Mob * enemy) {
	assert(enemy->data != NULL);

	Mob * player = enemy->level->player;
	Target * data = (Target *) enemy->data;

	int xdiff = enemy->xpos - player->xpos;
	int ydiff = enemy->ypos - player->ypos;
   
	/* If (diagonally) adjacent to the player, damage them */
	if((abs(xdiff) == 1 && ydiff == 0)
	   || (xdiff == 0 && abs(ydiff) == 1)
	   || (abs(xdiff) == 1 && abs(ydiff) == 1)) {
		attack_mob(enemy, player);
		return;
	}

	/* Player found, update the shared state */
	if(can_see_other(enemy, player)) {
		data->x = player->xpos;
		data->y = player->ypos;
		data->chase = true;
	} else if(data->chase &&
	          can_see_point(enemy->level,
	                        enemy->xpos, enemy->ypos,
	                        data->x, data->y)) {
		/* If we can see the target, but not the player, return to wandering */
		data->chase = false;
	}

	/* If the chase is on, pursue */
	if (data->chase) {
		move_towards(enemy,
		             data->x, data->y,
		             true);
	} else {
		/* No information: move randomly */
		random_move_diagonals(enemy);
	}
}

/**
 * Free a hunter's state. As hunters share the state, there is a count
 * of how many hunters are still surviving.
 * @param enemy The enemy to kill/free
 */
void hunter_death(Mob * enemy) {
	Target * data = (Target *) enemy->data;

	if(data->refcount == 1) {
		xfree(data);
	} else {
		data->refcount --;
	}

	drop_corpse(enemy);
}
