#include <assert.h>
#include <stdlib.h>
#include <stddef.h>
#include <curses.h>
#include "enemy.h"
#include "mob.h"
#include "effect.h"

extern Item special_items[];

/**
 * Definitions of enemies
 */
#define ENEMY(sym, n, col, hlth, atk, def, cn, dep) {	  \
		.symbol = (sym), .colour = (col), .name = (n), .is_bold = false,\
		.hostile = true,\
		.health = (hlth), .max_health = (hlth),\
		.attack = (atk), .defense = (def), .con = (cn),\
		.level = NULL, .xpos = 0, .ypos = 0,\
        .moblist = {.next = NULL, .prev=NULL},\
        .turn_action = NULL,\
		.score = 0,\
	    .darksight = true, .luminosity = 0,\
	    .min_depth = (dep)}

/* should keep the same structure as EnemyType in enemy.h.
 * should also be ordered by dep. */
const struct Mob default_enemies[] = {
	ENEMY('H', "Hedgehog",     COLOR_YELLOW, 5,  1,  0,   0,  0),
	ENEMY('S', "Squirrel",     COLOR_YELLOW, 10, 2,  0,   0,  0),
	ENEMY('D', "Duck",         COLOR_GREEN,  10, 1,  1,   1,  1),
	ENEMY('o', "Orc",          COLOR_YELLOW, 15, 3,  2,   7,  2),
	ENEMY('W', "Wolfman",      COLOR_YELLOW, 25, 10, 3,   10, 10),
	ENEMY('A', "Fallen Angel", COLOR_YELLOW, 50, 12, 10,  100, 25)
};

#undef ENEMY

/**
 * Create and return an enemy of the specified type.
 * @param mobtype The type of mob to make.
 * @return The mob created.
 */
Mob * create_enemy(enum EnemyType mobtype){
	Mob * new = xalloc(Mob);
	*new = default_enemies[mobtype];
	new->turn_action = &simple_enemy_turn;
	new->death_action = &drop_corpse;

	if (mobtype == ORC){
		Item * sword = xalloc(Item);
		*sword = special_items[O_SWORD];
		new->inventory = insert(new->inventory, &sword->inventory);
		wield_item(new, sword);

		if (rand() % 2) {
			Item * food = xalloc(Item);

			food->count =1;
			food->symbol = '%';
			food->name = "Food Ration";
			food->type = FOOD;
			food->value = 5;
			new->inventory = insert(new->inventory, &food->inventory);
		}
	} else if(mobtype == WOLFMAN) {
		new->turn_action = &hunter_turn;
		new->death_action = &hunter_death;

		Item * food = xalloc(Item);

		food->count = 1;
		food->symbol = '%';
		food->name = "Nourishing Food Ration";
		food->type = FOOD;
		food->value = 7;
		new->inventory = insert(new->inventory, &food->inventory);
	} else if(mobtype == FALLEN_ANGEL) {
		Item * sword = xalloc(Item);
		*sword = special_items[F_SWORD];
		new->inventory = insert(new->inventory, &sword->inventory);
		wield_item(new, sword);

		Item * food = xalloc(Item);

		food->count = 1;
		food->symbol = '%';
		food->name = "Manna";
		food->type = FOOD;
		food->value = 50;
		new->inventory = insert(new->inventory, &food->inventory);

		new->is_bold = true;
		new->luminosity = 1;
	}

	return new;
}

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
	HunterState * data = (HunterState *) enemy->data;

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
	HunterState * data = (HunterState *) enemy->data;

	if(data->refcount == 1) {
		xfree(data);
	} else {
		data->refcount --;
	}

	drop_corpse(enemy);
}
