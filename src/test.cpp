#include <cstdio>
#include <ctime>
#include <cstdlib>

#include "myers.h"

using namespace std;


void calcEditDistanceSimple(char* query, int queryLength, char* target, int targetLength,
                            char* alphabet, int alphabetLength, int* score);
void fillRandomly(char* seq, int seqLength, char* alphabet, int alphabetLength);


int main() {
    srand(time(NULL));
    
    /*char query[5] = {'m', 'a', 't', 'c', 'h'};
      char target[9] = {'r', 'e', 'm', 'a', 'c', 'h', 'i', 'n', 'e'};*/

    char alphabet[] = {'A', 'C', 'T', 'G'};
    int alphabetLength = 4;
    int numTests = 50;
    int numTestsFailed = 0;
    for (int i = 0; i < numTests; i++) {
        int queryLength = 10 + rand() % 500;
        int targetLength = 10 + rand() % 2000;
        char query[queryLength];
        char target[targetLength];
        fillRandomly(query, queryLength, alphabet, alphabetLength);
        fillRandomly(target, targetLength, alphabet, alphabetLength);
        
        int score1;
        calcEditDistance(query, queryLength, target, targetLength, alphabet, alphabetLength, &score1);
        int score2;
        calcEditDistanceSimple(query, queryLength, target, targetLength, alphabet, alphabetLength, &score2);

        if (score1 != score2)
            numTestsFailed++;
        printf("%d, %d\n", score1, score2); 
    }
    
    if (numTestsFailed > 0)
        printf("%d/%d tests failed!\n", numTestsFailed, numTests);
    else
        printf("All tests passed!\n");
    
    return 0;
}



int min(int x, int y) {
    return x < y ? x : y;
}

void fillRandomly(char* seq, int seqLength, char* alphabet, int alphabetLength) {
    for (int i = 0; i < seqLength; i++)
        seq[i] = alphabet[rand() % alphabetLength];
}

void calcEditDistanceSimple(char* query, int queryLength, char* target, int targetLength,
                            char* alphabet, int alphabetLength, int* score) {
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

    *score = bestScore;
    delete[] C;
    delete[] newC;
}
