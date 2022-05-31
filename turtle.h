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
void generateRandomSeed();
void shuffleBlockList();
int selectRandomBlock();

// score.c
int loadHighestScore();
void updateHighestScore(int currentScore);

// utils.c
bool checkWindowSize(int x, int y);
void timeToStr(char *strTime, time_t timeStart);
void initColorHex(int n, const char *cc);

// memory.c
int **mallocFieldAllocation(int w, int h);
void freeFieldAllocation(int **ap, int h);

// turtle.c
void drawGameWindow(int cx, int cy, int **ap, int maxScore, TARGET *next, time_t timeStart, bool rflag);
void initField(int **ap);
void drawSkip(int cx, int cy);
void drawElapsedTime(int cx, int cy, time_t timeStart);
void drawNext(int cx, int cy, TARGET *np);
void drawInst(int cx, int cy, bool rflag);
void drawScore(int cx, int cy, int maxScore);
void drawLineScore(int cx, int cy);
void drawLevel(int cx, int cy);
void drawTitle(int cx, int cy);
void drawGameover(int cx, int cy);
void drawField(int cx, int cy, int **ap);
void refreshField(int **ap);
BLOCK rotateBlockRight(TARGET *tp, int **ap);
BLOCK rotateBlockLeft(TARGET *tp, int **ap);
bool setBlock(TARGET *tp);
void updateBlock(int state, int **ap);
bool canMove(int dx, int dy, TARGET *tp, int **ap);
bool canRotateRight(TARGET *tp, int **ap);
bool canRotateLeft(TARGET *tp, int **ap);
bool changeBlockState(TARGET *tp, int **ap);
void searchAlign(int **ap);
void deleteAlign(int dy, int **ap);
void moveDOWN(TARGET *tp, int **ap);
void moveRIGHT(TARGET *tp, int **ap);
void moveLEFT(TARGET *tp, int **ap);
bool checkGameover(int **ap);