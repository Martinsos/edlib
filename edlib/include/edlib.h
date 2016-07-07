#ifndef EDLIB_H
#define EDLIB_H

/**
 * @file
 * @author Martin Sosic
 * @brief Main header file, containing all public functions and structures.
 */

#ifdef __cplusplus
extern "C" {
#endif

    // TODO: turn all these defines into enums.

// Status codes
#define EDLIB_STATUS_OK 0
#define EDLIB_STATUS_ERROR 1

// Alignment modes - how should Edlib treat gaps before and after query?
typedef enum {
    EDLIB_MODE_HW,
    EDLIB_MODE_NW,
    EDLIB_MODE_SHW
} EdlibAlignMode;

// Alignment tasks - what do you want Edlib to do?
typedef enum {
    EDLIB_TASK_DISTANCE,  // Find edit distance and end locations.
    EDLIB_TASK_LOC,       // Find edit distance, end locations and start locations.
    EDLIB_TASK_PATH       // Find edit distance, end locations and start locations and alignment path.
} EdlibAlignTask;

// Edit operations
#define EDLIB_EDOP_MATCH 0
#define EDLIB_EDOP_INSERT 1  // Insertion to target = deletion from query.
#define EDLIB_EDOP_DELETE 2  // Deletion from target = insertion to query.
#define EDLIB_EDOP_MISMATCH 3

// Cigar formats
#define EDLIB_CIGAR_EXTENDED 0
#define EDLIB_CIGAR_STANDARD 1



    /**
     * @brief Configuration object for edlibAlign() function.
     */
    typedef struct {
        /**
         * Set k to non-negative value to tell edlib that edit distance is not larger than k.
         * Smaller k can significantly improve speed of computation.
         * If edit distance is larger than k, edlib will set edit distance to -1.
         * Set k to negative value and edlib will internally auto-adjust k until score is found.
         */
        int k;

        /**
         * Alignment method.
         * EDLIB_MODE_NW: global (Needleman-Wunsch)
         * EDLIB_MODE_SHW: prefix. Gap after query is not penalized.
         * EDLIB_MODE_HW: infix. Gaps before and after query are not penalized.
         */
        EdlibAlignMode mode;

        /**
         * Alignment task - tells Edlib what to calculate. Less to calculate, faster it is.
         * EDLIB_TASK_DISTANCE - find edit distance and end locations of optimal alignment paths in target.
         * EDLIB_TASK_LOC - find edit distance and start and end locations of optimal alignment paths in target.
         * EDLIB_TASK_PATH - find edit distance, alignment path (and start and end locations of it in target).
         */
        EdlibAlignTask task;
    } EdlibAlignConfig;

    /**
     * Helper method for easy construction of configuration object.
     * @return Configuration object filled with given parameters.
     */
    EdlibAlignConfig edlibNewAlignConfig(int k, EdlibAlignMode mode, EdlibAlignTask task);

    /**
     * @return Default configuration object, with following defaults:
     *         k = -1, mode = EDLIB_MODE_NW, task = EDLIB_TASK_DISTANCE.
     */
    EdlibAlignConfig edlibDefaultAlignConfig();


    /**
     * Container for results of alignment done by edlibAlign() function.
     */
    typedef struct {
        /**
         * -1 if k is non-negative and edit distance is larger than k.
         */
        int editDistance;
        /**
         * Array of zero-based positions in target where optimal alignment paths end.
         * If gap after query is penalized, gap counts as part of query (NW), otherwise not.
         * Set to NULL if edit distance is larger than k.
         * If you do not free whole result object using edlibFreeAlignResult(), do not forget to use free().
         */
        int* endLocations;
        /**
         * Array of zero-based positions in target where optimal alignment paths start,
         * they correspond to endLocations.
         * If gap before query is penalized, gap counts as part of query (NW), otherwise not.
         * Set to NULL if not calculated or if edit distance is larger than k.
         * If you do not free whole result object using edlibFreeAlignResult(), do not forget to use free().
         */
        int* startLocations;
        /**
         * Number of end (and start) locations.
         */
        int numLocations;
        /**
         * Alignment is found for first pair of start and end locations.
         * Set to NULL if not calculated.
         * Alignment is sequence of numbers: 0, 1, 2, 3.
         * 0 stands for match.
         * 1 stands for insertion to target.
         * 2 stands for insertion to query.
         * 3 stands for mismatch.
         * Alignment aligns query to target from begining of query till end of query.
         * If gaps are not penalized, they are not in alignment.
         * If you do not free whole result object using edlibFreeAlignResult(), do not forget to use free().
         */
        unsigned char* alignment;
        /**
         * Length of alignment.
         */
        int alignmentLength;
        /**
         * Number of different characters in query and target together.
         */
        int alphabetLength;
    } EdlibAlignResult;

    /**
     * Frees memory in EdlibAlignResult that was allocated by edlib.
     * If you do not use it, make sure to free needed members manually using free().
     */
    void edlibFreeAlignResult(EdlibAlignResult result);


    /**
     * Aligns two sequences (query and target) using edit distance (levenshtein distance).
     * Through config parameter, this function supports different alignment methods (global, prefix, infix),
     * as well as different modes of search (tasks).
     * It always returns edit distance and end locations of optimal alignment in target.
     * It optionally returns start locations of optimal alignment in target and alignment path,
     * if you choose appropriate tasks.
     * @param [in] query  First sequence. Character codes should be in range [0, 127].
     * @param [in] queryLength  Number of characters in first sequence.
     * @param [in] target  Second sequence. Character codes should be in range [0, 127].
     * @param [in] targetLength  Number of characters in second sequence.
     * @param [in] config  Additional alignment parameters, like alignment method and wanted results.
     * @return  Result of alignment, which can contain edit distance, start and end locations and alignment path.
     *          Make sure to clean up the object using edlibFreeAlignResult() or by manually freeing needed members.
     */
    EdlibAlignResult edlibAlign(const char* query, const int queryLength,
                                const char* target, const int targetLength,
                                EdlibAlignConfig config);


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
