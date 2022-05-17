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
  init_color(30, 1000, 550, 0); // ORANGE
  init_color(31, 1000, 1000, 0); // YELLOW
  init_color(32, 0, 682, 936); // SKY BLUE
  init_color(33, 544, 321, 631); // PURPLE
  init_color(34, 750, 750, 750); // GRAY

  init_pair(1, 32, 32); // CYAN
  init_pair(2, 31, 31); // YELLOW
  init_pair(3, COLOR_GREEN, COLOR_GREEN); // GREEN
  init_pair(4, COLOR_RED, COLOR_RED); // RED
  init_pair(5, COLOR_BLUE, COLOR_BLUE); // BLUE
  init_pair(6, 30, 30); // ORANGE
  init_pair(7, 33, 33); // PURPLE
  init_pair(8, 34, 34); // WHITE
  init_pair(9, 34, COLOR_BLACK); // For String
  init_pair(10, COLOR_BLACK, 34); // For Border
  init_pair(20, 32, COLOR_BLACK); // For String (Strong)
}