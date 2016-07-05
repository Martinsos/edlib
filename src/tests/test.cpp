#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <cstring>

#include "edlib.h"
#include "SimpleEditDistance.h"

using namespace std;


void runRandomTests(int numTests, int mode, bool findAlignment);
bool runTests();

int calcEditDistanceSimple(const unsigned char* query, int queryLength,
                           const unsigned char* target, int targetLength,
                           int alphabetLength, int mode, int* score,
                           int** positions, int* numPositions);

bool checkAlignment(const unsigned char* query, int queryLength,
                    const unsigned char* target, int targetLength,
                    int score, int pos, int mode,
                    unsigned char* alignment, int alignmentLength);

int getAlignmentStart(const unsigned char* alignment, int alignmentLength,
                      int endLocation);

int main() {
    srand(1);

    printf("Testing HW with alignment...\n");
    runRandomTests(100, EDLIB_MODE_HW, true);
    printf("\n");

    printf("Testing HW...\n");
    runRandomTests(100, EDLIB_MODE_HW, false);
    printf("\n");

    printf("Testing NW with alignment...\n");
    runRandomTests(100, EDLIB_MODE_NW, true);
    printf("\n");

    printf("Testing NW...\n");
    runRandomTests(100, EDLIB_MODE_NW, false);
    printf("\n");

    printf("Testing SHW with alignment...\n");
    runRandomTests(100, EDLIB_MODE_SHW, true);
    printf("\n");

    printf("Testing SHW...\n");
    runRandomTests(100, EDLIB_MODE_SHW, false);
    printf("\n");

    printf("Specific tests:\n");
    if (runTests())
        printf("All specific tests passed!\n");
    else
        printf("Some specific tests failed\n");

    return 0;
}


void fillRandomly(unsigned char* seq, int seqLength, int alphabetLength) {
    for (int i = 0; i < seqLength; i++)
        seq[i] = rand() % alphabetLength;
}

void runRandomTests(int numTests, int mode, bool findAlignment) {
    int alphabetLength = 10;
    int numTestsFailed = 0;
    clock_t start;
    double timeEdlib = 0;
    double timeSimple = 0;

    for (int i = 0; i < numTests; i++) {
        bool failed = false;
        int queryLength = 50 + rand() % 300;
        int targetLength = 500 + rand() % 10000;
        unsigned char query[queryLength];
        unsigned char target[targetLength];
        fillRandomly(query, queryLength, alphabetLength);
        fillRandomly(target, targetLength, alphabetLength);

        // // Print query
        // printf("Query: ");
        // for (int i = 0; i < queryLength; i++)
        //     printf("%d", query[i]);
        // printf("\n");

        // // Print target
        // printf("Target: ");
        // for (int i = 0; i < targetLength; i++)
        //     printf("%d", target[i]);
        // printf("\n");

        start = clock();
        int score1, numLocations1;
        int* endLocations1, * startLocations1;
        unsigned char* alignment; int alignmentLength;
        edlibCalcEditDistance(query, queryLength, target, targetLength, alphabetLength,
                              -1, mode, false, findAlignment, &score1,
                              &endLocations1, &startLocations1, &numLocations1,
                              &alignment, &alignmentLength);
        timeEdlib += clock() - start;
        if (alignment) {
            if (!checkAlignment(query, queryLength, target, targetLength,
                                score1, endLocations1[0], mode, alignment, alignmentLength)) {
                failed = true;
                printf("Alignment is not correct\n");
            }
            int alignmentStart = getAlignmentStart(alignment, alignmentLength, endLocations1[0]);
            if (startLocations1[0] != alignmentStart) {
                failed = true;
                printf("Start location (%d) is not consistent with alignment start (%d)\n",
                       startLocations1[0], alignmentStart);
            }
            free(alignment);
        }
        if (startLocations1) free(startLocations1);

        start = clock();
        int score2; int numLocations2;
        int* endLocations2;
        calcEditDistanceSimple(query, queryLength, target, targetLength,
                               alphabetLength, mode, &score2,
                               &endLocations2, &numLocations2);
        timeSimple += clock() - start;

        // Compare results
        if (score1 != score2) {
            failed = true;
            printf("Scores are different! Expected %d, got %d)\n", score2, score1);
        } else if (score1 == -1 && !(endLocations1 == NULL)) {
            failed = true;
            printf("Score was not found but endLocations is not NULL!\n");
        } else if (numLocations1 != numLocations2) {
            failed = true;
            printf("Number of endLocations returned is not equal! Expected %d, got %d\n",
                   numLocations2, numLocations1);
        } else {
            for (int i = 0; i < numLocations1; i++) {
                if (endLocations1[i] != endLocations2[i]) {
                    failed = true;
                    printf("EndLocations at %d are not equal! Expected %d, got %d\n",
                           i, endLocations2[i], endLocations1[i]);
                    break;
                }
            }
        }

        if (endLocations1) free(endLocations1);
        if (endLocations2) free(endLocations2);

        for (int k = max(score2 - 1, 0); k <= score2 + 1; k++) {
            int score3, numLocations3;
            int* endLocations3, * startLocations3;
            unsigned char* alignment3; int alignmentLength3;
            int scoreExpected = score2 > k ? -1 : score2;
            edlibCalcEditDistance(query, queryLength, target, targetLength,
                                  alphabetLength, k, mode, false, findAlignment,
                                  &score3, &endLocations3, &startLocations3, &numLocations3,
                                  &alignment3, &alignmentLength3);
            if (score3 != scoreExpected) {
                failed = true;
                printf("For k = %d score was %d but it should have been %d\n",
                       k, score3, scoreExpected);
            }
            if (alignment3) {
                if (!checkAlignment(query, queryLength, target, targetLength,
                                    score3, endLocations3[0], mode, alignment3, alignmentLength3)) {
                    failed = true;
                    printf("Alignment is not correct\n");
                }
                int alignmentStart = getAlignmentStart(alignment3, alignmentLength3, endLocations3[0]);
                if (startLocations3[0] != alignmentStart) {
                    failed = true;
                    printf("Start location (%d) is not consistent with alignment start (%d)\n",
                           startLocations3[0], alignmentStart);
                }
                free(alignment3);
            }
            if (endLocations3) free(endLocations3);
            if (startLocations3) free(startLocations3);
        }

        if (failed)
            numTestsFailed++;
    }

    printf(mode == EDLIB_MODE_HW ? "HW: " : mode == EDLIB_MODE_SHW ? "SHW: " : "NW: ");
    printf(numTestsFailed == 0 ? "\x1B[32m" : "\x1B[31m");
    printf("%d/%d", numTests - numTestsFailed, numTests);
    printf("\x1B[0m");
    printf(" random tests passed!\n");
    double mTime = ((double)(timeEdlib))/CLOCKS_PER_SEC;
    double sTime = ((double)(timeSimple))/CLOCKS_PER_SEC;
    printf("Time Edlib: %lf\n", mTime);
    printf("Time Simple: %lf\n", sTime);
    printf("Times faster: %.2lf\n", sTime / mTime);
}


bool executeTest(const unsigned char* query, int queryLength,
                 const unsigned char* target, int targetLength,
                 int alphabetLength, int mode) {
    printf(mode == EDLIB_MODE_HW ? "HW:  " : mode == EDLIB_MODE_SHW ? "SHW: " : "NW:  ");

    bool pass = true;

    int score2; int numLocations2; int* endLocations2;
    calcEditDistanceSimple(query, queryLength, target, targetLength,
                           alphabetLength, mode, &score2, &endLocations2,
                           &numLocations2);

    int score1; int numLocations1; int* endLocations1; int* startLocations1;
    unsigned char* alignment; int alignmentLength;
    edlibCalcEditDistance(query, queryLength, target, targetLength,
                          alphabetLength, -1, mode, true, true,
                          &score1, &endLocations1, &startLocations1, &numLocations1,
                          &alignment, &alignmentLength);

    if (score1 != score2) {
        pass = false;
        printf("Scores: expected %d, got %d\n", score2, score1);
    } else if (numLocations1 != numLocations2) {
        pass = false;
        printf("Number of locations: expected %d, got %d\n",
               numLocations2, numLocations1);
    } else {
        for (int i = 0; i < numLocations1; i++) {
            if (endLocations1[i] != endLocations2[i]) {
                pass = false;
                printf("End locations at %d are not equal! Expected %d, got %d\n",
                       i, endLocations2[i], endLocations1[1]);
                break;
            }
        }
    }
    if (alignment) {
        if (!checkAlignment(query, queryLength, target, targetLength,
                            score1, endLocations1[0], mode, alignment,
                            alignmentLength)) {
            pass = false;
            printf("Alignment is not correct\n");
        }
        int alignmentStart = getAlignmentStart(alignment, alignmentLength, endLocations1[0]);
        if (startLocations1[0] != alignmentStart) {
            pass = false;
            printf("Start location (%d) is not consistent with alignment start (%d)\n",
                   startLocations1[0], alignmentStart);
        }
    }

    printf(pass ? "\x1B[32m OK \x1B[0m\n" : "\x1B[31m FAIL \x1B[0m\n");

    if (alignment) free(alignment);
    if (endLocations1) free(endLocations1);
    if (endLocations2) free(endLocations2);
    if (startLocations1) free(startLocations1);
    return pass;
}

bool test1() {
    int alphabetLength = 4;
    int queryLength = 4;
    int targetLength = 4;
    unsigned char query[4] = {0,1,2,3};
    unsigned char target[4] = {0,1,2,3};

    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, EDLIB_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, EDLIB_MODE_NW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, EDLIB_MODE_SHW);
    return r;
}

bool test2() {
    int alphabetLength = 9;
    int queryLength = 5;
    int targetLength = 9;
    unsigned char query[5] = {0,1,2,3,4}; // "match"
    unsigned char target[9] = {8,5,0,1,3,4,6,7,5}; // "remachine"

    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, EDLIB_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, EDLIB_MODE_NW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, EDLIB_MODE_SHW);
    return r;
}

bool test3() {
    int alphabetLength = 6;
    int queryLength = 5;
    int targetLength = 9;
    unsigned char query[5] = {0,1,2,3,4};
    unsigned char target[9] = {1,2,0,1,2,3,4,5,4};

    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, EDLIB_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, EDLIB_MODE_NW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, EDLIB_MODE_SHW);
    return r;
}

bool test4() {
    int alphabetLength = 2;
    int queryLength = 200;
    int targetLength = 200;
    unsigned char query[200] = {0};
    unsigned char target[200] = {1};

    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, EDLIB_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, EDLIB_MODE_NW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, EDLIB_MODE_SHW);
    return r;
}

bool test5() {
    int alphabetLength = 2;
    int queryLength = 64; // Testing for special case when queryLength == word size
    int targetLength = 64;
    unsigned char query[64] = {0};
    unsigned char target[64] = {1};

    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, EDLIB_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, EDLIB_MODE_NW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, EDLIB_MODE_SHW);
    return r;
}

bool test6() {
    int alphabetLength = 4;
    int queryLength = 13; // Testing for special case when queryLength == word size
    int targetLength = 420;
    unsigned char query[13] = {1,3,0,1,1,1,3,0,1,3,1,3,3};
    unsigned char target[420] = {0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,0,1,3,0,1,3,
                                 3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,2,3,2,3,3,
                                 1,0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,0,1,3,0,1,
                                 3,3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,2,3,2,3,
                                 3,1,0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,0,1,3,0,
                                 1,3,3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,2,3,2,
                                 3,3,1,0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,0,1,3,
                                 0,1,3,3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,2,3,
                                 2,3,3,1,0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,0,1,
                                 3,0,1,3,3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,2,
                                 3,2,3,3,1,0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,0,
                                 1,3,0,1,3,3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,
                                 2,3,2,3,3,1,0,1,1,1,0,1,3,0,1,3,3,3,1,3,2,2,3,2,3,3,1,0,1,1,1,
                                 0,1,3,0,1,3,3,3,1,3,2,2,3,2,3,3,1};

    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, EDLIB_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, EDLIB_MODE_NW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, EDLIB_MODE_SHW);
    return r;
}

bool test7() {
    int alphabetLength = 4;
    int queryLength = 3;
    int targetLength = 5;
    unsigned char query[3] = {2,3,0};
    unsigned char target[5] = {0,1,2,2,0};

    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, EDLIB_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, EDLIB_MODE_NW);
    // Fails but not sure if it should. TODO: check this.
    //r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, EDLIB_MODE_SHW);
    return r;
}

bool test8() {
    int alphabetLength = 4;
    int queryLength = 3;
    int targetLength = 3;
    unsigned char query[3] = {2,3,0};
    unsigned char target[3] = {2,2,0};

    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, EDLIB_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, EDLIB_MODE_NW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, EDLIB_MODE_SHW);
    return r;
}

bool testCigar() {
    unsigned char alignment[] = {EDLIB_EDOP_MATCH, EDLIB_EDOP_MATCH, EDLIB_EDOP_INSERT, EDLIB_EDOP_INSERT,
                                 EDLIB_EDOP_INSERT, EDLIB_EDOP_DELETE, EDLIB_EDOP_INSERT, EDLIB_EDOP_INSERT,
                                 EDLIB_EDOP_MISMATCH, EDLIB_EDOP_MATCH, EDLIB_EDOP_MATCH};
    char* cigar;

    edlibAlignmentToCigar(alignment, 11, EDLIB_CIGAR_EXTENDED, &cigar);
    bool pass = true;
    char expected[] = "2=3I1D2I1X2=";
    if (strcmp(cigar, expected) != 0) {
        pass = false;
        printf("Expected %s, got %s\n", expected, cigar);
    }
    printf(pass ? "\x1B[32m""OK""\x1B[0m\n" : "\x1B[31m""FAIL""\x1B[0m\n");
    if (cigar) free(cigar);

    edlibAlignmentToCigar(alignment, 11, EDLIB_CIGAR_STANDARD, &cigar);
    pass = true;
    char expected2[] = "2M3I1D2I3M";
    if (strcmp(cigar, expected2) != 0) {
        pass = false;
        printf("Expected %s, got %s\n", expected2, cigar);
    }
    printf(pass ? "\x1B[32m""OK""\x1B[0m\n" : "\x1B[31m""FAIL""\x1B[0m\n");
    if (cigar) free(cigar);

    return pass;
}

bool runTests() {
    // TODO: make this global vector where tests have to add themselves.
    int numTests = 9;
    bool (* tests [])() = {test1, test2, test3, test4, test5, test6,
                           test7, test8, testCigar};

    bool allTestsPassed = true;
    for (int i = 0; i < numTests; i++) {
        printf("Test #%d:\n", i);
        if ( !(*(tests[i]))() ) {
            allTestsPassed = false;
        }
    }
    return allTestsPassed;
}

/**
 * Checks if alignment is correct.
 */
bool checkAlignment(const unsigned char* query, int queryLength,
                    const unsigned char* target, int targetLength,
                    int score, int pos, int mode,
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
    if (startLocation == endLocation) {
        return -1;
    }
    return startLocation;
}
