#ifndef EDLIB_H
#define EDLIB_H


#ifdef __cplusplus
extern "C" {
#endif

// Status codes
#define EDLIB_STATUS_OK 0
#define EDLIB_STATUS_ERROR 1

// Alignment modes
#define EDLIB_MODE_HW  0
#define EDLIB_MODE_NW  1
#define EDLIB_MODE_SHW 2
#define EDLIB_MODE_OV  3

// Edit operations
#define EDLIB_EDOP_MATCH 0
#define EDLIB_EDOP_INSERT 1  // Insertion to target = deletion from query.
#define EDLIB_EDOP_DELETE 2  // Deletion from target = insertion to query.
#define EDLIB_EDOP_MISMATCH 3

// Cigar formats
#define EDLIB_CIGAR_EXTENDED 0
#define EDLIB_CIGAR_STANDARD 1

    /**
     * Calculates Levenshtein distance of query and target
     * using Myers's fast bit-vector algorithm and Ukkonen's algorithm.
     * In Levenshtein distance mismatch and indel have cost of 1, while match has cost of 0.
     * Query and target are represented as arrays of numbers, where each number is
     * index of corresponding letter in alphabet. So for example if alphabet is ['A','C','T','G']
     * and query string is "AACG" and target string is "GATTCGG" then our input query should be
     * [0,0,1,3] and input target should be [3,0,2,2,1,3,3] (and alphabetLength would be 4).
     * @param [in] query  Array of alphabet indices.
     * @param [in] queryLength
     * @param [in] target  Array of alphabet indices.
     * @param [in] targetLength
     * @param [in] alphabetLength
     * @param [in] k  Non-negative number, constraint for Ukkonen.
     *     Only best score <= k will be searched for.
     *     If k is smaller then calculation is faster.
     *     If you are interested in score only if it is <= K, set k to K.
     *     If k is negative then k will be auto-adjusted (increased) until score is found.
     * @param [in] mode  Mode that determines alignment algorithm.
     *     EDLIB_MODE_NW: global (Needleman-Wunsch)
     *     EDLIB_MODE_HW: semi-global. Gaps before and after query are not penalized.
     *     EDLIB_MODE_SHW: semi-global. Gap after query is not penalized.
     *     EDLIB_MODE_OV: semi-global. Gaps before and after query and target are not penalized.
     * @param [in] findStartLocations  If true, start locations are returned.
     *                                 May somewhat slow down the calculation.
     *                                 If findAlignment is true, start locations will also be found.
     * @param [in] findAlignment  If true and if score != -1, reconstruction of alignment will be performed
     *                            and alignment will be returned.
     *                            Notice: Finding aligment will increase execution time.
     * @param [out] bestScore  Best score (smallest edit distance) or -1 if there is no score <= k.
     * @param [out] endLocations  Array of zero-based positions in target where
     *     query ends (position of last character) with the best score.
     *     If gap after query is penalized, gap counts as part of query (NW), otherwise not.
     *     If there is no score <= k, endLocations is set to NULL.
     *     Otherwise, array is returned and it is on you to free it with free().
     * @param [out] startLocations  Array of zero-based positions in target where
     *     query starts, they correspond to endLocations.
     *     If gap before query is penalized, gap counts as part of query (NW), otherwise not.
     *     If there is no score <= k, startLocations is set to NULL.
     *     Otherwise, array is returned and it is on you to free it with free().
     * @param [out] numLocations  Number of positions returned.
     * @param [out] alignment  Alignment is found for first position returned.
     *                         Will contain alignment if findAlignment is true and score != -1.
     *                         Otherwise it will be set NULL.
     *                         Alignment is sequence of numbers: 0, 1, 2, 3.
     *                         0 stands for match.
     *                         1 stands for insertion to target.
     *                         2 stands for insertion to query.
     *                         3 stands for mismatch.
     *                         Alignment aligns query to target from begining of query till end of query.
     *                         Alignment ends at @param positions[0] in target.
     *                         If gaps are not penalized, they are not in alignment.
     *                         Needed memory is allocated and given pointer is set to it.
     *                         Important: Do not forget to free memory allocated for alignment!
     *                                    Use free().
     * @param [out] alignmentLength  Length of alignment.
     * @return Status code.
     */
    int edlibCalcEditDistance(
        const unsigned char* query, int queryLength,
        const unsigned char* target, int targetLength,
        int alphabetLength, int k, int mode,
        bool findStartLocations, bool findAlignment,
        int* bestScore, int** endLocations, int** startLocations, int* numLocations,
        unsigned char** alignment, int* alignmentLength);

    /**
     * Builds cigar string from given alignment sequence.
     * @param [in] alignment  Alignment sequence.
     *     0 stands for match.
     *     1 stands for insertion to target.
     *     2 stands for insertion to query.
     *     3 stands for mismatch.
     * @param [in] alignmentLength
     * @param [in] cigarFormat
     *     If EDLIB_CIGAR_EXTENDED, extended cigar is returned.
     *     If EDLIB_CIGAR_STANDARD, standard cigar is returned (contains only I, D and M).
     * @param [out] cigar  Will contain cigar string.
     *     I stands for insertion.
     *     D stands for deletion.
     *     X stands for mismatch. (used only in extended format)
     *     = stands for match. (used only in extended format)
     *     M stands for (mis)match. (used only in standard format)
     *     String is null terminated.
     *     Needed memory is allocated and given pointer is set to it.
     *     Do not forget to free it later using free()!
     * @return Status code.
     */
    int edlibAlignmentToCigar(unsigned char* alignment, int alignmentLength,
                              int cigarFormat, char** cigar);

#ifdef __cplusplus
}
#endif

#endif // EDLIB_H
