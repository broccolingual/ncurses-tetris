#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <ncurses.h>

#define FIELD_WIDTH   10
#define FIELD_HEIGHT  20
#define FIELD_HEIGHT_MARGIN 4
#define BLOCK_MAX 7
#define INTERVAL 0.5

#define WIDTH_RATIO 2
#define HEIGHT_RATIO 1

typedef struct {
  int x;
  int y;
} POSITION;

typedef struct {
  POSITION p[4];
} BLOCK;

typedef struct {
  BLOCK type;
  POSITION p;
} TARGET;

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

void makeField();
void drawGameover(int cx, int cy);
void drawField(int cx, int cy);
void refreshField();
BLOCK rotateBlock();
BLOCK selectRandomBlock();
void setCurrentBlock();
void updateBlock(int state);
bool canMove(int dx, int dy);
bool canRotate();
bool changeBlockState();
void searchAlign();
void deleteAlign(int dy);
void moveUP();
void moveDOWN();
void moveRIGHT();
void moveLEFT();
bool checkGameover();

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

      erase(); // 画面消去
      refreshField();
      updateBlock(2);
      drawField(cx, cy);
      refresh(); // 画面再描画
    }

    key = getch();
    if (key == 'q') break;
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
      setCurrentBlock(); // 新ブロック生成
      updateBlock(2);
      drawField(cx, cy);
    } else {
      refreshField();
      updateBlock(2);
      drawField(cx, cy);
    }

    refresh(); // 画面再描画
  }

  if (isGameover) {
    erase(); // 画面消去
    drawGameover(cx, cy);
    refresh(); // 画面再描画

    while (1) {
      key = getch();
      if (key == 'q') break;
    }
  }

	endwin(); // スクリーンの終了

  return 0;
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
  for (int y = FIELD_HEIGHT_MARGIN; y < FIELD_HEIGHT + FIELD_HEIGHT_MARGIN; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      if (FIELD[y][x] == 0) break;
      if (x == FIELD_WIDTH - 1) {
        deleteAlign(y);
      }
    }
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