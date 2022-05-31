#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include <ncurses.h>

#include "utils.h"

bool checkWindowSize(int x, int y) {
    if (x >= 60 && y >= 30) return true;
    return false;
}

void timeToStr(char *strTime, time_t timeStart) {
    char hour[3], min[3], sec[3];

    sprintf(hour, "%02d", (((int) difftime(time(NULL), timeStart)) / 3600) % 3600);
    sprintf(min, "%02d", (((int) difftime(time(NULL), timeStart)) / 60) % 60);
    sprintf(sec, "%02d", (((int) difftime(time(NULL), timeStart)) % 60));
    strcpy(strTime, hour);
    strcat(strTime, ":"); strcat(strTime, min);
    strcat(strTime, ":"); strcat(strTime, sec);
}

void initColorHex(int n, const char *cc) {
    if (*cc++ != '#') return;
    char sr[3], sg[3], sb[3];
    int r, g, b;
    strncpy(sr, cc, 2); sr[2] = '\0';
    r = (int) strtol(sr, NULL, 16);
    strncpy(sg, cc+2, 2); sg[2] = '\0';
    g = (int) strtol(sg, NULL, 16);
    strncpy(sb, cc+4, 2); sb[2] = '\0';
    b = (int) strtol(sb, NULL, 16);
    r = (int) (r / (double) 255 * 1000);
    g = (int) (g / (double) 255 * 1000);
    b = (int) (b / (double) 255 * 1000);
    init_color(n, r, g, b);
}
