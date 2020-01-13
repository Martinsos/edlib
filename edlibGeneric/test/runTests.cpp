//
// Created by mobin on 12/20/19.
//

#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <climits>

#include "../include/edlibGeneric.h"
#include "SimpleEditDistance_Generic.h"

using namespace std;


bool runRandomTests(int numTests, EdlibAlignMode mode, bool findAlignment);
bool runTests();

int calcEditDistanceSimple(const AlphabetType* query, int queryLength,
                           const AlphabetType* target, int targetLength,
                           EdlibAlignMode mode, int* score,
                           int** positions, int* numPositions);

bool checkAlignment(const AlphabetType * query, int queryLength,
                    const AlphabetType * target,
                    int score, int pos, EdlibAlignMode mode,
                    unsigned char* alignment, int alignmentLength);

int getAlignmentStart(const unsigned char* alignment, int alignmentLength,
                      int endLocation);


int main(int argc, char* argv[]) {
    // This program has optional first parameter, which is number of random tests to run
    // per each algorithm. Default is 100.
    int numRandomTests = 10;
    if (argc > 1) {
        numRandomTests = static_cast<int>(strtol(argv[1], NULL, 10));
    }

    srand(42);
    bool allTestsPassed = true;

    printf("Testing HW with alignment...\n");
    allTestsPassed &= runRandomTests(numRandomTests, EDLIB_MODE_HW, true);
    printf("\n");

    printf("Testing HW...\n");
    allTestsPassed &= runRandomTests(numRandomTests, EDLIB_MODE_HW, false);
    printf("\n");

    printf("Testing NW with alignment...\n");
    allTestsPassed &= runRandomTests(numRandomTests, EDLIB_MODE_NW, true);
    printf("\n");

    printf("Testing NW...\n");
    allTestsPassed &= runRandomTests(numRandomTests, EDLIB_MODE_NW, false);
    printf("\n");

    printf("Testing SHW with alignment...\n");
    allTestsPassed &= runRandomTests(numRandomTests, EDLIB_MODE_SHW, true);
    printf("\n");

    printf("Testing SHW...\n");
    allTestsPassed &= runRandomTests(numRandomTests, EDLIB_MODE_SHW, false);
    printf("\n");

    return !allTestsPassed;
}


void fillRandomly(AlphabetType * seq, int seqLength, int alphabetLength) {
    for (int i = 0; i < seqLength; i++)
        seq[i] = static_cast<AlphabetType>(rand()) % alphabetLength;
}

// Returns true if all tests passed, false otherwise.
bool runRandomTests(int numTests, EdlibAlignMode mode, bool findAlignment) {
    int alphabetLength = 200;
    int numTestsFailed = 0;
    clock_t start;
    double timeEdlib = 0;
    double timeSimple = 0;

    for (int i = 0; i < numTests; i++) {
        bool failed = false;
        int queryLength = 10000 + rand() % 300;
        int targetLength = 10000 + rand() % 1000;
        AlphabetType * query = static_cast<AlphabetType *>(malloc(sizeof(AlphabetType) * queryLength));
        AlphabetType * target = static_cast<AlphabetType *>(malloc(sizeof(AlphabetType) * targetLength));
        fillRandomly(query, queryLength, alphabetLength);
        fillRandomly(target, targetLength, alphabetLength);

        // // Print query
        // printf("Query: ");
        // for (int i = 0; i < queryLength; i++)
        //     printf("%d ", query[i]);
        // printf("\n");
        // // Print target
        // printf("Target: ");
        // for (int i = 0; i < targetLength; i++)
        //     printf("%d ", target[i]);
        // printf("\n");

        start = clock();
        EdlibAlignResult result = edlibAlign<AlphabetType , IdxType >(
                query, queryLength, target, targetLength,
                        edlibNewAlignConfig(-1, mode, findAlignment ? EDLIB_TASK_PATH : EDLIB_TASK_DISTANCE, NULL, 0));
        timeEdlib += clock() - start;
        if (result.alignment) {
            if (!checkAlignment(query, queryLength, target,
                                result.editDistance, result.endLocations[0], mode,
                                result.alignment, result.alignmentLength)) {
                failed = true;
                printf("Alignment is not correct\n");
            }
            int alignmentStart = getAlignmentStart(result.alignment, result.alignmentLength,
                                                   result.endLocations[0]);
            if (result.startLocations[0] != alignmentStart) {
                failed = true;
                printf("Start location (%d) is not consistent with alignment start (%d)\n",
                       result.startLocations[0], alignmentStart);
            }
        }

        start = clock();
        int score2; int numLocations2;
        int* endLocations2;
        calcEditDistanceSimple(query, queryLength, target, targetLength,
                               mode, &score2, &endLocations2, &numLocations2);
        timeSimple += clock() - start;

        // Compare results
        if (result.editDistance != score2) {
            failed = true;
            printf("Scores are different! Expected %d, got %d)\n", score2, result.editDistance);
        } else if (result.editDistance == -1 && !(result.endLocations == NULL)) {
            failed = true;
            printf("Score was not found but endLocations is not NULL!\n");
        } else if (result.numLocations != numLocations2) {
            failed = true;
            printf("Number of endLocations returned is not equal! Expected %d, got %d\n",
                   numLocations2, result.numLocations);
        } else {
            for (int j = 0; j < result.numLocations; j++) {
                if (result.endLocations[j] != endLocations2[j]) {
                    failed = true;
                    printf("EndLocations at %d are not equal! Expected %d, got %d\n",
                           j, endLocations2[j], result.endLocations[j]);
                    break;
                }
            }
        }

        edlibFreeAlignResult(result);
        if (endLocations2) delete [] endLocations2;

        for (int k = max(score2 - 1, 0); k <= score2 + 1; k++) {
            int scoreExpected = score2 > k ? -1 : score2;
            EdlibAlignResult result3 = edlibAlign<AlphabetType , IdxType >(
                    query, queryLength, target, targetLength,
                            edlibNewAlignConfig(k, mode, findAlignment ? EDLIB_TASK_PATH : EDLIB_TASK_DISTANCE, NULL, 0));
            if (result3.editDistance != scoreExpected) {
                failed = true;
                printf("For k = %d score was %d but it should have been %d\n",
                       k, result3.editDistance, scoreExpected);
            }
            if (result3.alignment) {
                if (!checkAlignment(query, queryLength, target,
                                    result3.editDistance, result3.endLocations[0],
                                    mode, result3.alignment, result3.alignmentLength)) {
                    failed = true;
                    printf("Alignment is not correct\n");
                }
                int alignmentStart = getAlignmentStart(result3.alignment, result3.alignmentLength,
                                                       result3.endLocations[0]);
                if (result3.startLocations[0] != alignmentStart) {
                    failed = true;
                    printf("Start location (%d) is not consistent with alignment start (%d)\n",
                           result3.startLocations[0], alignmentStart);
                }
            }
            edlibFreeAlignResult(result3);
        }

        if (failed)
            numTestsFailed++;

        free(query);
        free(target);
    }

    printf(mode == EDLIB_MODE_HW ? "HW: " : mode == EDLIB_MODE_SHW ? "SHW: " : "NW: ");
    printf(numTestsFailed == 0 ? "\x1B[32m" : "\x1B[31m");
    printf("%d/%d", numTests - numTestsFailed, numTests);
    printf("\x1B[0m");
    printf(" random tests passed!\n");
    double mTime = static_cast<double>(timeEdlib)/CLOCKS_PER_SEC;
    double sTime = static_cast<double>(timeSimple)/CLOCKS_PER_SEC;
    printf("Time Edlib: %lf\n", mTime);
    printf("Time Simple: %lf\n", sTime);
    printf("Times faster: %.2lf\n", sTime / mTime);
    return numTestsFailed == 0;
}

/**
 * Checks if alignment is correct.
 */
bool checkAlignment(const AlphabetType * query, int queryLength,
                    const AlphabetType * target,
                    int score, int pos, EdlibAlignMode mode,
                    unsigned char* alignment, int alignmentLength) {
    int alignScore = 0;
    int qIdx = queryLength - 1;
    int tIdx = pos;
    for (int i = alignmentLength - 1; i >= 0; i--) {
        if (alignment[i] == EDLIB_EDOP_MATCH) { // match
            if (query[qIdx] != target[tIdx]) {
                printf("Should be match but is a mismatch! (tIdx, qIdx, i): (%d, %d, %d)\n", tIdx, qIdx, i);
                return false;
            }
            qIdx--;
            tIdx--;
        } else if (alignment[i] == EDLIB_EDOP_MISMATCH) { // mismatch
            if (query[qIdx] == target[tIdx]) {
                printf("Should be mismatch but is a match! (tIdx, qIdx, i): (%d, %d, %d)\n", tIdx, qIdx, i);
                return false;
            }
            alignScore += 1;
            qIdx--;
            tIdx--;
        } else if (alignment[i] == EDLIB_EDOP_INSERT) {
            alignScore += 1;
            qIdx--;
        } else if (alignment[i] == EDLIB_EDOP_DELETE) {
            if (!(mode == EDLIB_MODE_HW && qIdx == -1))
                alignScore += 1;
            tIdx--;
        }
        if (tIdx < -1 || qIdx < -1) {
            printf("Alignment went outside of matrix! (tIdx, qIdx, i): (%d, %d, %d)\n", tIdx, qIdx, i);
            return false;
        }
    }
    if (qIdx != -1) {
        printf("Alignment did not reach end!\n");
        return false;
    }
    if (alignScore != score) {
        printf("Wrong score in alignment! %d should be %d\n", alignScore, score);
        return false;
    }
    if (alignmentLength > 0 && alignment[0] == EDLIB_EDOP_INSERT && tIdx >= 0) {
        printf("Alignment starts with insertion in target, while it could start with mismatch!\n");
        return false;
    }
    return true;
}

/**
 * @param alignment
 * @param alignmentLength
 * @param endLocation
 * @return Return start location of alignment in target, if there is none return -1.
 */
int getAlignmentStart(const unsigned char* alignment, int alignmentLength,
                      int endLocation) {
    int startLocation = endLocation + 1;
    for (int i = 0; i < alignmentLength; i++) {
        if (alignment[i] != EDLIB_EDOP_INSERT) {
            startLocation--;
        }
    }
    if (startLocation > endLocation) {
        return -1;
    }
    return startLocation;
}

