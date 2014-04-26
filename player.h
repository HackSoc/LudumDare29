#ifndef PLAYER_H
#define PLAYER_H

Mob * create_player(void);
bool attackmove(struct Mob * player, unsigned int xdiff, unsigned int ydiff,
				unsigned int damage);
bool attackmove_relative(struct Mob * player, int xdiff, int ydiff,
						 unsigned int damage);
void player_turn(Mob * player);
void player_death(Mob * player);

#endif
