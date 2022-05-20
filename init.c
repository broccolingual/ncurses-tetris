#include <stdio.h>
#include <stdbool.h>

#include <ncurses.h>

void initColorHex(int n, const char *cc);

void initWindow() {
  initscr(); // 端末の初期化
  curs_set(0); // カーソルを非表示
  noecho(); // 入力した文字を非表示
  cbreak(); // Enter不要の入力モード
  crmode(); // 文字をバッファに溜めない
  nodelay(stdscr, TRUE); // getchのノンブロッキング化
  keypad(stdscr, TRUE); // カーソルキーの有効化
}

void initColors() {
  start_color();

  initColorHex(30, "#66ccff"); // CYAN
  initColorHex(31, "#faf500"); // YELLOW
  initColorHex(32, "#35a16b"); // LIGHTGREEN
  initColorHex(33, "#ff2800"); // RED
  initColorHex(34, "#0041ff"); // BLUE
  initColorHex(35, "#ff9900"); // ORANGE
  initColorHex(36, "#9a0079"); // PURPLE
  initColorHex(37, "#252525"); // GRAY

  init_pair(1, 30, 30); // CYAN
  init_pair(2, 31, 31); // YELLOW
  init_pair(3, 32, 32); // LIGHTGREEN
  init_pair(4, 33, 33); // RED
  init_pair(5, 34, 34); // BLUE
  init_pair(6, 35, 35); // ORANGE
  init_pair(7, 36, 36); // PURPLE
  init_pair(8, 37, 37); // GRAY
  init_pair(9, COLOR_WHITE, COLOR_BLACK); // For String
  init_pair(20, 32, COLOR_BLACK); // For String (Strong)
}