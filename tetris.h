#define FIELD_WIDTH   10
#define FIELD_HEIGHT  20
#define FIELD_HEIGHT_MARGIN 4

#define WIDTH_RATIO 2
#define HEIGHT_RATIO 1

#define MARGIN -1
#define VOID 8
#define STRING_C 9
#define BORDER_C 10
#define STRONG_C 20

typedef struct {
  int x;
  int y;
} POSITION;

typedef struct {
  int color;
  POSITION p[4];
} BLOCK;

typedef struct {
  BLOCK type;
  POSITION p;
} TARGET;

// init.c
void initWindow();
void initColors();

// random.c
void initRandomSeed();
void shuffleBlocklist();
int selectRandomBlock();

void drawGameWindow(int cx, int cy, int maxScore, TARGET *next, time_t timeStart);
int loadHighestScore();
void updateHighestScore(int currentScore);
void makeField();
void drawSkip(int cx, int cy);
void drawElapsedTime(int cx, int cy, time_t timeStart);
void drawNext(int cx, int cy, TARGET *np);
void drawInst(int cx, int cy);
void drawScore(int cx, int cy, int maxScore);
void drawLineScore(int cx, int cy);
void drawLevel(int cx, int cy);
void drawGameover(int cx, int cy);
void drawField(int cx, int cy);
void refreshField();
BLOCK rotateBlockRight(TARGET *tp);
BLOCK rotateBlockLeft(TARGET *tp);
void setBlock(TARGET *tp);
void updateBlock(int state);
bool canMove(int dx, int dy, TARGET *tp);
bool canRotateRight(TARGET *tp);
bool canRotateLeft(TARGET *tp);
bool changeBlockState(TARGET *tp);
void searchAlign();
void deleteAlign(int dy);
void moveUP(TARGET *tp);
void moveDOWN(TARGET *tp);
void moveRIGHT(TARGET *tp);
void moveLEFT(TARGET *tp);
bool checkGameover();