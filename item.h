#ifndef ITEM_H
#define ITEM_H

typedef struct Item {
  char symbol;
  struct Item * next;
  struct Item * prev;
} Item;

#endif /*ITEM_H*/
