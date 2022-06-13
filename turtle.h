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

void gameLoop(int cx, int cy, bool rflag, bool *isGameover, int *score);
bool displayTitle(int cx, int cy);
void displayGameover(int cx, int cy, int score);
bool getReverseOption(int argc, char *argv[]);
void getWindowCenter(int *cx, int *cy, int w, int h);
void initField(int **ap);
void refreshField(int **ap);
bool setBlock(TARGET *tp);
void updateBlock(TARGET *tp, int **ap, int state);
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
