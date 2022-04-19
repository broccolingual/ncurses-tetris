#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <ncurses.h>

#include "tetris.h"

// テトリミノの定義
BLOCK BLOCKS[BLOCK_MAX + 1] = {
  // BLOCK_NULL
  {
    0,
    {{0, 0}, {0, 0}, {0, 0}, {0, 0}}
  },
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
int DROP_COUNT = 0; // ブロックの落下処理が行われた回数
double INTERVAL = 0.5; // 秒/1ブロック落下
int FIELD[FIELD_HEIGHT+FIELD_HEIGHT_MARGIN][FIELD_WIDTH]; // テトリスのフィールド
TARGET target; // 現在操作しているブロックのデータ
TARGET next; // 次に操作するブロックのデータ

void setWindow() {
  curs_set(0); // カーソルを非表示
  noecho(); // 入力した文字を非表示
  cbreak(); // Enter不要の入力モード
  nodelay(stdscr, TRUE); // getchのノンブロッキング化
}

void setColors() {
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
}

void drawGameWindow(int cx, int cy, int maxScore, TARGET *np, time_t timeStart) {
  drawField(cx, cy);
  drawScore(cx, cy, maxScore);
  drawInst(cx, cy);
  drawNext(cx, cy, np);
  drawElapsedTime(cx, cy, timeStart);
}

int main(void) {
  initscr(); // 端末の初期化
  setWindow(); // windowの初期設定
  setColors(); // 色の設定

  int cx, cy, w, h;
  getmaxyx(stdscr, h, w); // 画面幅の取得
	cy = (h - (FIELD_HEIGHT + FIELD_HEIGHT_MARGIN) * HEIGHT_RATIO) / 2; // 縦座標の中心を計算
	cx = (w - FIELD_WIDTH * WIDTH_RATIO) / 2; // 横座標の中心を計算

  int maxScore = loadHighestScore();
  bool isGameover = false;
  bool dropDelay = false;
  clock_t lastDelayClock;
  time_t elapsedTimeStart = time(NULL);

  makeField(); // フィールドの初期化
  setBlock(&target); // 操作ブロックを設定
  setBlock(&next); // 次のブロックを設定
  updateBlock(target.type.color);
  drawGameWindow(cx, cy, maxScore, &next, elapsedTimeStart);
  
  clock_t lastClock = clock();
  while (1) {

    // ゲームオーバー判定
    if (checkGameover()) {
      isGameover = true;
      break;
    }

    // 落下処理
    clock_t nowClock = clock();
    if (nowClock >= lastClock + (INTERVAL * CLOCKS_PER_SEC)) {
      lastClock = nowClock;
      moveDOWN(&target);
      SCORE++;

      DROP_COUNT++;
      if (DROP_COUNT % 120 == 0 && INTERVAL >= 0.1) {
        INTERVAL -= 0.05;
      }

      erase(); // 画面消去
      refreshField();
      updateBlock(target.type.color);
      drawGameWindow(cx, cy, maxScore, &next, elapsedTimeStart);
      refresh(); // 画面再描画
    }

    int key = getch(); // キー入力

    // ゲームの終了
    if (key == 'q') {
      updateHighestScore();
      break;
    }

    // テトリミノの操作
    switch (key) {
      case 'a':
        moveLEFT(&target);
        break;
      case 's':
        moveDOWN(&target);
        break;
      case 'd':
        moveRIGHT(&target);
        break;
      case 'w':
        target.type = rotateBlock(&target);
        break;
    }

    erase(); // 画面消去

    refreshField();

    // 接触判定
    if (changeBlockState(&target)) {
      if (!dropDelay) {
        dropDelay = true;
        lastDelayClock = clock();
      }

      clock_t nowDelayClock = clock();
      if (nowDelayClock >= lastDelayClock + (INTERVAL * CLOCKS_PER_SEC)) {
        dropDelay = false;

        updateBlock(target.type.color + 10);
        searchAlign();
        target = next;
        setBlock(&next);
      }
    }

    updateBlock(target.type.color);

    drawGameWindow(cx, cy, maxScore, &next, elapsedTimeStart);

    refresh(); // 画面再描画
  }

  if (isGameover) {
    erase(); // 画面消去
    drawGameover(cx, cy);
    refresh(); // 画面再描画

    while (1) {
      if (getch() == 'q') {
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

void drawElapsedTime(int cx, int cy, time_t timeStart) {
  char strTimeMin[8];
  char strTimeSec[8];

  attrset(COLOR_PAIR(STRING_C));

  sprintf(strTimeMin, "%02d", ((int) difftime(time(NULL), timeStart)) / 60);
  sprintf(strTimeSec, "%02d", ((int) difftime(time(NULL), timeStart)) % 60);
  strcat(strTimeMin, ":");
  strcat(strTimeMin, strTimeSec);

  mvaddstr(cy + 6, cx - 10, "- TIME -");
  mvaddstr(cy + 8, cx - 9, strTimeMin);
}

void drawNext(int cx, int cy, TARGET *np) {
  attrset(COLOR_PAIR(STRING_C));
  mvaddstr(cy, cx - 10, "- NEXT -");

  if (np->type.color != 6) {
    attrset(COLOR_PAIR(np->type.color));
  } else {
    attrset(COLOR_PAIR(VOID));
  }
  for (int i = 0; i < 4; i++) {
    mvaddstr(cy + 3 + np->type.p[i].y, cx - 8 + np->type.p[i].x * WIDTH_RATIO, "  ");
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
  mvaddstr(cy + 15, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "Copyright © 2022 Broccolingual");
  mvaddstr(cy + 16, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "All Rights Reserved.");
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

bool canMove(int dx, int dy, TARGET *tp) {
  for (int i = 0; i < 4; i++) {
    int nx = tp->p.x + tp->type.p[i].x + dx;
    int ny = tp->p.y + tp->type.p[i].y + dy;

    if (!((0 <= nx && nx < FIELD_WIDTH) && (ny < FIELD_HEIGHT + FIELD_HEIGHT_MARGIN) && !(FIELD[ny][nx] > 10))) {
      return false;
    }
  }
  return true;
}

void moveDOWN(TARGET *tp) {
  if (canMove(0, 1, tp)) {
    tp->p.y++;
  }
}

void moveRIGHT(TARGET *tp) {
  if (canMove(1, 0, tp)) {
    tp->p.x++;
  }
}

void moveLEFT(TARGET *tp) {
  if (canMove(-1, 0, tp)) {
    tp->p.x--;
  }
}

bool canRotate(TARGET *tp) {
  for (int i = 0; i < 4; i++) {
    int nx = tp->p.x - tp->type.p[i].y;
    int ny = tp->p.y + tp->type.p[i].x;

    if (!((0 <= nx && nx < FIELD_WIDTH) && (ny < FIELD_HEIGHT + FIELD_HEIGHT_MARGIN) && !(FIELD[ny][nx] > 10))) {
      return false;
    }
  }
  return true;
}

BLOCK rotateBlock(TARGET *tp) {
  BLOCK after = tp->type;

  if (canRotate(tp)) {
    for (int i = 0; i < 4; i++) {
      int bx = tp->type.p[i].x;
      int by = tp->type.p[i].y;

      after.p[i].x = -by;
      after.p[i].y = bx;
    }
    return after;
  }
  return tp->type;
}

BLOCK selectRandomBlock() {
  srand((unsigned int)time(NULL));
  return BLOCKS[rand() % BLOCK_MAX + 1];
}

void setBlock(TARGET *tp) {
  tp->type = selectRandomBlock();
  tp->p.x = (FIELD_WIDTH / 2) - 1;
  tp->p.y = 2;
}

void updateBlock(int state) {
  int cx = target.p.x; int cy = target.p.y;
  BLOCK block = target.type;

  for (int i = 0; i < 4; i++) {
    FIELD[cy + block.p[i].y][cx + block.p[i].x] = state;
  }
}

bool changeBlockState(TARGET *tp) {
  for (int i = 0; i < 4; i++) {
    int nx = tp->p.x + tp->type.p[i].x + 0;
    int ny = tp->p.y + tp->type.p[i].y + 1;

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
