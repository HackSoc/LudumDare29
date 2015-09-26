#ifdef AUTOPLAY
#include <stdlib.h>

#include "autoplay.h"
#include "item.h"
#include "player.h"

const void ** autoplay_list_choice(const char * choices[],
				   const void * results[]){
  const void ** selected = xcalloc(2,void *);
  (void) choices;
  // choose the first list item
  selected[0] = results[0];
  selected[1] = NULL;
  return selected;
}

// wield lantern and starting weapon, then wear starting armour
static char opening_moves[] = {'w','x','w','x','W'};
static unsigned int move_count = 0;

Direction autoplay_select_direction(void){
  Direction out = {.dx = 0, .dy = 0, .ch = 0};

  // perform a standard set of moves to begin with
  if(move_count < sizeof(opening_moves)){
    out.ch = opening_moves[move_count];
    move_count++;
    return out;
  }

  // if no standard opening move to do, move randomly
  out.dx = (rand() % 3) - 1;
  out.dy = (rand() % 3) - 1;

  return out;
  
}

#endif // AUTOPLAY
