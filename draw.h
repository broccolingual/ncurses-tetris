void drawSkip(int cx, int cy, int skipCount);
void drawElapsedTime(int cx, int cy, time_t timeStart);
void drawNext(int cx, int cy, TARGET *np);
void drawInst(int cx, int cy, bool rflag);
void drawScore(int cx, int cy, int score, int maxScore);
void drawLineScore(int cx, int cy, int lineScore);
void drawLevel(int cx, int cy, int level);
void drawTitle(int cx, int cy);
void drawGameover(int cx, int cy, int score);
void drawField(int cx, int cy, int **ap);
void drawGameWindow(int cx, int cy, int **ap, int maxScore, TARGET *next, time_t timeStart, bool rflag, int score, int level, int lineScore, int skipCount);
