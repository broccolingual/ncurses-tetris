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
#include "input.h"

bool inputSwitcher(int **FIELD, TARGET *target, TARGET *next, bool rflag, int *skipCount) {
  switch (getch()) {
    case KEY_LEFT:
      if (rflag) {
        moveRIGHT(target, FIELD);
      } else {
        moveLEFT(target, FIELD);
      }
      break;
    case KEY_DOWN:
      if (rflag) {
        ;
      } else {
        moveDOWN(target, FIELD);
      }
      break;
    case KEY_RIGHT:
      if (rflag) {
        moveLEFT(target, FIELD);
      } else {
        moveRIGHT(target, FIELD);
      }
      break;
    case KEY_UP:
      if (rflag) {
        moveDOWN(target, FIELD);
      } else {
        ;
      }
      break;
    case 'x':
      if (rflag) {
        if (target->p.y > 1) {
          target->type = rotateBlockLeft(target, FIELD);
        }
      } else {
        if (target->p.y > 1) {
          target->type = rotateBlockRight(target, FIELD);
        }
      }
      break;
    case 'z':
      if (rflag) {
        if (target->p.y > 1) {
          target->type = rotateBlockRight(target, FIELD);
        }
      } else {
        if (target->p.y > 1) {
          target->type = rotateBlockLeft(target, FIELD);
        }
      }
      break;
    case 'c':
      useSkip(target, next, skipCount);
      break;
    case 'q':
      return true;
  }
  return false;
}

void useSkip(TARGET *cp, TARGET *np, int *skipCount) {
  if (*skipCount > 0) {
    *skipCount -= 1;
    *cp = *np;
    setBlock(np);
  }
}

bool canMove(int dx, int dy, TARGET *tp, int **ap) {
  for (int i = 0; i < 4; i++) {
    int nx = tp->p.x + tp->type.p[i].x + dx;
    int ny = tp->p.y + tp->type.p[i].y + dy;

    if (!((0 <= nx && nx < FIELD_WIDTH) && (ny < FIELD_HEIGHT) && !(ap[ny][nx] > 10))) {
      return false;
    }
  }
  return true;
}

void moveDOWN(TARGET *tp, int **ap) {
  if (canMove(0, 1, tp, ap)) tp->p.y++;
}

void moveRIGHT(TARGET *tp, int **ap) {
  if (canMove(1, 0, tp, ap)) tp->p.x++;
}

void moveLEFT(TARGET *tp, int **ap) {
  if (canMove(-1, 0, tp, ap)) tp->p.x--;
}

bool canRotateRight(TARGET *tp, int **ap) {
  for (int i = 0; i < 4; i++) {
    int nx = tp->p.x - tp->type.p[i].y;
    int ny = tp->p.y + tp->type.p[i].x;

    if (!((0 <= nx && nx < FIELD_WIDTH) && (ny < FIELD_HEIGHT) && !(ap[ny][nx] > 10))) {
      return false;
    }
  }
  return true;
}

bool canRotateLeft(TARGET *tp, int **ap) {
  for (int i = 0; i < 4; i++) {
    int nx = tp->p.x + tp->type.p[i].y;
    int ny = tp->p.y - tp->type.p[i].x;

    if (!((0 <= nx && nx < FIELD_WIDTH) && (ny < FIELD_HEIGHT) && !(ap[ny][nx] > 10))) {
      return false;
    }
  }
  return true;
}

BLOCK rotateBlockRight(TARGET *tp, int **ap) {
  BLOCK after = tp->type;

  if (canRotateRight(tp, ap)) {
    for (int i = 0; i < 4; i++) {
      int bx = tp->type.p[i].x;
      int by = tp->type.p[i].y;

      after.p[i].x = -by;
      after.p[i].y = bx;
    }
    return after;
  }
  return tp->type;
}

BLOCK rotateBlockLeft(TARGET *tp, int **ap) {
  BLOCK after = tp->type;

  if (canRotateLeft(tp, ap)) {
    for (int i = 0; i < 4; i++) {
      int bx = tp->type.p[i].x;
      int by = tp->type.p[i].y;

      after.p[i].x = by;
      after.p[i].y = -bx;
    }
    return after;
  }
  return tp->type;
}
