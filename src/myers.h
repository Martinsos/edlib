#ifndef MYERS_H
#define MYERS_H

/**
 * TODO:
 * - add k parameter
 */

#ifdef __cplusplus 
extern "C" {
#endif

    /**
     * Calculates edit distance using Myers's fast bit-vector algorithm.
     * In edit distance problem mismatch and indel have cost of 1, while match has cost of 0.
     * Query can start anywhere and can end anywhere inside target, 
     * gaps before and after are not penalized.
     * Smallest edit distance is returned.
     * @return best score (smallest edit distance).
     */
    int calcEditDistance(const unsigned char* query, int queryLength,
                         const unsigned char* target, int targetLength,
                         int alphabetLength);

#ifdef __cplusplus 
}
#endif

#endif // MYERS_H
