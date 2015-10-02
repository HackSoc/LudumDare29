#ifdef AUTOPLAY
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "autoplay.h"
#include "item.h"
#include "player.h"
#include "mob.h"
#include "level.h"

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

// get the cell at an offset from the player
static Cell * cell_at(Mob * player, int dx, int dy) {
  return player->level->cells[player->xpos + dx][player->ypos + dy];
}

// move into an enemy if there is one
static bool find_enemy(Mob * player, Direction * out) {
  for (int dx = -1; dx <= 1; dx ++) {
    for (int dy = -1; dy <= 1; dy ++) {
      Cell * cell = cell_at(player, dx, dy);
      if(cell->occupant != NULL && cell->occupant != player) {
        out->dx = dx;
        out->dy = dy;
        return true;
      }
    }
  }
  return false;
}

static int path_dx[32] = {0};
static int path_dy[32] = {0};
static unsigned int path_pos = 0;
static unsigned int path_maxpos = 32;

// distance between two points
static float distance(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
  float dx = (float)x1 - (float)x2;
  float dy = (float)y1 - (float)y2;

  return sqrt(dx * dx + dy * dy);
}

// simple greedy parthfinding algorithm
static void greedy_pathfind(Mob * player, unsigned int tx, unsigned int ty, unsigned int cx, unsigned int cy) {
  // check for termination
  if ((cx == tx && cy == ty) || path_pos == path_maxpos) {
    path_dx[path_pos] = 0;
    path_dy[path_pos] = 0;
    path_pos = 0;

    return;
  }

  // pick the next move: don't undo the last move, if there are no
  // better (known empty) places than the current one, terminate.
  int dx = 0;
  int dy = 0;

  float dist = distance(tx, ty, cx, cy);

  for (int xoff = -1; xoff <= 1; xoff ++) {
    if (cx == 0 && xoff == -1) continue;
    if (cx == LEVELWIDTH && xoff == 1) continue;

    for (int yoff = -1; yoff <= 1; yoff ++) {
      if (cy == 0 && yoff == -1) continue;
      if (cy == LEVELHEIGHT && yoff == 1) continue;

      if(player->level->cells[cx + xoff][cy + yoff]->solid)
        continue;

      float new_dist = distance(tx, ty, cx + xoff, cy + yoff);
      if(new_dist < dist) {
        dx = xoff;
        dy = yoff;
        dist = new_dist;
      }
    }
  }

  if (dx == 0 && dy == 0) {
    // no move found, terminate
    path_dx[path_pos] = 0;
    path_dy[path_pos] = 0;
    path_pos = 0;
  } else {
    // record the move
    path_dx[path_pos] = dx;
    path_dy[path_pos] = dy;
    path_pos ++;

    greedy_pathfind(player, tx, ty, cx + dx, cy + dy);
  }
}

// find a path from the current point.
static void pathfind(Mob * player, unsigned int tx, unsigned int ty) {
  path_pos = 0;
  greedy_pathfind(player, tx, ty, player->xpos, player->ypos);
}

Direction autoplay_select_direction(Mob * player){
  Direction out = {.dx = 0, .dy = 0, .ch = 0};

  // perform a standard set of moves to begin with
  if(move_count < sizeof(opening_moves)){
    out.ch = opening_moves[move_count];
    move_count++;
    return out;
  }

  // if standing on the stairs, go down
  if(cell_at(player, 0, 0)->baseSymbol == '>') {
    out.ch = '>';
    return out;
  }

  // if adjacent to an enemy, attack
  if(find_enemy(player, &out)) {
    return out;
  }

  // if no path to follow, or following would hit a wall (why does
  // this happen?) move randomly now and then pathfind for later.
  if((path_dx[path_pos] == 0 && path_dy[path_pos] == 0) || cell_at(player, path_dx[path_pos], path_dy[path_pos])->solid) {
    // move randomly
    do {
      out.dx = (rand() % 3) - 1;
      out.dy = (rand() % 3) - 1;
    }
    while(cell_at(player, out.dx, out.dy)->solid);

    // pathfind
    for(unsigned int x = 1; x < LEVELWIDTH; x++) {
      for(unsigned int y = 1; y < LEVELHEIGHT; y++) {
        if(player->level->cells[x][y]->baseSymbol == '>') {
          pathfind(player, x, y);
        }
      }
    }
  } else {
    out.dx = path_dx[path_pos];
    out.dy = path_dy[path_pos];
    path_pos ++;
  }

  return out;
}

#endif // AUTOPLAY
