//
// Created by mobin on 12/20/19.
// Test the runtime of generic edlib for different query lengths
// and different alphabet size
//

#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <climits>
#include <iostream>
#include <fstream>


#include "../include/edlibGeneric.h"
#include "SimpleEditDistance_Generic.h"

using namespace std;
using namespace edlibGeneric;

void runRandomTests(int numTests, EdlibAlignMode mode, bool findAlignment,
                    int alphabetLength, int queryLength, int targetLength,
                    ofstream& file);


int main(int argc, char* argv[]) {
    // This program has optional first parameter, which is number of random tests to run
    // per each algorithm.
    int numRandomTests = 5;
    if (argc > 1) {
        numRandomTests = static_cast<int>(strtol(argv[1], NULL, 10));
    }

    std::ofstream ofs;
    ofs.open ("performanceGeneric_alphabet.txt", std::ofstream::out | std::ofstream::app);
    ofs << "modeStr" << "\t" << "alphaLen" << "\t" << "queryLen" << "\t";
    ofs << "targetLen" << "\t" << "findAlignment" << "\t" << "mTime" << "\n";

    srand(clock());
    for(int queryLength = 10000; queryLength <= 20000; queryLength *= 10) {
        int targetLength = queryLength;
        for (int alphabetLength = 4000; alphabetLength < 11000 ; alphabetLength += 1000) {

            printf("alphabetLength = %d \n", alphabetLength);
            printf("alphabet length = %d \n", alphabetLength);
            printf("######################### \n");

            printf("Testing HW with alignment...\n");
            runRandomTests(numRandomTests, EDLIB_MODE_HW, true,
                           alphabetLength, queryLength, targetLength, ofs);
            printf("\n");

            printf("Testing HW...\n");
            runRandomTests(numRandomTests, EDLIB_MODE_HW, false,
                           alphabetLength, queryLength, targetLength, ofs);
            printf("\n");

            printf("Testing NW with alignment...\n");
            runRandomTests(numRandomTests, EDLIB_MODE_NW, true,
                           alphabetLength, queryLength, targetLength, ofs);
            printf("\n");

            printf("Testing NW...\n");
            runRandomTests(numRandomTests, EDLIB_MODE_NW, false,
                           alphabetLength, queryLength, targetLength, ofs);
            printf("\n");

            printf("Testing SHW with alignment...\n");
            runRandomTests(numRandomTests, EDLIB_MODE_SHW, true,
                           alphabetLength, queryLength, targetLength, ofs);
            printf("\n");

            printf("Testing SHW...\n");
            runRandomTests(numRandomTests, EDLIB_MODE_SHW, false,
                           alphabetLength, queryLength, targetLength, ofs);
            printf("\n");
        }
    }
    ofs.close();
    return 0;
}


void fillRandomly(AlphabetType * seq, int seqLength, int alphabetLength) {
    for (int i = 0; i < seqLength; i++)
        seq[i] = static_cast<AlphabetType>(rand() % alphabetLength);
}

// Returns true if all tests passed, false otherwise.
void runRandomTests(int numTests, EdlibAlignMode mode, bool findAlignment,
                    int alphabetLength, int queryLength, int targetLength,
                    ofstream& file) {

    clock_t start;
    double timeEdlib = 0;

    for (int i = 0; i < numTests; i++) {
        AlphabetType * query = static_cast<AlphabetType *>(malloc(sizeof(AlphabetType) * queryLength));
        AlphabetType * target = static_cast<AlphabetType *>(malloc(sizeof(AlphabetType) * targetLength));
        fillRandomly(query, queryLength, alphabetLength);
        fillRandomly(target, targetLength, alphabetLength);

        start = clock();
        EdlibAlignResult result = edlibAlign<AlphabetType , IdxType >(
                query, queryLength, target, targetLength,
                edlibNewAlignConfig<AlphabetType>(-1, mode, findAlignment ? EDLIB_TASK_PATH : EDLIB_TASK_DISTANCE, NULL, 0));
        timeEdlib += clock() - start;
        edlibFreeAlignResult(result);
        free(query);
        free(target);
    }


    string modeString = mode == EDLIB_MODE_HW ? "HW" : mode == EDLIB_MODE_SHW ? "SHW" : "NW";
    printf("\x1B[0m");
    printf("random tests passed!\n");
    double mTime = static_cast<double>(timeEdlib)/CLOCKS_PER_SEC;
    printf("Time Edlib: %lf\n", mTime);

    //writing on output file for visualization
    file << modeString << "\t" << alphabetLength << "\t" << queryLength << "\t";
    file << targetLength << "\t" << findAlignment << "\t" << mTime << "\n";

}
