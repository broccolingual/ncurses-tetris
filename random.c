#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "block.h"
#include "random.h"

int BLOCK_LIST[BLOCK_MAX] = {BLOCK_I, BLOCK_O, BLOCK_S, BLOCK_Z, BLOCK_J, BLOCK_L, BLOCK_T};
int RANDOM_BLOCK_INDEX = 0;

void generateRandomSeed() {
  srand((unsigned int)time(NULL));
}

void shuffleBlockList() {
  for(int i = 0; i < BLOCK_MAX; i++) {
    int j = rand() % BLOCK_MAX;
    int t = BLOCK_LIST[i];
    BLOCK_LIST[i] = BLOCK_LIST[j];
    BLOCK_LIST[j] = t;
  }
}

int selectRandomBlock() {
  if (RANDOM_BLOCK_INDEX < BLOCK_MAX) {
    return BLOCK_LIST[RANDOM_BLOCK_INDEX++];
  } else {
    shuffleBlockList();
    RANDOM_BLOCK_INDEX = 0;
    return BLOCK_LIST[RANDOM_BLOCK_INDEX++];
  }
}