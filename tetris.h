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

void updateHighestScore();
void makeField();
void drawInst(int cx, int cy);
void drawScore(int cx, int cy);
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