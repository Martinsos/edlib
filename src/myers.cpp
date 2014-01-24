#include "myers.h"

#include <stdint.h>
#include <stdio.h>

using namespace std;

typedef uint64_t Word;
static const int WORD_SIZE = sizeof(Word) * 8; // Size of Word in bits
static const Word HIGH_BIT_MASK = ((Word)1) << (WORD_SIZE-1);

static void printBits(Word w);   // TODO: remove this declaration
static int ceilDiv(int x, int y);  // TODO: remove this declaration
static int calcEditDistanceUkkonen(const unsigned char* query, int queryLength,
                                   const unsigned char* target, int targetLength,
                                   int alphabetLength, int k);


int calcEditDistance(const unsigned char* query, int queryLength,
                     const unsigned char* target, int targetLength,
                     int alphabetLength) {
    
    int k = queryLength;
    return calcEditDistanceUkkonen(query, queryLength, target, targetLength, alphabetLength, k);
}

// For debugging
static void printBits(Word w) {
    for (int i = 0; i < WORD_SIZE; i++) {
        if (w & HIGH_BIT_MASK)
            printf("1");
        else
            printf("0");
        w <<= 1;
    }
    printf("\n");
}

/**
 * Corresponds to Advance_Block function from Myers.
 * Calculates one word(block), which is part of a column.
 * Highest bit of word is most bottom cell of block from column.
 * @param [in] Pv  Bitset, Pv[i] == 1 if vin is +1, otherwise Pv[i] == 0.
 * @param [in] Mv  Bitset, Mv[i] == 1 if vin is -1, otherwise Mv[i] == 0.
 * @param [in] Eq  Bitset, Eq[i] == 1 if match, 0 if mismatch.
 * @param [in] hin  Will be +1, 0 or -1.
 * @param [out] PvOut  Bitset, PvOut[i] == 1 if vout is +1, otherwise PvOut[i] == 0.
 * @param [out] MvOut  Bitset, MvOut[i] == 1 if vout is -1, otherwise MvOut[i] == 0.
 * @param [out] hout  Will be +1, 0 or -1.
 */
static inline int calculateBlock(const Word Pv, const Word Mv, const Word Eq, const int hin,
                                 Word &PvOut, Word &MvOut) {
    Word Xv = Eq | Mv;
    Word Xh = (((Eq & Pv) + Pv) ^ Pv) | (hin < 0 ? Eq | (Word)1 : Eq);

    Word Ph = Mv | ~(Xh | Pv);
    Word Mh = Pv & Xh;

    int hout = 0;
    if (Ph & HIGH_BIT_MASK)
        hout = 1;
    else if (Mh & HIGH_BIT_MASK)
        hout = -1;

    Ph <<= 1;
    Mh <<= 1;

    if (hin < 0)
        Mh |= (Word)1;
    else if (hin > 0)
        Ph |= (Word)1;
    PvOut = Mh | ~(Xv | Ph);
    MvOut = Ph & Xv;

    return hout;
}

/**
 * Does ceiling division x / y.
 * Note: x and y must be non-negative and x + y must not overflow.
 */
static inline int ceilDiv(int x, int y) {
    return (x + y - 1) / y;
}

static int calcEditDistanceUkkonen(const unsigned char* query, int queryLength,
                                   const unsigned char* target, int targetLength,
                                   int alphabetLength, int k) {
    int maxNumBlocks = ceilDiv(queryLength, WORD_SIZE); // bmax in Myers

    Word* P = new Word[maxNumBlocks];
    Word* M = new Word[maxNumBlocks];
    Word* score = new Word[maxNumBlocks];
    Word** Peq = new Word*[alphabetLength+1]; // [alphabetLength+1][maxNumBlocks]. Last letter is wildcard.

    int currNumBlocks = ceilDiv(k, WORD_SIZE); // y in Myers
    int W = WORD_SIZE - (queryLength % WORD_SIZE); // number of redundant cells in last level blocks

    // Build Peq -> last column is wildcard with just zeroes
    for (int symbol = 0; symbol <= alphabetLength; symbol++) {
        Peq[symbol] = new Word[maxNumBlocks];
        for (int b = 0; b < maxNumBlocks; b++) {
            Peq[symbol][b] = 0;
            if (symbol < alphabetLength) { // NOTE: last symbol is wildcard, so it is all 0s
                for (int r = (b+1) * WORD_SIZE - 1; r >= b * WORD_SIZE; r--) {
                    Peq[symbol][b] <<= 1;
                    // NOTE: We pretend like query is padded at the end with W wildcard symbols
                    if (r < queryLength && query[r] != symbol)
                        Peq[symbol][b] += 1;
                }
            }
        }
    }
    
    // Initialize P, M and score
    for (int b = 0; b < currNumBlocks; b++) {
        score[b] = (b+1) * WORD_SIZE;
        P[b] = (Word)-1; // All 1s
        M[b] = (Word)0;
    }

    int bestScore = -1;
    for (int c = 0; c < targetLength + W; c++) { // for each column
        // We pretend like target is padded at end with W wildcard symbols 
        Word* Peq_c = c < targetLength ? Peq[target[c]] : Peq[alphabetLength];
        
        //----------------------- Calculate column -------------------------//
        int hout = 0;
        for (int b = 0; b < currNumBlocks; b++) {
            hout = calculateBlock(P[b], M[b], Peq_c[b], hout, P[b], M[b]);
            score[b] += hout;
        }
        //------------------------------------------------------------------//

        //---------- Adjust number of blocks according to Ukkonen ----------//
        if ((score[currNumBlocks-1] - hout <= k) && (currNumBlocks < maxNumBlocks)
            && ((Peq_c[currNumBlocks] & (Word)1) || hout < 0)) {
            // Calculate one more block
            currNumBlocks++;
            int b = currNumBlocks-1; // index of last block (one we just added)
            P[b] = (Word)-1; // All 1s
            M[b] = (Word)0;
            score[b] = score[b-1] - hout + WORD_SIZE + calculateBlock(P[b], M[b], Peq_c[b], hout, P[b], M[b]);
        }
        else
            while (score[currNumBlocks-1] >= k + WORD_SIZE)
                currNumBlocks--;
        //------------------------------------------------------------------//

        //------------------------- Update best score ----------------------//
        if (currNumBlocks == maxNumBlocks) {
            int colScore = score[maxNumBlocks-1];
            if (colScore <= k) { // Scores > k dont have correct values (so we cannot use them), but are certainly > k. 
                // NOTE: Score that I find in column c is actually score from column c-W
                if (bestScore == -1 || colScore < bestScore)
                    bestScore = colScore;
            }
        }   // TODO: What about scores from first W columns, ignore them? They do not have good scores in them for sure
        //------------------------------------------------------------------//
    }

    //--- Free memory ---//
    delete[] P;
    delete[] M;
    delete[] score;
    for (int i = 0; i < alphabetLength+1; i++)
        delete[] Peq[i];
    delete[] Peq;
    //-------------------//
    
    return bestScore;
}
