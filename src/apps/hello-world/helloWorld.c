#include <cstdio>
#include <cstdlib>

#include "edlib.h"

int main() {

    // We are goint to map letters in following way:
    // D = 0, E = 1, H = 2, L = 3, O = 4, R = 5, W = 6.
    int alphabetLength = 9, helloSeqLength = 5, worldSeqLength = 5;
    unsigned char helloSeq[5] = {2,1,3,3,4};
    unsigned char worldSeq[5] = {6,4,5,3,0};

    // Result is going to be stored in variables below.
    int editDistance, numLocations, alignmentLength;
    int* startLocations, * endLocations;
    unsigned char* alignment;

    edlibCalcEditDistance(helloSeq, helloSeqLength, worldSeq, worldSeqLength, alphabetLength,
                          -1, EDLIB_MODE_NW, false, false,
                          &editDistance, &endLocations, &startLocations, &numLocations,
                          &alignment, &alignmentLength);

    printf("edit_distance('hello', 'world') = %d\n", editDistance);
    if (endLocations) free(endLocations);
}
