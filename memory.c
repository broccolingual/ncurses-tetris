#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "memory.h"

int **mallocFieldAllocation(int w, int h) {
    int **ap;
    ap = malloc(sizeof(int *) * h);
    if (ap == NULL) return NULL;
    for (int i = 0; i < h; i++) {
        ap[i] = malloc(sizeof(int) * w);
        if (ap[i] == NULL) return NULL;
    }
    return ap;
}

void freeFieldAllocation(int **ap, int h) {
    for (int i = 0; i < h; i++) {
        free(ap[i]);
    }
    free(ap);
}