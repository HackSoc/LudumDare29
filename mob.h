#ifndef MOB_H
#define MOB_H

struct Item;
struct Level;

#include "item.h"
#include "level.h"

typedef struct Mob{
  struct Level * level;
  char symbol;
  struct Mob * next;
  struct Mob * prev;
  struct Item * items;
} Mob;

#endif /*MOB_H*/
