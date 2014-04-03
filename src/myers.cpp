#include "myers.h"

#include <stdint.h>

using namespace std;

typedef uint64_t Word;
static const int WORD_SIZE = sizeof(Word) * 8; // Size of Word in bits
static const Word HIGH_BIT_MASK = ((Word)1) << (WORD_SIZE-1);


static int myersCalcEditDistanceSemiGlobal(Word* P, Word* M, int* score, Word** Peq, int W, int maxNumBlocks,
                                           const unsigned char* query, int queryLength,
                                           const unsigned char* target, int targetLength,
                                           int alphabetLength, int k, int mode, int* bestScore, int* position);
static int myersCalcEditDistanceNW(Word* P, Word* M, int* score, Word** Peq, int W, int maxNumBlocks,
                                   const unsigned char* query, int queryLength,
                                   const unsigned char* target, int targetLength,
                                   int alphabetLength, int k, int* bestScore, int* position); 

static inline int ceilDiv(int x, int y);



int myersCalcEditDistance(const unsigned char* query, int queryLength,
                          const unsigned char* target, int targetLength,
                          int alphabetLength, int k, int mode, int* bestScore, int* position) {
    
    /*--------------------- INITIALIZATION ------------------*/
    int maxNumBlocks = ceilDiv(queryLength, WORD_SIZE); // bmax in Myers

    Word* P = new Word[maxNumBlocks]; // Contains Pvin for each block (column is divided into blocks)
    Word* M = new Word[maxNumBlocks]; // Contains Mvin for each block
    int* score = new int[maxNumBlocks]; // Contains score for each block
    Word** Peq = new Word*[alphabetLength+1]; // [alphabetLength+1][maxNumBlocks]. Last symbol is wildcard.
    
    int W = maxNumBlocks * WORD_SIZE - queryLength; // number of redundant cells in last level blocks

    // Build Peq (1 is match, 0 is mismatch). NOTE: last column is wildcard(symbol that matches anything) with just 1s
    for (int symbol = 0; symbol <= alphabetLength; symbol++) {
        Peq[symbol] = new Word[maxNumBlocks];
        for (int b = 0; b < maxNumBlocks; b++) {
            if (symbol < alphabetLength) {
                Peq[symbol][b] = 0;
                for (int r = (b+1) * WORD_SIZE - 1; r >= b * WORD_SIZE; r--) {
                    Peq[symbol][b] <<= 1;
                    // NOTE: We pretend like query is padded at the end with W wildcard symbols
                    if (r >= queryLength || query[r] == symbol)
                        Peq[symbol][b] += 1;
                }
            } else { // Last symbol is wildcard, so it is all 1s
                Peq[symbol][b] = (Word)-1;
            }
        }
    }
    /*-------------------------------------------------------*/

    
    /*------------------ MAIN CALCULATION -------------------*/
    *bestScore = -1;
    *position = -1;
    if (k < 0) { // If valid k is not given, auto-adjust k until solution is found.
        k = WORD_SIZE; // Gives better results then smaller k
        while (*bestScore == -1) {
            if (mode == MYERS_MODE_HW || mode == MYERS_MODE_SHW)
                myersCalcEditDistanceSemiGlobal(P, M, score, Peq, W, maxNumBlocks,
                                                query, queryLength, target, targetLength,
                                                alphabetLength, k, mode, bestScore, position);
            else  // mode == MYERS_MODE_NW
                myersCalcEditDistanceNW(P, M, score, Peq, W, maxNumBlocks,
                                        query, queryLength, target, targetLength,
                                        alphabetLength, k, bestScore, position);
            k *= 2;
        }
    } else {
        if (mode == MYERS_MODE_HW || mode == MYERS_MODE_SHW)
            myersCalcEditDistanceSemiGlobal(P, M, score, Peq, W, maxNumBlocks,
                                            query, queryLength, target, targetLength,
                                            alphabetLength, k, mode, bestScore, position);
        else  // mode == MYERS_MODE_NW
            myersCalcEditDistanceNW(P, M, score, Peq, W, maxNumBlocks,
                                    query, queryLength, target, targetLength,
                                    alphabetLength, k, bestScore, position);
    }
    /*-------------------------------------------------------*/
    
    //--- Free memory ---//
    delete[] P;
    delete[] M;
    delete[] score;
    for (int i = 0; i < alphabetLength+1; i++)
        delete[] Peq[i];
    delete[] Peq;
    //-------------------//

    return MYERS_STATUS_OK;
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
static inline int calculateBlock(Word Pv, Word Mv, Word Eq, const int hin,
                                 Word &PvOut, Word &MvOut) {
    Word Xv = Eq | Mv;
    if (hin < 0)
        Eq |= (Word)1;
    Word Xh = (((Eq & Pv) + Pv) ^ Pv) | Eq;

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
    return x % y ? x / y + 1 : x / y;
}

static inline int min(int x, int y) {
    return x < y ? x : y;
}

static inline int max(int x, int y) {
    return x > y ? x : y;
}

static inline int abs(int x) {
    return x < 0 ? -1 * x : x;
}

/**
 * @param [in] mode  MYERS_MODE_HW or MYERS_MODE_SHW
 */
static int myersCalcEditDistanceSemiGlobal(Word* P, Word* M, int* score, Word** Peq, int W, int maxNumBlocks,
                                           const unsigned char* query, int queryLength,
                                           const unsigned char* target, int targetLength,
                                           int alphabetLength, int k, int mode, int* bestScore_, int* position_) {
    // firstBlock is 0-based index of first block in Ukkonen band.
    // lastBlock is 0-based index of block AFTER last block in Ukkonen band. <- WATCH OUT!
    int firstBlock = 0;
    int lastBlock = min(ceilDiv(k + 1, WORD_SIZE), maxNumBlocks); // y in Myers
    
    // Initialize P, M and score
    for (int b = 0; b < lastBlock; b++) {
        score[b] = (b+1) * WORD_SIZE;
        P[b] = (Word)-1; // All 1s
        M[b] = (Word)0;
    }

    int bestScore = -1;
    int position = -1;
    for (int c = 0; c < targetLength + W; c++) { // for each column
        // We pretend like target is padded at end with W wildcard symbols 
        Word* Peq_c = c < targetLength ? Peq[target[c]] : Peq[alphabetLength];

        //----------------------- Calculate column -------------------------//
        int hout = mode == MYERS_MODE_HW ? 0 : 1; // If 0 then gap before query is not penalized
        for (int b = firstBlock; b < lastBlock; b++) {
            hout = calculateBlock(P[b], M[b], Peq_c[b], hout, P[b], M[b]);
            score[b] += hout;
        }
        //------------------------------------------------------------------//

        //---------- Adjust number of blocks according to Ukkonen ----------//
        if (mode != MYERS_MODE_HW)
            if (score[firstBlock] >= k + WORD_SIZE) {
                firstBlock++;
            }
        
        if ((score[lastBlock-1] - hout <= k) && (lastBlock < maxNumBlocks)
            && ((Peq_c[lastBlock] & (Word)1) || hout < 0)) {
            // If score of left block is not too big, calculate one more block
            lastBlock++;
            int b = lastBlock-1; // index of last block (one we just added)
            P[b] = (Word)-1; // All 1s
            M[b] = (Word)0;
            score[b] = score[b-1] - hout + WORD_SIZE + calculateBlock(P[b], M[b], Peq_c[b], hout, P[b], M[b]);
        } else {
            while (lastBlock > 0 && score[lastBlock-1] >= k + WORD_SIZE)
                lastBlock--;
        }
        
        // If band stops to exist finish
        if (lastBlock <= firstBlock) {
            *bestScore_ = bestScore;
            *position_ = position;
            return MYERS_STATUS_OK;
        }
        //------------------------------------------------------------------//

        //------------------------- Update best score ----------------------//
        if (c >= W && lastBlock == maxNumBlocks) { // We ignore scores from first W columns, they are not relevant.
            int colScore = score[maxNumBlocks-1];
            if (colScore <= k) { // Scores > k dont have correct values (so we cannot use them), but are certainly > k. 
                // NOTE: Score that I find in column c is actually score from column c-W
                if (bestScore == -1 || colScore < bestScore) {
                    bestScore = colScore;
                    position = c - W;
                    k = bestScore - 1; // Change k so we will look only for better scores then the best found so far.
                }
            }
        }
        //------------------------------------------------------------------//
    }

    *bestScore_ = bestScore;
    *position_ = position;
    return MYERS_STATUS_OK;
}





static int myersCalcEditDistanceNW(Word* P, Word* M, int* score, Word** Peq, int W, int maxNumBlocks,
                                   const unsigned char* query, int queryLength,
                                   const unsigned char* target, int targetLength,
                                   int alphabetLength, int k, int* bestScore_, int* position_) {
    targetLength += W;
    queryLength += W;
    
    if (k < abs(targetLength - queryLength)) {
        *bestScore_ = *position_ = -1;
        return MYERS_STATUS_OK;
    }

    // firstBlock is 0-based index of first block in Ukkonen band.
    // lastBlock is 0-based index of block AFTER last block in Ukkonen band. <- WATCH OUT!
    int firstBlock = 0;
    // This is optimal now, by my formula.
    int lastBlock = min(maxNumBlocks, ceilDiv(min(k, (k + queryLength - targetLength) / 2) + 1, WORD_SIZE)); // y in Myers

    // Initialize P, M and score
    for (int b = 0; b < lastBlock; b++) {
        score[b] = (b+1) * WORD_SIZE;
        P[b] = (Word)-1; // All 1s
        M[b] = (Word)0;
    }

    for (int c = 0; c < targetLength; c++) { // for each column
        Word* Peq_c = c < targetLength - W ? Peq[target[c]] : Peq[alphabetLength];

        //----------------------- Calculate column -------------------------//
        int hout = 1;
        for (int b = firstBlock; b < lastBlock; b++) {
            hout = calculateBlock(P[b], M[b], Peq_c[b], hout, P[b], M[b]);
            score[b] += hout;
        }
        //------------------------------------------------------------------//
        
        //---------- Adjust number of blocks according to Ukkonen ----------//        
        // Adjust first block - this is optimal now, by my formula.
        // While outside of band, advance block
        while (score[firstBlock] >= k + WORD_SIZE
               || (firstBlock + 1) * WORD_SIZE - 1 < score[firstBlock] - k - targetLength + queryLength + c) {
            firstBlock++;
        }

        // if (firstBlock == lastBlock)
        //    printf("Ohohoho\n");
        
        //--- Adjust last block ---//
        // While block is not beneath band, calculate next block
        while (lastBlock < maxNumBlocks
               && (firstBlock == lastBlock // If above band
                   || !(score[lastBlock - 1] >= k + WORD_SIZE
                        || (lastBlock * WORD_SIZE - 1 > k - score[lastBlock - 1] + 2 * WORD_SIZE - 2 - targetLength + c + queryLength)))
               ) {
            lastBlock++;
            int b = lastBlock-1; // index of last block (one we just added)
            P[b] = (Word)-1; // All 1s
            M[b] = (Word)0;
            int newHout = calculateBlock(P[b], M[b], Peq_c[b], hout, P[b], M[b]);
            score[b] = score[b-1] - hout + WORD_SIZE + newHout;
            hout = newHout;
        }
        
        // While block is out of band, move one block up. - This is optimal now, by my formula.
        // NOT WORKING!
        /*while (lastBlock > 0
               && (score[lastBlock - 1] >= k + WORD_SIZE
                   || (lastBlock * WORD_SIZE - 1 > k - score[lastBlock - 1] + 2 * WORD_SIZE - 2 - targetLength + c + queryLength))) {
            lastBlock--;    // PROBLEM: Cini se da cesto/uvijek smanji za 1 previse!
            }*/
        
        
        while (lastBlock > 0 && score[lastBlock-1] >= k + WORD_SIZE) { // TODO: put some stronger constraint
            lastBlock--;
        }
        //-------------------------//

        // If band stops to exist finish
        if (lastBlock <= firstBlock) {
            //printf("Stopped to exist\n");
            *bestScore_ = *position_ = -1;
            return MYERS_STATUS_OK;
        }
        //------------------------------------------------------------------//
        
    }

    if (lastBlock == maxNumBlocks) { // If last block of last column was calculated
        int bestScore = score[maxNumBlocks-1];
        /*
        for (int i = 0; i < W; i++) {
            if (P[maxNumBlocks-1] & HIGH_BIT_MASK)
                bestScore--;
            if (M[maxNumBlocks-1] & HIGH_BIT_MASK)
                bestScore++;
            P[maxNumBlocks-1] <<= 1;
            M[maxNumBlocks-1] <<= 1;
        }
        */
        if (bestScore <= k) {
            *bestScore_ = bestScore;
            *position_ = targetLength - 1 - W;
            return MYERS_STATUS_OK;
        }
    }
    
    *bestScore_ = *position_ = -1;
    return MYERS_STATUS_OK;
}
