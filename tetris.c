#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <ncurses.h>

#include "tetris.h"

BLOCK BLOCKS[BLOCK_MAX] = {
  // BLOCK_I
  {
    {{0, 0}, {0, 1}, {0, -1}, {0, -2}}
  },

  // BLOCK_O
  {
    {{0, 0}, {0, 1}, {1, 1}, {1, 0}}
  },

  // BLOCK_S
  { 
    {{0, 0}, {0, 1}, {-1, 1}, {1, 0}}
  },

  // BLOCK_Z
  {
    {{0, 0}, {0, 1}, {1, 1}, {-1, 0}}
  },

  // BLOCK_J
  {
    {{0, 0}, {0, -1}, {1, 0}, {2, 0}}
  },

  // BLOCK_L
  {
    {{0, 0}, {0, -1}, {-1, 0}, {-2, 0}}
  },

  // BLOCK_T
  {
    {{0, 0}, {0, -1}, {1, 0}, {-1, 0}}
  }
};

int POINT = 0;
int FIELD[FIELD_HEIGHT+FIELD_HEIGHT_MARGIN][FIELD_WIDTH];
TARGET target;

int main(void) {
  initscr(); // 端末の初期化
  curs_set(0); // カーソルを非表示
  noecho(); // 入力した文字を非表示
  cbreak(); // Enter不要の入力モード
  nodelay(stdscr, TRUE); // getchをノンブロッキングにする

  start_color();
  init_pair(1, COLOR_WHITE, COLOR_WHITE);
  init_pair(2, COLOR_WHITE, COLOR_BLACK);
  init_pair(3, COLOR_CYAN, COLOR_CYAN);
  init_pair(4, COLOR_RED, COLOR_RED);
  init_pair(5, COLOR_BLACK, COLOR_BLACK);

  int cx, cy, w, h;
  getmaxyx(stdscr, h, w); // 画面幅の取得
	cy = (h - (FIELD_HEIGHT + FIELD_HEIGHT_MARGIN) * HEIGHT_RATIO) / 2; // 縦座標の中心を計算
	cx = (w - FIELD_WIDTH * WIDTH_RATIO) / 2; // 横座標の中心を計算

  int key;
  bool isGameover = false;

  makeField();
  setCurrentBlock();
  updateBlock(2);
  drawField(cx, cy);
  drawScore(cx, cy);
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
      POINT++;

      erase(); // 画面消去
      refreshField();
      updateBlock(2);
      drawField(cx, cy);
      drawScore(cx, cy);
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
      updateBlock(1);
      searchAlign();
      setCurrentBlock();
      updateBlock(2);
    } else {
      refreshField();
      updateBlock(2);
    }

    drawField(cx, cy);
    drawScore(cx, cy);
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

void updateHighestScore() {
  char lastPoint[256];
  char buf[256];
  int maxPoint = 0;

  FILE *fp = fopen("point.txt", "r");
  if (fp == NULL) return;
  if (fgets(lastPoint, 256, fp) != NULL) {
    maxPoint = atoi(lastPoint);
  }
  fclose(fp);

  if (maxPoint < POINT) {
    FILE *fp = fopen("point.txt", "w");
    if (fp == NULL) return;
    fprintf(fp, "%d\n", POINT);
    fclose(fp);
  }
}

void makeField() {
  for (int y = 0; y < FIELD_HEIGHT_MARGIN; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      FIELD[y][x] = 3;
    }
  }

  for (int y = FIELD_HEIGHT_MARGIN; y < FIELD_HEIGHT + FIELD_HEIGHT_MARGIN; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      FIELD[y][x] = 0;
    }
  }
}

void drawInst(int cx, int cy) {
  attrset(COLOR_PAIR(2));
  mvaddstr(cy + 4, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "- INSTRUCTION -");
  mvaddstr(cy + 6, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "W : ROTATE");
  mvaddstr(cy + 7, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "A : MOVE LEFT");
  mvaddstr(cy + 8, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "D : MOVE RIGHT");
  mvaddstr(cy + 9, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "S : MOVE DOWN");
  mvaddstr(cy + 10, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "Q : EXIT");
}

void drawScore(int cx, int cy) {
  attrset(COLOR_PAIR(2));
  mvaddstr(cy, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "- SCORE -");
  char buf[10];
  sprintf(buf, "%d", POINT);
  mvaddstr(cy + 1, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, buf);
}

void drawGameover(int cx, int cy) {
  attrset(COLOR_PAIR(2));
  mvaddstr(cy, cx, "G A M E     O V E R ");
}

void drawField(int cx, int cy) {
  for (int y = 0; y < FIELD_HEIGHT + FIELD_HEIGHT_MARGIN; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      if (FIELD[y][x] == 0) {
        attrset(COLOR_PAIR(1));
      } else if (FIELD[y][x] == 1) {
        attrset(COLOR_PAIR(4));
      } else if (FIELD[y][x] == 2) {
        attrset(COLOR_PAIR(3));
      } else if (FIELD[y][x] == 3) {
        attrset(COLOR_PAIR(5));
      }
      mvaddstr(cy + y * HEIGHT_RATIO, cx + x * WIDTH_RATIO, "  ");
    }
  }
}

void refreshField() {
  for (int y = 0; y < FIELD_HEIGHT_MARGIN; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      FIELD[y][x] = 3;
    }
  }

  for (int y = FIELD_HEIGHT_MARGIN; y < FIELD_HEIGHT + FIELD_HEIGHT_MARGIN; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      if (FIELD[y][x] == 2) {
        FIELD[y][x] = 0;
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

    if (!((0 <= nx && nx < FIELD_WIDTH) && (ny < FIELD_HEIGHT + FIELD_HEIGHT_MARGIN) && FIELD[ny][nx] != 1)) {
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

    if (!((0 <= nx && nx < FIELD_WIDTH) && (ny < FIELD_HEIGHT + FIELD_HEIGHT_MARGIN) && FIELD[ny][nx] != 1)) {
      return false;
    }
  }
  return true;
}

BLOCK rotateBlock() {
  BLOCK before = target.type;
  BLOCK after;
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

    if (FIELD[ny][nx] == 1 || ny == FIELD_HEIGHT + FIELD_HEIGHT_MARGIN) {
      return true;
    }
  }
  return false;
}

void searchAlign() {
  int lineCount = 0;

  for (int y = FIELD_HEIGHT_MARGIN; y < FIELD_HEIGHT + FIELD_HEIGHT_MARGIN; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      if (FIELD[y][x] == 0) break;
      if (x == FIELD_WIDTH - 1) {
        lineCount++;
        deleteAlign(y);
      }
    }
  }

  switch (lineCount) {
    case 1:
      POINT += 40;
      break;
    case 2:
      POINT += 100;
      break;
    case 3:
      POINT += 300;
      break;
    case 4:
      POINT += 1200;
      break;
    default:
      break;
  }
}

void deleteAlign(int dy) {
  for (int x = 0; x < FIELD_WIDTH; x++) {
    FIELD[dy][x] = 0;
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
    if (FIELD[FIELD_HEIGHT_MARGIN][x] == 1) return true;
  }
  return false;
}
