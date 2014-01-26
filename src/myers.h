#ifndef MYERS_H
#define MYERS_H


#ifdef __cplusplus 
extern "C" {
#endif

#define MYERS_MODE_HW 0
#define MYERS_MODE_NW 1

    /**
     * Calculates edit distance of query and target 
     * using Myers's fast bit-vector algorithm and Ukkonen's algorithm.
     * In edit distance problem mismatch and indel have cost of 1, while match has cost of 0.
     * Query can start anywhere and can end anywhere inside target, 
     *  which means gaps before and after are not penalized.
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
     * @return Best score (smallest edit distance) or -1 if there is no best score <= k.
     */
    int myersCalcEditDistance(const unsigned char* query, int queryLength,
                              const unsigned char* target, int targetLength,
                              int alphabetLength, int k, int mode);

#ifdef __cplusplus 
}
#endif

#endif // MYERS_H
