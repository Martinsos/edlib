#include "myers.h"

#include <stdint.h>
#include <cstdlib>
#include <algorithm>
#include <cstdio>

using namespace std;

typedef uint64_t Word;
static const int WORD_SIZE = sizeof(Word) * 8; // Size of Word in bits
static const Word WORD_1 = (Word)1;
static const Word HIGH_BIT_MASK = WORD_1 << (WORD_SIZE - 1);

// Data needed to find alignment.
struct AlignmentData {
    Word* Ps;
    Word* Ms;
    int* scores;
    int* firstBlocks;
    int* lastBlocks;

    AlignmentData(int maxNumBlocks, int targetLength) {
         Ps     = new Word[maxNumBlocks * targetLength];
         Ms     = new Word[maxNumBlocks * targetLength];
         scores = new  int[maxNumBlocks * targetLength];
         firstBlocks = new int[targetLength];
         lastBlocks  = new int[targetLength];
    }

    ~AlignmentData() {
        delete[] Ps;
        delete[] Ms;
        delete[] scores;
        delete[] firstBlocks;
        delete[] lastBlocks;
    }
};


static int myersCalcEditDistanceSemiGlobal(Word* P, Word* M, int* score, Word* Peq, int W, int maxNumBlocks,
                                           const unsigned char* query, int queryLength,
                                           const unsigned char* target, int targetLength,
                                           int alphabetLength, int k, int mode, int* bestScore, int* position);

static int myersCalcEditDistanceNW(Word* P, Word* M, int* score, Word* Peq, int W, int maxNumBlocks,
                                   const unsigned char* query, int queryLength,
                                   const unsigned char* target, int targetLength,
                                   int alphabetLength, int k, int* bestScore, int* position,
                                   bool findAlignment, AlignmentData** alignData);

static void obtainAlignment(int maxNumBlocks, int queryLength, int targetLength, int W, int bestScore,
                            int position, AlignmentData* alignData, 
                            unsigned char** alignment, int* alignmentLength);

static inline int ceilDiv(int x, int y);

static inline unsigned char* createReverseCopy(const unsigned char* seq, int length);

static inline Word* buildPeq(int alphabetLength, const unsigned char* query, int queryLength);



/**
 * Entry function.
 */
int myersCalcEditDistance(const unsigned char* query, int queryLength,
                          const unsigned char* target, int targetLength,
                          int alphabetLength, int k, int mode,
                          int* bestScore, int* position, 
                          bool findAlignment, unsigned char** alignment, int* alignmentLength) {
    *alignment = NULL;
    /*--------------------- INITIALIZATION ------------------*/
    int maxNumBlocks = ceilDiv(queryLength, WORD_SIZE); // bmax in Myers
    int W = maxNumBlocks * WORD_SIZE - queryLength; // number of redundant cells in last level blocks

    Word* P = new Word[maxNumBlocks]; // Contains Pvin for each block (column is divided into blocks)
    Word* M = new Word[maxNumBlocks]; // Contains Mvin for each block
    int* score = new int[maxNumBlocks]; // Contains score for each block
    Word* Peq = buildPeq(alphabetLength, query, queryLength);
    /*-------------------------------------------------------*/

    
    /*------------------ MAIN CALCULATION -------------------*/
    // TODO: Store alignment data only after k is determined? That could make things faster
    *bestScore = -1;
    *position = -1;
    AlignmentData* alignData = NULL;
    bool dynamicK = false;
    if (k < 0) { // If valid k is not given, auto-adjust k until solution is found.
        dynamicK = true;
        k = WORD_SIZE; // Gives better results then smaller k
    }

    do {
        if (alignData) delete alignData;
        if (mode == MYERS_MODE_HW || mode == MYERS_MODE_SHW)
            myersCalcEditDistanceSemiGlobal(P, M, score, Peq, W, maxNumBlocks,
                                            query, queryLength, target, targetLength,
                                            alphabetLength, k, mode, bestScore, position);
        else  // mode == MYERS_MODE_NW
            myersCalcEditDistanceNW(P, M, score, Peq, W, maxNumBlocks,
                                    query, queryLength, target, targetLength,
                                    alphabetLength, k, bestScore, position,
                                    findAlignment, &alignData);
        k *= 2;
    } while(dynamicK && *bestScore == -1);
    
    // Finding alignment -> all comes down to finding alignment for NW
    if (findAlignment && *bestScore >= 0) {
        if (mode != MYERS_MODE_NW) {
            int targetStart;
            int targetEnd = *position;
            if (mode == MYERS_MODE_HW) { // If HW, I need to find target start
                const unsigned char* rTarget = createReverseCopy(target, *position + 1);
                const unsigned char* rQuery  = createReverseCopy(query, queryLength);
                Word* rPeq = buildPeq(alphabetLength, rQuery, queryLength); // Peq for reversed query
                int bestScoreSHW, positionSHW;
                myersCalcEditDistanceSemiGlobal(P, M, score, rPeq, W, maxNumBlocks,
                                                rQuery, queryLength, rTarget, *position + 1,
                                                alphabetLength, *bestScore, MYERS_MODE_SHW,
                                                &bestScoreSHW, &positionSHW);
                targetStart = targetEnd - positionSHW;
                delete[] rTarget;
                delete[] rQuery;
                delete[] rPeq;
            }
            if (mode == MYERS_MODE_SHW) {
                targetStart = 0;
            }
            int alnBestScore, alnPosition;
            myersCalcEditDistanceNW(P, M, score, Peq, W, maxNumBlocks,
                                    query, queryLength, target + targetStart, targetEnd - targetStart + 1,
                                    alphabetLength, *bestScore, &alnBestScore, &alnPosition,
                                    true, &alignData);
            obtainAlignment(maxNumBlocks, queryLength, targetEnd - targetStart + 1, W, alnBestScore, 
                            alnPosition, alignData, alignment, alignmentLength);
        } else {
            obtainAlignment(maxNumBlocks, queryLength, targetLength, W, *bestScore, 
                            *position, alignData, alignment, alignmentLength);
        }
    }
    /*-------------------------------------------------------*/

    
    //--- Free memory ---//
    if (alignData) delete alignData;
    delete[] P;
    delete[] M;
    delete[] score;
    delete[] Peq;
    //-------------------//

    return MYERS_STATUS_OK;
}


/**
 * Build Peq table for given query and alphabet.
 * Peq is table of dimensions alphabetLength+1 x maxNumBlocks.
 * Bit i of Peq[s * maxNumBlocks + b] is 1 if i-th symbol from block b of query equals symbol s, otherwise it is 0.
 * NOTICE: free returned array with delete[]!
 */
static inline Word* buildPeq(int alphabetLength, const unsigned char* query, int queryLength) {
    int maxNumBlocks = ceilDiv(queryLength, WORD_SIZE);
    int W = maxNumBlocks * WORD_SIZE - queryLength; // number of redundant cells in last level blocks
    // table of dimensions alphabetLength+1 x maxNumBlocks. Last symbol is wildcard.
    Word* Peq = new Word[(alphabetLength + 1) * maxNumBlocks];

    // Build Peq (1 is match, 0 is mismatch). NOTE: last column is wildcard(symbol that matches anything) with just 1s
    for (int symbol = 0; symbol <= alphabetLength; symbol++) {
        for (int b = 0; b < maxNumBlocks; b++) {
            if (symbol < alphabetLength) {
                Peq[symbol * maxNumBlocks + b] = 0;
                for (int r = (b+1) * WORD_SIZE - 1; r >= b * WORD_SIZE; r--) {
                    Peq[symbol * maxNumBlocks + b] <<= 1;
                    // NOTE: We pretend like query is padded at the end with W wildcard symbols
                    if (r >= queryLength || query[r] == symbol)
                        Peq[symbol * maxNumBlocks + b] += 1;
                }
            } else { // Last symbol is wildcard, so it is all 1s
                Peq[symbol * maxNumBlocks + b] = (Word)-1;
            }
        }
    }

    return Peq;
}


/**
 * Returns new sequence that is reverse of given sequence.
 */
static inline unsigned char* createReverseCopy(const unsigned char* seq, int length) {
    unsigned char* rSeq = new unsigned char[length];
    for (int i = 0; i < length; i++) {
        rSeq[i] = seq[length - i - 1];
    }
    return rSeq;
}



/**
 * Corresponds to Advance_Block function from Myers.
 * Calculates one word(block), which is part of a column.
 * Highest bit of word (one most to the left) is most bottom cell of block from column.
 * Pv[i] and Mv[i] define vin of cell[i]: vin = cell[i] - cell[i-1].
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
    // hin can be 1, -1 or 0.
    // 1  -> 00...01
    // 0  -> 00...00
    // -1 -> 11...11 (2-complement)

    Word hinIsNeg = (Word)(hin >> 2) & WORD_1; // 00...001 if hin is -1, 00...000 if 0 or 1

    Word Xv = Eq | Mv;
    // This is instruction below written using 'if': if (hin < 0) Eq |= (Word)1;
    Eq |= hinIsNeg;
    Word Xh = (((Eq & Pv) + Pv) ^ Pv) | Eq;

    Word Ph = Mv | ~(Xh | Pv);
    Word Mh = Pv & Xh;

    int hout = 0;
    // This is instruction below written using 'if': if (Ph & HIGH_BIT_MASK) hout = 1;
    hout = (Ph & HIGH_BIT_MASK) >> (WORD_SIZE - 1);
    // This is instruction below written using 'if': if (Mh & HIGH_BIT_MASK) hout = -1;
    hout -= (Mh & HIGH_BIT_MASK) >> (WORD_SIZE - 1);

    Ph <<= 1;
    Mh <<= 1;

    // This is instruction below written using 'if': if (hin < 0) Mh |= (Word)1;
    Mh |= hinIsNeg;
    // This is instruction below written using 'if': if (hin > 0) Ph |= (Word)1;
    Ph |= (Word)((hin + 1) >> 1);

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




/**
 * @param [in] mode  MYERS_MODE_HW or MYERS_MODE_SHW
 */
static int myersCalcEditDistanceSemiGlobal(Word* P, Word* M, int* score, Word* Peq, int W, int maxNumBlocks,
                                           const unsigned char* query, int queryLength,
                                           const unsigned char* target, int targetLength,
                                           int alphabetLength, int k, int mode, int* bestScore_, int* position_) {
    // firstBlock is 0-based index of first block in Ukkonen band.
    // lastBlock is 0-based index of last block in Ukkonen band.
    int firstBlock = 0;
    int lastBlock = min(ceilDiv(k + 1, WORD_SIZE), maxNumBlocks) - 1; // y in Myers
    
    // Initialize P, M and score
    for (int b = 0; b <= lastBlock; b++) {
        score[b] = (b+1) * WORD_SIZE;
        P[b] = (Word)-1; // All 1s
        M[b] = (Word)0;
    }

    int bestScore = -1;
    int position = -1;
    for (int c = 0; c < targetLength + W; c++) { // for each column
        // We pretend like target is padded at end with W wildcard symbols 
        Word* Peq_c = c < targetLength ? Peq + target[c] * maxNumBlocks : Peq + alphabetLength * maxNumBlocks;

        //----------------------- Calculate column -------------------------//
        int hout = mode == MYERS_MODE_HW ? 0 : 1; // If 0 then gap before query is not penalized
        for (int b = firstBlock; b <= lastBlock; b++) {
            hout = calculateBlock(P[b], M[b], Peq_c[b], hout, P[b], M[b]);
            score[b] += hout;
        }
        //------------------------------------------------------------------//

        //---------- Adjust number of blocks according to Ukkonen ----------//
        if (mode != MYERS_MODE_HW)
            if (score[firstBlock] >= k + WORD_SIZE) {
                firstBlock++;
            }
        
        if ((score[lastBlock] - hout <= k) && (lastBlock < maxNumBlocks - 1)
            && ((Peq_c[lastBlock + 1] & (Word)1) || hout < 0)) {
            // If score of left block is not too big, calculate one more block
            lastBlock++;
            int b = lastBlock; // index of last block (one we just added)
            P[b] = (Word)-1; // All 1s
            M[b] = (Word)0;
            score[b] = score[b-1] - hout + WORD_SIZE + calculateBlock(P[b], M[b], Peq_c[b], hout, P[b], M[b]);
        } else {
            while (lastBlock >= 0 && score[lastBlock] >= k + WORD_SIZE)
                lastBlock--;
        }

        // If band stops to exist finish
        if (lastBlock < firstBlock) {
            *bestScore_ = bestScore;
            *position_ = position;
            return MYERS_STATUS_OK;
        }
        //------------------------------------------------------------------//

        //------------------------- Update best score ----------------------//
        if (c >= W && lastBlock == maxNumBlocks - 1) { // We ignore scores from first W columns, they are not relevant.
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




/**
 * @param alignData  Data generated during calculation, that is needed for reconstruction of alignment.
 *                   I it is allocated with new, so free it with delete.
 *                   Data is generated only if findAlignment is true.
 */
static int myersCalcEditDistanceNW(Word* P, Word* M, int* score, Word* Peq, int W, int maxNumBlocks,
                                   const unsigned char* query, int queryLength,
                                   const unsigned char* target, int targetLength,
                                   int alphabetLength, int k, int* bestScore_, int* position_,
                                   bool findAlignment, AlignmentData** alignData) {
    targetLength += W;
    int queryLengthPadded = queryLength + W;
    
    if (k < abs(targetLength - queryLengthPadded)) {
        *bestScore_ = *position_ = -1;
        return MYERS_STATUS_OK;
    }

    k = min(k, max(queryLengthPadded, targetLength));  // Upper bound for k

    // firstBlock is 0-based index of first block in Ukkonen band.
    // lastBlock is 0-based index of last block in Ukkonen band.
    int firstBlock = 0;
    // This is optimal now, by my formula.
    int lastBlock = min(maxNumBlocks, ceilDiv(min(k, (k + queryLengthPadded - targetLength) / 2) + 1, WORD_SIZE)) - 1;


    // Initialize P, M and score
    for (int b = 0; b <= lastBlock; b++) {
        score[b] = (b+1) * WORD_SIZE;
        P[b] = (Word)-1; // All 1s
        M[b] = (Word)0;
    }

    // If we want to find alignment, we have to store needed data.
    if (findAlignment)
        *alignData = new AlignmentData(maxNumBlocks, targetLength - W);
    else
        *alignData = NULL;

    for (int c = 0; c < targetLength; c++) { // for each column
        Word* Peq_c = c < targetLength - W ? Peq + target[c] * maxNumBlocks : Peq + alphabetLength * maxNumBlocks;

        //----------------------- Calculate column -------------------------//
        int hout = 1;
        for (int b = firstBlock; b <= lastBlock; b++) {
            hout = calculateBlock(P[b], M[b], Peq_c[b], hout, P[b], M[b]);
            score[b] += hout;
        }
        //------------------------------------------------------------------//

        // Update k. I do it only on end of column because it would slow calculation too much otherwise.
        k = min(k, score[lastBlock] + max(targetLength - c - 1,
                                          queryLengthPadded - ((1 + lastBlock) * WORD_SIZE - 1) - 1));
        
        //---------- Adjust number of blocks according to Ukkonen ----------//
        //--- Adjust last block ---//
        // If block is not beneath band, calculate next block. Only next because others are certainly beneath band.
        if (lastBlock + 1 < maxNumBlocks
            && !(//score[lastBlock] >= k + WORD_SIZE ||  // NOTICE: this condition could be satisfied if above block also!
                 ((lastBlock + 1) * WORD_SIZE - 1
                  > k - score[lastBlock] + 2 * WORD_SIZE - 2 - targetLength + c + queryLengthPadded))) {
            lastBlock++;
            int b = lastBlock; // index of last block (one we just added)
            P[b] = (Word)-1; // All 1s
            M[b] = (Word)0;
            int newHout = calculateBlock(P[b], M[b], Peq_c[b], hout, P[b], M[b]);
            score[b] = score[b-1] - hout + WORD_SIZE + newHout;
            hout = newHout;
        }
        
        // While block is out of band, move one block up. - This is optimal now, by my formula.
        // NOT WORKING!
        /*while (lastBlock  >= 0
               && (score[lastBlock] >= k + WORD_SIZE
                   || ((lastBlock + 1) * WORD_SIZE - 1 > k - score[lastBlock] + 2 * WORD_SIZE - 2 - targetLength + c + queryLengthPadded))) {
            lastBlock--;    // PROBLEM: Cini se da cesto/uvijek smanji za 1 previse!
            }*/
        
        
        while (lastBlock >= firstBlock && score[lastBlock] >= k + WORD_SIZE) { // TODO: put some stronger constraint
            lastBlock--;
        }
        //-------------------------//

        //--- Adjust first block ---//
        // While outside of band, advance block
        while (firstBlock <= lastBlock &&
               (score[firstBlock] >= k + WORD_SIZE
                || (firstBlock + 1) * WORD_SIZE - 1 < score[firstBlock] - k - targetLength + queryLengthPadded + c)) {
            firstBlock++;
        }
        //--------------------------//

        // If band stops to exist finish
        if (lastBlock < firstBlock) {
            *bestScore_ = *position_ = -1;
            return MYERS_STATUS_OK;
        }
        //------------------------------------------------------------------//
        

        //---- Save column so it can be used for reconstruction ----//
        if (findAlignment && c < targetLength - W) {
            for (int b = firstBlock; b <= lastBlock; b++) {
                (*alignData)->Ps[maxNumBlocks * c + b] = P[b];
                (*alignData)->Ms[maxNumBlocks * c + b] = M[b];
                (*alignData)->scores[maxNumBlocks * c + b] = score[b];
                (*alignData)->firstBlocks[c] = firstBlock;
                (*alignData)->lastBlocks[c] = lastBlock;
            }
        }
        //----------------------------------------------------------//
    }

    if (lastBlock == maxNumBlocks - 1) { // If last block of last column was calculated
        int bestScore = score[maxNumBlocks-1];
        /*int bestScore = score[lastBlock];
        Word P_ = P[lastBlock];
        Word M_ = M[lastBlock];
        printf("%d\n", W);
        for (int i = 0; i < W; i++) {
            if (P_ & HIGH_BIT_MASK) bestScore--;
            if (M_ & HIGH_BIT_MASK) bestScore++;
            P_ <<= 1;
            M_ <<= 1;
            }*/
        if (bestScore <= k) {
            *bestScore_ = bestScore;
            *position_ = targetLength - 1 - W;
            return MYERS_STATUS_OK;
        }
    }
    

    *bestScore_ = *position_ = -1;
    return MYERS_STATUS_OK;
}


/**
 * Finds one possible alignment that gives optimal score.
 * @param [in] maxNumBlocks
 * @param [in] queryLength  Normal length, without W.
 * @param [in] targetLength  Normal length, without W.
 * @param [in] W  Padding.
 * @param [in] bestScore  Best score.
 * @param [in] position  Position in target where best score was found.
 * @param [in] alignData  Data obtained during finding best score that is useful for finding alignment.
 * @param [out] alignment  Alignment.
 * @param [out] alignmentLength  Length of alignment.
 */
static void obtainAlignment(int maxNumBlocks, int queryLength, int targetLength,
                            int W, int bestScore, int position, AlignmentData* alignData,
                            unsigned char** alignment, int* alignmentLength) {
    // TODO: reallocate when done
    *alignment = (unsigned char*) malloc((queryLength + targetLength - 1) * sizeof(unsigned char));
    *alignmentLength = 0;
    int c = position; // index of column
    int b = maxNumBlocks - 1; // index of block in column
    int currScore = bestScore; // Score of current cell
    int lScore  = -1; // Score of left cell
    int uScore  = -1; // Score of upper cell
    int ulScore = -1; // Score of upper left cell
    Word currP = alignData->Ps[c * maxNumBlocks + b]; // P of current block
    Word currM = alignData->Ms[c * maxNumBlocks + b]; // M of current block
    // True if block to left exists and is in band
    bool thereIsLeftBlock = c > 0 && b >= alignData->firstBlocks[c-1] && b <= alignData->lastBlocks[c-1];
    Word lP, lM;
    if (thereIsLeftBlock) {
        lP = alignData->Ps[(c - 1) * maxNumBlocks + b]; // P of block to the left
        lM = alignData->Ms[(c - 1) * maxNumBlocks + b]; // M of block to the left
    }
    currP <<= W;
    currM <<= W;
    int blockPos = WORD_SIZE - W - 1; // 0 based index of current cell in blockPos
    if (c == 0) {
         thereIsLeftBlock = true;
         lScore = b * WORD_SIZE + blockPos + 1;
         ulScore = lScore - 1;
    }
    while (true) {
        // TODO: improvement: calculate only those cells that are needed,
        //       for example if I calculate upper cell and can move up, 
        //       there is no need to calculate left and upper left cell
        //---------- Calculate scores ---------//
        if (lScore == -1 && thereIsLeftBlock) {
            lScore = alignData->scores[(c - 1) * maxNumBlocks + b]; // score of block to the left
            for (int i = 0; i < WORD_SIZE - blockPos - 1; i++) {
                if (lP & HIGH_BIT_MASK) lScore--;
                if (lM & HIGH_BIT_MASK) lScore++;
                lP <<= 1;
                lM <<= 1;
            }
        }
        if (ulScore == -1) {
            if (lScore != -1) {
                ulScore = lScore;
                if (lP & HIGH_BIT_MASK) ulScore--;
                if (lM & HIGH_BIT_MASK) ulScore++;
            } 
            else if (c > 0 && b-1 >= alignData->firstBlocks[c-1] && b-1 <= alignData->lastBlocks[c-1]) {
                // This is the case when upper left cell is last cell in block,
                // and block to left is not in band so lScore is -1.
                ulScore = alignData->scores[(c - 1) * maxNumBlocks + b - 1];
            }
        }
        if (uScore == -1) {
            uScore = currScore;
            if (currP & HIGH_BIT_MASK) uScore--;
            if (currM & HIGH_BIT_MASK) uScore++;
            currP <<= 1;
            currM <<= 1;
        }
        //-------------------------------------//

        // TODO: should I check if there is upper block?

        //-------------- Move --------------//
        // Move up - insertion to target - deletion from query
        if (uScore != -1 && uScore + 1 == currScore) {
            currScore = uScore;
            lScore = ulScore;
            uScore = ulScore = -1;
            if (blockPos == 0) { // If entering new (upper) block
                if (b == 0) { // If there are no cells above (only boundary cells)
                    (*alignment)[(*alignmentLength)++] = 1; // Move up
                    for (int i = 0; i < c + 1; i++) // Move left until end
                        (*alignment)[(*alignmentLength)++] = 2;
                    break;
                } else {
                    blockPos = WORD_SIZE - 1;
                    b--;
                    currP = alignData->Ps[c * maxNumBlocks + b];
                    currM = alignData->Ms[c * maxNumBlocks + b];
                    if (c > 0 && b >= alignData->firstBlocks[c-1] && b <= alignData->lastBlocks[c-1]) {
                        thereIsLeftBlock = true;
                        lP = alignData->Ps[(c - 1) * maxNumBlocks + b]; // TODO: improve this, too many operations
                        lM = alignData->Ms[(c - 1) * maxNumBlocks + b];
                    } else {
                        thereIsLeftBlock = false;
                    }
                }
            } else {
                blockPos--;
                lP <<= 1;
                lM <<= 1;
            }
            // Mark move
            (*alignment)[(*alignmentLength)++] = 1; // TODO: enumeration?
        }
        // Move left - deletion from target - insertion to query
        else if (lScore != -1 && lScore + 1 == currScore) {
            currScore = lScore;
            uScore = ulScore;
            lScore = ulScore = -1;
            c--;
            if (c == -1) { // If there are no cells to the left (only boundary cells)
                (*alignment)[(*alignmentLength)++] = 2; // Move left
                int numUp = b * WORD_SIZE + blockPos + 1;
                for (int i = 0; i < numUp; i++) // Move up until end
                    (*alignment)[(*alignmentLength)++] = 1;
                break;
            }
            currP = lP;
            currM = lM;
            if (c > 0 && b >= alignData->firstBlocks[c-1] && b <= alignData->lastBlocks[c-1]) {
                thereIsLeftBlock = true;
                lP = alignData->Ps[(c - 1) * maxNumBlocks + b];
                lM = alignData->Ms[(c - 1) * maxNumBlocks + b];
            } else {
                if (c == 0) { // If there are no cells to the left (only boundary cells)
                    thereIsLeftBlock = true;
                    lScore = b * WORD_SIZE + blockPos + 1;
                    ulScore = lScore - 1;
                } else {
                    thereIsLeftBlock = false;
                }
            }
            // Mark move
            (*alignment)[(*alignmentLength)++] = 2;
        }
        // Move up left - (mis)match
        else if (ulScore != -1) {
            currScore = ulScore;
            uScore = lScore = ulScore = -1;
            c--;
            if (c == -1) { // If there are no cells to the left (only boundary cells)
                (*alignment)[(*alignmentLength)++] = 0; // Move left
                int numUp = b * WORD_SIZE + blockPos;
                for (int i = 0; i < numUp; i++) // Move up until end
                    (*alignment)[(*alignmentLength)++] = 1;
                break;
            }
            if (blockPos == 0) { // If entering upper left block
                if (b == 0) { // If there are no more cells above (only boundary cells)
                    (*alignment)[(*alignmentLength)++] = 0; // Move up left
                    for (int i = 0; i < c + 1; i++) // Move left until end
                        (*alignment)[(*alignmentLength)++] = 2;
                    break;
                }
                blockPos = WORD_SIZE - 1;
                b--;
                currP = alignData->Ps[c * maxNumBlocks + b];
                currM = alignData->Ms[c * maxNumBlocks + b];
            } else { // If entering left block
                blockPos--;
                currP = lP;
                currM = lM;
                currP <<= 1;
                currM <<= 1;
            }
            // Set new left block
            if (c > 0 && b >= alignData->firstBlocks[c-1] && b <= alignData->lastBlocks[c-1]) {
                thereIsLeftBlock = true;
                lP = alignData->Ps[(c - 1) * maxNumBlocks + b];
                lM = alignData->Ms[(c - 1) * maxNumBlocks + b];
            } else {
                if (c == 0) { // If there are no cells to the left (only boundary cells)
                    thereIsLeftBlock = true;
                    lScore = b * WORD_SIZE + blockPos + 1;
                    ulScore = lScore - 1;
                } else {
                    thereIsLeftBlock = false;
                }
            }
            // Mark move
            (*alignment)[(*alignmentLength)++] = 0;
        } else {
            // Reached end - finished!
            break;
        }
        //----------------------------------//
    }   

    *alignment = (unsigned char*) realloc(*alignment, (*alignmentLength) * sizeof(unsigned char));
    reverse(*alignment, *alignment + (*alignmentLength));
    return;
}
