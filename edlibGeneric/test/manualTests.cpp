//
// Created by mobin on 1/8/20.
// Testing generic edlib
//
#include <iostream>
#include <stdio.h>
#include "edlibGeneric.h"
#include <iomanip>

using namespace edlibGeneric;

template <class AlphabetType>
void printAlignment(const AlphabetType* query, const AlphabetType* target,
                    const unsigned char* alignment, const int alignmentLength,
                    const int position, const EdlibAlignMode modeCode, const int space);

int main() {

    const int queryLength = 10;
    const int targetLength = 10;

    //space between symbols in printAlignment
    //increase when symbols are long (ex. 3 digit symbols need at least 4 space)
    const int space = 4;

    uint16_t query[queryLength] = {402, 403, 405, 404, 403, 405, 8, 9, 4, 7};
    uint16_t target[targetLength] = {1, 402, 403, 404, 403, 405, 4, 9, 2, 3};
    const EdlibAlignMode testMode = EDLIB_MODE_NW;

    //change alphabet type and index type based on your inputs
    EdlibEqualityPair<uint16_t> additionalEqualities[4] = {{8, 1}, {1,14}, {14, 4}, {14, 2}};
    EdlibAlignResult result =
            edlibAlign<uint16_t , uint8_t >(query, queryLength, target, targetLength,
                                            edlibNewAlignConfig<uint16_t>(-1, testMode, EDLIB_TASK_PATH, additionalEqualities, 4));
    printf("edit_distance = %d\n", result.editDistance);

    if(result.alignment != NULL) {
        printAlignment<uint16_t>(query, target, result.alignment, result.alignmentLength,
                                 *(result.endLocations), testMode, space);
    }
    edlibFreeAlignResult(result);
}

template <class AlphabetType>
void printAlignment(const AlphabetType* query, const AlphabetType* target,
                    const unsigned char* alignment, const int alignmentLength,
                    const int position, const EdlibAlignMode modeCode, const int space) {
    int tIdx = -1;
    int qIdx = -1;
    if (modeCode == EDLIB_MODE_HW) {
        tIdx = position;
        for (int i = 0; i < alignmentLength; i++) {
            if (alignment[i] != EDLIB_EDOP_INSERT)
                tIdx--;
        }
    }
    for (int start = 0; start < alignmentLength; start += 50) {
        // target
        printf("T: ");
        int startTIdx = -1;
        for (int j = start; j < start + 50 && j < alignmentLength; j++) {
            if (alignment[j] == EDLIB_EDOP_INSERT)
                cout << setw(space) << "-" ;
            else
                cout << setw(space) << target[++tIdx] ;
            if (j == start)
                startTIdx = tIdx;
        }
        printf(" (%d - %d)\n", max(startTIdx, 0), tIdx);

        // match / mismatch
        cout << "   ";
        for (int j = start; j < start + 50 && j < alignmentLength; j++) {
            cout << setw(space) << (alignment[j] == EDLIB_EDOP_MATCH ? "|" : " ");
        }
        printf("\n");

        // query
        printf("Q: ");
        int startQIdx = qIdx;
        for (int j = start; j < start + 50 && j < alignmentLength; j++) {
            if (alignment[j] == EDLIB_EDOP_DELETE)
                cout << setw(space) << "-";
            else
                cout << setw(space) << query[++qIdx];
            if (j == start)
                startQIdx = qIdx;
        }
        printf(" (%d - %d)\n\n", max(startQIdx, 0), qIdx);
    }
}

