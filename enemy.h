#ifndef ENEMY_H
#define ENEMY_H

#include <stdbool.h>
#include "mob.h"

/**
 * Shared state for hunter enemies
 */
typedef struct Target {
	unsigned int x; /**< The target X, not valid unless chasing */
	unsigned int y; /**< The target Y, not valid unless chasing */
	bool chase; /**< Whether the target has been sighted or not */
	unsigned int refcount; /**< Number of hunters sharing this state */
} Target;

void random_move(Mob * enemy);
void random_move_diagonals(Mob * enemy);
void move_towards(Mob * enemy,
                  unsigned int x, unsigned int y,
                  bool diagonal);

void simple_enemy_turn(Mob * enemy);
void hunter_turn(Mob * enemy);

void hunter_death(Mob * enemy);

#endif
