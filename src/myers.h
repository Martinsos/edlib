#ifndef MYERS_H
#define MYERS_H


#ifdef __cplusplus 
extern "C" {
#endif

    /**
     * Calculates edit distance using Myers's fast bit-vector algorithm and Ukkonens algorithm.
     * In edit distance problem mismatch and indel have cost of 1, while match has cost of 0.
     * Query can start anywhere and can end anywhere inside target, 
     * which means gaps before and after are not penalized.
     * @return best score (smallest edit distance).
     */
    int myersCalcEditDistance(const unsigned char* query, int queryLength,
                              const unsigned char* target, int targetLength,
                              int alphabetLength, int k);

#ifdef __cplusplus 
}
#endif

#endif // MYERS_H
