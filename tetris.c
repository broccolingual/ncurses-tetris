#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <ncurses.h>

#include "tetris.h"

BLOCK BLOCKS[BLOCK_MAX] = {
  // BLOCK_I | CYAN
  { 
    1,
    {{0, 0}, {1, 0}, {-1, 0}, {-2, 0}}
  },

  // BLOCK_O | YELLOW
  { 
    2,
    {{0, 0}, {0, 1}, {1, 1}, {1, 0}}
  },

  // BLOCK_S | GREEN
  { 
    3,
    {{0, 0}, {0, 1}, {-1, 1}, {1, 0}}
  },

  // BLOCK_Z | RED
  { 
    4,
    {{0, 0}, {0, 1}, {1, 1}, {-1, 0}}
  },

  // BLOCK_J | BLUE
  { 
    5,
    {{0, 0}, {0, -1}, {1, 0}, {2, 0}}
  },

  // BLOCK_L | WHITE
  { 
    6,
    {{0, 0}, {0, -1}, {-1, 0}, {-2, 0}}
  },

  // BLOCK_T | MAGENTA
  { 
    7,
    {{0, 0}, {0, -1}, {1, 0}, {-1, 0}}
  }
};

int SCORE = 0;
int DROP_COUNT = 0;
int INTERVAL 0.5
int FIELD[FIELD_HEIGHT+FIELD_HEIGHT_MARGIN][FIELD_WIDTH];
TARGET target;

int main(void) {
  initscr(); // 端末の初期化
  curs_set(0); // カーソルを非表示
  noecho(); // 入力した文字を非表示
  cbreak(); // Enter不要の入力モード
  nodelay(stdscr, TRUE); // getchをノンブロッキングにする

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

  int cx, cy, w, h;
  getmaxyx(stdscr, h, w); // 画面幅の取得
	cy = (h - (FIELD_HEIGHT + FIELD_HEIGHT_MARGIN) * HEIGHT_RATIO) / 2; // 縦座標の中心を計算
	cx = (w - FIELD_WIDTH * WIDTH_RATIO) / 2; // 横座標の中心を計算

  int maxScore = loadHighestScore();
  int key;
  bool isGameover = false;

  makeField();
  setCurrentBlock();
  updateBlock(target.type.color);
  drawField(cx, cy);
  drawScore(cx, cy, maxScore);
  drawInst(cx, cy);
  
  clock_t lastClock = clock();
  while (1) {
    if (checkGameover()) {
      isGameover = true;
      break;
    }

    clock_t nowClock = clock();

    if (nowClock >= lastClock + (INTERVAL * CLOCKS_PER_SEC)) {
      lastClock = nowClock;
      moveDOWN();
      SCORE++;
      
      DROP_COUNT++;
      if (DROP_COUNT % 120 == 0 && INTERVAL >= 0.1) {
        INTERVAL -= 0.05;
      }

      erase(); // 画面消去
      refreshField();
      updateBlock(target.type.color);
      drawField(cx, cy);
      drawScore(cx, cy, maxScore);
      drawInst(cx, cy);
      refresh(); // 画面再描画
    }

    key = getch();
    if (key == 'q') {
      updateHighestScore();
      break;
    }
    switch (key) {
      case 'a':
        moveLEFT();
        break;
      case 's':
        moveDOWN();
        break;
      case 'd':
        moveRIGHT();
        break;
      case 'w':
        target.type = rotateBlock();
        break;
    }

    erase(); // 画面消去

    // 接触判定
    if (changeBlockState()) {
      refreshField();
      updateBlock(target.type.color + 10);
      searchAlign();
      setCurrentBlock();
      updateBlock(target.type.color);
    } else {
      refreshField();
      updateBlock(target.type.color);
    }

    drawField(cx, cy);
    drawScore(cx, cy, maxScore);
    drawInst(cx, cy);

    refresh(); // 画面再描画
  }

  if (isGameover) {
    erase(); // 画面消去
    drawGameover(cx, cy);
    refresh(); // 画面再描画

    while (1) {
      key = getch();
      if (key == 'q') {
        updateHighestScore();
        break;
      }
    }
  }

	endwin(); // スクリーンの終了

  return 0;
}

int loadHighestScore() {
  char lastScore[256];
  int maxScore = 0;

  FILE *fp = fopen("score.txt", "r");
  if (fp == NULL) return 0;
  if (fgets(lastScore, 256, fp) != NULL) {
    maxScore = atoi(lastScore);
  }
  fclose(fp);
  return maxScore;
}

void updateHighestScore() {
  if (loadHighestScore() < SCORE) {
    FILE *fp = fopen("score.txt", "w");
    if (fp == NULL) return;
    fprintf(fp, "%d\n", SCORE);
    fclose(fp);
  }
}

void makeField() {
  for (int y = 0; y < FIELD_HEIGHT_MARGIN - 1; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      FIELD[y][x] = MARGIN;
    }
  }

  for (int x = 0; x < FIELD_WIDTH; x++) {
    FIELD[FIELD_HEIGHT_MARGIN - 1][x] = BORDER_C;
  }

  for (int y = FIELD_HEIGHT_MARGIN; y < FIELD_HEIGHT + FIELD_HEIGHT_MARGIN; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      FIELD[y][x] = VOID;
    }
  }
}

void drawInst(int cx, int cy) {
  attrset(COLOR_PAIR(STRING_C));
  mvaddstr(cy + 7, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "- INSTRUCTION -");
  mvaddstr(cy + 9, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "W : ROTATE");
  mvaddstr(cy + 10, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "A : MOVE LEFT");
  mvaddstr(cy + 11, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "D : MOVE RIGHT");
  mvaddstr(cy + 12, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "S : MOVE DOWN");
  mvaddstr(cy + 13, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "Q : EXIT");
}

void drawScore(int cx, int cy, int maxScore) {
  char highestScore[256];
  char score[256]; 

  attrset(COLOR_PAIR(STRING_C));
  mvaddstr(cy, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "- HIGHEST SCORE -");
  sprintf(highestScore, "%d", maxScore);
  mvaddstr(cy + 1, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, highestScore);
  mvaddstr(cy + 3, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "- SCORE -");
  sprintf(score, "%d", SCORE);
  mvaddstr(cy + 4, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, score);
}

void drawGameover(int cx, int cy) {
  attrset(COLOR_PAIR(STRING_C));
  mvaddstr(cy, cx, "G A M E     O V E R ");
}

void drawField(int cx, int cy) {
  for (int y = 0; y < FIELD_HEIGHT + FIELD_HEIGHT_MARGIN; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      int colorCode;

      if (FIELD[y][x] > 10) {
        colorCode = FIELD[y][x] - 10;
      } else if (FIELD[y][x] == MARGIN) {
        colorCode = VOID;
      } else {
        colorCode = FIELD[y][x];
      }
      
      attrset(COLOR_PAIR(colorCode));
      if (colorCode != BORDER_C) {
        mvaddstr(cy + y * HEIGHT_RATIO, cx + x * WIDTH_RATIO, "  ");
      } else {
        mvaddstr(cy + y * HEIGHT_RATIO, cx + x * WIDTH_RATIO, "__");
      }
    }
  }
}

void refreshField() {
  for (int y = 0; y < FIELD_HEIGHT_MARGIN - 1; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      FIELD[y][x] = MARGIN;
    }
  }

  for (int x = 0; x < FIELD_WIDTH; x++) {
    FIELD[FIELD_HEIGHT_MARGIN - 1][x] = BORDER_C;
  }

  for (int y = FIELD_HEIGHT_MARGIN; y < FIELD_HEIGHT + FIELD_HEIGHT_MARGIN; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      if (BLOCK_I <= FIELD[y][x] && FIELD[y][x] <= BLOCK_T) {
        FIELD[y][x] = VOID;
      }
    }
  }
}

bool canMove(int dx, int dy) {
  int cx = target.p.x; int cy = target.p.y;
  BLOCK block = target.type;

  for (int i = 0; i < 4; i++) {
    int nx = cx + block.p[i].x + dx;
    int ny = cy + block.p[i].y + dy;

    if (!((0 <= nx && nx < FIELD_WIDTH) && (ny < FIELD_HEIGHT + FIELD_HEIGHT_MARGIN) && !(FIELD[ny][nx] > 10))) {
      return false;
    }
  }
  return true;
}

void moveDOWN() {
  if (canMove(0, 1)) {
    target.p.y++;
  }
}

void moveRIGHT() {
  if (canMove(1, 0)) {
    target.p.x++;
  }
}

void moveLEFT() {
  if (canMove(-1, 0)) {
    target.p.x--;
  }
}

bool canRotate() {
  int cx = target.p.x; int cy = target.p.y;
  BLOCK block = target.type;

  for (int i = 0; i < 4; i++) {
    int nx = cx - block.p[i].y;
    int ny = cy + block.p[i].x;

    if (!((0 <= nx && nx < FIELD_WIDTH) && (ny < FIELD_HEIGHT + FIELD_HEIGHT_MARGIN) && !(FIELD[ny][nx] > 10))) {
      return false;
    }
  }
  return true;
}

BLOCK rotateBlock() {
  BLOCK before = target.type;
  BLOCK after;
  after.color = before.color;

  if (canRotate()) {
    for (int i = 0; i < 4; i++) {
      int bx = before.p[i].x;
      int by = before.p[i].y;

      after.p[i].x = -by;
      after.p[i].y = bx;
    }
    return after;
  }
  return before;
}

BLOCK selectRandomBlock() {
  srand((unsigned int)time(NULL));
  return BLOCKS[rand() % BLOCK_MAX];
}

void setCurrentBlock() {
  target.type = selectRandomBlock();
  target.p.x = (FIELD_WIDTH / 2) - 1;
  target.p.y = 2;
}

void updateBlock(int state) {
  int cx = target.p.x; int cy = target.p.y;
  BLOCK block = target.type;

  for (int i = 0; i < 4; i++) {
    FIELD[cy + block.p[i].y][cx + block.p[i].x] = state;
  }
}

bool changeBlockState() {
  int cx = target.p.x; int cy = target.p.y;
  BLOCK block = target.type;

  for (int i = 0; i < 4; i++) {
    int nx = cx + block.p[i].x + 0;
    int ny = cy + block.p[i].y + 1;

    if (FIELD[ny][nx] > 10 || ny == FIELD_HEIGHT + FIELD_HEIGHT_MARGIN) {
      return true;
    }
  }
  return false;
}

void searchAlign() {
  int lineCount = 0;

  for (int y = FIELD_HEIGHT_MARGIN; y < FIELD_HEIGHT + FIELD_HEIGHT_MARGIN; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      if (FIELD[y][x] == VOID) break;
      if (x == FIELD_WIDTH - 1) {
        lineCount++;
        deleteAlign(y);
      }
    }
  }

  switch (lineCount) {
    case 1:
      SCORE += 40;
      break;
    case 2:
      SCORE += 100;
      break;
    case 3:
      SCORE += 300;
      break;
    case 4:
      SCORE += 1200;
      break;
    default:
      break;
  }
}

void deleteAlign(int dy) {
  for (int x = 0; x < FIELD_WIDTH; x++) {
    FIELD[dy][x] = VOID;
  }

  int TMP_FIELD[dy][FIELD_WIDTH];

  for (int y = FIELD_HEIGHT_MARGIN; y < dy; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      TMP_FIELD[y][x] = FIELD[y][x];
    }
  }

  for (int y = FIELD_HEIGHT_MARGIN; y < dy; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      FIELD[y + 1][x] = TMP_FIELD[y][x];
    }
  }
}

bool checkGameover() {
  for (int x = 0; x < FIELD_WIDTH; x++) {
    if (FIELD[FIELD_HEIGHT_MARGIN][x] > 10) return true;
  }
  return false;
}
