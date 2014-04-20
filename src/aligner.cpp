#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <vector>
#include <ctime>
#include <string>
#include <climits>
#include <queue>

#include "myers.h"

using namespace std;

int readFastaSequences(const char* path, vector< vector<unsigned char> >* seqs,
                       unsigned char* letterIdx, char* idxToLetter, bool* inAlphabet, int &alphabetLength);

// For debugging
void printSeq(const vector<unsigned char> &seq) {
    for (int i = 0; i < seq.size(); i++)
        printf("%d ", seq[i]);
    printf("\n");
}

int main(int argc, char * const argv[]) {
    
    //----------------------------- PARSE COMMAND LINE ------------------------//
    // If true, there will be no output.
    bool silent = false;
    // Alignment mode.
    char mode[16] = "NW";
    // How many best sequences (those with smallest score) do we want.
    // If 0, then we want them all.
    int numBestSeqs = 0;
    bool findAlignment = false;
    int option;
    while ((option = getopt(argc, argv, "a:n:sc")) >= 0) { // : is not a delimiter but indicator of parameter
        switch (option) {
        case 'a': strcpy(mode, optarg); break;
        case 'n': numBestSeqs = atoi(optarg); break;
        case 's': silent = true; break;
        case 'c': findAlignment = true; break;
        }
    }
    if (optind + 2 != argc) {
        fprintf(stderr, "\n");
        fprintf(stderr, "Usage: aligner [options...] <queries.fasta> <target.fasta>\n");        
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "\t-s  If specified, there will be no score output (silent mode).\n");
        fprintf(stderr, "\t-a HW|NW|SHW  Alignment mode that will be used. [default: NW]\n");
        fprintf(stderr, "\t-n N  Score will be calculated only for N best sequences (best = with smallest score)."
                        "If N = 0 then all sequences will be calculated. " 
                        "Specifying small N can make total calculation much faster. [default: 0]\n");
        fprintf(stderr, "\t-c If specified, alignment will be found and printed.\n");
        return 1;
    }
    //-------------------------------------------------------------------------//


    int modeCode;
    if (!strcmp(mode, "SHW"))
        modeCode = MYERS_MODE_SHW;
    else if (!strcmp(mode, "HW"))
        modeCode = MYERS_MODE_HW;
    else if (!strcmp(mode, "NW"))
        modeCode = MYERS_MODE_NW;
    else {
        printf("Invalid mode!\n");
        return 1;
    }
    printf("Using mode %s\n", mode);


    // Alphabet information, will be constructed on fly while reading sequences
    unsigned char letterIdx[128]; //!< letterIdx[c] is index of letter c in alphabet
    char idxToLetter[128]; //!< numToLetter[i] is letter that has index i in alphabet
    bool inAlphabet[128]; // inAlphabet[c] is true if c is in alphabet
    for (int i = 0; i < 128; i++) {
        inAlphabet[i] = false;
    }
    int alphabetLength = 0;

    int readResult;
    // Read queries
    char* queriesFilepath = argv[optind];
    vector< vector<unsigned char> >* querySequences = new vector< vector<unsigned char> >();
    printf("Reading queries fasta file...\n");
    readResult = readFastaSequences(queriesFilepath, querySequences, letterIdx, idxToLetter,
                                    inAlphabet, alphabetLength);
    if (readResult) {
        printf("Error: There is no file with name %s\n", queriesFilepath);
        return 1;
    }
    int numQueries = querySequences->size();

    // Read target
    char* targetFilepath = argv[optind+1];    
    vector< vector<unsigned char> >* targetSequences = new vector< vector<unsigned char> >();
    printf("Reading target fasta file...\n");
    readResult = readFastaSequences(targetFilepath, targetSequences, letterIdx, idxToLetter,
                                    inAlphabet, alphabetLength);
    if (readResult) {
        printf("Error: There is no file with name %s\n", targetFilepath);
        return 1;
    }
    unsigned char* target = (*targetSequences)[0].data();
    int targetLength = (*targetSequences)[0].size();

    printf("Alphabet: ");
    for (int c = 0; c < 128; c++)
        if (inAlphabet[c])
            printf("%c ", c);
    printf("\n");
    printf("Alphabet length: %d\n", alphabetLength);


    // ----------------------------- MAIN CALCULATION ----------------------------- //
    printf("\nSearching...\n");
    int* scores = new int[numQueries];
    int* pos    = new int[numQueries];
    priority_queue<int> bestScores; // Contains numBestSeqs best scores
    int k = -1;
    unsigned char* alignment = NULL; int alignmentLength;
    clock_t start = clock();

    if (silent) {
        printf("0/%d", numQueries);
        fflush(stdout);
    }
    for (int i = 0; i < numQueries; i++) {
        unsigned char* query = (*querySequences)[i].data();
        int queryLength = (*querySequences)[i].size();
        // Calculate score
        myersCalcEditDistance(query, queryLength, target, targetLength,
                              alphabetLength, k, modeCode, scores + i, pos + i,
                              findAlignment, &alignment, &alignmentLength);
        
        // If we want only numBestSeqs best sequences, update best scores and adjust k to largest score.
        if (numBestSeqs > 0) {
            if (scores[i] >= 0) {
                bestScores.push(scores[i]);
                if (bestScores.size() > numBestSeqs) {
                    bestScores.pop();
                }
                if (bestScores.size() == numBestSeqs) {
                    k = bestScores.top() - 1;
                }
            }
        }
        
        if (silent) {
            printf("\r%d/%d", i+1, numQueries);
            fflush(stdout);
        } else {
            // Print alignment if it was found
            if (alignment) {
                printf("\n");
                printf("%d: query length = %d, target length = %d\n", i, queryLength, targetLength);
                int tIdx = 0;
                int qIdx = 0;
                int targetLettersToSkip = 0;
                if (modeCode == MYERS_MODE_HW) {
                    for (int j = 0; j < alignmentLength && alignment[j] == 2; j++) {
                        targetLettersToSkip++;
                        tIdx++;
                    }
                }
                for (int start = targetLettersToSkip; start < alignmentLength; start += 50) {
                    // target
                    printf("T: ");
                    int startTIdx = tIdx;
                    for (int j = start; j < start + 50 && j < alignmentLength; j++) {
                        if (alignment[j] == 1)
                            printf("_");
                        else
                            printf("%c", idxToLetter[target[tIdx++]]);
                    }
                    printf(" (%d - %d)\n", startTIdx, tIdx - 1);
                    // query
                    printf("Q: ");
                    int startQIdx = qIdx;
                    for (int j = start; j < start + 50 && j < alignmentLength; j++) {
                        if (alignment[j] == 2)
                            printf("_");
                        else
                            printf("%c", idxToLetter[query[qIdx++]]);
                    }
                    printf(" (%d - %d)\n", startQIdx, qIdx - 1);
                    printf("\n");
                }
            }
        }

        if (alignment)
            free(alignment);
    }
    printf("\n");

    if (!silent) {
        int scoreLimit = -1; // Only scores <= then scoreLimit will be printed (we consider -1 as infinity)
        printf("\n");

        if (bestScores.size() > 0) {
            printf("%d best scores:\n", (int)bestScores.size());
            scoreLimit = bestScores.top();
        }

        printf("Scores (score, position) \n");
        for (int i = 0; i < numQueries; i++)
            if (scores[i] > -1 && (scoreLimit == -1 || scores[i] <= scoreLimit))
                printf("%d: (%d, %d)\n", i, scores[i], pos[i]);
        
    }

    clock_t finish = clock();
    double cpuTime = ((double)(finish-start))/CLOCKS_PER_SEC;
    printf("\nCpu time of searching: %lf\n", cpuTime);
    // ---------------------------------------------------------------------------- //

    // Free allocated space
    delete querySequences;
    delete targetSequences;
    delete[] scores;
    delete[] pos;
    
    return 0;
}




/** Reads sequences from fasta file.
 * Function is passed current alphabet information and will update it if needed.
 * @param [in] path Path to fasta file containing sequences.
 * @param [out] seqs Sequences will be stored here, each sequence as vector of indexes from alphabet.
 * @param [inout] letterIdx  Array of length 128. letterIdx[c] is index of letter c in alphabet.
 * @param [inout] inAlphabet  Array of length 128. inAlphabet[c] is true if c is in alphabet.
 * @param [inout] alphabetLength
 * @return 0 if all ok, positive number otherwise.
 */
int readFastaSequences(const char* path, vector< vector<unsigned char> >* seqs,
                       unsigned char* letterIdx, char* idxToLetter, bool* inAlphabet, int &alphabetLength) {
    seqs->clear();
    
    FILE* file = fopen(path, "r");
    if (file == 0)
        return 1;

    bool inHeader = false;
    bool inSequence = false;
    int buffSize = 4096;
    char buffer[buffSize];
    while (!feof(file)) {
        int read = fread(buffer, sizeof(char), buffSize, file);
        for (int i = 0; i < read; ++i) {
            char c = buffer[i];
            if (inHeader) { // I do nothing if in header
                if (c == '\n')
                    inHeader = false;
            } else {
                if (c == '>') {
                    inHeader = true;
                    inSequence = false;
                } else {
                    if (c == '\r' || c == '\n')
                        continue;
                    // If starting new sequence, initialize it.
                    if (inSequence == false) {
                        inSequence = true;
                        seqs->push_back(vector<unsigned char>());
                    }

                    if (!inAlphabet[c]) { // I construct alphabet on fly
                        inAlphabet[c] = true;
                        letterIdx[c] = alphabetLength;
                        idxToLetter[alphabetLength] = c;
                        alphabetLength++;
                    }
                    seqs->back().push_back(letterIdx[c]);
                }
            }
        }
    }

    fclose(file);
    return 0;
}
