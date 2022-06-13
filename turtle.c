#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>

#include <ncurses.h>

#include "turtle.h"
#include "block.h"
#include "field.h"
#include "init.h"
#include "memory.h"
#include "random.h"
#include "score.h"
#include "utils.h"
#include "draw.h"

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
    {{0, 0}, {0, -1}, {1, -1}, {1, 0}}
  },

  // BLOCK_S | GREEN
  { 
    3,
    {{0, 0}, {0, -1}, {-1, 0}, {1, -1}}
  },

  // BLOCK_Z | RED
  { 
    4,
    {{0, 0}, {0, -1}, {1, 0}, {-1, -1}}
  },

  // BLOCK_J | BLUE
  { 
    5,
    {{0, 0}, {0, -1}, {1, 0}, {2, 0}}
  },

  // BLOCK_L | ORANGE
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

bool getReverseOption(int argc, char *argv[]) {
  int opt;

  opterr = 0;
  while ((opt = getopt(argc, argv, "r")) != -1) {
    switch (opt) {
    case 'r':
      return true;
      break;
    default:
      fprintf(stderr, "コマンドのオプションが違います。-%c\n", optopt);
      exit(1);
    }
  }
  return false;
}

bool displayTitle(int cx, int cy) {
  while (1) {
    erase();
    drawTitle(cx, cy);
    refresh();

    switch (getch()) {
      case 'q': return false; break;
      case 's': return true; break;
    }
  }
}

void displayGameover(int cx, int cy, int score) {
  while (1) {
    erase();
    drawGameover(cx, cy, score);
    refresh();

    if (getch() == 'q') return;
  }
}

void gameLoop(int cx, int cy, bool rflag, bool *isGameover, int *score) {
  int level = 1; // 現在のレベル
  int lineScore = 0; // ライン消しをした回数
  int DROP_COUNT = 0; // ブロックの落下処理が行われた回数
  double INTERVAL = 0.5; // 秒/1ブロック落下
  double GRACE_AFTER_FALLING = 0.5; // 落下後の猶予時間
  int skipCount = 5; // 現在利用できるスキップの回数

  TARGET target; // 現在操作しているブロックのデータ
  TARGET next; // 次に操作するブロックのデータ

  int maxScore = loadHighestScore();
  bool dropDelay = false;
  clock_t lastDelayClock;
  time_t elapsedTimeStart = time(NULL);
  int **FIELD = NULL;
  FIELD = mallocFieldAllocation(FIELD_WIDTH, FIELD_HEIGHT);

  initField(FIELD); // フィールドの初期化
  setBlock(&target); // 操作ブロックを設定
  setBlock(&next); // 次のブロックを設定
  updateBlock(&target, FIELD, target.type.color);
  drawGameWindow(cx, cy, FIELD, maxScore, &next, elapsedTimeStart, rflag, *score, level, lineScore, skipCount);
  
  clock_t lastClock = clock();
  while (1) {
    if (checkGameover(FIELD)) { *isGameover = true; return; }

    // 落下処理
    clock_t nowClock = clock();
    if (nowClock >= lastClock + (INTERVAL * CLOCKS_PER_SEC)) {
      lastClock = nowClock;
      moveDOWN(&target, FIELD);
      *score += 1;

      DROP_COUNT++;
      if (DROP_COUNT % 120 == 0 && INTERVAL >= 0.1) {
        INTERVAL -= 0.05;
      }

      if (DROP_COUNT % 120 == 0) {
        level++;
      }

      erase();
      refreshField(FIELD);
      updateBlock(&target, FIELD, target.type.color);
      drawGameWindow(cx, cy, FIELD, maxScore, &next, elapsedTimeStart, rflag, *score, level, lineScore, skipCount);
      refresh();
      continue;
    }

    switch (getch()) {
      case KEY_LEFT:
        if (rflag) {
          moveRIGHT(&target, FIELD);
        } else {
          moveLEFT(&target, FIELD);
        }
        break;
      case KEY_DOWN:
        if (rflag) {
          ;
        } else {
          moveDOWN(&target, FIELD);
        }
        break;
      case KEY_RIGHT:
        if (rflag) {
          moveLEFT(&target, FIELD);
        } else {
          moveRIGHT(&target, FIELD);
        }
        break;
      case KEY_UP:
        if (rflag) {
          moveDOWN(&target, FIELD);
        } else {
          ;
        }
        break;
      case 'x':
        if (rflag) {
          target.type = rotateBlockLeft(&target, FIELD);
        } else {
          target.type = rotateBlockRight(&target, FIELD);
        }
        break;
      case 'z':
        if (rflag) {
          target.type = rotateBlockRight(&target, FIELD);
        } else {
          target.type = rotateBlockLeft(&target, FIELD);
        }
        break;
      case 'c':
        useSkip(&target, &next, &skipCount);
        break;
      case 'q':
        return;
    }

    erase();
    refreshField(FIELD);

    // 接触判定
    if (changeBlockState(&target, FIELD)) {
      if (!dropDelay) {
        dropDelay = true;
        lastDelayClock = clock();
      }

      clock_t nowDelayClock = clock();
      if (nowDelayClock >= lastDelayClock + (GRACE_AFTER_FALLING * CLOCKS_PER_SEC)) {
        dropDelay = false;

        updateBlock(&target, FIELD, target.type.color + 10);
        searchAlign(FIELD, score, &level, &lineScore);
        target = next;
        setBlock(&next);
      }
    }

    updateBlock(&target, FIELD, target.type.color);
    drawGameWindow(cx, cy, FIELD, maxScore, &next, elapsedTimeStart, rflag, *score, level, lineScore, skipCount);
    refresh();
  }

  freeFieldAllocation(FIELD, FIELD_HEIGHT);
  updateHighestScore(*score);
}

int main(int argc, char *argv[]) {
  bool rflag = false;
  bool endFlag = false;
  bool isGameover = false;
  int cx, cy, w, h;
  int score = 0;

  rflag = getReverseOption(argc, argv);

  generateRandomSeed(); // ランダムシードの生成
  initWindow(); // windowの初期設定
  initColors(); // 色の設定

  getmaxyx(stdscr, h, w); // 画面幅の取得
  getWindowCenter(&cx, &cy, w, h);

  if (!checkWindowSize(w, h)) endFlag = true;
  // fprintf(stderr, "画面サイズを60 x 30以上にしてください。現在の画面サイズ : %d x %d\n", w, h);
  if(!displayTitle(cx, cy)) endFlag = true;

  if (!endFlag) {
    gameLoop(cx, cy, rflag, &isGameover, &score);
  }

  if (isGameover) {
    displayGameover(cx, cy, score);
  }

	endwin();
  return 0;
}

void getWindowCenter(int *cx, int *cy, int w, int h) {
  *cy = (h - (FIELD_HEIGHT) * HEIGHT_RATIO) / 2;
	*cx = (w - FIELD_WIDTH * WIDTH_RATIO) / 2;
}

void initField(int **ap) {
  for (int y = 0; y < FIELD_HEIGHT; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      ap[y][x] = VOID;
    }
  }
}


void refreshField(int **ap) {
  for (int y = 0; y < FIELD_HEIGHT; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      if (BLOCK_I <= ap[y][x] && ap[y][x] <= BLOCK_T) {
        ap[y][x] = VOID;
      }
    }
  }
}

void useSkip(TARGET *cp, TARGET *np, int *skipCount) {
  if (*skipCount > 0) {
    *skipCount -= 1;
    cp = np;
    setBlock(np);
  }
}

bool canMove(int dx, int dy, TARGET *tp, int **ap) {
  for (int i = 0; i < 4; i++) {
    int nx = tp->p.x + tp->type.p[i].x + dx;
    int ny = tp->p.y + tp->type.p[i].y + dy;

    if (!((0 <= nx && nx < FIELD_WIDTH) && (ny < FIELD_HEIGHT) && !(ap[ny][nx] > 10))) {
      return false;
    }
  }
  return true;
}

void moveDOWN(TARGET *tp, int **ap) {
  if (canMove(0, 1, tp, ap)) tp->p.y++;
}

void moveRIGHT(TARGET *tp, int **ap) {
  if (canMove(1, 0, tp, ap)) tp->p.x++;
}

void moveLEFT(TARGET *tp, int **ap) {
  if (canMove(-1, 0, tp, ap)) tp->p.x--;
}

bool canRotateRight(TARGET *tp, int **ap) {
  for (int i = 0; i < 4; i++) {
    int nx = tp->p.x - tp->type.p[i].y;
    int ny = tp->p.y + tp->type.p[i].x;

    if (!((0 <= nx && nx < FIELD_WIDTH) && (ny < FIELD_HEIGHT) && !(ap[ny][nx] > 10))) {
      return false;
    }
  }
  return true;
}

bool canRotateLeft(TARGET *tp, int **ap) {
  for (int i = 0; i < 4; i++) {
    int nx = tp->p.x + tp->type.p[i].y;
    int ny = tp->p.y - tp->type.p[i].x;

    if (!((0 <= nx && nx < FIELD_WIDTH) && (ny < FIELD_HEIGHT) && !(ap[ny][nx] > 10))) {
      return false;
    }
  }
  return true;
}

BLOCK rotateBlockRight(TARGET *tp, int **ap) {
  BLOCK after = tp->type;

  if (canRotateRight(tp, ap)) {
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

BLOCK rotateBlockLeft(TARGET *tp, int **ap) {
  BLOCK after = tp->type;

  if (canRotateLeft(tp, ap)) {
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

bool setBlock(TARGET *tp) {
  BLOCK targetBlock = BLOCKS[selectRandomBlock()];
  int setX = (FIELD_WIDTH / 2) - 1;
  int setY = 1;

  // ブロック設置判定
  // for (int i = 0; i < FIELD_WIDTH; i++) {
  //   for (int j = 0; j < 4; j++) {
  //     int nx = i + tp->type.p[j].x;
  //     int ny = setY + tp->type.p[j].y;
  //     if (!(nx >= 0 && nx < FIELD_WIDTH)) break;
  //     if (FIELD[ny][nx] > 10) break;
  //   }
  // }

  tp->type = targetBlock;
  tp->p.x = setX;
  tp->p.y = setY;
  return true;
}

void updateBlock(TARGET *tp, int **ap, int state) {
  for (int i = 0; i < 4; i++) {
    int nx = tp->p.x + tp->type.p[i].x;
    int ny = tp->p.y + tp->type.p[i].y;

    if (nx < FIELD_WIDTH && ny < FIELD_HEIGHT) ap[ny][nx] = state;
  }
}

bool changeBlockState(TARGET *tp, int **ap) {
  for (int i = 0; i < 4; i++) {
    int nx = tp->p.x + tp->type.p[i].x + 0;
    int ny = tp->p.y + tp->type.p[i].y + 1;

    if (ny == FIELD_HEIGHT) return true;
    if ((nx < FIELD_WIDTH && ny < FIELD_HEIGHT) && ap[ny][nx] > 10) {
      return true;
    }
  }
  return false;
}

void searchAlign(int **ap, int *score, int *level, int *lineScore) {
  int lineCount = 0;

  for (int y = 0; y < FIELD_HEIGHT; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      if (ap[y][x] == VOID) break;
      if (x == FIELD_WIDTH - 1) {
        lineCount++; *lineScore += 1; deleteAlign(y, ap);
      }
    }
  }

  switch (lineCount) {
    case 1:
      *score += *level * 100;
      break;
    case 2:
      *score += *level * 300;
      break;
    case 3:
      *score += *level * 500;
      break;
    case 4:
      *score += *level * 800;
      break;
  }
}

void deleteAlign(int dy, int **ap) {
  for (int x = 0; x < FIELD_WIDTH; x++) {
    ap[dy][x] = VOID;
  }

  int tp[dy][FIELD_WIDTH]; // コピー用配列

  for (int y = 0; y < dy; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      tp[y][x] = ap[y][x];
    }
  }

  for (int y = 0; y < dy; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      ap[y + 1][x] = tp[y][x];
    }
  }
}

bool checkGameover(int **ap) {
  for (int x = 0; x < FIELD_WIDTH; x++) {
    if (ap[FIELD_MARGIN][x] > 10) return true;
  }
  return false;
}
