#include <curses.h>

#include "level.h"
#include "mob.h"
#include "item.h"

int main() {

  //set up curses
  initscr();
  cbreak();
  noecho();
  nonl();
  intrflush(stdscr, FALSE);
  keypad(stdscr, TRUE);

  addstr("Hello world!");
  getch();

  //end curses
  nl();
  echo();
  nocbreak();
  endwin();
}
