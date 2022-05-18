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
void timeToStr(char *strTime, time_t timeStart);
void initColorHex(int n, const char *cc);

void drawGameWindow(int cx, int cy, int maxScore, TARGET *next, time_t timeStart);
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
bool setBlock(TARGET *tp);
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