#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <ncurses.h>

#include "tetris.h"
#include "block.h"

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

int LEVEL = 1; // 現在のレベル
int SCORE = 0; // 現在のスコア
int LINE_SCORE = 0; // ライン消しをした回数
int DROP_COUNT = 0; // ブロックの落下処理が行われた回数
double INTERVAL = 0.5; // 秒/1ブロック落下
double GRACE_AFTER_FALLING = 0.5; // 落下後の猶予時間

int FIELD[FIELD_HEIGHT+FIELD_HEIGHT_MARGIN][FIELD_WIDTH]; // テトリスのフィールド

int SKIP_COUNT = 5; // 現在利用できるスキップの回数

TARGET target; // 現在操作しているブロックのデータ
TARGET next; // 次に操作するブロックのデータ

void drawGameWindow(int cx, int cy, int maxScore, TARGET *np, time_t timeStart) {
  drawField(cx, cy);
  drawScore(cx, cy, maxScore);
  drawInst(cx, cy);
  drawNext(cx, cy, np);
  drawElapsedTime(cx, cy, timeStart);
  drawSkip(cx, cy);
  drawLineScore(cx, cy);
  drawLevel(cx, cy);
}

int main(void) {
  initscr(); // 端末の初期化
  initWindow(); // windowの初期設定
  initColors(); // 色の設定

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

      if (DROP_COUNT % 120 == 0) {
        LEVEL++;
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
      case KEY_LEFT:
        moveLEFT(&target);
        break;
      case KEY_DOWN:
        moveDOWN(&target);
        break;
      case KEY_RIGHT:
        moveRIGHT(&target);
        break;
      case 'x':
        target.type = rotateBlockRight(&target);
        break;
      case 'z':
        target.type = rotateBlockLeft(&target);
        break;
      case 'c':
        if (SKIP_COUNT > 0) {
          SKIP_COUNT--;
          target = next;
          setBlock(&next);
        }
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
      if (nowDelayClock >= lastDelayClock + (GRACE_AFTER_FALLING * CLOCKS_PER_SEC)) {
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

void drawScore(int cx, int cy, int maxScore) {
  char highestScore[256];
  char score[256]; 

  attrset(COLOR_PAIR(STRING_C));

  mvprintw(cy, cx - 16, "| HIGHEST:");
  sprintf(highestScore, "%d", maxScore);
  attrset(COLOR_PAIR(STRONG_C));
  mvprintw(cy + 1, cx - 14, highestScore);
  attrset(COLOR_PAIR(STRING_C));
  mvprintw(cy + 2, cx - 16, "| SCORE:");
  sprintf(score, "%d", SCORE);
  attrset(COLOR_PAIR(STRONG_C));
  mvprintw(cy + 3, cx - 14, score);
}

void drawElapsedTime(int cx, int cy, time_t timeStart) {
  char strTimeMin[8];
  char strTimeSec[8];

  attrset(COLOR_PAIR(STRING_C));

  sprintf(strTimeMin, "%02d", ((int) difftime(time(NULL), timeStart)) / 60);
  sprintf(strTimeSec, "%02d", ((int) difftime(time(NULL), timeStart)) % 60);
  strcat(strTimeMin, ":");
  strcat(strTimeMin, strTimeSec);

  mvprintw(cy + 4, cx - 16, "| TIME:");
  attrset(COLOR_PAIR(STRONG_C));
  mvprintw(cy + 5, cx - 14, strTimeMin);
}

void drawLineScore(int cx, int cy) {
  char lineScore[256]; 

  attrset(COLOR_PAIR(STRING_C));

  mvprintw(cy + 6, cx - 16, "| LINES:");
  sprintf(lineScore, "%d", LINE_SCORE);
  attrset(COLOR_PAIR(STRONG_C));
  mvprintw(cy + 7, cx - 14, lineScore);
}

void drawLevel(int cx, int cy) {
  char level[256]; 

  attrset(COLOR_PAIR(STRING_C));

  mvprintw(cy + 8, cx - 16, "| LEVEL:");
  sprintf(level, "%d", LEVEL);
  attrset(COLOR_PAIR(STRONG_C));
  mvprintw(cy + 9, cx - 14, level);
}

void drawNext(int cx, int cy, TARGET *np) {
  attrset(COLOR_PAIR(STRING_C));
  mvprintw(cy, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "| NEXT:");

  if (np->type.color != 6) {
    attrset(COLOR_PAIR(np->type.color));
  } else {
    attrset(COLOR_PAIR(VOID));
  }
  for (int i = 0; i < 4; i++) {
    mvprintw(cy + 3 + np->type.p[i].y, cx + (FIELD_WIDTH * WIDTH_RATIO) + 6 + np->type.p[i].x * WIDTH_RATIO, "  ");
  }
}

void drawSkip(int cx, int cy) {
  char skipLeft[16];

  attrset(COLOR_PAIR(STRING_C));

  sprintf(skipLeft, "%d", SKIP_COUNT);
  strcat(skipLeft, " / 5");

  mvprintw(cy + 6, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "| SKIP:");
  attrset(COLOR_PAIR(STRONG_C));
  mvprintw(cy + 7, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, skipLeft);
}

void drawInst(int cx, int cy) {
  attrset(COLOR_PAIR(STRING_C));

  mvprintw(cy + 9, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "| INSTRUCTION:");
  mvprintw(cy + 11, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "X     : ROTATE RIGHT");
  mvprintw(cy + 12, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "Z     : ROTATE LEFT");
  mvprintw(cy + 13, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "C    : SKIP");
  mvprintw(cy + 14, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "LEFT : MOVE LEFT");
  mvprintw(cy + 15, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "RIGHT: MOVE RIGHT");
  mvprintw(cy + 16, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "DOWN : SOFT DROP");
  mvprintw(cy + 17, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "Q    : EXIT");
  mvprintw(cy + 19, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "Copyright © 2022 Broccolingual");
  mvprintw(cy + 20, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "All Rights Reserved.");
}

void drawGameover(int cx, int cy) {
  char base[64] = "SCORE: ";
  char score[32];

  attrset(COLOR_PAIR(STRING_C));
  mvprintw(cy, cx, "G A M E     O V E R ");
  sprintf(score, "%d", SCORE);
  strcat(base, score);
  attrset(COLOR_PAIR(STRONG_C));
  mvprintw(cy + 2, cx + 5, base);
  attrset(COLOR_PAIR(STRING_C));
  mvprintw(cy + 6, cx + 6, "Q : EXIT");
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
        mvprintw(cy + y * HEIGHT_RATIO, cx + x * WIDTH_RATIO, "  ");
      } else {
        mvprintw(cy + y * HEIGHT_RATIO, cx + x * WIDTH_RATIO, "__");
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
    SCORE += 2;
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

bool canRotateRight(TARGET *tp) {
  for (int i = 0; i < 4; i++) {
    int nx = tp->p.x - tp->type.p[i].y;
    int ny = tp->p.y + tp->type.p[i].x;

    if (!((0 <= nx && nx < FIELD_WIDTH) && (ny < FIELD_HEIGHT + FIELD_HEIGHT_MARGIN) && !(FIELD[ny][nx] > 10))) {
      return false;
    }
  }
  return true;
}

bool canRotateLeft(TARGET *tp) {
  for (int i = 0; i < 4; i++) {
    int nx = tp->p.x + tp->type.p[i].y;
    int ny = tp->p.y - tp->type.p[i].x;

    if (!((0 <= nx && nx < FIELD_WIDTH) && (ny < FIELD_HEIGHT + FIELD_HEIGHT_MARGIN) && !(FIELD[ny][nx] > 10))) {
      return false;
    }
  }
  return true;
}

BLOCK rotateBlockRight(TARGET *tp) {
  BLOCK after = tp->type;

  if (canRotateRight(tp)) {
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

BLOCK rotateBlockLeft(TARGET *tp) {
  BLOCK after = tp->type;

  if (canRotateLeft(tp)) {
    for (int i = 0; i < 4; i++) {
      int bx = tp->type.p[i].x;
      int by = tp->type.p[i].y;

      after.p[i].x = by;
      after.p[i].y = -bx;
    }
    return after;
  }
  return tp->type;
}

void setBlock(TARGET *tp) {
  tp->type = BLOCKS[selectRandomBlock()];
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
        LINE_SCORE++;
        deleteAlign(y);
      }
    }
  }

  switch (lineCount) {
    case 1:
      SCORE += LEVEL * 100;
      break;
    case 2:
      SCORE += LEVEL * 300;
      break;
    case 3:
      SCORE += LEVEL * 500;
      break;
    case 4:
      SCORE += LEVEL * 800;
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
