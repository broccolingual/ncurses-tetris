#include <stdio.h>
#include <string.h>
#include <time.h>

void timeToStr(char *strTime, time_t timeStart) {
    char min[3]; char sec[3];

    sprintf(min, "%02d", ((int) difftime(time(NULL), timeStart)) / 60);
    sprintf(sec, "%02d", ((int) difftime(time(NULL), timeStart)) % 60);
    strcpy(strTime, min);
    strcat(strTime, ":");
    strcat(strTime, sec);
}