#include "myers.h"

#include <stdint.h>
#include <cstdlib>
#include <algorithm>
#include <cstdio>

extern "C" {
#include <immintrin.h> // SSE (AVX)
}

using namespace std;

//#undef __SSE4_1__

#ifdef __SSE4_1__
typedef __m128i  Word;
static const int WORD_SIZE = 128;
static const Word WORD_1 = _mm_set_epi32(0, 0, 0, 1);
static const Word WORD_0 = _mm_setzero_si128();
static const Word WORD_ALL_1 = _mm_set_epi32(-1, -1, -1, -1);
static const Word HIGH_BIT_MASK = _mm_set_epi32(1 << 31, 0, 0, 0);
#define TEST_ANY_BITS_SET(word, mask) (!_mm_test_all_zeros(word, mask))
static const Word SHIFT_LEFT_MASK = _mm_set_epi32(0, 0, 1 << 31, 0);
// Shift whole SSE register for one bit to the left
static inline __m128i mmShiftLeft(__m128i mm) {
    // I use mask to detect which 64bit words start with 1. These 1s are lost durin normal shift.
    // Therefore I shift this 1s to be the last bit in 64 bit word,
    // then shift all words to the left for one word and than `or` it with shifted mm.
    // That way I moved those overlfowing 1s to where they should be after shift.
    __m128i overflowBits = _mm_slli_si128(_mm_srli_epi64(mm & SHIFT_LEFT_MASK, 63), 8); // 8 is for 8 bytes
    return _mm_slli_epi64(mm, 1) | overflowBits;
}
static const Word SHIFT_RIGHT_MASK = _mm_set_epi32(0, 1, 0, 0);
// Shift whole SSE register for one bit to the right
static inline __m128i mmShiftRight(__m128i mm) {
    __m128i overflowBits = _mm_srli_si128(_mm_slli_epi64(mm & SHIFT_RIGHT_MASK, 63), 8); // 8 is for 8 bytes
    return _mm_srli_epi64(mm, 1) | overflowBits;
}
#else
typedef uint64_t Word;
static const int WORD_SIZE = sizeof(Word) * 8; // Size of Word in bits
static const Word WORD_1 = (Word)1; // 00..01
static const Word WORD_0 = (Word)0; // 00..00
static const Word WORD_ALL_1 = (Word)-1; // 11..11
static const Word HIGH_BIT_MASK = WORD_1 << (WORD_SIZE - 1);  // 10..00
#define TEST_ANY_BITS_SET(word, mask) (word & mask) // Return 0 if not even one bit from mask is set in word, otherwise return some value != 0.
#endif


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

struct Block {
    Word P;  // Pvin
    Word M;  // Mvin
    int score; // score of last cell in block;
};


// For debugging
// Print number as bits.
template<class T>
void print_bits(T x) {
    char result[sizeof(T) * 8];
    for (int i = sizeof(T) * 8 - 1; i >= 0; i--) {
        result[i] = (1 & x) ? '1' : '0';
        x >>= 1;
    };
    for (int i = 0; i < sizeof(T) * 8; i++) {
        printf("%c", result[i]);
    }
}

// For debugging
void print_mm128i(__m128i mm) {
    uint64_t unpacked[2] __attribute__((aligned(16)));
    _mm_store_si128((__m128i*)unpacked, mm);
    for (int i = 1; i >= 0; i--) {
        print_bits(unpacked[i]);
    }
}


static int myersCalcEditDistanceSemiGlobal(Block* blocks, Word* Peq, int W, int maxNumBlocks,
                                           const unsigned char* query, int queryLength,
                                           const unsigned char* target, int targetLength,
                                           int alphabetLength, int k, int mode, int* bestScore, int* position);

static int myersCalcEditDistanceNW(Block* blocks, Word* Peq, int W, int maxNumBlocks,
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
    print_mm128i(WORD_1); printf("\n");
    print_mm128i(WORD_1 << 1); printf("\n");
    print_mm128i(_mm_set_epi32(9, 5, 3, 1)); printf("\n");
    print_mm128i(_mm_set_epi32(9, 5, 3, 1) >> 1); printf("\n");
    print_mm128i(WORD_0); printf("\n");
    print_mm128i(WORD_0 << 1); printf("\n");
    print_mm128i(WORD_0 >> 1); printf("\n");
    print_mm128i(WORD_ALL_1); printf("\n");
    print_mm128i(WORD_ALL_1 << 1); printf("\n");
    print_mm128i(WORD_ALL_1 >> 1); printf("\n");
    print_mm128i(HIGH_BIT_MASK); printf("\n");
    print_mm128i(HIGH_BIT_MASK << 1); printf("\n");
    print_mm128i(HIGH_BIT_MASK >> 1); printf("\n");
    print_mm128i(WORD_ALL_1 + WORD_1); printf("\n");
    print_mm128i(_mm_set_epi32(0, 0, 1 << 31, 0)); printf("\n");
    print_mm128i(_mm_set_epi32(0, 0, 1 << 31, 0) << 1); printf("\n");
    print_mm128i(mmShiftLeft(_mm_set_epi32(0, 0, 1 << 31, 0))); printf("\n");
    print_mm128i(_mm_set_epi32(0, 1, 0, 0)); printf("\n");
    print_mm128i(_mm_set_epi32(0, 1, 0, 0) >> 1); printf("\n");
    print_mm128i(mmShiftRight(_mm_set_epi32(0, 1, 0, 0))); printf("\n");
    print_mm128i(mmShiftRight(_mm_set_epi32(-1, -1, 1 << 31, 0))); printf("\n");

    

    *alignment = NULL;
    /*--------------------- INITIALIZATION ------------------*/
    int maxNumBlocks = ceilDiv(queryLength, WORD_SIZE); // bmax in Myers
    int W = maxNumBlocks * WORD_SIZE - queryLength; // number of redundant cells in last level blocks

    Block* blocks = new Block[maxNumBlocks];
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
            myersCalcEditDistanceSemiGlobal(blocks, Peq, W, maxNumBlocks,
                                            query, queryLength, target, targetLength,
                                            alphabetLength, k, mode, bestScore, position);
        else  // mode == MYERS_MODE_NW
            myersCalcEditDistanceNW(blocks, Peq, W, maxNumBlocks,
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
                myersCalcEditDistanceSemiGlobal(blocks, rPeq, W, maxNumBlocks,
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
            myersCalcEditDistanceNW(blocks, Peq, W, maxNumBlocks,
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
    delete[] blocks;
    delete[] Peq;
    if (alignData) delete alignData;
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
    printf("Building Peq\n");
    int maxNumBlocks = ceilDiv(queryLength, WORD_SIZE);
    int W = maxNumBlocks * WORD_SIZE - queryLength; // number of redundant cells in last level blocks
    // table of dimensions alphabetLength+1 x maxNumBlocks. Last symbol is wildcard.
    Word* Peq = new Word[(alphabetLength + 1) * maxNumBlocks];

#ifdef __SSE4_1__
    const int numSegs = WORD_SIZE / 64;
#endif

    // Build Peq (1 is match, 0 is mismatch). NOTE: last column is wildcard(symbol that matches anything) with just 1s
    for (int symbol = 0; symbol <= alphabetLength; symbol++) {
        for (int b = 0; b < maxNumBlocks; b++) {
#ifdef __SSE4_1__
            if (symbol < alphabetLength) {
                // One cell(block) in Peq is represented with consecutive 64bit words (segments).
                // I have to do it this way because largest word I have is 64bit, while SSE register can be 128, 256, ...
                // Then I load this array of segments into one single SSE register.
                uint64_t PeqSegs[numSegs] __attribute__((aligned(16)));
                for (int seg = 0; seg < numSegs; seg++) { // build segment by segment
                    PeqSegs[seg] = 0;
                    for (int r = b * WORD_SIZE + (seg + 1) * 64 - 1; r >= b * WORD_SIZE + seg * 64; r--) {
                        PeqSegs[seg] <<= 1;
                        // NOTE: We pretend like query is padded at the end with W wildcard symbols
                        if (r >= queryLength || query[r] == symbol)
                            PeqSegs[seg] += 1;
                    }
                }
                Peq[symbol * maxNumBlocks + b] = _mm_load_si128((Word const*) PeqSegs);
            } else { // Last symbol is wildcard, so it is all 1s
                Peq[symbol * maxNumBlocks + b] = WORD_ALL_1;
            }
            print_mm128i(Peq[symbol * maxNumBlocks + b]); printf("\n");
#else
            if (symbol < alphabetLength) {
                Peq[symbol * maxNumBlocks + b] = 0;
                for (int r = (b+1) * WORD_SIZE - 1; r >= b * WORD_SIZE; r--) {
                    Peq[symbol * maxNumBlocks + b] <<= 1;
                    // NOTE: We pretend like query is padded at the end with W wildcard symbols
                    if (r >= queryLength || query[r] == symbol)
                        Peq[symbol * maxNumBlocks + b] += 1;
                }
            } else { // Last symbol is wildcard, so it is all 1s
                Peq[symbol * maxNumBlocks + b] = WORD_ALL_1;
            }
            print_uint64_t(Peq[symbol * maxNumBlocks + b]); printf("\n");
#endif
        }
    }
    printf("Finished building Peq\n");
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
#ifdef __SSE4_1__
    Word Xv = Eq | Mv;
    if (hin < 0) Eq |= WORD_1;
    Word Xh = (((Eq & Pv) + Pv) ^ Pv) | Eq;

    Word Ph = Mv | ~(Xh | Pv);
    Word Mh = Pv & Xh;

    int hout = 0;
    if (TEST_ANY_BITS_SET(Ph, HIGH_BIT_MASK)) hout = 1;
    if (TEST_ANY_BITS_SET(Mh, HIGH_BIT_MASK)) hout = -1;

    Ph <<= 1;
    Mh <<= 1;

    if (hin < 0) Mh |= WORD_1;
    if (hin > 0) Ph |= WORD_1;
    
    PvOut = Mh | ~(Xv | Ph);
    MvOut = Ph & Xv;

    return hout;
#else
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
#endif
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
 * @param [in] mode  MYERS_MODE_HW or MYERS_MODE_SHW or MYERS_MODE_OV
 */
static int myersCalcEditDistanceSemiGlobal(Block* const blocks, Word* const Peq, const int W, const int maxNumBlocks,
                                           const unsigned char* const query,  const int queryLength,
                                           const unsigned char* const target, const int targetLength,
                                           const int alphabetLength, int k, const int mode, int* bestScore_, int* position_) {
    // firstBlock is 0-based index of first block in Ukkonen band.
    // lastBlock is 0-based index of last block in Ukkonen band.
    int firstBlock = 0;
    int lastBlock = min(ceilDiv(k + 1, WORD_SIZE), maxNumBlocks) - 1; // y in Myers
    Block *bl; // Current block
    
    // Each STRONG_REDUCE_NUM column is reduced in more expensive way.
    // This gives speed up of about 2 times for small k.
    const int STRONG_REDUCE_NUM = 2048;
    
    // Initialize P, M and score
    bl = blocks;
    for (int b = 0; b <= lastBlock; b++) {
        bl->score = (b + 1) * WORD_SIZE;
        bl->P = WORD_ALL_1;
        bl->M = WORD_0;
        bl++;
    }

    int bestScore = -1;
    int position = -1;
    const int startHout = mode == MYERS_MODE_HW ? 0 : 1; // If 0 then gap before query is not penalized;
    const unsigned char* targetChar = target;
    for (int c = 0; c < targetLength; c++) { // for each column
        const Word* Peq_c = Peq + (*targetChar) * maxNumBlocks;

        //----------------------- Calculate column -------------------------//
        int hout = startHout;
        bl = blocks + firstBlock;
        Peq_c += firstBlock;
        for (int b = firstBlock; b <= lastBlock; b++) {
            hout = calculateBlock(bl->P, bl->M, *Peq_c, hout, bl->P, bl->M);
            bl->score += hout;
            bl++; Peq_c++;
        }
        bl--; Peq_c--;
        //------------------------------------------------------------------//

        //---------- Adjust number of blocks according to Ukkonen ----------//
        if (mode != MYERS_MODE_HW) {
            while (firstBlock < maxNumBlocks && blocks[firstBlock].score >= k + WORD_SIZE) {
                firstBlock++;
            }
            if (c % STRONG_REDUCE_NUM == 0) { // Do strong reduction every some blocks
                while (firstBlock < maxNumBlocks) {
                    // If all cells > k, remove block
                    int score = blocks[firstBlock].score;
                    Word P = blocks[firstBlock].P;
                    Word M = blocks[firstBlock].M;
                    Word mask = HIGH_BIT_MASK;
                    for (int i = 0; i < WORD_SIZE - 1; i++) {
                        if (score <= k) break;
#ifdef __SSE4_1__
                        if (TEST_ANY_BITS_SET(P, mask)) score--;
                        if (TEST_ANY_BITS_SET(M, mask)) score++;
#else
                        if (P & mask) score--;
                        if (M & mask) score++;
#endif
                        mask >>= 1;
                    }
                    if (score <= k) break;
                    firstBlock++;
                }
            }
        }

        if ((lastBlock < maxNumBlocks - 1) && (bl->score - hout <= k) // bl is pointing to last block
            && (TEST_ANY_BITS_SET(*(Peq_c + 1), WORD_1) || hout < 0)) { // Peq_c is pointing to last block
            // If score of left block is not too big, calculate one more block
            lastBlock++; bl++; Peq_c++;
            bl->P = WORD_ALL_1;
            bl->M = WORD_0;
            bl->score = (bl - 1)->score - hout + WORD_SIZE + calculateBlock(bl->P, bl->M, *Peq_c, hout, bl->P, bl->M);
        } else {
            while (lastBlock >= 0 && bl->score >= k + WORD_SIZE) {
                lastBlock--; bl--; Peq_c--;
            }
        }

        // Every some columns, do some expensive but also more efficient block reducing -> this is important!
        if (c % STRONG_REDUCE_NUM == 0) {
            while (lastBlock >= 0) {
                // If all cells > k, remove block
                int score = bl->score;
                Word mask = HIGH_BIT_MASK;
                for (int i = 0; i < WORD_SIZE - 1; i++) {
                    if (score <= k) break;
                    if (TEST_ANY_BITS_SET(bl->P, mask)) score--;
                    if (TEST_ANY_BITS_SET(bl->M, mask)) score++;
                    mask >>= 1;
                }
                if (score <= k) break;
                lastBlock--; bl--; Peq_c--;
            }
        }

        // If band stops to exist finish
        if (lastBlock < firstBlock) {
            *bestScore_ = bestScore;
            *position_ = position;
            return MYERS_STATUS_OK;
        }
        //------------------------------------------------------------------//

        //------------------------- Update best score ----------------------//
        if (lastBlock == maxNumBlocks - 1) {
            int colScore = bl->score;
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

        targetChar++;
    }


    // Obtain results for last W columns from last column.
    if (lastBlock == maxNumBlocks - 1) {
        int colScore = bl->score;
        Word mask = HIGH_BIT_MASK;
        for (int i = W - 1; i >= 0; i--) {
            if (TEST_ANY_BITS_SET(bl->P, mask)) colScore--;
            if (TEST_ANY_BITS_SET(bl->M, mask)) colScore++;
            mask >>= 1;
            if (colScore <= k && (bestScore == -1 || colScore < bestScore)) {
                bestScore = colScore;
                position = targetLength - 1 - i;
                k = bestScore - 1;
            }
        }
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
static int myersCalcEditDistanceNW(Block* blocks, Word* Peq, int W, int maxNumBlocks,
                                   const unsigned char* query, int queryLength,
                                   const unsigned char* target, int targetLength,
                                   int alphabetLength, int k, int* bestScore_, int* position_,
                                   bool findAlignment, AlignmentData** alignData) {

    // Each STRONG_REDUCE_NUM column is reduced in more expensive way.
    const int STRONG_REDUCE_NUM = 2048; // TODO: Choose this number dinamically (based on query and target lengths?), so it does not affect speed of computation

    if (k < abs(targetLength - queryLength)) {
        *bestScore_ = *position_ = -1;
        return MYERS_STATUS_OK;
    }

    k = min(k, max(queryLength, targetLength));  // Upper bound for k

    // firstBlock is 0-based index of first block in Ukkonen band.
    // lastBlock is 0-based index of last block in Ukkonen band.
    int firstBlock = 0;
    // This is optimal now, by my formula.
    int lastBlock = min(maxNumBlocks, ceilDiv(min(k, (k + queryLength - targetLength) / 2) + 1, WORD_SIZE)) - 1;
    Block* bl; // Current block

    // Initialize P, M and score
    bl = blocks;
    for (int b = 0; b <= lastBlock; b++) {
        bl->score = (b + 1) * WORD_SIZE;
        bl->P = WORD_ALL_1;
        bl->M = WORD_0;
        bl++;
    }

    // If we want to find alignment, we have to store needed data.
    if (findAlignment)
        *alignData = new AlignmentData(maxNumBlocks, targetLength);
    else
        *alignData = NULL;

    const unsigned char* targetChar = target;
    for (int c = 0; c < targetLength; c++) { // for each column
        Word* Peq_c = Peq + *targetChar * maxNumBlocks;

        //----------------------- Calculate column -------------------------//
        int hout = 1;
        bl = blocks + firstBlock;
        for (int b = firstBlock; b <= lastBlock; b++) {
            hout = calculateBlock(bl->P, bl->M, Peq_c[b], hout, bl->P, bl->M);
            bl->score += hout;
            bl++;
        }
        bl--;
        //------------------------------------------------------------------//
        // bl now points to last block

        // Update k. I do it only on end of column because it would slow calculation too much otherwise.
        // NOTICE: I add W when in last block because it is actually result from W cells to the left and W cells up.
        k = min(k, bl->score
                + max(targetLength - c - 1, queryLength - ((1 + lastBlock) * WORD_SIZE - 1) - 1)
                + (lastBlock == maxNumBlocks - 1 ? W : 0));
        
        //---------- Adjust number of blocks according to Ukkonen ----------//
        //--- Adjust last block ---//
        // If block is not beneath band, calculate next block. Only next because others are certainly beneath band.
        if (lastBlock + 1 < maxNumBlocks
            && !(//score[lastBlock] >= k + WORD_SIZE ||  // NOTICE: this condition could be satisfied if above block also!
                 ((lastBlock + 1) * WORD_SIZE - 1
                  > k - bl->score + 2 * WORD_SIZE - 2 - targetLength + c + queryLength))) {
            lastBlock++; bl++;
            bl->P = WORD_ALL_1;
            bl->M = WORD_0;
            int newHout = calculateBlock(bl->P, bl->M, Peq_c[lastBlock], hout, bl->P, bl->M);
            bl->score = (bl - 1)->score - hout + WORD_SIZE + newHout;
            hout = newHout;
        }

        // While block is out of band, move one block up. - This is optimal now, by my formula.
        // NOTICE: I added + W, and now it works! This has to be added because query is padded with W cells.
        while (lastBlock >= 0
               && (bl->score >= k + WORD_SIZE
                   || ((lastBlock + 1) * WORD_SIZE - 1 > 
                       k - bl->score + 2 * WORD_SIZE - 2 - targetLength + c + queryLength + W))) {
            lastBlock--; bl--;
        }
        //-------------------------//

        //--- Adjust first block ---//
        // While outside of band, advance block
        while (firstBlock <= lastBlock &&
               (blocks[firstBlock].score >= k + WORD_SIZE
                || (firstBlock + 1) * WORD_SIZE - 1 < blocks[firstBlock].score - k - targetLength + queryLength + c)) {
            firstBlock++;
        }
        //--------------------------/

        
        // TODO: consider if this part is useful, it does not seem to help much
        if (c % STRONG_REDUCE_NUM == 0) { // Every some columns do more expensive but more efficient reduction
            while (lastBlock >= 0) {
                // If all cells outside of band, remove block
                int score = bl->score;
                Word mask = HIGH_BIT_MASK;
                int r = (lastBlock + 1) * WORD_SIZE - 1;
                for (int i = 0; i < WORD_SIZE - 1; i++) {
                    if (score <= k && r <= k - score - targetLength + c + queryLength + W + 1) break; // TODO: Does not work if do not put +1! Why???
                    if (TEST_ANY_BITS_SET(bl->P, mask)) score--;
                    if (TEST_ANY_BITS_SET(bl->M, mask)) score++;
                    mask >>= 1;
                    r--;
                }
                if (score <= k && r <= k - score - targetLength + c + queryLength + W + 1) break; // TODO: Same as above
                lastBlock--; bl--;
            }

            while (firstBlock < maxNumBlocks) {
                // If all cells outside of band, remove block
                int score = blocks[firstBlock].score;
                Word mask = HIGH_BIT_MASK;
                int r = (firstBlock + 1) * WORD_SIZE - 1;
                for (int i = 0; i < WORD_SIZE - 1; i++) {
                    if (score <= k && r >= score - k - targetLength + c + queryLength) break;
                    if (TEST_ANY_BITS_SET(blocks[firstBlock].P, mask)) score--;
                    if (TEST_ANY_BITS_SET(blocks[firstBlock].M, mask)) score++;
                    mask >>= 1;
                    r--;
                }
                if (score <= k && r >= score - k - targetLength + c + queryLength) break;
                firstBlock++;
            }
        }
        

        // If band stops to exist finish
        if (lastBlock < firstBlock) {
            *bestScore_ = *position_ = -1;
            return MYERS_STATUS_OK;
        }
        //------------------------------------------------------------------//
        

        //---- Save column so it can be used for reconstruction ----//
        if (findAlignment && c < targetLength) {
            bl = blocks + firstBlock;
            for (int b = firstBlock; b <= lastBlock; b++) {
                (*alignData)->Ps[maxNumBlocks * c + b] = bl->P;
                (*alignData)->Ms[maxNumBlocks * c + b] = bl->M;
                (*alignData)->scores[maxNumBlocks * c + b] = bl->score;
                (*alignData)->firstBlocks[c] = firstBlock;
                (*alignData)->lastBlocks[c] = lastBlock;
                bl++;
            }
        }
        //----------------------------------------------------------//

        targetChar++;
    }

    if (lastBlock == maxNumBlocks - 1) { // If last block of last column was calculated
        // Obtain best score from block -> it is complicated because query is padded with W cells
        bl = blocks + lastBlock;
        int bestScore = bl->score;
        Word mask = HIGH_BIT_MASK;
        for (int i = 0; i < W; i++) {
            if (TEST_ANY_BITS_SET(bl->P, mask)) bestScore--;
            if (TEST_ANY_BITS_SET(bl->M, mask)) bestScore++;
            mask >>= 1;
        }

        if (bestScore <= k) {
            *bestScore_ = bestScore;
            *position_ = targetLength - 1;
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
                if (TEST_ANY_BITS_SET(lP, HIGH_BIT_MASK)) lScore--;
                if (TEST_ANY_BITS_SET(lM, HIGH_BIT_MASK)) lScore++;
                lP <<= 1;
                lM <<= 1;
            }
        }
        if (ulScore == -1) {
            if (lScore != -1) {
                ulScore = lScore;
                if (TEST_ANY_BITS_SET(lP, HIGH_BIT_MASK)) ulScore--;
                if (TEST_ANY_BITS_SET(lM, HIGH_BIT_MASK)) ulScore++;
            } 
            else if (c > 0 && b-1 >= alignData->firstBlocks[c-1] && b-1 <= alignData->lastBlocks[c-1]) {
                // This is the case when upper left cell is last cell in block,
                // and block to left is not in band so lScore is -1.
                ulScore = alignData->scores[(c - 1) * maxNumBlocks + b - 1];
            }
        }
        if (uScore == -1) {
            uScore = currScore;
            if (TEST_ANY_BITS_SET(currP, HIGH_BIT_MASK)) uScore--;
            if (TEST_ANY_BITS_SET(currM, HIGH_BIT_MASK)) uScore++;
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
