#include <cstdio>
#include <ctime>
#include <cstdlib>

#include "myers.h"
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

int main() {
    srand(1);
    
    printf("Testing HW with alignment...\n");
    runRandomTests(1000, MYERS_MODE_HW, true);
    printf("\n");
    
    printf("Testing HW...\n");
    runRandomTests(1000, MYERS_MODE_HW, false);
    printf("\n");
    
    printf("Testing NW with alignment...\n");
    runRandomTests(1000, MYERS_MODE_NW, true);
    printf("\n");
    
    
    printf("Testing NW...\n");
    runRandomTests(1000, MYERS_MODE_NW, false);
    printf("\n");
    
    printf("Testing SHW with alignment...\n");
    runRandomTests(1000, MYERS_MODE_SHW, true);
    printf("\n");
    
    printf("Testing SHW...\n");
    runRandomTests(1000, MYERS_MODE_SHW, false);
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
    double timeMyers = 0;
    double timeSimple = 0;
    
    for (int i = 0; i < numTests; i++) {
        bool failed = false;
        int queryLength = 10 + rand() % 200;
        int targetLength = 100 + rand() % 20000;
        unsigned char query[queryLength];
        unsigned char target[targetLength];
        fillRandomly(query, queryLength, alphabetLength);
        fillRandomly(target, targetLength, alphabetLength);
        /*
        // Print query
        printf("Query: ");
        for (int i = 0; i < queryLength; i++)
            printf("%d", query[i]);
        printf("\n");

        // Print target
        printf("Target: ");
        for (int i = 0; i < targetLength; i++)
            printf("%d", target[i]);
        printf("\n");
        */  
        start = clock();
        int score1, numPositions1;
        int* positions1;
        unsigned char* alignment; int alignmentLength;
        myersCalcEditDistance(query, queryLength, target, targetLength, alphabetLength,
                              -1, mode, &score1, &positions1, &numPositions1,
                              findAlignment, &alignment, &alignmentLength);
        timeMyers += clock() - start;
        if (alignment) {
            if (!checkAlignment(query, queryLength, target, targetLength,
                                score1, positions1[0], mode, alignment, alignmentLength)) {
                failed = true;
                printf("Alignment is not correct\n");
            }
            free(alignment);
        }
        
        start = clock();
        int score2; int numPositions2;
        int* positions2;
        calcEditDistanceSimple(query, queryLength, target, targetLength,
                               alphabetLength, mode, &score2,
                               &positions2, &numPositions2);
        timeSimple += clock() - start;
        
        // Compare results
        if (score1 != score2) {
            failed = true;
            printf("Scores are different! Expected %d, got %d)\n", score2, score1);
        } else if (score1 == -1 && !(positions1 == NULL)) {
            failed = true;
            printf("Score was not found but positions is not NULL!\n");
        } else if (numPositions1 != numPositions2) {
            failed = true;
            printf("Number of positions returned is not equal! Expected %d, got %d\n",
                   numPositions2, numPositions1);
        } else {
            for (int i = 0; i < numPositions1; i++) {
                if (positions1[i] != positions2[i]) {
                    failed = true;
                    printf("Positions at %d are not equal! Expected %d, got %d\n",
                           i, positions2[i], positions1[i]);
                    break;
                }
            }
        }
        
        if (positions1) free(positions1);
        if (positions2) free(positions2);

        for (int k = score2 - 1; k <= score2 + 1; k++) {
            int score3, numPositions3;
            int* positions3;
            unsigned char* alignment3; int alignmentLength3;
            int scoreExpected = score2 > k ? -1 : score2;
            myersCalcEditDistance(query, queryLength, target, targetLength,
                                  alphabetLength, k, mode, &score3, &positions3,
                                  &numPositions3, findAlignment, &alignment3,
                                  &alignmentLength3);
            if (score3 != scoreExpected ) {
                failed = true;
                printf("For k = %d score was %d but it should have been %d\n",
                       k, score3, scoreExpected);
            }
            if (alignment3) {
                if (!checkAlignment(query, queryLength, target, targetLength,
                                    score3, positions3[0], mode, alignment3, alignmentLength3)) {
                    failed = true;
                    printf("Alignment is not correct\n");
                }
                free(alignment3);
            }
            if (positions3) free(positions3);
        }

        if (failed)
            numTestsFailed++;
    }
    
    printf(mode == MYERS_MODE_HW ? "HW: " : mode == MYERS_MODE_SHW ? "SHW: " : "NW: ");
    printf(numTestsFailed == 0 ? "\x1B[32m" : "\x1B[31m");
    printf("%d/%d", numTests - numTestsFailed, numTests);
    printf("\x1B[0m");
    printf(" random tests passed!\n");
    double mTime = ((double)(timeMyers))/CLOCKS_PER_SEC;
    double sTime = ((double)(timeSimple))/CLOCKS_PER_SEC;
    printf("Time Myers: %lf\n", mTime);
    printf("Time Simple: %lf\n", sTime);
    printf("Times faster: %.2lf\n", sTime / mTime);
}


bool executeTest(const unsigned char* query, int queryLength,
                 const unsigned char* target, int targetLength,
                 int alphabetLength, int mode) {
    printf(mode == MYERS_MODE_HW ? "HW:  " : mode == MYERS_MODE_SHW ? "SHW: " : "NW:  ");
    
    bool pass = true;

    int score2; int numPositions2; int* positions2;
    calcEditDistanceSimple(query, queryLength, target, targetLength,
                           alphabetLength, mode, &score2, &positions2,
                           &numPositions2);

    int score1; int numPositions1; int* positions1;
    unsigned char* alignment; int alignmentLength;
    myersCalcEditDistance(query, queryLength, target, targetLength,
                          alphabetLength, -1, mode, &score1, &positions1,
                          &numPositions1, false, &alignment, &alignmentLength);

    if (score1 != score2) {
        pass = false;
        printf("Scores: expected %d, got %d\n", score2, score1);
    } else if (numPositions1 != numPositions2) {
        pass = false;
        printf("Number of positions: expected %d, got %d\n",
               numPositions2, numPositions1);
    } else {
        for (int i = 0; i < numPositions1; i++) {
            if (positions1[i] != positions2[i]) {
                pass = false;
                printf("Positions at %d are not equal! Expected %d, got %d\n",
                       i, positions2[i], positions1[1]);
                break;
            }
        }
    }
    if (alignment) {
        if (!checkAlignment(query, queryLength, target, targetLength,
                            score1, positions1[0], mode, alignment,
                            alignmentLength)) {
            pass = false;
            printf("Alignment is not correct\n");
        }
    }

    printf(pass ? "\x1B[32m OK \x1B[0m\n" : "\x1B[31m FAIL \x1B[0m\n");
    
    if (alignment) free(alignment);
    if (positions1) free(positions1);
    if (positions2) free(positions2);
    return pass;
}

bool test1() {
    int alphabetLength = 4;
    int queryLength = 4;
    int targetLength = 4;
    unsigned char query[4] = {0,1,2,3};
    unsigned char target[4] = {0,1,2,3};

    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, MYERS_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, MYERS_MODE_NW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, MYERS_MODE_SHW);
    return r;
}

bool test2() {
    int alphabetLength = 9;
    int queryLength = 5;
    int targetLength = 9;
    unsigned char query[5] = {0,1,2,3,4}; // "match"
    unsigned char target[9] = {8,5,0,1,3,4,6,7,5}; // "remachine"

    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, MYERS_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, MYERS_MODE_NW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, MYERS_MODE_SHW);
    return r;
}

bool test3() {
    int alphabetLength = 6;
    int queryLength = 5;
    int targetLength = 9;
    unsigned char query[5] = {0,1,2,3,4};
    unsigned char target[9] = {1,2,0,1,2,3,4,5,4};

    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, MYERS_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, MYERS_MODE_NW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, MYERS_MODE_SHW);
    return r;
}

bool test4() {
    int alphabetLength = 2;
    int queryLength = 200;
    int targetLength = 200;
    unsigned char query[200] = {0};
    unsigned char target[200] = {1};

    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, MYERS_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, MYERS_MODE_NW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, MYERS_MODE_SHW);
    return r;
}

bool test5() {
    int alphabetLength = 2;
    int queryLength = 64; // Testing for special case when queryLength == word size
    int targetLength = 64;
    unsigned char query[64] = {0};
    unsigned char target[64] = {1};

    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, MYERS_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, MYERS_MODE_NW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, MYERS_MODE_SHW);
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
    
    bool r = executeTest(query, queryLength, target, targetLength, alphabetLength, MYERS_MODE_HW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, MYERS_MODE_NW);
    r = r && executeTest(query, queryLength, target, targetLength, alphabetLength, MYERS_MODE_SHW);
    return r;
}

bool runTests() {
    int numTests = 6;
    bool (* tests [])() = {test1, test2, test3, test4, test5, test6};
    
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
        if (alignment[i] == 0) { // (mis)match
            alignScore += query[qIdx] == target[tIdx] ? 0 : 1;
            qIdx--;
            tIdx--;
        }
        else if (alignment[i] == 1) {
            alignScore += 1;
            qIdx--;
        }
        else if (alignment[i] == 2) {
            if (!(mode == MYERS_MODE_HW && qIdx == -1))
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
    return true;
}
