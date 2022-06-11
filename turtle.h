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

bool titleLoop(int cx, int cy);
bool getReverseOption(int argc, char *argv[]);
void getWindowCenter(int *cx, int *cy, int w, int h);
void initField(int **ap);
void refreshField(int **ap);
bool setBlock(TARGET *tp);
void updateBlock(int state, int **ap);
void useSkip(TARGET *cp, TARGET *np, int *skipCount);
bool canMove(int dx, int dy, TARGET *tp, int **ap);
void moveDOWN(TARGET *tp, int **ap);
void moveRIGHT(TARGET *tp, int **ap);
void moveLEFT(TARGET *tp, int **ap);
bool canRotateRight(TARGET *tp, int **ap);
BLOCK rotateBlockRight(TARGET *tp, int **ap);
bool canRotateLeft(TARGET *tp, int **ap);
BLOCK rotateBlockLeft(TARGET *tp, int **ap);
bool changeBlockState(TARGET *tp, int **ap);
void searchAlign(int **ap, int *score, int *level, int *lineScore);
void deleteAlign(int dy, int **ap);
bool checkGameover(int **ap);

void drawSkip(int cx, int cy, int skipCount);
void drawElapsedTime(int cx, int cy, time_t timeStart);
void drawNext(int cx, int cy, TARGET *np);
void drawInst(int cx, int cy, bool rflag);
void drawScore(int cx, int cy, int score, int maxScore);
void drawLineScore(int cx, int cy, int lineScore);
void drawLevel(int cx, int cy, int level);
void drawTitle(int cx, int cy);
void drawGameover(int cx, int cy);
void drawField(int cx, int cy, int **ap);
void drawGameWindow(int cx, int cy, int **ap, int maxScore, TARGET *next, time_t timeStart, bool rflag, int score, int level, int lineScore, int skipCount);
