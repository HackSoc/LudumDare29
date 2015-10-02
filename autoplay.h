#ifndef AUTOPLAY_H
#define AUTOPLAY_H
#ifdef AUTOPLAY

#include "player.h"
#include "mob.h"

const void ** autoplay_list_choice(const char * choices[],
				   const void * results[]);
Direction autoplay_select_direction(Mob * player);

#endif // AUTOPLAY
#endif // AUTOPLAY_H
