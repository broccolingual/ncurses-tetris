#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <ncurses.h>

#include "turtle.h"
#include "block.h"
#include "field.h"
#include "utils.h"
#include "draw.h"

void drawGameWindow(int cx, int cy, int **ap, int maxScore, TARGET *np, time_t timeStart, bool rflag, int score, int level, int lineScore, int skipCount) {
  drawField(cx, cy, ap);
  drawScore(cx, cy, score, maxScore);
  drawInst(cx, cy, rflag);
  drawNext(cx, cy, np);
  drawElapsedTime(cx, cy, timeStart);
  drawSkip(cx, cy, skipCount);
  drawLineScore(cx, cy, lineScore);
  drawLevel(cx, cy, level);
}

void drawScore(int cx, int cy, int score, int maxScore) {
  char highestScore[256];
  char s[256]; 

  attrset(COLOR_PAIR(STRING_C));

  mvprintw(cy, cx - 16, "| HIGHEST:");
  sprintf(highestScore, "%d", maxScore);
  attrset(COLOR_PAIR(STRONG_C));
  mvprintw(cy + 1, cx - 14, highestScore);
  attrset(COLOR_PAIR(STRING_C));
  mvprintw(cy + 2, cx - 16, "| SCORE:");
  sprintf(s, "%d", score);
  attrset(COLOR_PAIR(STRONG_C));
  mvprintw(cy + 3, cx - 14, s);
}

void drawElapsedTime(int cx, int cy, time_t timeStart) {
  char strTime[9];
  timeToStr(strTime, timeStart);

  attrset(COLOR_PAIR(STRING_C));

  mvprintw(cy + 4, cx - 16, "| TIME:");
  attrset(COLOR_PAIR(STRONG_C));
  mvprintw(cy + 5, cx - 14, strTime);
}

void drawLineScore(int cx, int cy, int lineScore) {
  char ls[256]; 

  attrset(COLOR_PAIR(STRING_C));

  mvprintw(cy + 6, cx - 16, "| LINES:");
  sprintf(ls, "%d", lineScore);
  attrset(COLOR_PAIR(STRONG_C));
  mvprintw(cy + 7, cx - 14, ls);
}

void drawLevel(int cx, int cy, int level) {
  char l[256]; 

  attrset(COLOR_PAIR(STRING_C));

  mvprintw(cy + 8, cx - 16, "| LEVEL:");
  sprintf(l, "%d", level);
  attrset(COLOR_PAIR(STRONG_C));
  mvprintw(cy + 9, cx - 14, l);
}

void drawNext(int cx, int cy, TARGET *np) {
  attrset(COLOR_PAIR(STRING_C));
  mvprintw(cy, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "| NEXT:");

  attrset(COLOR_PAIR(np->type.color));

  for (int i = 0; i < 4; i++) {
    mvprintw(cy + 3 + np->type.p[i].y, cx + (FIELD_WIDTH * WIDTH_RATIO) + 6 + np->type.p[i].x * WIDTH_RATIO, "  ");
  }
}

void drawSkip(int cx, int cy, int skipCount) {
  char skipLeft[16];

  attrset(COLOR_PAIR(STRING_C));

  sprintf(skipLeft, "%d", skipCount);
  strcat(skipLeft, " / 5");

  mvprintw(cy + 6, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "| SKIP:");
  attrset(COLOR_PAIR(STRONG_C));
  mvprintw(cy + 7, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, skipLeft);
}

void drawInst(int cx, int cy, bool rflag) {
  attrset(COLOR_PAIR(STRING_C));

  if (rflag) {
    mvprintw(cy + 9, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "| INSTRUCTION:");
    mvprintw(cy + 11, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "Z    : ROTATE RIGHT");
    mvprintw(cy + 12, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "X    : ROTATE LEFT");
    mvprintw(cy + 13, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "C    : SKIP");
    mvprintw(cy + 14, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "RIGHT: MOVE LEFT");
    mvprintw(cy + 15, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "LEFT : MOVE RIGHT");
    mvprintw(cy + 16, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "UP   : SOFT DROP");
    mvprintw(cy + 17, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "Q    : EXIT");
    } else {
    mvprintw(cy + 9, cx + (FIELD_WIDTH * WIDTH_RATIO) + 2, "| INSTRUCTION:");
    mvprintw(cy + 11, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "X    : ROTATE RIGHT");
    mvprintw(cy + 12, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "Z    : ROTATE LEFT");
    mvprintw(cy + 13, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "C    : SKIP");
    mvprintw(cy + 14, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "LEFT : MOVE LEFT");
    mvprintw(cy + 15, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "RIGHT: MOVE RIGHT");
    mvprintw(cy + 16, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "DOWN : SOFT DROP");
    mvprintw(cy + 17, cx + (FIELD_WIDTH * WIDTH_RATIO) + 4, "Q    : EXIT");
  }
}

void drawTitle(int cx, int cy) {
  attrset(COLOR_PAIR(STRING_C));
  mvprintw(cy, cx + 3, "T U R T L E");
  mvprintw(cy + 6, cx + 4, "S : START");
  mvprintw(cy + 7, cx + 4, "Q : EXIT");
}

void drawGameover(int cx, int cy, int score) {
  char base[64] = "SCORE: ";
  char s[32];

  attrset(COLOR_PAIR(STRING_C));
  mvprintw(cy, cx, "G A M E     O V E R ");
  sprintf(s, "%d", score);
  strcat(base, s);
  attrset(COLOR_PAIR(STRONG_C));
  mvprintw(cy + 2, cx + 5, base);
  attrset(COLOR_PAIR(STRING_C));
  mvprintw(cy + 6, cx + 6, "Q : EXIT");
}

void drawField(int cx, int cy, int **ap) {
  for (int y = 0; y < FIELD_HEIGHT; y++) {
    for (int x = 0; x < FIELD_WIDTH; x++) {
      int colorCode;

      if (ap[y][x] > 10) {
        colorCode = ap[y][x] - 10;
      } else {
        colorCode = ap[y][x];
      }
      
      attrset(COLOR_PAIR(colorCode));
      mvprintw(cy - 2 + y * HEIGHT_RATIO, cx + x * WIDTH_RATIO, "  ");
    }
  }
}
