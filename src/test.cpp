#include <cstdio>
#include <ctime>
#include <cstdlib>

#include "myers.h"

using namespace std;


int calcEditDistanceSimple(const unsigned char* query, int queryLength,
                           const unsigned char* target, int targetLength,
                           int alphabetLength);
void fillRandomly(unsigned char* seq, int seqLength, int alphabetLength);

void runRandomTests(int numTests);
bool runTests();




int min(int x, int y) {
    return x < y ? x : y;
}

void fillRandomly(unsigned char* seq, int seqLength, int alphabetLength) {
    for (int i = 0; i < seqLength; i++)
        seq[i] = rand() % alphabetLength;
}

int calcEditDistanceSimple(const unsigned char* query, int queryLength,
                           const unsigned char* target, int targetLength,
                           int alphabetLength) {
    int* C = new int[queryLength];
    int* newC = new int[queryLength];

    int bestScore = -1;

    // set first column (column zero)
    for (int i = 0; i < queryLength; i++)
        C[i] = i+1;

      /*for (int i = 0; i < queryLength; i++)
        printf("%d ", C[i]);
        printf("\n");*/

    for (int c = 0; c < targetLength; c++) { // for each column
        newC[0] = min(1, min(target[c] == query[0] ? 0 : 1, C[0] + 1));
        for (int r = 1; r < queryLength; r++) {
            newC[r] = min(newC[r-1] + 1, // up
                          min(C[r-1] + (target[c] == query[r] ? 0 : 1), // up left
                              C[r] + 1)); // left
        }
        
        /*for (int i = 0; i < queryLength; i++)
            printf("%d ", newC[i]);
            printf("\n");*/

        if (bestScore == -1 || newC[queryLength-1] < bestScore)
            bestScore = newC[queryLength-1];
        
        int *tmp = C;
        C = newC;
        newC = tmp;
    }

    delete[] C;
    delete[] newC;

    return bestScore;
}

void runRandomTests(int numTests) {
    int alphabetLength = 4;
    int numTestsFailed = 0;
    for (int i = 0; i < numTests; i++) {
        int queryLength = 10 + rand() % 500;
        int targetLength = 10 + rand() % 2000;
        unsigned char query[queryLength];
        unsigned char target[targetLength];
        fillRandomly(query, queryLength, alphabetLength);
        fillRandomly(target, targetLength, alphabetLength);
        
        int score1 = calcEditDistance(query, queryLength, target, targetLength, alphabetLength);
        int score2 = calcEditDistanceSimple(query, queryLength, target, targetLength, alphabetLength);

        if (score1 != score2)
            numTestsFailed++;
        printf("%d, %d\n", score1, score2); 
    }
    
    if (numTestsFailed > 0)
        printf("%d/%d random tests failed!\n", numTestsFailed, numTests);
    else
        printf("All random tests passed!\n");
}
    
bool test1() {
    int alphabetLength = 4;
    int queryLength = 4;
    int targetLength = 4;
    unsigned char query[4] = {0,1,2,3};
    unsigned char target[4] = {0,1,2,3};
    int correct = 0;

    int score1 = calcEditDistance(query, queryLength, target, targetLength, alphabetLength);
    int score2 = calcEditDistanceSimple(query, queryLength, target, targetLength, alphabetLength);

    printf("Myers -> %d, simple -> %d, correct -> %d\n", score1, score2, correct);
    return score1 == score2 && score2 == correct;
}

bool test2() {
    int alphabetLength = 9;
    int queryLength = 5;
    int targetLength = 9;
    unsigned char query[5] = {0,1,2,3,4}; // "match"
    unsigned char target[9] = {8,5,0,1,3,4,6,7,5}; // "remachine"
    int correct = 1;

    int score1 = calcEditDistance(query, queryLength, target, targetLength, alphabetLength);
    int score2 = calcEditDistanceSimple(query, queryLength, target, targetLength, alphabetLength);

    printf("Myers -> %d, simple -> %d, correct -> %d\n", score1, score2, correct);
    return score1 == score2 && score2 == correct;
}

bool test3() {
    int alphabetLength = 6;
    int queryLength = 5;
    int targetLength = 9;
    unsigned char query[5] = {0,1,2,3,4};
    unsigned char target[9] = {1,2,0,1,2,3,4,5,4};
    int correct = 0;

    int score1 = calcEditDistance(query, queryLength, target, targetLength, alphabetLength);
    int score2 = calcEditDistanceSimple(query, queryLength, target, targetLength, alphabetLength);

    printf("Myers -> %d, simple -> %d, correct -> %d\n", score1, score2, correct);
    return score1 == score2 && score2 == correct;
}

bool runTests() {
    bool t1 = test1();
    bool t2 = test2();
    bool t3 = test3();
    return t1 && t2 && t3;
}


int main() {
    srand(time(NULL));

    //    runRandomTests(50);
    //runTests();

    test1();
    
    return 0;
}
