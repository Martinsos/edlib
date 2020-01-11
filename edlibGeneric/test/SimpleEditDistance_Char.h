#ifndef SIMPLE_EDIT_DISTANCE_H
#define SIMPLE_EDIT_DISTANCE_H

#include <algorithm>
#include <cstdio>
#include <vector>
#include "edlib.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif


int min(int x, int y) {
    return x < y ? x : y;
}

int min3(int x, int y, int z) {
    return min(x, min(y, z));
}

int calcEditDistanceSimple(const char* query, int queryLength,
                           const char* target, int targetLength,
                           const EdlibAlignMode mode, int* score,
                           int** positions_, int* numPositions_) {
    int bestScore = -1;
    vector<int> positions;

    // Handle as a special situation when one of the sequences has length 0.
    if (queryLength == 0 || targetLength == 0) {
        if (mode == EDLIB_MODE_NW) {
            *score = std::max(queryLength, targetLength);
            *positions_ = new int[1];
            *positions_[0] = targetLength - 1;
            *numPositions_ = 1;
        } else if (mode == EDLIB_MODE_SHW || mode == EDLIB_MODE_HW) {
            *score = queryLength;
            *positions_ = new int[1];
            *positions_[0] = -1;
            *numPositions_ = 1;
        } else {
            return EDLIB_STATUS_ERROR;
        }
        return EDLIB_STATUS_OK;
    }

    int* C = new int[queryLength];
    int* newC = new int[queryLength];

    // set first column (column zero)
    for (int i = 0; i < queryLength; i++) {
        C[i] = i + 1;
    }
    /*
    for (int i = 0; i < queryLength; i++)
        printf("%3d ", C[i]);
    printf("\n");
    */
    for (int c = 0; c < targetLength; c++) { // for each column
        newC[0] = min3((mode == EDLIB_MODE_HW ? 0 : c + 1) + 1, // up
                       (mode == EDLIB_MODE_HW ? 0 : c)
                       + (target[c] == query[0] ? 0 : 1), // up left
                       C[0] + 1); // left
        for (int r = 1; r < queryLength; r++) {
            newC[r] = min3(newC[r-1] + 1, // up
                           C[r-1] + (target[c] == query[r] ? 0 : 1), // up left
                           C[r] + 1); // left
        }

        /*  for (int i = 0; i < queryLength; i++)
            printf("%3d ", newC[i]);
            printf("\n");*/

        if (mode != EDLIB_MODE_NW || c == targetLength - 1) { // For NW check only last column
            int newScore = newC[queryLength - 1];
            if (bestScore == -1 || newScore <= bestScore) {
                if (newScore < bestScore) {
                    positions.clear();
                }
                bestScore = newScore;
                positions.push_back(c);
            }
        }

        int *tmp = C;
        C = newC;
        newC = tmp;
    }

    delete[] C;
    delete[] newC;

    *score = bestScore;
    if (positions.size() > 0) {
        *positions_ = new int[positions.size()];
        *numPositions_ = static_cast<int>(positions.size());
        copy(positions.begin(), positions.end(), *positions_);
    } else {
        *positions_ = NULL;
        *numPositions_ = 0;
    }

    return EDLIB_STATUS_OK;
}



#ifdef __cplusplus
}
#endif

#endif // SIMPLE_EDIT_DISTANCE_H
