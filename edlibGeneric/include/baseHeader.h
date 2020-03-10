//
// Created by mobin on 1/13/20.
//

#ifndef EDLIB_BASEHEADER_H
#define EDLIB_BASEHEADER_H

#ifdef __cplusplus
extern "C" {
#endif

// Status codes
#define EDLIB_STATUS_OK 0
#define EDLIB_STATUS_ERROR 1

/**
 * Alignment methods - how should Edlib treat gaps before and after query?
 */
typedef enum {
    /**
     * Global method. This is the standard method.
     * Useful when you want to find out how similar is first sequence to second sequence.
     */
            EDLIB_MODE_NW,
    /**
     * Prefix method. Similar to global method, but with a small twist - gap at query end is not penalized.
     * What that means is that deleting elements from the end of second sequence is "free"!
     * For example, if we had "AACT" and "AACTGGC", edit distance would be 0, because removing "GGC" from the end
     * of second sequence is "free" and does not count into total edit distance. This method is appropriate
     * when you want to find out how well first sequence fits at the beginning of second sequence.
     */
            EDLIB_MODE_SHW,
    /**
     * Infix method. Similar as prefix method, but with one more twist - gaps at query end and start are
     * not penalized. What that means is that deleting elements from the start and end of second sequence is "free"!
     * For example, if we had ACT and CGACTGAC, edit distance would be 0, because removing CG from the start
     * and GAC from the end of second sequence is "free" and does not count into total edit distance.
     * This method is appropriate when you want to find out how well first sequence fits at any part of
     * second sequence.
     * For example, if your second sequence was a long text and your first sequence was a sentence from that text,
     * but slightly scrambled, you could use this method to discover how scrambled it is and where it fits in
     * that text. In bioinformatics, this method is appropriate for aligning read to a sequence.
     */
            EDLIB_MODE_HW
} EdlibAlignMode;

/**
 * Alignment tasks - what do you want Edlib to do?
 */
typedef enum {
    EDLIB_TASK_DISTANCE,  //!< Find edit distance and end locations.
    EDLIB_TASK_LOC,       //!< Find edit distance, end locations and start locations.
    EDLIB_TASK_PATH       //!< Find edit distance, end locations and start locations and alignment path.
} EdlibAlignTask;

/**
 * Describes cigar format.
 * @see http://samtools.github.io/hts-specs/SAMv1.pdf
 * @see http://drive5.com/usearch/manual/cigar.html
 */
typedef enum {
    EDLIB_CIGAR_STANDARD,  //!< Match: 'M', Insertion: 'I', Deletion: 'D', Mismatch: 'M'.
    EDLIB_CIGAR_EXTENDED   //!< Match: '=', Insertion: 'I', Deletion: 'D', Mismatch: 'X'.
} EdlibCigarFormat;

// Edit operations.
#define EDLIB_EDOP_MATCH 0    //!< Match.
#define EDLIB_EDOP_INSERT 1   //!< Insertion to target = deletion from query.
#define EDLIB_EDOP_DELETE 2   //!< Deletion from target = insertion to query.
#define EDLIB_EDOP_MISMATCH 3 //!< Mismatch.

/**
 * Container for results of alignment done by edlibAlign() function.
 */
typedef struct {
    /**
     * EDLIB_STATUS_OK or EDLIB_STATUS_ERROR. If error, all other fields will have undefined values.
     */
    int status;

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
 * Builds cigar string from given alignment sequence.
 * @param [in] alignment  Alignment sequence.
 *     0 stands for match.
 *     1 stands for insertion to target.
 *     2 stands for insertion to query.
 *     3 stands for mismatch.
 * @param [in] alignmentLength
 * @param [in] cigarFormat  Cigar will be returned in specified format.
 * @return Cigar string.
 *     I stands for insertion.
 *     D stands for deletion.
 *     X stands for mismatch. (used only in extended format)
 *     = stands for match. (used only in extended format)
 *     M stands for (mis)match. (used only in standard format)
 *     String is null terminated.
 *     Needed memory is allocated and given pointer is set to it.
 *     Do not forget to free it later using free()!
 */
char* edlibAlignmentToCigar(const unsigned char* alignment, int alignmentLength,
                            EdlibCigarFormat cigarFormat);

/**
 * Frees memory in EdlibAlignResult that was allocated by edlib.
 * If you do not use it, make sure to free needed members manually using free().
 */
void edlibFreeAlignResult(EdlibAlignResult result);

#ifdef __cplusplus
}
#endif
#endif //EDLIB_BASEHEADER_H

