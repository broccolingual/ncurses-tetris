#include <stdio.h>
#include <stdbool.h>

#include <ncurses.h>

void initWindow() {
  curs_set(0); // カーソルを非表示
  noecho(); // 入力した文字を非表示
  cbreak(); // Enter不要の入力モード
  nodelay(stdscr, TRUE); // getchのノンブロッキング化
  keypad(stdscr, TRUE); // カーソルキーの有効化
}

void initColors() {
  start_color();
  init_pair(1, COLOR_CYAN, COLOR_CYAN); // CYAN
  init_pair(2, COLOR_YELLOW, COLOR_YELLOW); // YELLOW
  init_pair(3, COLOR_GREEN, COLOR_GREEN); // GREEN
  init_pair(4, COLOR_RED, COLOR_RED); // RED
  init_pair(5, COLOR_BLUE, COLOR_BLUE); // BLUE
  init_pair(6, COLOR_BLACK, COLOR_BLACK); // BLACK
  init_pair(7, COLOR_MAGENTA, COLOR_MAGENTA); // MAGENTA
  init_pair(8, COLOR_WHITE, COLOR_WHITE); // WHITE
  init_pair(9, COLOR_WHITE, COLOR_BLACK); // For String
  init_pair(10, COLOR_BLACK, COLOR_WHITE); // For Border
  init_pair(20, COLOR_CYAN, COLOR_BLACK); // For String (Strong)
}