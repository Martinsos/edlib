#ifndef SIMPLE_EDIT_DISTANCE_H
#define SIMPLE_EDIT_DISTANCE_H

#include <cstdio>

#ifdef __cplusplus 
extern "C" {
#endif


int min(int x, int y) {
    return x < y ? x : y;
}

int min3(int x, int y, int z) {
    return min(x, min(y, z));
}

int calcEditDistanceSimple(const unsigned char* query, int queryLength,
                           const unsigned char* target, int targetLength,
                           int alphabetLength, int mode, int* score, int* position) {
    int* C = new int[queryLength];
    int* newC = new int[queryLength];

    int bestScore = -1;
    int pos = -1;

    // set first column (column zero)
    for (int i = 0; i < queryLength; i++)
        C[i] = i+1;
    /*
    for (int i = 0; i < queryLength; i++)
        printf("%3d ", C[i]);
    printf("\n");
    */
    for (int c = 0; c < targetLength; c++) { // for each column
        newC[0] = min3((mode == MYERS_MODE_HW ? 0 : c + 1) + 1, // up
                       (mode == MYERS_MODE_HW ? 0 : c) + (target[c] == query[0] ? 0 : 1), // up left
                       C[0] + 1); // left
        for (int r = 1; r < queryLength; r++) {
            newC[r] = min3(newC[r-1] + 1, // up
                           C[r-1] + (target[c] == query[r] ? 0 : 1), // up left
                           C[r] + 1); // left
        }
        
        /*  for (int i = 0; i < queryLength; i++)
            printf("%3d ", newC[i]);
            printf("\n");*/

        if (mode == MYERS_MODE_HW || mode == MYERS_MODE_SHW
            || c == targetLength - 1) // For NW check only last column
            if (bestScore == -1 || newC[queryLength-1] < bestScore) {
                bestScore = newC[queryLength-1];
                pos = c;
            }
        
        int *tmp = C;
        C = newC;
        newC = tmp;
    }

    delete[] C;
    delete[] newC;

    *score = bestScore;
    *position = pos;

    return MYERS_STATUS_OK;
}



#ifdef __cplusplus 
}
#endif

#endif // SIMPLE_EDIT_DISTANCE_H
