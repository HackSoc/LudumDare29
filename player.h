#ifndef PLAYER_H
#define PLAYER_H

/**
 * Move and attack at the same time - if a mob is in the target cell,
 * damage it, but don't move.
 */
bool attackmove(struct Mob * player, unsigned int xdiff, unsigned int ydiff,
				unsigned int damage);

/**
 * Like attackmove, but relative position
 */
bool attackmove_relative(struct Mob * player, int xdiff, int ydiff,
						 unsigned int damage);

/**
 * Wait for user input, and then act accordingly.
 */
void player_turn(Mob * player);

/**
 * Say that the player is dead, and do stuff.
 */
void player_death(Mob * player);

#endif
