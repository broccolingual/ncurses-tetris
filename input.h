bool inputSwitcher(int **FIELD, TARGET *target, TARGET *next, bool rflag, int *skipCount);
void useSkip(TARGET *cp, TARGET *np, int *skipCount);
bool canMove(int dx, int dy, TARGET *tp, int **ap);
void moveDOWN(TARGET *tp, int **ap);
void moveRIGHT(TARGET *tp, int **ap);
void moveLEFT(TARGET *tp, int **ap);
bool canRotateRight(TARGET *tp, int **ap);
BLOCK rotateBlockRight(TARGET *tp, int **ap);
bool canRotateLeft(TARGET *tp, int **ap);
BLOCK rotateBlockLeft(TARGET *tp, int **ap);