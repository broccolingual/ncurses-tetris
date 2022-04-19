#define FIELD_WIDTH   10
#define FIELD_HEIGHT  20
#define FIELD_HEIGHT_MARGIN 4
#define BLOCK_MAX 7

#define WIDTH_RATIO 2
#define HEIGHT_RATIO 1

#define MARGIN -1
#define BLOCK_NULL 0
#define BLOCK_I 1
#define BLOCK_O 2
#define BLOCK_S 3
#define BLOCK_Z 4
#define BLOCK_J 5
#define BLOCK_L 6
#define BLOCK_T 7
#define VOID 8
#define STRING_C 9
#define BORDER_C 10

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

void setWindow();
void setColors();
void drawGameWindow(int cx, int cy, int maxScore, TARGET *next);
int loadHighestScore();
void updateHighestScore();
void makeField();
void drawNext(int cx, int cy, TARGET *np);
void drawInst(int cx, int cy);
void drawScore(int cx, int cy, int maxScore);
void drawGameover(int cx, int cy);
void drawField(int cx, int cy);
void refreshField();
BLOCK rotateBlock(TARGET *tp);
BLOCK selectRandomBlock();
void setBlock(TARGET *tp);
void updateBlock(int state);
bool canMove(int dx, int dy, TARGET *tp);
bool canRotate(TARGET *tp);
bool changeBlockState(TARGET *tp);
void searchAlign();
void deleteAlign(int dy);
void moveUP(TARGET *tp);
void moveDOWN(TARGET *tp);
void moveRIGHT(TARGET *tp);
void moveLEFT(TARGET *tp);
bool checkGameover();