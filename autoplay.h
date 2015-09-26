#ifndef AUTOPLAY_H
#define AUTOPLAY_H
#ifdef AUTOPLAY

#include "player.h"

const void ** autoplay_list_choice(const char * choices[],
				   const void * results[]);
Direction autoplay_select_direction(void);

#endif // AUTOPLAY
#endif // AUTOPLAY_H
