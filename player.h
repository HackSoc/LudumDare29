#ifndef PLAYER_H
#define PLAYER_H

#include "level.h"

/**
 * Record of what the player has seen in a level
 */
typedef struct Terrain {
	List levels; /**< Doubly-linked list of terrain in other levels */
	char symbols[LEVELWIDTH][LEVELHEIGHT]; /**< Known symbols */
} Terrain;

/**
 * Data specific to the player
 */
typedef struct PlayerData {
	Terrain * terrain; /**< Record of what the player has seen in the current level */
} PlayerData;

/**
 * A direction. If the direction is valid, ch is 0, otherwise it is
 * the selected character.
 */
typedef struct Direction {
	int dx; /**< The X offset (in [-1,0,1]) */
	int dy; /**< The Y offset (in [-1,0,1]) */
	int ch; /**< The character, if it wasn't a direction */
} Direction;

Mob * create_player(void);
bool attackmove(struct Mob * player, unsigned int xdiff, unsigned int ydiff);
bool attackmove_relative(struct Mob * player, int xdiff, int ydiff);
void player_turn(Mob * player);
void player_death(Mob * player);

#endif
