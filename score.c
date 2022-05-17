#include <stdio.h>
#include <stdlib.h>

char SCORE_FILE[] = "score.txt";
int LINE_MAX = 255;

int loadHighestScore() {
  char lastScore[LINE_MAX];
  int maxScore = 0;

  FILE *fp = fopen(SCORE_FILE, "r");
  if (fp == NULL) return 0;
  if (fgets(lastScore, LINE_MAX, fp) != NULL) {
    maxScore = atoi(lastScore);
  }
  fclose(fp);
  return maxScore;
}

void updateHighestScore(int currentScore) {
  if (loadHighestScore() < currentScore) {
    FILE *fp = fopen(SCORE_FILE, "w");
    if (fp == NULL) return;
    fprintf(fp, "%d\n", currentScore);
    fclose(fp);
  }
}