#ifndef ENEMY_H
#define ENEMY_H

#include <stdbool.h>
#include "mob.h"

/* should keep the same structure as default_mobs in enemy.c */
enum EnemyType { HEDGEHOG, SQUIRREL, DUCK, ORC, WOLFMAN, FALLEN_ANGEL, DRAGON, NUM_ENEMY_TYPES };

/**
 * Shared state for hunter enemies
 */
typedef struct HunterState {
	unsigned int x; /**< The target X, not valid unless chasing */
	unsigned int y; /**< The target Y, not valid unless chasing */
	bool chase; /**< Whether the target has been sighted or not */
	unsigned int refcount; /**< Number of hunters sharing this state */
} HunterState;

Mob * create_enemy(enum EnemyType mobtype);

void random_move(Mob * enemy);
void random_move_diagonals(Mob * enemy);
void move_towards(Mob * enemy,
                  unsigned int x, unsigned int y,
                  bool diagonal);

void simple_enemy_turn(Mob * enemy);
void hunter_turn(Mob * enemy);

void hunter_death(Mob * enemy);

#endif
