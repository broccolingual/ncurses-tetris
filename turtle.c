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

int LEVEL = 1; // 現在のレベル
int SCORE = 0; // 現在のスコア
int LINE_SCORE = 0; // ライン消しをした回数
int DROP_COUNT = 0; // ブロックの落下処理が行われた回数
double INTERVAL = 0.5; // 秒/1ブロック落下
double GRACE_AFTER_FALLING = 0.5; // 落下後の猶予時間
int SKIP_COUNT = 5; // 現在利用できるスキップの回数

TARGET target; // 現在操作しているブロックのデータ
TARGET next; // 次に操作するブロックのデータ

void drawGameWindow(int cx, int cy, int **ap, int maxScore, TARGET *np, time_t timeStart, bool rflag, int score, int level, int lineScore, int skipCount) {
  drawField(cx, cy, ap);
  drawScore(cx, cy, score, maxScore);
  drawInst(cx, cy, rflag);
  drawNext(cx, cy, np);
  drawElapsedTime(cx, cy, timeStart);
  drawSkip(cx, cy, skipCount);
  drawLineScore(cx, cy, lineScore);
  drawLevel(cx, cy, level);
}

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

bool titleLoop(int cx, int cy) {
  erase(); // 画面消去
  drawTitle(cx, cy);
  refresh(); // 画面再描画

  while (1) {
    int key = getch();
    if (key == 'q') return false;
    if (key == 's') return true;
  }
}

int main(int argc, char *argv[]) {
  bool rflag = false;
  rflag = getReverseOption(argc, argv);

  generateRandomSeed(); // ランダムシードの生成
  initWindow(); // windowの初期設定
  initColors(); // 色の設定

  int cx, cy, w, h;
  getmaxyx(stdscr, h, w); // 画面幅の取得
  getWindowCenter(&cx, &cy, w, h);

  // windowサイズチェック
  if (!checkWindowSize(w, h)) {
    endwin();
    fprintf(stderr, "画面サイズを60 x 30以上にしてください。現在の画面サイズ : %d x %d\n", w, h);
    return 1;
  }

  // タイトル画面
  if(!titleLoop(cx, cy)) {
    endwin(); return 0;
  }

  int key;
  int maxScore = loadHighestScore();
  bool isGameover = false;
  bool dropDelay = false;
  clock_t lastDelayClock;
  time_t elapsedTimeStart = time(NULL);
  int **FIELD = NULL; // フィールド配列のポインタ

  FIELD = mallocFieldAllocation(FIELD_WIDTH, FIELD_HEIGHT); // フィールドのメモリ領域確保

  initField(FIELD); // フィールドの初期化
  setBlock(&target); // 操作ブロックを設定
  setBlock(&next); // 次のブロックを設定
  updateBlock(target.type.color, FIELD);
  drawGameWindow(cx, cy, FIELD, maxScore, &next, elapsedTimeStart, rflag, SCORE, LEVEL, LINE_SCORE, SKIP_COUNT);
  
  clock_t lastClock = clock();
  while (1) {
    // ゲームオーバー判定
    if (checkGameover(FIELD)) {
      isGameover = true;
      break;
    }

    // 落下処理
    clock_t nowClock = clock();
    if (nowClock >= lastClock + (INTERVAL * CLOCKS_PER_SEC)) {
      lastClock = nowClock;
      moveDOWN(&target, FIELD);
      SCORE++;

      DROP_COUNT++;
      if (DROP_COUNT % 120 == 0 && INTERVAL >= 0.1) {
        INTERVAL -= 0.05;
      }

      if (DROP_COUNT % 120 == 0) {
        LEVEL++;
      }

      erase(); // 画面消去
      refreshField(FIELD);
      updateBlock(target.type.color, FIELD);
      drawGameWindow(cx, cy, FIELD, maxScore, &next, elapsedTimeStart, rflag, SCORE, LEVEL, LINE_SCORE, SKIP_COUNT);
      refresh(); // 画面再描画
    }

    key = getch();

    // ゲームの終了
    if (key == 'q') {
      updateHighestScore(SCORE);
      break;
    }

    // テトリミノの操作
    switch (key) {
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
        useSkip(&target, &next, &SKIP_COUNT);
        break;
    }

    erase(); // 画面消去
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

        updateBlock(target.type.color + 10, FIELD);
        searchAlign(FIELD, &SCORE, &LEVEL, &LINE_SCORE);
        target = next;
        setBlock(&next);
      }
    }

    updateBlock(target.type.color, FIELD);
    drawGameWindow(cx, cy, FIELD, maxScore, &next, elapsedTimeStart, rflag, SCORE, LEVEL, LINE_SCORE, SKIP_COUNT);
    refresh(); // 画面再描画
  }

  // ゲームオーバー処理
  if (isGameover) {
    erase(); // 画面消去
    drawGameover(cx, cy);
    refresh(); // 画面再描画

    while (1) {
      if (getch() == 'q') break;
    }
  }

  freeFieldAllocation(FIELD, FIELD_HEIGHT); // フィールドのメモリ領域開放
  updateHighestScore(SCORE);
	endwin(); // スクリーンの終了
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

void drawScore(int cx, int cy, int score, int maxScore) {
  char highestScore[256];
  char s[256]; 

  attrset(COLOR_PAIR(STRING_C));

  mvprintw(cy, cx - 16, "| HIGHEST:");
  sprintf(highestScore, "%d", maxScore);
  attrset(COLOR_PAIR(STRONG_C));
  mvprintw(cy + 1, cx - 14, highestScore);
  attrset(COLOR_PAIR(STRING_C));
  mvprintw(cy + 2, cx - 16, "| SCORE:");
  sprintf(s, "%d", score);
  attrset(COLOR_PAIR(STRONG_C));
  mvprintw(cy + 3, cx - 14, s);
}

void drawElapsedTime(int cx, int cy, time_t timeStart) {
  char strTime[9];
  timeToStr(strTime, timeStart);

  attrset(COLOR_PAIR(STRING_C));

  mvprintw(cy + 4, cx - 16, "| TIME:");
  attrset(COLOR_PAIR(STRONG_C));
  mvprintw(cy + 5, cx - 14, strTime);
}

void drawLineScore(int cx, int cy, int lineScore) {
  char ls[256]; 

  attrset(COLOR_PAIR(STRING_C));

  mvprintw(cy + 6, cx - 16, "| LINES:");
  sprintf(ls, "%d", lineScore);
  attrset(COLOR_PAIR(STRONG_C));
  mvprintw(cy + 7, cx - 14, ls);
}

void drawLevel(int cx, int cy, int level) {
  char l[256]; 

  attrset(COLOR_PAIR(STRING_C));

  mvprintw(cy + 8, cx - 16, "| LEVEL:");
  sprintf(l, "%d", level);
  attrset(COLOR_PAIR(STRONG_C));
  mvprintw(cy + 9, cx - 14, l);
}

void drawNext(int cx, int cy, TARGET *np) {
  attrset(COLOR_PAIR(STRING_C));
  mvprintw(cy, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "| NEXT:");

  attrset(COLOR_PAIR(np->type.color));

  for (int i = 0; i < 4; i++) {
    mvprintw(cy + 3 + np->type.p[i].y, cx + (FIELD_WIDTH * WIDTH_RATIO) + 6 + np->type.p[i].x * WIDTH_RATIO, "  ");
  }
}

void drawSkip(int cx, int cy, int skipCount) {
  char skipLeft[16];

  attrset(COLOR_PAIR(STRING_C));

  sprintf(skipLeft, "%d", skipCount);
  strcat(skipLeft, " / 5");

  mvprintw(cy + 6, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "| SKIP:");
  attrset(COLOR_PAIR(STRONG_C));
  mvprintw(cy + 7, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, skipLeft);
}

void drawInst(int cx, int cy, bool rflag) {
  attrset(COLOR_PAIR(STRING_C));

  if (rflag) {
    mvprintw(cy + 9, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "| INSTRUCTION:");
    mvprintw(cy + 11, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "Z    : ROTATE RIGHT");
    mvprintw(cy + 12, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "X    : ROTATE LEFT");
    mvprintw(cy + 13, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "C    : SKIP");
    mvprintw(cy + 14, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "RIGHT: MOVE LEFT");
    mvprintw(cy + 15, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "LEFT : MOVE RIGHT");
    mvprintw(cy + 16, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "UP   : SOFT DROP");
    mvprintw(cy + 17, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "Q    : EXIT");
    } else {
    mvprintw(cy + 9, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "| INSTRUCTION:");
    mvprintw(cy + 11, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "X    : ROTATE RIGHT");
    mvprintw(cy + 12, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "Z    : ROTATE LEFT");
    mvprintw(cy + 13, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "C    : SKIP");
    mvprintw(cy + 14, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "LEFT : MOVE LEFT");
    mvprintw(cy + 15, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "RIGHT: MOVE RIGHT");
    mvprintw(cy + 16, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "DOWN : SOFT DROP");
    mvprintw(cy + 17, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "Q    : EXIT");
  }
}

void drawTitle(int cx, int cy) {
  attrset(COLOR_PAIR(STRING_C));
  mvprintw(cy, cx + 3, "T U R T L E");
  mvprintw(cy + 6, cx + 4, "S : START");
  mvprintw(cy + 7, cx + 4, "Q : EXIT");
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

void drawField(int cx, int cy, int **ap) {
  for (int y = 0; y < FIELD_HEIGHT; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      int colorCode;

      if (ap[y][x] > 10) {
        colorCode = ap[y][x] - 10;
      } else {
        colorCode = ap[y][x];
      }
      
      attrset(COLOR_PAIR(colorCode));
      mvprintw(cy - 2 + y * HEIGHT_RATIO, cx + x * WIDTH_RATIO, "  ");
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

void updateBlock(int state, int **ap) {
  int cx = target.p.x; int cy = target.p.y;
  BLOCK block = target.type;

  for (int i = 0; i < 4; i++) {
    int nx = cx + block.p[i].x;
    int ny = cy + block.p[i].y;

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
