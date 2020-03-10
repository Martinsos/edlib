//
// Created by mobin on 2/27/20.
//

#ifndef EDLIBGENERIC_PYTHON_CHARHEADER_H
#define EDLIBGENERIC_PYTHON_CHARHEADER_H

#include "baseHeader.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Defines two given characters as equal.
 */
typedef struct {
    char first;
    char second;
} EdlibEqualityPair;

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

    /**
     * List of pairs of characters, where each pair defines two characters as equal.
     * This way you can extend edlib's definition of equality (which is that each character is equal only
     * to itself).
     * This can be useful if you have some wildcard characters that should match multiple other characters,
     * or e.g. if you want edlib to be case insensitive.
     * Can be set to NULL if there are none.
     */
    EdlibEqualityPair* additionalEqualities;

    /**
     * Number of additional equalities, which is non-negative number.
     * 0 if there are none.
     */
    int additionalEqualitiesLength;
} EdlibAlignConfig;

/**
 * Helper method for easy construction of configuration object.
 * @return Configuration object filled with given parameters.
 */
EdlibAlignConfig edlibNewAlignConfig(int k, EdlibAlignMode mode, EdlibAlignTask task,
                                     EdlibEqualityPair* additionalEqualities,
                                     int additionalEqualitiesLength);

/**
 * @return Default configuration object, with following defaults:
 *         k = -1, mode = EDLIB_MODE_NW, task = EDLIB_TASK_DISTANCE, no additional equalities.
 */
EdlibAlignConfig edlibDefaultAlignConfig(void);
#ifdef __cplusplus
}
#endif
#endif //EDLIBGENERIC_PYTHON_CHARHEADER_H
