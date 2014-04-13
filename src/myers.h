#ifndef MYERS_H
#define MYERS_H


#ifdef __cplusplus 
extern "C" {
#endif

// Status codes
#define MYERS_STATUS_OK 0

// Alignment modes
#define MYERS_MODE_HW 0
#define MYERS_MODE_NW 1
#define MYERS_MODE_SHW 2

    /**
     * Calculates Levenshtein distance of query and target 
     * using Myers's fast bit-vector algorithm and Ukkonen's algorithm.
     * In Levenshtein distance mismatch and indel have cost of 1, while match has cost of 0.
     * Query and target are represented as arrays of numbers, where each number is 
     *  index of corresponding letter in alphabet. So for example if alphabet is ['A','C','T','G']
     *  and query string is "AACG" and target string is "GATTCGG" then our input query should be
     *  [0,0,1,3] and input target should be [3,0,2,2,1,3,3] (and alphabetLength would be 4).
     * @param [in] query  Array of alphabet indices.
     * @param [in] queryLength
     * @param [in] target  Array of alphabet indices.
     * @param [in] targetLength
     * @param [in] alphabetLength
     * @param [in] k  Non-negative number, constraint for Ukkonen. 
     *                 Only best score <= k will be searched for.
     *                 If k is smaller then calculation is faster.
     *                 If you are interested in score only if it is <= K, set k to K.
     *                 If k is negative then k will be auto-adjusted (increased) until score is found.
     * @param [in] mode  Mode that determines alignment algorithm.
     *                    MYERS_MODE_NW: global (Needleman-Wunsch)
     *                    MYERS_MODE_HW: semi-global. Gaps before and after query are not penalized.
     *                    MYERS_MODE_SHW: semi-global. Gap after query is not penalized.
     * @param [in] findAlignment  If true and if score != -1, reconstruction of alignment will be performed
     *                            and alignment will be returned. 
     *                            Notice: Finding aligment will increase execution time
     *                                    and could take large amount of memory.
     * @param [out] score  Best score (smallest edit distance) or -1 if there is no score <= k.
     * @param [out] position  Zero-based position in target where query ends (position of last character).
     *                        If gap after query is penalized then it counts as part of query,
     *                        otherwise not. -1 if there is no score <= k.
     * @param [out] alignment  Will contain alignment if findAlignment is true and score != -1.
     *                         Otherwise it will be set NULL.
     *                         Alignment is sequence of numbers: 0, 1, 2.
     *                         0 stands for (mis)match.
     *                         1 stands for insertion to target.
     *                         2 stands for insertion to query.
     *                         Important: Do not forget to free memory allocated for alignment!
     *                                    Use free().
     * @param [out] alignmentLength  Length of alignment.
     * @return Status code.
     */
    int myersCalcEditDistance(const unsigned char* query, int queryLength,
                              const unsigned char* target, int targetLength,
                              int alphabetLength, int k, int mode,
                              int* bestScore, int* position, 
                              bool findAlignment, unsigned char** alignment, int* alignmentLength);

#ifdef __cplusplus 
}
#endif

#endif // MYERS_H
